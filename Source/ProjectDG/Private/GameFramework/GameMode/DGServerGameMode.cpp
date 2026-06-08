#include "GameFramework/GameMode/DGServerGameMode.h"

#include "Core/DG_Debug.h"
#include "Dom/JsonObject.h"
#include "Engine/World.h"
#include "GameFramework/Controller.h"
#include "GameFramework/GameSession.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/PlayerState.h"
#include "GameFramework/DG_PlayerState.h"
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

	if (!ErrorMessage.IsEmpty())
	{
		return;
	}

	if (GetNetMode() != NM_DedicatedServer)
	{
		return;
	}

	if (SessionId.IsEmpty())
	{
		ErrorMessage = TEXT("Missing SessionId.");
		return;
	}

	if (JoinToken.IsEmpty())
	{
		ErrorMessage = TEXT("Missing JoinToken.");
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

	if (GetNetMode() != NM_DedicatedServer)
	{
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
		return;
	}

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

	if (GetNetMode() == NM_DedicatedServer)
	{
		ConnectedPlayerCount++;
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

		ExitingPawn->Destroy();
	}

	Super::Logout(Exiting);

	if (GetNetMode() != NM_DedicatedServer)
	{
		return;
	}

	ConnectedPlayerCount = FMath::Max(0, ConnectedPlayerCount - 1);

	if (bHasMemberInfo)
	{
		const bool bWasLastKnownPlayer = ConnectedPlayerCount <= 0;
		ReportMemberLeftAsync(LeavingMemberInfo, bWasLastKnownPlayer);
		return;
	}

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

	Super::RestartPlayer(NewPlayer);

	APawn* PawnAfter = IsValid(NewPlayer)
		                   ? NewPlayer->GetPawn()
		                   : nullptr;

	const FString PawnAfterName = IsValid(PawnAfter)
		                              ? PawnAfter->GetName()
		                              : TEXT("None");
}

UClass* ADGServerGameMode::GetDefaultPawnClassForController_Implementation(AController* InController)
{
	if (IsValid(InController))
	{
		const TObjectKey<AController> ControllerKey(InController);

		if (const FDGConnectedMemberInfo* MemberInfo = ConnectedMemberInfos.Find(ControllerKey))
		{
			FString ClassTag = MemberInfo->ClassTag;
			ClassTag.TrimStartAndEndInline();
			ClassTag.RemoveFromStart(TEXT("\""));
			ClassTag.RemoveFromEnd(TEXT("\""));
			ClassTag.TrimStartAndEndInline();

			if ((ClassTag == TEXT("Character.Class.Warrior") || ClassTag == TEXT("Class.Warrior")) && WarriorPawnClass)
			{
				return WarriorPawnClass;
			}

			if ((ClassTag == TEXT("Character.Class.Archer") || ClassTag == TEXT("Class.Archer")) && ArcherPawnClass)
			{
				return ArcherPawnClass;
			}

			if ((ClassTag == TEXT("Character.Class.Mage") || ClassTag == TEXT("Class.Mage")) && MagePawnClass)
			{
				return MagePawnClass;
			}

			if ((ClassTag == TEXT("Character.Class.Assassin") || ClassTag == TEXT("Class.Assassin")) && AssassinPawnClass)
			{
				return AssassinPawnClass;
			}

			
		}
		
	}

	return Super::GetDefaultPawnClassForController_Implementation(InController);
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
}

void ADGServerGameMode::ValidateJoinTokenAsync(
	APlayerController* PlayerController,
	const FString& SessionId,
	const FString& JoinToken
)
{
	if (!PlayerController)
	{
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

	Request->OnProcessRequestComplete().BindLambda(
		[this, WeakPlayerController, SessionId](
			FHttpRequestPtr HttpRequest,
			FHttpResponsePtr HttpResponse,
			bool bWasSuccessful
		)
		{
			if (!WeakPlayerController.IsValid())
			{
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

			FString ResponseMessage;
			FString ResponseSessionId;
			int64 ResponseAccountId = 0;
			int64 ResponseCharacterId = 0;
			FString ResponseClassTag;
			FString ResponseRole;

			const bool bValidJoin = ParseValidateJoinResponse(
				ResponseBody,
				ResponseMessage,
				ResponseSessionId,
				ResponseAccountId,
				ResponseCharacterId,
				ResponseClassTag,
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
			MemberInfo.ClassTag = ResponseClassTag;
			MemberInfo.Role = ResponseRole;

			ConnectedMemberInfos.Add(TObjectKey<AController>(PlayerController), MemberInfo);
			
			if (ADG_PlayerState* DGPlayerState = PlayerController->GetPlayerState<ADG_PlayerState>())
			{
				DGPlayerState->SetSessionMemberInfo(
					MemberInfo.SessionId,
					MemberInfo.AccountId,
					MemberInfo.CharacterId,
					MemberInfo.ClassTag,
					MemberInfo.Role
				);
			}
			

			if (APawn* ExistingPawn = PlayerController->GetPawn())
			{
				PlayerController->UnPossess();
				ExistingPawn->Destroy();
			}

			RestartPlayer(PlayerController);

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

	Request->OnProcessRequestComplete().BindLambda(
		[this, SessionId](
			FHttpRequestPtr HttpRequest,
			FHttpResponsePtr HttpResponse,
			bool bWasSuccessful
		)
		{
			if (!bWasSuccessful || !HttpResponse.IsValid())
			{
				return;
			}

			const int32 ResponseCode = HttpResponse->GetResponseCode();
			const FString ResponseBody = HttpResponse->GetContentAsString();

			if (ResponseCode < 200 || ResponseCode >= 300)
			{
				return;
			}

			StartSessionHeartbeat(SessionId);
		}
	);

	const bool bRequestStarted = Request->ProcessRequest();
}

void ADGServerGameMode::ReportSessionEndedAsync(
	const FString& SessionId
)
{
	if (SessionId.IsEmpty())
	{
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

	Request->OnProcessRequestComplete().BindLambda(
		[](FHttpRequestPtr HttpRequest, FHttpResponsePtr HttpResponse, bool bWasSuccessful)
		{
			if (!bWasSuccessful || !HttpResponse.IsValid())
			{
				return;
			}

			const int32 ResponseCode = HttpResponse->GetResponseCode();
			const FString ResponseBody = HttpResponse->GetContentAsString();

			if (ResponseCode < 200 || ResponseCode >= 300)
			{
				return;
			}
		}
	);

	const bool bRequestStarted = Request->ProcessRequest();
}

void ADGServerGameMode::ReportMemberLeftAsync(
	const FDGConnectedMemberInfo& MemberInfo,
	bool bWasLastKnownPlayer
)
{
	if (MemberInfo.SessionId.IsEmpty())
	{
		return;
	}

	if (MemberInfo.AccountId <= 0 || MemberInfo.CharacterId <= 0)
	{
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

	Request->OnProcessRequestComplete().BindLambda(
		[this, MemberInfo, bWasLastKnownPlayer](
			FHttpRequestPtr HttpRequest,
			FHttpResponsePtr HttpResponse,
			bool bWasSuccessful
		)
		{
			if (!bWasSuccessful || !HttpResponse.IsValid())
			{
				if (bWasLastKnownPlayer)
				{
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
				if (bWasLastKnownPlayer)
				{
					TryReportSessionEndedIfNoPlayers();
				}

				return;
			}

			if (!bShouldShutdownServer)
			{
				return;
			}

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
		if (bWasLastKnownPlayer)
		{
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
		return;
	}

	UWorld* World = GetWorld();

	if (!World)
	{
		return;
	}

	if (ActiveSessionId == SessionId && World->GetTimerManager().IsTimerActive(SessionHeartbeatTimerHandle))
	{
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
}

void ADGServerGameMode::StopSessionHeartbeat()
{
	UWorld* World = GetWorld();

	if (World)
	{
		World->GetTimerManager().ClearTimer(SessionHeartbeatTimerHandle);
	}
}

void ADGServerGameMode::SendActiveSessionHeartbeat()
{
	if (ActiveSessionId.IsEmpty())
	{
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

	Request->OnProcessRequestComplete().BindLambda(
		[](
			FHttpRequestPtr HttpRequest,
			FHttpResponsePtr HttpResponse,
			bool bWasSuccessful
		)
		{
			if (!bWasSuccessful || !HttpResponse.IsValid())
			{
				return;
			}

			const int32 ResponseCode = HttpResponse->GetResponseCode();
			const FString ResponseBody = HttpResponse->GetContentAsString();

			if (ResponseCode < 200 || ResponseCode >= 300)
			{
				return;
			}
		}
	);

	const bool bRequestStarted = Request->ProcessRequest();
}

void ADGServerGameMode::TryReportSessionEndedIfNoPlayers()
{
	if (ConnectedPlayerCount > 0)
	{
		return;
	}

	if (ActiveSessionId.IsEmpty())
	{
		return;
	}

	if (bSessionEndReported)
	{
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

	APawn* PawnToDestroy = PlayerController->GetPawn();

	if (IsValid(PawnToDestroy))
	{
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
	FString& OutClassTag,
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
	JsonObject->TryGetStringField(TEXT("classTag"), OutClassTag);
	JsonObject->TryGetStringField(TEXT("role"), OutRole);

	OutClassTag.TrimStartAndEndInline();
	OutClassTag.RemoveFromStart(TEXT("\""));
	OutClassTag.RemoveFromEnd(TEXT("\""));
	OutClassTag.TrimStartAndEndInline();

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

	if (bSuccess && OutClassTag.IsEmpty())
	{
		OutMessage = TEXT("Validate-join response is missing ClassTag.");
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