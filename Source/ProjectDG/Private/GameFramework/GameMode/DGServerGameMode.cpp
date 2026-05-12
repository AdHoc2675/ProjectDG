#include "GameFramework/GameMode/DGServerGameMode.h"

#include "Core/DG_Debug.h"
#include "Dom/JsonObject.h"
#include "Engine/World.h"
#include "GameFramework/Controller.h"
#include "GameFramework/GameSession.h"
#include "GameFramework/PlayerController.h"
#include "HttpModule.h"
#include "Interfaces/IHttpRequest.h"
#include "Interfaces/IHttpResponse.h"
#include "Kismet/GameplayStatics.h"
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

	Super::Logout(Exiting);

	if (GetNetMode() != NM_DedicatedServer)
	{
		return;
	}

	ConnectedPlayerCount = FMath::Max(0, ConnectedPlayerCount - 1);

	Debug::Print(FString::Printf(
		TEXT("[DGServerGameMode] Logout. Player=%s ConnectedPlayerCount=%d"),
		*ExitingName,
		ConnectedPlayerCount
	));

	TryReportSessionEndedIfNoPlayers();
}

void ADGServerGameMode::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (GetNetMode() == NM_DedicatedServer && !ActiveSessionId.IsEmpty() && !bSessionEndReported)
	{
		const FString SessionIdToEnd = ActiveSessionId;

		bSessionEndReported = true;

		StopSessionHeartbeat();

		ReportSessionEndedAsync(SessionIdToEnd);
	}
	else
	{
		StopSessionHeartbeat();
	}

	Super::EndPlay(EndPlayReason);
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
			const bool bValidJoin = ParseValidateJoinResponse(ResponseBody, ResponseMessage);

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

			Debug::Print(FString::Printf(
				TEXT("[DGServerGameMode] Validate Join Success. Player=%s Message=%s"),
				*PlayerController->GetName(),
				*ResponseMessage
			));

			ReportSessionStartedAsync(SessionId);
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

bool ADGServerGameMode::ParseValidateJoinResponse(
	const FString& ResponseBody,
	FString& OutMessage
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

	return bSuccess;
}