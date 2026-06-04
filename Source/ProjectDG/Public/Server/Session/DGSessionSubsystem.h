#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Server/Backend/DGBackendTypes.h"
#include "DGSessionSubsystem.generated.h"

class UDGBackendClient;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FDGSessionOnSessionCreated, const FString&, SessionId);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FDGSessionOnRequestFailed, const FString&, ErrorMessage);

/**
 * 방 생성 / 방 참가 / Dedicated Server 이동 담당 Subsystem
 *
 * 역할:
 * - 방 생성 요청
 * - 방 참가 요청
 * - Backend에서 받은 SessionId / JoinToken / ServerIP / ServerPort 저장
 * - Dedicated Server로 ClientTravel
 *
 * 계정 / 로그인 / 캐릭터 목록 / 캐릭터 선택은 DGAuthSubsystem에서 처리한다.
 */
UCLASS(BlueprintType)
class PROJECTDG_API UDGSessionSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	UFUNCTION(BlueprintCallable, Category = "DG|Session")
	void CreateRoomAndTravel(
		const FString& RoomName,
		const FString& RoomPassword,
		int64 AccountId,
		int64 CharacterId,
		const FString& RegionId
	);

	UFUNCTION(BlueprintCallable, Category = "DG|Session")
	void JoinRoomAndTravel(
		const FString& RoomName,
		const FString& RoomPassword,
		int64 AccountId,
		int64 CharacterId
	);

	UFUNCTION(BlueprintCallable, Category = "DG|Session")
	void TravelToLastSession();

	UFUNCTION(BlueprintPure, Category = "DG|Session")
	FString GetLastSessionId() const;

	UPROPERTY(BlueprintAssignable, Category = "DG|Session")
	FDGSessionOnSessionCreated OnSessionCreated;

	UPROPERTY(BlueprintAssignable, Category = "DG|Session")
	FDGSessionOnRequestFailed OnSessionRequestFailed;

private:
	UPROPERTY()
	TObjectPtr<UDGBackendClient> BackendClient;

	/**
	 * 기본값은 Test Backend 기준.
	 * 실행 인자 -BackendUrl=... 이 들어오면 해당 값으로 덮어쓴다.
	 */
	UPROPERTY()
	FString BackendBaseUrl = TEXT("http://61.80.6.36:8081");

	UPROPERTY()
	FDGSessionConnectionInfo LastSessionConnectionInfo;

	void InitializeBackendBaseUrlFromCommandLine();

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

	bool ValidateRoomInput(
		const FString& RoomName,
		const FString& RoomPassword
	);

	bool ValidateAccountCharacterInput(
		int64 AccountId,
		int64 CharacterId
	);
};