#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "DGBackendTypes.h"
#include "DGBackendClient.generated.h"

DECLARE_DELEGATE_TwoParams(FDGSessionApiResultCallback, bool /*bSuccess*/, const FDGSessionConnectionInfo& /*Result*/);
DECLARE_DELEGATE_TwoParams(FDGAuthApiResultCallback, bool /*bSuccess*/, const FDGAuthResult& /*Result*/);
DECLARE_DELEGATE_TwoParams(FDGCharacterListApiResultCallback, bool /*bSuccess*/, const FDGCharacterListResult& /*Result*/);

/**
 * Backend API HTTP 통신 전담 클래스
 *
 * 역할:
 * - Auth API 호출
 * - Session API 호출
 * - JSON 요청 생성
 * - JSON 응답 파싱
 */
UCLASS()
class PROJECTDG_API UDGBackendClient : public UObject
{
	GENERATED_BODY()

public:
	void Initialize(const FString& InBaseUrl);

	void RegisterAccount(const FDGRegisterRequest& RequestData, FDGAuthApiResultCallback Callback);

	void Login(const FDGLoginRequest& RequestData, FDGAuthApiResultCallback Callback);

	void GetCharacters(int64 AccountId, FDGCharacterListApiResultCallback Callback);

	void CreateSession(const FDGCreateSessionRequest& RequestData, FDGSessionApiResultCallback Callback);

	void JoinSession(const FDGJoinSessionRequest& RequestData, FDGSessionApiResultCallback Callback);

private:
	FString BaseUrl = TEXT("http://localhost:8080");

	void SendPostRequest(
		const FString& EndPoint,
		const FString& BodyJson,
		FDGSessionApiResultCallback Callback
	);

	void SendAuthPostRequest(
		const FString& EndPoint,
		const FString& BodyJson,
		FDGAuthApiResultCallback Callback
	);

	void SendCharacterListGetRequest(
		const FString& EndPoint,
		FDGCharacterListApiResultCallback Callback
	);

	static FString BuildRegisterJson(const FDGRegisterRequest& RequestData);

	static FString BuildLoginJson(const FDGLoginRequest& RequestData);

	static FString BuildCreateSessionJson(const FDGCreateSessionRequest& RequestData);

	static FString BuildJoinSessionJson(const FDGJoinSessionRequest& RequestData);

	static FDGAuthResult ParseAuthResult(
		bool bRequestSucceeded,
		int32 ResponseCode,
		const FString& ResponseBody
	);

	static FDGCharacterListResult ParseCharacterListResult(
		bool bRequestSucceeded,
		int32 ResponseCode,
		const FString& ResponseBody
	);

	static FDGSessionConnectionInfo ParseSessionConnectionInfo(
		bool bRequestSucceeded,
		int32 ResponseCode,
		const FString& ResponseBody
	);
};