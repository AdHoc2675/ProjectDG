#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "DGBackendTypes.h"
#include "DGBackendClient.generated.h"

DECLARE_DELEGATE_TwoParams(FDGSessionApiResultCallback, bool /*bSuccess*/, const FDGSessionConnectionInfo& /*Result*/);

//백앤드 API-HTTP 통신 전담하는 클래스

//API/Session/Create 호출
//API/Session/Join 호출
//JSON 생성
//JSON 응답

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

	static FDGSessionConnectionInfo ParseSessionConnectionInfo(bool bRequestSucceeded, int32 ResponseCode,
	                                                           const FString& ResponseBody);
};
