#include "Server/Backend/DGBackendClient.h"

#include "HttpModule.h"
#include "Interfaces/IHttpRequest.h"
#include "Interfaces/IHttpResponse.h"

#include "Dom/JsonObject.h"
#include "Serialization/JsonSerializer.h"
#include "Serialization/JsonWriter.h"

void UDGBackendClient::Initialize(const FString& InBaseUrl)
{
	BaseUrl = InBaseUrl;

	if (BaseUrl.EndsWith(TEXT("/")))
	{
		BaseUrl.LeftChopInline(1);
	}
}

void UDGBackendClient::RegisterAccount(
	const FDGRegisterRequest& RequestData,
	FDGAuthApiResultCallback Callback
)
{
	const FString BodyJson = BuildRegisterJson(RequestData);

	SendAuthPostRequest(TEXT("/api/auth/register"), BodyJson, Callback);
}

void UDGBackendClient::Login(
	const FDGLoginRequest& RequestData,
	FDGAuthApiResultCallback Callback
)
{
	const FString BodyJson = BuildLoginJson(RequestData);

	SendAuthPostRequest(TEXT("/api/auth/login"), BodyJson, Callback);
}

void UDGBackendClient::GetCharacters(
	int64 AccountId,
	FDGCharacterListApiResultCallback Callback
)
{
	const FString EndPoint = FString::Printf(
		TEXT("/api/accounts/%lld/characters"),
		AccountId
	);

	SendCharacterListGetRequest(EndPoint, Callback);
}

void UDGBackendClient::CreateSession(
	const FDGCreateSessionRequest& RequestData,
	FDGSessionApiResultCallback Callback
)
{
	const FString BodyJson = BuildCreateSessionJson(RequestData);

	SendPostRequest(TEXT("/api/sessions/create"), BodyJson, Callback);
}

void UDGBackendClient::JoinSession(
	const FDGJoinSessionRequest& RequestData,
	FDGSessionApiResultCallback Callback
)
{
	const FString BodyJson = BuildJoinSessionJson(RequestData);

	SendPostRequest(TEXT("/api/sessions/join"), BodyJson, Callback);
}

void UDGBackendClient::SendPostRequest(
	const FString& EndPoint,
	const FString& BodyJson,
	FDGSessionApiResultCallback Callback
)
{
	const FString Url = BaseUrl + EndPoint;

	TSharedRef<IHttpRequest, ESPMode::ThreadSafe> HttpRequest = FHttpModule::Get().CreateRequest();

	HttpRequest->SetURL(Url);
	HttpRequest->SetVerb(TEXT("POST"));
	HttpRequest->SetHeader(TEXT("Content-Type"), TEXT("application/json"));
	HttpRequest->SetHeader(TEXT("Accept"), TEXT("application/json"));
	HttpRequest->SetContentAsString(BodyJson);

	HttpRequest->OnProcessRequestComplete().BindLambda(
		[Callback](FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful) mutable
		{
			int32 ResponseCode = 0;
			FString ResponseBody;

			if (Response.IsValid())
			{
				ResponseCode = Response->GetResponseCode();
				ResponseBody = Response->GetContentAsString();
			}

			const FDGSessionConnectionInfo Result = ParseSessionConnectionInfo(
				bWasSuccessful,
				ResponseCode,
				ResponseBody
			);

			Callback.ExecuteIfBound(Result.bSuccess, Result);
		}
	);

	const bool bStarted = HttpRequest->ProcessRequest();

	if (!bStarted)
	{
		FDGSessionConnectionInfo Result;
		Result.bSuccess = false;
		Result.ErrorMessage = TEXT("Failed to start HTTP request.");

		Callback.ExecuteIfBound(false, Result);
	}
}

void UDGBackendClient::SendAuthPostRequest(
	const FString& EndPoint,
	const FString& BodyJson,
	FDGAuthApiResultCallback Callback
)
{
	const FString Url = BaseUrl + EndPoint;

	TSharedRef<IHttpRequest, ESPMode::ThreadSafe> HttpRequest = FHttpModule::Get().CreateRequest();

	HttpRequest->SetURL(Url);
	HttpRequest->SetVerb(TEXT("POST"));
	HttpRequest->SetHeader(TEXT("Content-Type"), TEXT("application/json"));
	HttpRequest->SetHeader(TEXT("Accept"), TEXT("application/json"));
	HttpRequest->SetContentAsString(BodyJson);

	HttpRequest->OnProcessRequestComplete().BindLambda(
		[Callback](FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful) mutable
		{
			int32 ResponseCode = 0;
			FString ResponseBody;

			if (Response.IsValid())
			{
				ResponseCode = Response->GetResponseCode();
				ResponseBody = Response->GetContentAsString();
			}

			const FDGAuthResult Result = ParseAuthResult(
				bWasSuccessful,
				ResponseCode,
				ResponseBody
			);

			Callback.ExecuteIfBound(Result.bSuccess, Result);
		}
	);

	const bool bStarted = HttpRequest->ProcessRequest();

	if (!bStarted)
	{
		FDGAuthResult Result;
		Result.bSuccess = false;
		Result.ErrorMessage = TEXT("Failed to start HTTP request.");

		Callback.ExecuteIfBound(false, Result);
	}
}

void UDGBackendClient::SendCharacterListGetRequest(
	const FString& EndPoint,
	FDGCharacterListApiResultCallback Callback
)
{
	const FString Url = BaseUrl + EndPoint;

	TSharedRef<IHttpRequest, ESPMode::ThreadSafe> HttpRequest = FHttpModule::Get().CreateRequest();

	HttpRequest->SetURL(Url);
	HttpRequest->SetVerb(TEXT("GET"));
	HttpRequest->SetHeader(TEXT("Accept"), TEXT("application/json"));

	HttpRequest->OnProcessRequestComplete().BindLambda(
		[Callback](FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful) mutable
		{
			int32 ResponseCode = 0;
			FString ResponseBody;

			if (Response.IsValid())
			{
				ResponseCode = Response->GetResponseCode();
				ResponseBody = Response->GetContentAsString();
			}

			const FDGCharacterListResult Result = ParseCharacterListResult(
				bWasSuccessful,
				ResponseCode,
				ResponseBody
			);

			Callback.ExecuteIfBound(Result.bSuccess, Result);
		}
	);

	const bool bStarted = HttpRequest->ProcessRequest();

	if (!bStarted)
	{
		FDGCharacterListResult Result;
		Result.bSuccess = false;
		Result.ErrorMessage = TEXT("Failed to start HTTP request.");

		Callback.ExecuteIfBound(false, Result);
	}
}

FString UDGBackendClient::BuildRegisterJson(
	const FDGRegisterRequest& RequestData
)
{
	TSharedRef<FJsonObject> JsonObject = MakeShared<FJsonObject>();

	JsonObject->SetStringField(TEXT("loginId"), RequestData.LoginId);
	JsonObject->SetStringField(TEXT("password"), RequestData.Password);
	JsonObject->SetStringField(TEXT("displayName"), RequestData.DisplayName);

	FString OutputString;
	TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&OutputString);
	FJsonSerializer::Serialize(JsonObject, Writer);

	return OutputString;
}

FString UDGBackendClient::BuildLoginJson(
	const FDGLoginRequest& RequestData
)
{
	TSharedRef<FJsonObject> JsonObject = MakeShared<FJsonObject>();

	JsonObject->SetStringField(TEXT("loginId"), RequestData.LoginId);
	JsonObject->SetStringField(TEXT("password"), RequestData.Password);

	FString OutputString;
	TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&OutputString);
	FJsonSerializer::Serialize(JsonObject, Writer);

	return OutputString;
}

FString UDGBackendClient::BuildCreateSessionJson(
	const FDGCreateSessionRequest& RequestData
)
{
	TSharedRef<FJsonObject> JsonObject = MakeShared<FJsonObject>();

	JsonObject->SetNumberField(TEXT("accountId"), static_cast<double>(RequestData.AccountId));
	JsonObject->SetNumberField(TEXT("characterId"), static_cast<double>(RequestData.CharacterId));
	JsonObject->SetStringField(TEXT("regionId"), RequestData.RegionId);
	JsonObject->SetStringField(TEXT("roomName"), RequestData.RoomName);
	JsonObject->SetStringField(TEXT("roomPassword"), RequestData.RoomPassword);

	FString OutputString;
	TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&OutputString);
	FJsonSerializer::Serialize(JsonObject, Writer);

	return OutputString;
}

FString UDGBackendClient::BuildJoinSessionJson(
	const FDGJoinSessionRequest& RequestData
)
{
	TSharedRef<FJsonObject> JsonObject = MakeShared<FJsonObject>();

	JsonObject->SetStringField(TEXT("roomName"), RequestData.RoomName);
	JsonObject->SetStringField(TEXT("roomPassword"), RequestData.RoomPassword);
	JsonObject->SetNumberField(TEXT("accountId"), static_cast<double>(RequestData.AccountId));
	JsonObject->SetNumberField(TEXT("characterId"), static_cast<double>(RequestData.CharacterId));

	FString OutputString;
	TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&OutputString);
	FJsonSerializer::Serialize(JsonObject, Writer);

	return OutputString;
}

FDGAuthResult UDGBackendClient::ParseAuthResult(
	bool bRequestSucceeded,
	int32 ResponseCode,
	const FString& ResponseBody
)
{
	FDGAuthResult Result;

	if (!bRequestSucceeded)
	{
		Result.bSuccess = false;
		Result.ErrorMessage = TEXT("HTTP request failed.");
		return Result;
	}

	if (ResponseCode < 200 || ResponseCode >= 300)
	{
		Result.bSuccess = false;
		Result.ErrorMessage = FString::Printf(
			TEXT("Backend returned HTTP error. Code: %d Body: %s"),
			ResponseCode,
			*ResponseBody
		);
		return Result;
	}

	TSharedPtr<FJsonObject> JsonObject;
	TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(ResponseBody);

	if (!FJsonSerializer::Deserialize(Reader, JsonObject) || !JsonObject.IsValid())
	{
		Result.bSuccess = false;
		Result.ErrorMessage = TEXT("Failed to parse backend JSON response.");
		return Result;
	}

	bool bApiSuccess = false;
	JsonObject->TryGetBoolField(TEXT("success"), bApiSuccess);

	Result.bSuccess = bApiSuccess;

	double AccountIdDouble = 0.0;
	if (JsonObject->TryGetNumberField(TEXT("accountId"), AccountIdDouble))
	{
		Result.AccountId = static_cast<int64>(AccountIdDouble);
	}

	JsonObject->TryGetStringField(TEXT("loginId"), Result.LoginId);
	JsonObject->TryGetStringField(TEXT("displayName"), Result.DisplayName);
	JsonObject->TryGetStringField(TEXT("message"), Result.Message);

	if (!Result.bSuccess)
	{
		if (Result.Message.IsEmpty())
		{
			Result.ErrorMessage = TEXT("Backend API returned success=false.");
		}
		else
		{
			Result.ErrorMessage = Result.Message;
		}
	}

	if (Result.bSuccess && Result.AccountId <= 0)
	{
		Result.bSuccess = false;
		Result.ErrorMessage = TEXT("Backend response is missing AccountId.");
	}

	return Result;
}

FDGCharacterListResult UDGBackendClient::ParseCharacterListResult(
	bool bRequestSucceeded,
	int32 ResponseCode,
	const FString& ResponseBody
)
{
	FDGCharacterListResult Result;

	if (!bRequestSucceeded)
	{
		Result.bSuccess = false;
		Result.ErrorMessage = TEXT("HTTP request failed.");
		return Result;
	}

	if (ResponseCode < 200 || ResponseCode >= 300)
	{
		Result.bSuccess = false;
		Result.ErrorMessage = FString::Printf(
			TEXT("Backend returned HTTP error. Code: %d Body: %s"),
			ResponseCode,
			*ResponseBody
		);
		return Result;
	}

	TSharedPtr<FJsonObject> JsonObject;
	TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(ResponseBody);

	if (!FJsonSerializer::Deserialize(Reader, JsonObject) || !JsonObject.IsValid())
	{
		Result.bSuccess = false;
		Result.ErrorMessage = TEXT("Failed to parse backend JSON response.");
		return Result;
	}

	bool bApiSuccess = false;
	JsonObject->TryGetBoolField(TEXT("success"), bApiSuccess);

	Result.bSuccess = bApiSuccess;

	double AccountIdDouble = 0.0;
	if (JsonObject->TryGetNumberField(TEXT("accountId"), AccountIdDouble))
	{
		Result.AccountId = static_cast<int64>(AccountIdDouble);
	}

	JsonObject->TryGetStringField(TEXT("message"), Result.Message);

	const TArray<TSharedPtr<FJsonValue>>* CharacterArray = nullptr;
	if (JsonObject->TryGetArrayField(TEXT("characters"), CharacterArray))
	{
		for (const TSharedPtr<FJsonValue>& CharacterValue : *CharacterArray)
		{
			if (!CharacterValue.IsValid())
			{
				continue;
			}

			const TSharedPtr<FJsonObject> CharacterObject = CharacterValue->AsObject();

			if (!CharacterObject.IsValid())
			{
				continue;
			}

			FDGCharacterSummary CharacterSummary;

			double CharacterIdDouble = 0.0;
			if (CharacterObject->TryGetNumberField(TEXT("characterId"), CharacterIdDouble))
			{
				CharacterSummary.CharacterId = static_cast<int64>(CharacterIdDouble);
			}

			double LevelDouble = 1.0;
			if (CharacterObject->TryGetNumberField(TEXT("level"), LevelDouble))
			{
				CharacterSummary.Level = static_cast<int32>(LevelDouble);
			}

			CharacterObject->TryGetStringField(TEXT("characterName"), CharacterSummary.CharacterName);
			CharacterObject->TryGetStringField(TEXT("classTag"), CharacterSummary.ClassTag);

			Result.Characters.Add(CharacterSummary);
		}
	}

	if (!Result.bSuccess)
	{
		if (Result.Message.IsEmpty())
		{
			Result.ErrorMessage = TEXT("Backend API returned success=false.");
		}
		else
		{
			Result.ErrorMessage = Result.Message;
		}
	}

	if (Result.bSuccess && Result.AccountId <= 0)
	{
		Result.bSuccess = false;
		Result.ErrorMessage = TEXT("Backend response is missing AccountId.");
	}

	return Result;
}

FDGSessionConnectionInfo UDGBackendClient::ParseSessionConnectionInfo(
	bool bRequestSucceeded,
	int32 ResponseCode,
	const FString& ResponseBody
)
{
	FDGSessionConnectionInfo Result;

	if (!bRequestSucceeded)
	{
		Result.bSuccess = false;
		Result.ErrorMessage = TEXT("HTTP request failed.");
		return Result;
	}

	if (ResponseCode < 200 || ResponseCode >= 300)
	{
		Result.bSuccess = false;
		Result.ErrorMessage = FString::Printf(
			TEXT("Backend returned HTTP error. Code: %d Body: %s"),
			ResponseCode,
			*ResponseBody
		);
		return Result;
	}

	TSharedPtr<FJsonObject> JsonObject;
	TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(ResponseBody);

	if (!FJsonSerializer::Deserialize(Reader, JsonObject) || !JsonObject.IsValid())
	{
		Result.bSuccess = false;
		Result.ErrorMessage = TEXT("Failed to parse backend JSON response.");
		return Result;
	}

	bool bApiSuccess = false;
	JsonObject->TryGetBoolField(TEXT("success"), bApiSuccess);

	Result.bSuccess = bApiSuccess;

	JsonObject->TryGetStringField(TEXT("sessionId"), Result.SessionId);
	JsonObject->TryGetStringField(TEXT("serverIp"), Result.ServerIP);
	JsonObject->TryGetStringField(TEXT("mapPath"), Result.MapPath);
	JsonObject->TryGetStringField(TEXT("joinToken"), Result.JoinToken);

	double ServerPortDouble = 0.0;
	if (JsonObject->TryGetNumberField(TEXT("serverPort"), ServerPortDouble))
	{
		Result.ServerPort = static_cast<int32>(ServerPortDouble);
	}

	if (!Result.bSuccess)
	{
		JsonObject->TryGetStringField(TEXT("message"), Result.ErrorMessage);

		if (Result.ErrorMessage.IsEmpty())
		{
			Result.ErrorMessage = TEXT("Backend API returned success=false.");
		}
	}

	if (Result.bSuccess)
	{
		if (Result.ServerIP.IsEmpty() || Result.ServerPort <= 0)
		{
			Result.bSuccess = false;
			Result.ErrorMessage = TEXT("Backend response is missing serverIp or serverPort.");
		}
	}

	return Result;
}