#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Server/Backend/DGBackendTypes.h"
#include "DGSessionSubsystem.generated.h"


class UDGBackendClient;

//게임 세션 흐름당담 system

//역할 
//-세션 생성요청
//-세션 합류요청
//백엔드에서 응답 받아서 Dedicated Server 로 넘겨줌

UCLASS()
class PROJECTDG_API UDGSessionSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	//개발용 세션 생성 + 데디서버 접속
	//현재는 127.0.0.1:7777 로 반환

	UFUNCTION(BlueprintCallable, Category="DG|Session")
	void CreateLocalSessionAndTravel(int64 AccountId = 1, int64 CharacterId = 1,
	                                 const FString& RegionId = TEXT("Region_Test"));

	//개발용 세션 합류 + 데디서버 접속
	UFUNCTION(BlueprintCallable, Category="DG|Session")
	void JoinSessionAndTravel(const FString& SessionId, int64 AccountId = 2, int64 CharacterId = 2);

private:
	UPROPERTY()
	TObjectPtr<UDGBackendClient> BackendClient;

	UPROPERTY()
	FString BackendBaseUrl = TEXT("http://localhost:8080");

	void HandleCreateSessionCompleted(
		bool bSuccess,
		const FDGSessionConnectionInfo& Result
	);

	void HandleJoinSessionCompleted(
		bool bSuccess,
		const FDGSessionConnectionInfo& Result
	);

	void TravelToDedicatedServer(
		const FDGSessionConnectionInfo& ConnectionInfo
	);
};
