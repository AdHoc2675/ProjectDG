#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "DGBackendTypes.h"
#include "DGBackendClient.generated.h"

DECLARE_DELEGATE_TwoParams(FDGSessionApiResultCallback, bool /*bSuccess*/, const FDGSessionConnectionInfo& /*Result*/);

/**
 * Backend API HTTP 통신 전담 클래스
 *
 * 역할:
 * - /api/sessions/create 호출
 * - /api/sessions/join 호출
 * - JSON 요청 생성
 * - JSON 응답 파싱
 *
 * 주의:
 * - RoomName / RoomPassword는 Backend로만 보낸다.
 * - Dedicated Server 접속에는 Backend가 반환한 SessionId / JoinToken을 사용한다.
 */
UCLASS()
class PROJECTDG_API UDGBackendClient : public UObject
{
	GENERATED_BODY()

public:
	void Initialize(const FString& InBaseUrl);

	void CreateSession(const FDGCreateSessionRequest& RequestData, FDGSessionApiResultCallback Callback);

	void JoinSession(const FDGJoinSessionRequest& RequestData, FDGSessionApiResultCallback Callback);

private:
	FString BaseUrl = TEXT("http://localhost:8080");

	void SendPostRequest(const FString& EndPoint, const FString& BodyJson, FDGSessionApiResultCallback Callback);

	static FString BuildCreateSessionJson(const FDGCreateSessionRequest& RequestData);

	static FString BuildJoinSessionJson(const FDGJoinSessionRequest& RequestData);

	static FDGSessionConnectionInfo ParseSessionConnectionInfo(
		bool bRequestSucceeded,
		int32 ResponseCode,
		const FString& ResponseBody
	);
};