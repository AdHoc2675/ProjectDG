#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Server/Backend/DGBackendTypes.h"
#include "DGSessionSubsystem.generated.h"

class UDGBackendClient;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FDGSessionOnLoginSucceeded, const FDGAuthResult&, Result);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FDGSessionOnCharacterListLoaded, const FDGCharacterListResult&, Result);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FDGSessionOnCharacterCreated, const FDGCreateCharacterResult&, Result);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FDGSessionOnCharacterSelected, int64, CharacterId, int32, SlotIndex);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FDGSessionOnSessionCreated, const FString&, SessionId);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FDGSessionOnSessionRequestFailed, const FString&, ErrorMessage);

/**
 * 게임 세션 흐름 담당 Subsystem
 *
 * 역할:
 * - 로그인
 * - 캐릭터 슬롯 조회
 * - 캐릭터 생성
 * - 캐릭터 선택
 * - 방 생성 요청
 * - 방 참가 요청
 * - Backend 응답으로 받은 SessionId / JoinToken을 이용해 Dedicated Server 접속
 */
UCLASS(BlueprintType)
class PROJECTDG_API UDGSessionSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	/**
	 * LoginId / Password로 로그인.
	 * 성공 시 CurrentAccountId 저장 후 자동으로 LoadMyCharacters 호출.
	 */
	UFUNCTION(BlueprintCallable, Category = "DG|Session|Auth")
	void Login(
		const FString& LoginId,
		const FString& Password
	);

	/**
	 * 현재 로그인된 AccountId 기준 캐릭터 3슬롯 조회.
	 */
	UFUNCTION(BlueprintCallable, Category = "DG|Session|Character")
	void LoadMyCharacters();

	/**
	 * 빈 슬롯에 캐릭터 생성.
	 */
	UFUNCTION(BlueprintCallable, Category = "DG|Session|Character")
	void CreateCharacter(
		int32 SlotIndex,
		const FString& CharacterName,
		const FString& ClassTag
	);

	/**
	 * CharacterId 기준 캐릭터 선택.
	 */
	UFUNCTION(BlueprintCallable, Category = "DG|Session|Character")
	void SelectCharacterById(int64 CharacterId);

	/**
	 * SlotIndex 기준 캐릭터 선택.
	 */
	UFUNCTION(BlueprintCallable, Category = "DG|Session|Character")
	void SelectCharacterBySlotIndex(int32 SlotIndex);

	UFUNCTION(BlueprintPure, Category = "DG|Session|Auth")
	bool IsLoggedIn() const;

	UFUNCTION(BlueprintPure, Category = "DG|Session|Character")
	bool HasSelectedCharacter() const;

	UFUNCTION(BlueprintPure, Category = "DG|Session|Auth")
	int64 GetCurrentAccountId() const;

	UFUNCTION(BlueprintPure, Category = "DG|Session|Character")
	int64 GetSelectedCharacterId() const;

	UFUNCTION(BlueprintPure, Category = "DG|Session|Auth")
	FString GetCurrentDisplayName() const;

	UFUNCTION(BlueprintPure, Category = "DG|Session|Character")
	TArray<FDGCharacterSummary> GetCachedCharacterSlots() const;

	/**
	 * 기존 테스트용 함수 유지.
	 * AccountId / CharacterId를 직접 넘기는 방식.
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
	 * 기존 테스트용 함수 유지.
	 * AccountId / CharacterId를 직접 넘기는 방식.
	 */
	UFUNCTION(BlueprintCallable, Category = "DG|Session")
	void JoinRoomAndTravel(
		const FString& RoomName,
		const FString& RoomPassword,
		int64 AccountId = 2,
		int64 CharacterId = 2
	);

	/**
	 * 새 UI용 방 생성.
	 * 로그인/캐릭터 선택으로 저장된 AccountId / CharacterId를 사용한다.
	 */
	UFUNCTION(BlueprintCallable, Category = "DG|Session")
	void CreateRoomAndTravelWithSelectedCharacter(
		const FString& RoomName,
		const FString& RoomPassword,
		const FString& RegionId = TEXT("Region_Test")
	);

	/**
	 * 새 UI용 방 참가.
	 * 로그인/캐릭터 선택으로 저장된 AccountId / CharacterId를 사용한다.
	 */
	UFUNCTION(BlueprintCallable, Category = "DG|Session")
	void JoinRoomAndTravelWithSelectedCharacter(
		const FString& RoomName,
		const FString& RoomPassword
	);

	/**
	 * 마지막으로 생성/합류한 세션 접속 정보로 Dedicated Server 접속.
	 * 디버그용으로 유지.
	 */
	UFUNCTION(BlueprintCallable, Category = "DG|Session")
	void TravelToLastSession();

	/**
	 * 마지막으로 받은 SessionId 반환.
	 * 디버그/표시용.
	 */
	UFUNCTION(BlueprintPure, Category = "DG|Session")
	FString GetLastSessionId() const;

	UPROPERTY(BlueprintAssignable, Category = "DG|Session|Auth")
	FDGSessionOnLoginSucceeded OnLoginSucceeded;

	UPROPERTY(BlueprintAssignable, Category = "DG|Session|Character")
	FDGSessionOnCharacterListLoaded OnCharacterListLoaded;

	UPROPERTY(BlueprintAssignable, Category = "DG|Session|Character")
	FDGSessionOnCharacterCreated OnCharacterCreated;

	UPROPERTY(BlueprintAssignable, Category = "DG|Session|Character")
	FDGSessionOnCharacterSelected OnCharacterSelected;

	UPROPERTY(BlueprintAssignable, Category = "DG|Session")
	FDGSessionOnSessionCreated OnSessionCreated;

	UPROPERTY(BlueprintAssignable, Category = "DG|Session")
	FDGSessionOnSessionRequestFailed OnSessionRequestFailed;

private:
	UPROPERTY()
	TObjectPtr<UDGBackendClient> BackendClient;

	/**
	 * 기본값:
	 * Test Backend = http://61.80.6.36:8081
	 *
	 * 실행 인자:
	 * -BackendUrl=http://61.80.6.36:8081
	 */
	UPROPERTY()
	FString BackendBaseUrl = TEXT("http://61.80.6.36:8081");

	UPROPERTY()
	FDGSessionConnectionInfo LastSessionConnectionInfo;

	UPROPERTY()
	int64 CurrentAccountId = 0;

	UPROPERTY()
	int64 SelectedCharacterId = 0;

	UPROPERTY()
	FString CurrentLoginId;

	UPROPERTY()
	FString CurrentDisplayName;

	UPROPERTY()
	TArray<FDGCharacterSummary> CachedCharacterSlots;

	void InitializeBackendBaseUrlFromCommandLine();

	void HandleLoginCompleted(
		bool bSuccess,
		const FDGAuthResult& Result
	);

	void HandleLoadCharactersCompleted(
		bool bSuccess,
		const FDGCharacterListResult& Result
	);

	void HandleCreateCharacterCompleted(
		bool bSuccess,
		const FDGCreateCharacterResult& Result
	);

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

	bool ValidateLoggedIn();

	bool ValidateSelectedCharacter();
};