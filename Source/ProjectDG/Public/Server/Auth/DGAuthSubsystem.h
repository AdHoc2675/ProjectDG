#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Server/Backend/DGBackendTypes.h"
#include "DGAuthSubsystem.generated.h"

class UDGBackendClient;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FDGOnAuthSucceeded, const FDGAuthResult&, Result);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FDGOnAuthFailed, const FString&, ErrorMessage);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FDGOnCharacterListLoaded, const FDGCharacterListResult&, Result);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FDGOnCharacterSelected, const FDGCharacterSummary&, Character);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FDGOnAuthCharacterCreated, const FDGCreateCharacterResult&, Result);

/**
 * 계정 / 로그인 / 캐릭터 선택 담당 Subsystem
 *
 * 역할:
 * - 회원가입 요청
 * - 로그인 요청
 * - 로그인 성공 후 AccountId 저장
 * - 계정 캐릭터 목록 조회
 * - 선택한 CharacterId 저장
 *
 * 세션 생성/참가는 DGSessionSubsystem에서 처리한다.
 */
UCLASS(BlueprintType)
class PROJECTDG_API UDGAuthSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	UFUNCTION(BlueprintCallable, Category = "DG|Auth")
	void RegisterAccount(
		const FString& LoginId,
		const FString& Password,
		const FString& DisplayName
	);

	UFUNCTION(BlueprintCallable, Category = "DG|Auth")
	void Login(
		const FString& LoginId,
		const FString& Password
	);

	UFUNCTION(BlueprintCallable, Category = "DG|Auth")
	void RequestCharacterList();
	
	UFUNCTION(BlueprintCallable, Category = "DG|Auth")
	void CreateCharacter(
		int32 SlotIndex,
		const FString& CharacterName,
		const FString& ClassTag
	);

	UFUNCTION(BlueprintCallable, Category = "DG|Auth")
	bool SelectCharacterBySlotIndex(int32 SlotIndex);

	UFUNCTION(BlueprintCallable, Category = "DG|Auth")
	bool SelectCharacterById(int64 CharacterId);

	UFUNCTION(BlueprintPure, Category = "DG|Auth")
	bool IsLoggedIn() const;

	UFUNCTION(BlueprintPure, Category = "DG|Auth")
	bool HasSelectedCharacter() const;

	UFUNCTION(BlueprintPure, Category = "DG|Auth")
	int64 GetCurrentAccountId() const;

	UFUNCTION(BlueprintPure, Category = "DG|Auth")
	int64 GetSelectedCharacterId() const;

	UFUNCTION(BlueprintPure, Category = "DG|Auth")
	FString GetCurrentLoginId() const;

	UFUNCTION(BlueprintPure, Category = "DG|Auth")
	FString GetCurrentDisplayName() const;

	UFUNCTION(BlueprintPure, Category = "DG|Auth")
	const TArray<FDGCharacterSummary>& GetCachedCharacters() const;

	UPROPERTY(BlueprintAssignable, Category = "DG|Auth")
	FDGOnAuthSucceeded OnRegisterSucceeded;

	UPROPERTY(BlueprintAssignable, Category = "DG|Auth")
	FDGOnAuthSucceeded OnLoginSucceeded;

	UPROPERTY(BlueprintAssignable, Category = "DG|Auth")
	FDGOnAuthFailed OnAuthFailed;

	UPROPERTY(BlueprintAssignable, Category = "DG|Auth")
	FDGOnCharacterListLoaded OnCharacterListLoaded;

	UPROPERTY(BlueprintAssignable, Category = "DG|Auth")
	FDGOnAuthFailed OnCharacterListFailed;

	UPROPERTY(BlueprintAssignable, Category = "DG|Auth")
	FDGOnAuthCharacterCreated OnCharacterCreated;

	UPROPERTY(BlueprintAssignable, Category = "DG|Auth")
	FDGOnAuthFailed OnCharacterCreateFailed;
	
	UPROPERTY(BlueprintAssignable, Category = "DG|Auth")
	FDGOnCharacterSelected OnCharacterSelected;

private:
	UPROPERTY()
	TObjectPtr<UDGBackendClient> BackendClient;

	/**
	 * Test Backend 기준.
	 * Stable 클라에서는 8080으로 변경 필요.
	 */
	UPROPERTY()
	FString BackendBaseUrl = TEXT("http://61.80.6.36:8081");

	UPROPERTY()
	int64 CurrentAccountId = 0;

	UPROPERTY()
	int64 SelectedCharacterId = 0;

	UPROPERTY()
	FString CurrentLoginId;

	UPROPERTY()
	FString CurrentDisplayName;

	UPROPERTY()
	TArray<FDGCharacterSummary> CachedCharacters;

	void HandleRegisterCompleted(
		bool bSuccess,
		const FDGAuthResult& Result
	);

	void HandleLoginCompleted(
		bool bSuccess,
		const FDGAuthResult& Result
	);

	void HandleCharacterListCompleted(
		bool bSuccess,
		const FDGCharacterListResult& Result
	);
	
	void HandleCreateCharacterCompleted(
	bool bSuccess,
	const FDGCreateCharacterResult& Result
);

	bool ValidateLoginInput(
		const FString& LoginId,
		const FString& Password
	) const;
};