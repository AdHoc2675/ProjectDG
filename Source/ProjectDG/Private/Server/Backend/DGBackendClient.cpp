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

void UDGBackendClient::CreateSession(const FDGCreateSessionRequest& RequestData, FDGSessionApiResultCallback Callback)
{
	const FString BodyJson = BuildCreateSessionJson(RequestData);
	SendPostRequest(TEXT("/api/sessions/create"), BodyJson, Callback);
}

void UDGBackendClient::JoinSession(const FDGJoinSessionRequest& RequestData, FDGSessionApiResultCallback Callback)

{
	const FString BodyJson = BuildJoinSessionJson(RequestData);
	SendPostRequest(TEXT("/api/sessions/join"), BodyJson, Callback);
}

void UDGBackendClient::SendPostRequest(const FString& EndPoint, const FString& BodyJson,
                                       FDGSessionApiResultCallback Callback)
{
	const FString Url = BaseUrl + EndPoint;

	TSharedRef<IHttpRequest, ESPMode::ThreadSafe> HttpRequest = FHttpModule::Get().CreateRequest();

	HttpRequest->SetURL(Url);
	HttpRequest->SetVerb(TEXT("POST"));
	HttpRequest->SetHeader(TEXT("Content-Type"),TEXT("application/json"));
	HttpRequest->SetHeader(TEXT("Accept"),TEXT("application/json"));
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
				bWasSuccessful, ResponseCode, ResponseBody);

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

FString UDGBackendClient::BuildCreateSessionJson(const FDGCreateSessionRequest& RequestData)
{
	TSharedRef<FJsonObject> JsonObject = MakeShared<FJsonObject>();

	JsonObject->SetNumberField(TEXT("accountId"), static_cast<double>(RequestData.AccountId));
	JsonObject->SetNumberField(TEXT("characterId"), static_cast<double>(RequestData.CharacterId));
	JsonObject->SetStringField(TEXT("regionId"), RequestData.RegionId);

	FString OutputString;
	TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&OutputString);
	FJsonSerializer::Serialize(JsonObject, Writer);

	return OutputString;
}

FString UDGBackendClient::BuildJoinSessionJson(const FDGJoinSessionRequest& RequestData)
{
	TSharedRef<FJsonObject> JsonObject = MakeShared<FJsonObject>();

	JsonObject->SetStringField(TEXT("sessionId"), RequestData.SessionId);
	JsonObject->SetNumberField(TEXT("accountId"), static_cast<double>(RequestData.AccountId));
	JsonObject->SetNumberField(TEXT("characterId"), static_cast<double>(RequestData.CharacterId));

	FString OutputString;
	TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&OutputString);
	FJsonSerializer::Serialize(JsonObject, Writer);

	return OutputString;
}

FDGSessionConnectionInfo UDGBackendClient::ParseSessionConnectionInfo(bool bRequestSucceeded, int32 ResponseCode,
                                                                      const FString& ResponseBody)
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
