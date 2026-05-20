#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Server/Backend/DGBackendTypes.h"
#include "DGSessionSubsystem.generated.h"

class UDGBackendClient;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FDGOnSessionCreated, const FString&, SessionId);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FDGOnSessionRequestFailed, const FString&, ErrorMessage);

/**
 * 게임 세션 흐름 담당 Subsystem
 *
 * 역할:
 * - 방 생성 요청
 * - 방 참가 요청
 * - Backend 응답으로 받은 SessionId / JoinToken을 이용해 Dedicated Server 접속
 *
 * 유저 입력:
 * - RoomName
 * - RoomPassword
 *
 * 내부 처리:
 * - Backend가 SessionId / JoinToken 발급
 * - Dedicated Server는 기존처럼 SessionId / JoinToken만 검증
 */
UCLASS(BlueprintType)
class PROJECTDG_API UDGSessionSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	/**
	 * 방 생성 후 즉시 Dedicated Server 접속
	 *
	 * Blueprint UI에서는:
	 * - RoomName 입력값
	 * - RoomPassword 입력값
	 * 을 넘기면 된다.
	 */
	UFUNCTION(BlueprintCallable, Category = "DG|Session")
	void CreateRoomAndTravel(
		const FString& RoomName,
		const FString& RoomPassword,
		int64 AccountId = 1,
		int64 CharacterId = 1,
		const FString& RegionId = TEXT("Region_Test")
	);

	/**
	 * 기존 방에 참가 후 즉시 Dedicated Server 접속
	 *
	 * Blueprint UI에서는:
	 * - RoomName 입력값
	 * - RoomPassword 입력값
	 * 을 넘기면 된다.
	 */
	UFUNCTION(BlueprintCallable, Category = "DG|Session")
	void JoinRoomAndTravel(
		const FString& RoomName,
		const FString& RoomPassword,
		int64 AccountId = 2,
		int64 CharacterId = 2
	);

	/**
	 * 마지막으로 생성/합류한 세션 접속 정보로 Dedicated Server 접속
	 * 디버그용으로 유지.
	 */
	UFUNCTION(BlueprintCallable, Category = "DG|Session")
	void TravelToLastSession();

	/**
	 * 마지막으로 받은 SessionId 반환
	 * 디버그/표시용.
	 */
	UFUNCTION(BlueprintPure, Category = "DG|Session")
	FString GetLastSessionId() const;

	UPROPERTY(BlueprintAssignable, Category = "DG|Session")
	FDGOnSessionCreated OnSessionCreated;

	UPROPERTY(BlueprintAssignable, Category = "DG|Session")
	FDGOnSessionRequestFailed OnSessionRequestFailed;

private:
	UPROPERTY()
	TObjectPtr<UDGBackendClient> BackendClient;

	/**
	 * 참가 PC / 서버 PC 모두 Backend는 공인 IP로 접근한다.
	 *
	 * 기본값:
	 * Stable Backend = http://61.80.6.36:8080
	 *
	 * 실행 인자:
	 * -BackendUrl=http://61.80.6.36:8081
	 */
	UPROPERTY()
	FString BackendBaseUrl = TEXT("http://61.80.6.36:8080");

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
	) const;
};