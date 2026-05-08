#include "GameFramework/GameMode/DGServerGameMode.h"

#include "Core/DG_Debug.h"
#include "Dom/JsonObject.h"
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
		[this, WeakPlayerController](FHttpRequestPtr HttpRequest, FHttpResponsePtr HttpResponse, bool bWasSuccessful)
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
		}
	);

	const bool bRequestStarted = Request->ProcessRequest();

	if (!bRequestStarted)
	{
		KickPlayerWithReason(PlayerController, TEXT("Failed to start backend validate request."));
	}
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