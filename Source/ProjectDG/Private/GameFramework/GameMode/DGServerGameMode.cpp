#include "GameFramework/GameMode/DGServerGameMode.h"

#include "Core/DG_Debug.h"
#include "Dom/JsonObject.h"
#include "Engine/World.h"
#include "GameFramework/Controller.h"
#include "GameFramework/GameSession.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/PlayerState.h"
#include "HAL/PlatformMisc.h"
#include "HttpModule.h"
#include "Interfaces/IHttpRequest.h"
#include "Interfaces/IHttpResponse.h"
#include "Kismet/GameplayStatics.h"
#include "Misc/CommandLine.h"
#include "Misc/Parse.h"
#include "Serialization/JsonSerializer.h"
#include "Serialization/JsonWriter.h"

void ADGServerGameMode::PreLogin(
	const FString& Options,
	const FString& Address,
	const FUniqueNetIdRepl& UniqueId,
	FString& ErrorMessage
)
{
	Super::PreLogin(Options, Address, UniqueId, ErrorMessage);

	const FString SessionId = UGameplayStatics::ParseOption(Options, TEXT("SessionId"));
	const FString JoinToken = UGameplayStatics::ParseOption(Options, TEXT("JoinToken"));

	Debug::Print(FString::Printf(
		TEXT("[DGServerGameMode] PreLogin. Address=%s SessionId=%s JoinToken=%s NetMode=%d"),
		*Address,
		*SessionId,
		*JoinToken,
		static_cast<int32>(GetNetMode())
	));

	if (!ErrorMessage.IsEmpty())
	{
		Debug::Print(FString::Printf(
			TEXT("[DGServerGameMode] PreLogin rejected by Super. Error=%s"),
			*ErrorMessage
		));
		return;
	}

	if (GetNetMode() != NM_DedicatedServer)
	{
		Debug::Print(TEXT("[DGServerGameMode] PreLogin skipped. Not Dedicated Server."));
		return;
	}

	if (SessionId.IsEmpty())
	{
		ErrorMessage = TEXT("Missing SessionId.");
		Debug::Print(TEXT("[DGServerGameMode] PreLogin rejected. Missing SessionId."));
		return;
	}

	if (JoinToken.IsEmpty())
	{
		ErrorMessage = TEXT("Missing JoinToken.");
		Debug::Print(TEXT("[DGServerGameMode] PreLogin rejected. Missing JoinToken."));
		return;
	}
}

FString ADGServerGameMode::InitNewPlayer(
	APlayerController* NewPlayerController,
	const FUniqueNetIdRepl& UniqueId,
	const FString& Options,
	const FString& Portal
)
{
	const FString ErrorMessage = Super::InitNewPlayer(
		NewPlayerController,
		UniqueId,
		Options,
		Portal
	);

	if (!ErrorMessage.IsEmpty())
	{
		return ErrorMessage;
	}

	if (!NewPlayerController)
	{
		return TEXT("NewPlayerController is null.");
	}

	const FString SessionId = UGameplayStatics::ParseOption(Options, TEXT("SessionId"));
	const FString JoinToken = UGameplayStatics::ParseOption(Options, TEXT("JoinToken"));

	Debug::Print(FString::Printf(
		TEXT("[DGServerGameMode] InitNewPlayer. Player=%s SessionId=%s JoinToken=%s NetMode=%d"),
		*NewPlayerController->GetName(),
		*SessionId,
		*JoinToken,
		static_cast<int32>(GetNetMode())
	));

	if (GetNetMode() != NM_DedicatedServer)
	{
		Debug::Print(TEXT("[DGServerGameMode] InitNewPlayer validation skipped. Not Dedicated Server."));
		return ErrorMessage;
	}

	InitializeBackendBaseUrlFromCommandLine();

	if (SessionId.IsEmpty())
	{
		return TEXT("Missing SessionId.");
	}

	if (JoinToken.IsEmpty())
	{
		return TEXT("Missing JoinToken.");
	}

	ValidateJoinTokenAsync(NewPlayerController, SessionId, JoinToken);

	return ErrorMessage;
}

void ADGServerGameMode::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);

	if (!NewPlayer)
	{
		Debug::Print(TEXT("[DGServerGameMode] PostLogin failed. NewPlayer is null."));
		return;
	}

	Debug::Print(FString::Printf(
		TEXT("[DGServerGameMode] PostLogin Success. Player=%s"),
		*NewPlayer->GetName()
	));

	APawn* ControlledPawn = NewPlayer->GetPawn();
	APlayerState* PS = NewPlayer->PlayerState.Get();

	const FString PlayerName = IsValid(NewPlayer)
		? NewPlayer->GetName()
		: TEXT("None");

	const FString PawnName = IsValid(ControlledPawn)
		? ControlledPawn->GetName()
		: TEXT("None");

	const FString PlayerStateName = IsValid(PS)
		? PS->GetName()
		: TEXT("None");

	Debug::Print(FString::Printf(
		TEXT("[DGServerGameMode] PostLogin Pawn Check. Player=%s Pawn=%s PlayerState=%s"),
		*PlayerName,
		*PawnName,
		*PlayerStateName
	));

	if (GetNetMode() == NM_DedicatedServer)
	{
		ConnectedPlayerCount++;

		Debug::Print(FString::Printf(
			TEXT("[DGServerGameMode] ConnectedPlayerCount increased. Count=%d"),
			ConnectedPlayerCount
		));
	}
}

void ADGServerGameMode::Logout(AController* Exiting)
{
	const FString ExitingName = IsValid(Exiting)
		? Exiting->GetName()
		: TEXT("None");

	bool bHasMemberInfo = false;
	FDGConnectedMemberInfo LeavingMemberInfo;

	if (GetNetMode() == NM_DedicatedServer && IsValid(Exiting))
	{
		const TObjectKey<AController> ExitingKey(Exiting);

		if (const FDGConnectedMemberInfo* FoundMemberInfo = ConnectedMemberInfos.Find(ExitingKey))
		{
			LeavingMemberInfo = *FoundMemberInfo;
			bHasMemberInfo = true;
		}

		ConnectedMemberInfos.Remove(ExitingKey);
	}

	APawn* ExitingPawn = IsValid(Exiting)
		? Exiting->GetPawn()
		: nullptr;

	if (GetNetMode() == NM_DedicatedServer && IsValid(ExitingPawn))
	{
		const FString ExitingPawnName = ExitingPawn->GetName();

		Debug::Print(FString::Printf(
			TEXT("[DGServerGameMode] Logout. Destroy exiting pawn. Player=%s Pawn=%s"),
			*ExitingName,
			*ExitingPawnName
		));

		ExitingPawn->Destroy();
	}

	Super::Logout(Exiting);

	if (GetNetMode() != NM_DedicatedServer)
	{
		return;
	}

	ConnectedPlayerCount = FMath::Max(0, ConnectedPlayerCount - 1);

	Debug::Print(FString::Printf(
		TEXT("[DGServerGameMode] Logout. Player=%s ConnectedPlayerCount=%d HasMemberInfo=%s"),
		*ExitingName,
		ConnectedPlayerCount,
		bHasMemberInfo ? TEXT("true") : TEXT("false")
	));

	if (bHasMemberInfo)
	{
		const bool bWasLastKnownPlayer = ConnectedPlayerCount <= 0;
		ReportMemberLeftAsync(LeavingMemberInfo, bWasLastKnownPlayer);
		return;
	}

	Debug::Print(TEXT("[DGServerGameMode] Logout member info not found. Fallback to session-ended check."));

	TryReportSessionEndedIfNoPlayers();
}

void ADGServerGameMode::RestartPlayer(AController* NewPlayer)
{
	const FString ControllerName = IsValid(NewPlayer)
		? NewPlayer->GetName()
		: TEXT("None");

	APawn* PawnBefore = IsValid(NewPlayer)
		? NewPlayer->GetPawn()
		: nullptr;

	const FString PawnBeforeName = IsValid(PawnBefore)
		? PawnBefore->GetName()
		: TEXT("None");

	Debug::Print(FString::Printf(
		TEXT("[DGServerGameMode] RestartPlayer Begin. Controller=%s PawnBefore=%s"),
		*ControllerName,
		*PawnBeforeName
	));

	Super::RestartPlayer(NewPlayer);

	APawn* PawnAfter = IsValid(NewPlayer)
		? NewPlayer->GetPawn()
		: nullptr;

	const FString PawnAfterName = IsValid(PawnAfter)
		? PawnAfter->GetName()
		: TEXT("None");

	Debug::Print(FString::Printf(
		TEXT("[DGServerGameMode] RestartPlayer End. Controller=%s PawnAfter=%s"),
		*ControllerName,
		*PawnAfterName
	));
}

void ADGServerGameMode::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (GetNetMode() == NM_DedicatedServer && !ActiveSessionId.IsEmpty() && !bSessionEndReported)
	{
		const FString SessionIdToEnd = ActiveSessionId;

		bSessionEndReported = true;

		StopSessionHeartbeat();

		ActiveSessionId.Empty();

		ReportSessionEndedAsync(SessionIdToEnd);
	}
	else
	{
		StopSessionHeartbeat();
	}

	Super::EndPlay(EndPlayReason);
}

void ADGServerGameMode::InitializeBackendBaseUrlFromCommandLine()
{
	FString CommandLineBackendUrl;

	if (FParse::Value(FCommandLine::Get(), TEXT("BackendUrl="), CommandLineBackendUrl))
	{
		CommandLineBackendUrl.TrimStartAndEndInline();

		if (!CommandLineBackendUrl.IsEmpty())
		{
			BackendBaseUrl = CommandLineBackendUrl;
		}
	}

	Debug::Print(FString::Printf(
		TEXT("[DGServerGameMode] BackendBaseUrl=%s"),
		*BackendBaseUrl
	));
}

void ADGServerGameMode::ValidateJoinTokenAsync(
	APlayerController* PlayerController,
	const FString& SessionId,
	const FString& JoinToken
)
{
	if (!PlayerController)
	{
		Debug::Print(TEXT("[DGServerGameMode] ValidateJoinTokenAsync failed. PlayerController is null."));
		return;
	}

	FString RequestUrl = BackendBaseUrl;

	if (RequestUrl.EndsWith(TEXT("/")))
	{
		RequestUrl.LeftChopInline(1);
	}

	RequestUrl += TEXT("/api/sessions/validate-join");

	const FString BodyJson = BuildValidateJoinJson(SessionId, JoinToken);

	TWeakObjectPtr<APlayerController> WeakPlayerController(PlayerController);

	TSharedRef<IHttpRequest, ESPMode::ThreadSafe> Request = FHttpModule::Get().CreateRequest();
	Request->SetURL(RequestUrl);
	Request->SetVerb(TEXT("POST"));
	Request->SetHeader(TEXT("Content-Type"), TEXT("application/json"));
	Request->SetHeader(TEXT("Accept"), TEXT("application/json"));
	Request->SetContentAsString(BodyJson);

	Debug::Print(FString::Printf(
		TEXT("[DGServerGameMode] Validate Join Request. Url=%s SessionId=%s"),
		*RequestUrl,
		*SessionId
	));

	Request->OnProcessRequestComplete().BindLambda(
		[this, WeakPlayerController, SessionId](
			FHttpRequestPtr HttpRequest,
			FHttpResponsePtr HttpResponse,
			bool bWasSuccessful
		)
		{
			if (!WeakPlayerController.IsValid())
			{
				Debug::Print(TEXT("[DGServerGameMode] Validate Join Response ignored. PlayerController is invalid."));
				return;
			}

			APlayerController* PlayerController = WeakPlayerController.Get();

			if (!bWasSuccessful || !HttpResponse.IsValid())
			{
				KickPlayerWithReason(PlayerController, TEXT("Backend validate request failed."));
				return;
			}

			const int32 ResponseCode = HttpResponse->GetResponseCode();
			const FString ResponseBody = HttpResponse->GetContentAsString();

			Debug::Print(FString::Printf(
				TEXT("[DGServerGameMode] Validate Join Response. Code=%d Body=%s"),
				ResponseCode,
				*ResponseBody
			));

			FString ResponseMessage;
			FString ResponseSessionId;
			int64 ResponseAccountId = 0;
			int64 ResponseCharacterId = 0;
			FString ResponseRole;

			const bool bValidJoin = ParseValidateJoinResponse(
				ResponseBody,
				ResponseMessage,
				ResponseSessionId,
				ResponseAccountId,
				ResponseCharacterId,
				ResponseRole
			);

			if (ResponseCode < 200 || ResponseCode >= 300)
			{
				if (ResponseMessage.IsEmpty())
				{
					ResponseMessage = FString::Printf(TEXT("Backend returned HTTP error. Code=%d"), ResponseCode);
				}

				KickPlayerWithReason(PlayerController, ResponseMessage);
				return;
			}

			if (!bValidJoin)
			{
				if (ResponseMessage.IsEmpty())
				{
					ResponseMessage = TEXT("Join token validation failed.");
				}

				KickPlayerWithReason(PlayerController, ResponseMessage);
				return;
			}

			FDGConnectedMemberInfo MemberInfo;
			MemberInfo.SessionId = ResponseSessionId.IsEmpty() ? SessionId : ResponseSessionId;
			MemberInfo.AccountId = ResponseAccountId;
			MemberInfo.CharacterId = ResponseCharacterId;
			MemberInfo.Role = ResponseRole;

			ConnectedMemberInfos.Add(TObjectKey<AController>(PlayerController), MemberInfo);

			Debug::Print(FString::Printf(
				TEXT("[DGServerGameMode] Validate Join Success. Player=%s SessionId=%s AccountId=%lld CharacterId=%lld Role=%s Message=%s"),
				*PlayerController->GetName(),
				*MemberInfo.SessionId,
				MemberInfo.AccountId,
				MemberInfo.CharacterId,
				*MemberInfo.Role,
				*ResponseMessage
			));

			ReportSessionStartedAsync(MemberInfo.SessionId);
		}
	);

	const bool bRequestStarted = Request->ProcessRequest();

	if (!bRequestStarted)
	{
		KickPlayerWithReason(PlayerController, TEXT("Failed to start backend validate request."));
	}
}

void ADGServerGameMode::ReportSessionStartedAsync(
	const FString& SessionId
)
{
	if (SessionId.IsEmpty())
	{
		Debug::Print(TEXT("[DGServerGameMode] ReportSessionStartedAsync failed. SessionId is empty."));
		return;
	}

	FString RequestUrl = BackendBaseUrl;

	if (RequestUrl.EndsWith(TEXT("/")))
	{
		RequestUrl.LeftChopInline(1);
	}

	RequestUrl += TEXT("/api/sessions/session-started");

	const FString BodyJson = BuildSessionStartedJson(SessionId);

	TSharedRef<IHttpRequest, ESPMode::ThreadSafe> Request = FHttpModule::Get().CreateRequest();
	Request->SetURL(RequestUrl);
	Request->SetVerb(TEXT("POST"));
	Request->SetHeader(TEXT("Content-Type"), TEXT("application/json"));
	Request->SetHeader(TEXT("Accept"), TEXT("application/json"));
	Request->SetContentAsString(BodyJson);

	Debug::Print(FString::Printf(
		TEXT("[DGServerGameMode] Session Started Request. Url=%s SessionId=%s"),
		*RequestUrl,
		*SessionId
	));

	Request->OnProcessRequestComplete().BindLambda(
		[this, SessionId](
			FHttpRequestPtr HttpRequest,
			FHttpResponsePtr HttpResponse,
			bool bWasSuccessful
		)
		{
			if (!bWasSuccessful || !HttpResponse.IsValid())
			{
				Debug::Print(TEXT("[DGServerGameMode] Session Started Request Failed."));
				return;
			}

			const int32 ResponseCode = HttpResponse->GetResponseCode();
			const FString ResponseBody = HttpResponse->GetContentAsString();

			if (ResponseCode < 200 || ResponseCode >= 300)
			{
				Debug::Print(FString::Printf(
					TEXT("[DGServerGameMode] Session Started Failed. Code=%d Body=%s"),
					ResponseCode,
					*ResponseBody
				));
				return;
			}

			Debug::Print(FString::Printf(
				TEXT("[DGServerGameMode] Session Started Success. Body=%s"),
				*ResponseBody
			));

			StartSessionHeartbeat(SessionId);
		}
	);

	const bool bRequestStarted = Request->ProcessRequest();

	if (!bRequestStarted)
	{
		Debug::Print(TEXT("[DGServerGameMode] Failed to start session-started request."));
	}
}

void ADGServerGameMode::ReportSessionEndedAsync(
	const FString& SessionId
)
{
	if (SessionId.IsEmpty())
	{
		Debug::Print(TEXT("[DGServerGameMode] ReportSessionEndedAsync failed. SessionId is empty."));
		return;
	}

	FString RequestUrl = BackendBaseUrl;

	if (RequestUrl.EndsWith(TEXT("/")))
	{
		RequestUrl.LeftChopInline(1);
	}

	RequestUrl += TEXT("/api/sessions/session-ended");

	const FString BodyJson = BuildSessionEndedJson(SessionId);

	TSharedRef<IHttpRequest, ESPMode::ThreadSafe> Request = FHttpModule::Get().CreateRequest();
	Request->SetURL(RequestUrl);
	Request->SetVerb(TEXT("POST"));
	Request->SetHeader(TEXT("Content-Type"), TEXT("application/json"));
	Request->SetHeader(TEXT("Accept"), TEXT("application/json"));
	Request->SetContentAsString(BodyJson);

	Debug::Print(FString::Printf(
		TEXT("[DGServerGameMode] Session Ended Request. Url=%s SessionId=%s"),
		*RequestUrl,
		*SessionId
	));

	Request->OnProcessRequestComplete().BindLambda(
		[](FHttpRequestPtr HttpRequest, FHttpResponsePtr HttpResponse, bool bWasSuccessful)
		{
			if (!bWasSuccessful || !HttpResponse.IsValid())
			{
				Debug::Print(TEXT("[DGServerGameMode] Session Ended Request Failed."));
				return;
			}

			const int32 ResponseCode = HttpResponse->GetResponseCode();
			const FString ResponseBody = HttpResponse->GetContentAsString();

			if (ResponseCode < 200 || ResponseCode >= 300)
			{
				Debug::Print(FString::Printf(
					TEXT("[DGServerGameMode] Session Ended Failed. Code=%d Body=%s"),
					ResponseCode,
					*ResponseBody
				));
				return;
			}

			Debug::Print(FString::Printf(
				TEXT("[DGServerGameMode] Session Ended Success. Body=%s"),
				*ResponseBody
			));
		}
	);

	const bool bRequestStarted = Request->ProcessRequest();

	if (!bRequestStarted)
	{
		Debug::Print(TEXT("[DGServerGameMode] Failed to start session-ended request."));
	}
}

void ADGServerGameMode::ReportMemberLeftAsync(
	const FDGConnectedMemberInfo& MemberInfo,
	bool bWasLastKnownPlayer
)
{
	if (MemberInfo.SessionId.IsEmpty())
	{
		Debug::Print(TEXT("[DGServerGameMode] ReportMemberLeftAsync failed. SessionId is empty."));
		return;
	}

	if (MemberInfo.AccountId <= 0 || MemberInfo.CharacterId <= 0)
	{
		Debug::Print(FString::Printf(
			TEXT("[DGServerGameMode] ReportMemberLeftAsync failed. Invalid member. AccountId=%lld CharacterId=%lld"),
			MemberInfo.AccountId,
			MemberInfo.CharacterId
		));
		return;
	}

	FString RequestUrl = BackendBaseUrl;

	if (RequestUrl.EndsWith(TEXT("/")))
	{
		RequestUrl.LeftChopInline(1);
	}

	RequestUrl += TEXT("/api/sessions/member-left");

	const FString BodyJson = BuildMemberLeftJson(
		MemberInfo.SessionId,
		MemberInfo.AccountId,
		MemberInfo.CharacterId
	);

	TSharedRef<IHttpRequest, ESPMode::ThreadSafe> Request = FHttpModule::Get().CreateRequest();
	Request->SetURL(RequestUrl);
	Request->SetVerb(TEXT("POST"));
	Request->SetHeader(TEXT("Content-Type"), TEXT("application/json"));
	Request->SetHeader(TEXT("Accept"), TEXT("application/json"));
	Request->SetContentAsString(BodyJson);

	Debug::Print(FString::Printf(
		TEXT("[DGServerGameMode] Member Left Request. Url=%s SessionId=%s AccountId=%lld CharacterId=%lld LastKnown=%s"),
		*RequestUrl,
		*MemberInfo.SessionId,
		MemberInfo.AccountId,
		MemberInfo.CharacterId,
		bWasLastKnownPlayer ? TEXT("true") : TEXT("false")
	));

	Request->OnProcessRequestComplete().BindLambda(
		[this, MemberInfo, bWasLastKnownPlayer](
			FHttpRequestPtr HttpRequest,
			FHttpResponsePtr HttpResponse,
			bool bWasSuccessful
		)
		{
			if (!bWasSuccessful || !HttpResponse.IsValid())
			{
				Debug::Print(TEXT("[DGServerGameMode] Member Left Request Failed."));

				if (bWasLastKnownPlayer)
				{
					Debug::Print(TEXT("[DGServerGameMode] Member Left failed for last known player. Fallback session-ended."));
					TryReportSessionEndedIfNoPlayers();
				}

				return;
			}

			const int32 ResponseCode = HttpResponse->GetResponseCode();
			const FString ResponseBody = HttpResponse->GetContentAsString();

			bool bShouldShutdownServer = false;
			FString ResponseMessage;

			const bool bParsed = ParseMemberLeftResponse(
				ResponseBody,
				bShouldShutdownServer,
				ResponseMessage
			);

			if (ResponseCode < 200 || ResponseCode >= 300)
			{
				Debug::Print(FString::Printf(
					TEXT("[DGServerGameMode] Member Left Failed. Code=%d Body=%s"),
					ResponseCode,
					*ResponseBody
				));

				if (bWasLastKnownPlayer)
				{
					Debug::Print(TEXT("[DGServerGameMode] Member Left HTTP failed for last known player. Fallback session-ended."));
					TryReportSessionEndedIfNoPlayers();
				}

				return;
			}

			Debug::Print(FString::Printf(
				TEXT("[DGServerGameMode] Member Left Success. Parsed=%s ShouldShutdown=%s Message=%s Body=%s"),
				bParsed ? TEXT("true") : TEXT("false"),
				bShouldShutdownServer ? TEXT("true") : TEXT("false"),
				*ResponseMessage,
				*ResponseBody
			));

			if (!bShouldShutdownServer)
			{
				return;
			}

			Debug::Print(FString::Printf(
				TEXT("[DGServerGameMode] Last member left. Shutdown dedicated server. SessionId=%s"),
				*MemberInfo.SessionId
			));

			bSessionEndReported = true;

			StopSessionHeartbeat();

			if (ActiveSessionId == MemberInfo.SessionId)
			{
				ActiveSessionId.Empty();
			}

			FGenericPlatformMisc::RequestExit(false);
		}
	);

	const bool bRequestStarted = Request->ProcessRequest();

	if (!bRequestStarted)
	{
		Debug::Print(TEXT("[DGServerGameMode] Failed to start member-left request."));

		if (bWasLastKnownPlayer)
		{
			Debug::Print(TEXT("[DGServerGameMode] Member Left request start failed for last known player. Fallback session-ended."));
			TryReportSessionEndedIfNoPlayers();
		}
	}
}

void ADGServerGameMode::StartSessionHeartbeat(
	const FString& SessionId
)
{
	if (SessionId.IsEmpty())
	{
		Debug::Print(TEXT("[DGServerGameMode] StartSessionHeartbeat failed. SessionId is empty."));
		return;
	}

	UWorld* World = GetWorld();

	if (!World)
	{
		Debug::Print(TEXT("[DGServerGameMode] StartSessionHeartbeat failed. World is null."));
		return;
	}

	if (ActiveSessionId == SessionId && World->GetTimerManager().IsTimerActive(SessionHeartbeatTimerHandle))
	{
		Debug::Print(FString::Printf(
			TEXT("[DGServerGameMode] Session heartbeat already running. SessionId=%s"),
			*SessionId
		));
		return;
	}

	ActiveSessionId = SessionId;
	bSessionEndReported = false;

	World->GetTimerManager().ClearTimer(SessionHeartbeatTimerHandle);

	SendSessionHeartbeatAsync(ActiveSessionId);

	World->GetTimerManager().SetTimer(
		SessionHeartbeatTimerHandle,
		this,
		&ADGServerGameMode::SendActiveSessionHeartbeat,
		HeartbeatIntervalSeconds,
		true
	);

	Debug::Print(FString::Printf(
		TEXT("[DGServerGameMode] Session heartbeat started. SessionId=%s Interval=%.1f"),
		*ActiveSessionId,
		HeartbeatIntervalSeconds
	));
}

void ADGServerGameMode::StopSessionHeartbeat()
{
	UWorld* World = GetWorld();

	if (World)
	{
		World->GetTimerManager().ClearTimer(SessionHeartbeatTimerHandle);
	}

	if (!ActiveSessionId.IsEmpty())
	{
		Debug::Print(FString::Printf(
			TEXT("[DGServerGameMode] Session heartbeat stopped. SessionId=%s"),
			*ActiveSessionId
		));
	}
}

void ADGServerGameMode::SendActiveSessionHeartbeat()
{
	if (ActiveSessionId.IsEmpty())
	{
		Debug::Print(TEXT("[DGServerGameMode] SendActiveSessionHeartbeat skipped. ActiveSessionId is empty."));
		return;
	}

	SendSessionHeartbeatAsync(ActiveSessionId);
}

void ADGServerGameMode::SendSessionHeartbeatAsync(
	const FString& SessionId
)
{
	if (SessionId.IsEmpty())
	{
		Debug::Print(TEXT("[DGServerGameMode] SendSessionHeartbeatAsync failed. SessionId is empty."));
		return;
	}

	FString RequestUrl = BackendBaseUrl;

	if (RequestUrl.EndsWith(TEXT("/")))
	{
		RequestUrl.LeftChopInline(1);
	}

	RequestUrl += TEXT("/api/sessions/heartbeat");

	const FString BodyJson = BuildSessionHeartbeatJson(SessionId);

	TSharedRef<IHttpRequest, ESPMode::ThreadSafe> Request = FHttpModule::Get().CreateRequest();
	Request->SetURL(RequestUrl);
	Request->SetVerb(TEXT("POST"));
	Request->SetHeader(TEXT("Content-Type"), TEXT("application/json"));
	Request->SetHeader(TEXT("Accept"), TEXT("application/json"));
	Request->SetContentAsString(BodyJson);

	Debug::Print(FString::Printf(
		TEXT("[DGServerGameMode] Session Heartbeat Request. Url=%s SessionId=%s"),
		*RequestUrl,
		*SessionId
	));

	Request->OnProcessRequestComplete().BindLambda(
		[](
			FHttpRequestPtr HttpRequest,
			FHttpResponsePtr HttpResponse,
			bool bWasSuccessful
		)
		{
			if (!bWasSuccessful || !HttpResponse.IsValid())
			{
				Debug::Print(TEXT("[DGServerGameMode] Session Heartbeat Request Failed."));
				return;
			}

			const int32 ResponseCode = HttpResponse->GetResponseCode();
			const FString ResponseBody = HttpResponse->GetContentAsString();

			if (ResponseCode < 200 || ResponseCode >= 300)
			{
				Debug::Print(FString::Printf(
					TEXT("[DGServerGameMode] Session Heartbeat Failed. Code=%d Body=%s"),
					ResponseCode,
					*ResponseBody
				));
				return;
			}

			Debug::Print(FString::Printf(
				TEXT("[DGServerGameMode] Session Heartbeat Success. Body=%s"),
				*ResponseBody
			));
		}
	);

	const bool bRequestStarted = Request->ProcessRequest();

	if (!bRequestStarted)
	{
		Debug::Print(TEXT("[DGServerGameMode] Failed to start heartbeat request."));
	}
}

void ADGServerGameMode::TryReportSessionEndedIfNoPlayers()
{
	if (ConnectedPlayerCount > 0)
	{
		Debug::Print(FString::Printf(
			TEXT("[DGServerGameMode] Session end skipped. Players still connected. Count=%d"),
			ConnectedPlayerCount
		));
		return;
	}

	if (ActiveSessionId.IsEmpty())
	{
		Debug::Print(TEXT("[DGServerGameMode] Session end skipped. ActiveSessionId is empty."));
		return;
	}

	if (bSessionEndReported)
	{
		Debug::Print(FString::Printf(
			TEXT("[DGServerGameMode] Session end skipped. Already reported. SessionId=%s"),
			*ActiveSessionId
		));
		return;
	}

	const FString SessionIdToEnd = ActiveSessionId;

	bSessionEndReported = true;

	StopSessionHeartbeat();

	ActiveSessionId.Empty();

	ReportSessionEndedAsync(SessionIdToEnd);
}

void ADGServerGameMode::KickPlayerWithReason(
	APlayerController* PlayerController,
	const FString& Reason
)
{
	if (!PlayerController)
	{
		return;
	}

	Debug::Print(FString::Printf(
		TEXT("[DGServerGameMode] Kick Player. Player=%s Reason=%s"),
		*PlayerController->GetName(),
		*Reason
	));

	APawn* PawnToDestroy = PlayerController->GetPawn();

	if (IsValid(PawnToDestroy))
	{
		Debug::Print(FString::Printf(
			TEXT("[DGServerGameMode] Kick Player. Destroy pawn. Pawn=%s"),
			*PawnToDestroy->GetName()
		));

		PawnToDestroy->Destroy();
	}

	if (GameSession)
	{
		GameSession->KickPlayer(PlayerController, FText::FromString(Reason));
		return;
	}

	PlayerController->Destroy();
}

FString ADGServerGameMode::BuildValidateJoinJson(
	const FString& SessionId,
	const FString& JoinToken
)
{
	TSharedRef<FJsonObject> JsonObject = MakeShared<FJsonObject>();

	JsonObject->SetStringField(TEXT("sessionId"), SessionId);
	JsonObject->SetStringField(TEXT("joinToken"), JoinToken);

	FString OutputString;
	TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&OutputString);
	FJsonSerializer::Serialize(JsonObject, Writer);

	return OutputString;
}

FString ADGServerGameMode::BuildSessionStartedJson(
	const FString& SessionId
)
{
	TSharedRef<FJsonObject> JsonObject = MakeShared<FJsonObject>();

	JsonObject->SetStringField(TEXT("sessionId"), SessionId);

	FString OutputString;
	TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&OutputString);
	FJsonSerializer::Serialize(JsonObject, Writer);

	return OutputString;
}

FString ADGServerGameMode::BuildSessionEndedJson(
	const FString& SessionId
)
{
	TSharedRef<FJsonObject> JsonObject = MakeShared<FJsonObject>();

	JsonObject->SetStringField(TEXT("sessionId"), SessionId);

	FString OutputString;
	TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&OutputString);
	FJsonSerializer::Serialize(JsonObject, Writer);

	return OutputString;
}

FString ADGServerGameMode::BuildSessionHeartbeatJson(
	const FString& SessionId
)
{
	TSharedRef<FJsonObject> JsonObject = MakeShared<FJsonObject>();

	JsonObject->SetStringField(TEXT("sessionId"), SessionId);

	FString OutputString;
	TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&OutputString);
	FJsonSerializer::Serialize(JsonObject, Writer);

	return OutputString;
}

FString ADGServerGameMode::BuildMemberLeftJson(
	const FString& SessionId,
	int64 AccountId,
	int64 CharacterId
)
{
	TSharedRef<FJsonObject> JsonObject = MakeShared<FJsonObject>();

	JsonObject->SetStringField(TEXT("sessionId"), SessionId);
	JsonObject->SetNumberField(TEXT("accountId"), static_cast<double>(AccountId));
	JsonObject->SetNumberField(TEXT("characterId"), static_cast<double>(CharacterId));

	FString OutputString;
	TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&OutputString);
	FJsonSerializer::Serialize(JsonObject, Writer);

	return OutputString;
}

bool ADGServerGameMode::ParseValidateJoinResponse(
	const FString& ResponseBody,
	FString& OutMessage,
	FString& OutSessionId,
	int64& OutAccountId,
	int64& OutCharacterId,
	FString& OutRole
)
{
	TSharedPtr<FJsonObject> JsonObject;
	TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(ResponseBody);

	if (!FJsonSerializer::Deserialize(Reader, JsonObject) || !JsonObject.IsValid())
	{
		OutMessage = TEXT("Failed to parse validate-join response.");
		return false;
	}

	bool bSuccess = false;
	JsonObject->TryGetBoolField(TEXT("success"), bSuccess);
	JsonObject->TryGetStringField(TEXT("message"), OutMessage);
	JsonObject->TryGetStringField(TEXT("sessionId"), OutSessionId);
	JsonObject->TryGetStringField(TEXT("role"), OutRole);

	double AccountIdValue = 0.0;
	double CharacterIdValue = 0.0;

	if (JsonObject->TryGetNumberField(TEXT("accountId"), AccountIdValue))
	{
		OutAccountId = static_cast<int64>(AccountIdValue);
	}

	if (JsonObject->TryGetNumberField(TEXT("characterId"), CharacterIdValue))
	{
		OutCharacterId = static_cast<int64>(CharacterIdValue);
	}

	if (bSuccess && (OutAccountId <= 0 || OutCharacterId <= 0))
	{
		OutMessage = TEXT("Validate-join response has invalid AccountId or CharacterId.");
		return false;
	}

	return bSuccess;
}

bool ADGServerGameMode::ParseMemberLeftResponse(
	const FString& ResponseBody,
	bool& bOutShouldShutdownServer,
	FString& OutMessage
)
{
	TSharedPtr<FJsonObject> JsonObject;
	TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(ResponseBody);

	if (!FJsonSerializer::Deserialize(Reader, JsonObject) || !JsonObject.IsValid())
	{
		OutMessage = TEXT("Failed to parse member-left response.");
		return false;
	}

	bool bSuccess = false;
	JsonObject->TryGetBoolField(TEXT("success"), bSuccess);
	JsonObject->TryGetBoolField(TEXT("shouldShutdownServer"), bOutShouldShutdownServer);
	JsonObject->TryGetStringField(TEXT("message"), OutMessage);

	return bSuccess;
}