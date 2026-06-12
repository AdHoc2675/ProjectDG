#pragma once

#include "CoreMinimal.h"
#include "UI/WidgetController/DGWidgetController.h"
#include "GameplayTagContainer.h"
#include "DGOverlayWidgetController.generated.h"

class UDG_AttributeSet;


// UI 로 값 변화를 방송할 다이내믹 멀티캐스트 델리게이트 선언
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnHealthChangedSignature, float, NewHealth);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnMaxHealthChangedSignature, float, NewMaxHealth);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnStaminaChangedSignature, float, NewStamina);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnMaxStaminaChangedSignature, float, NewMaxStamina);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnMentalChangedSignature, float, NewMental);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnMaxMentalChangedSignature, float, NewMaxMental);

// 채팅 관련 델리게이트
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnChatMessageReceivedSignature, const FString&, SenderName, const FString&, Message);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnChatFocusRequestedSignature);
// 적 정보 갱신 델리게이트
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnEnemyHealthChangedSignature, float, CurrentHealth, float, MaxHealth);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnEnemyGroggyChangedSignature, float, CurrentGroggy, float, MaxGroggy);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnEnemyTargetSetSignature, const FString&, EnemyName, int32, MaxHealthBars);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnEnemyTargetClearedSignature);

// 미니맵 마커 전방 선언
class UDGMinimapMarkerComponent;

// 미니맵 마커 업데이트용 델리게이트 (매개변수로 MarkerComponent 전달)
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnMinimapMarkerUpdatedSignature, UDGMinimapMarkerComponent*, Marker);


class ADG_PlayerState;
// 파티 멤버 변경 델리게이트 (가입/탈퇴 모두 사용, 매개변수로 변경된 멤버의 PlayerState 전달)
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPartyMemberChangedSignature, ADG_PlayerState*, MemberPS);


// UI로 보낼 스킬 정보 구조체
USTRUCT(BlueprintType)
struct FUIPlayerSkillInfo
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly) FGameplayTag SlotTag;
	UPROPERTY(BlueprintReadOnly) FGameplayTag CooldownTag;
	UPROPERTY(BlueprintReadOnly) UTexture2D* Icon = nullptr;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnSkillInfoSetSignature, const FUIPlayerSkillInfo&, SkillInfo);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnSkillIconUpdatedSignature, FGameplayTag, SlotTag, UTexture2D*, NewIcon);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnSkillCooldownChangedSignature, FGameplayTag, CooldownTag, float, TimeRemaining, float, Duration);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPlayerStatLevelChangedSignature, int32, NewLevel);


UCLASS()
class PROJECTDG_API UDGOverlayWidgetController : public UDGWidgetController
{
	GENERATED_BODY()

public:
	virtual void BroadcastInitialValues() override;
	virtual void BindCallbacksToDependencies() override;

	// --- 델리게이트 이벤트들 (Blueprint에서 바인딩 가능) --- //
	UPROPERTY(BlueprintAssignable, Category = "GAS|Attributes")
	FOnHealthChangedSignature OnHealthChanged;

	UPROPERTY(BlueprintAssignable, Category = "GAS|Attributes")
	FOnMaxHealthChangedSignature OnMaxHealthChanged;

	UPROPERTY(BlueprintAssignable, Category = "GAS|Attributes")
	FOnStaminaChangedSignature OnStaminaChanged;

	UPROPERTY(BlueprintAssignable, Category = "GAS|Attributes")
	FOnMaxStaminaChangedSignature OnMaxStaminaChanged;

	UPROPERTY(BlueprintAssignable, Category = "GAS|Attributes")
	FOnMentalChangedSignature OnMentalChanged;

	UPROPERTY(BlueprintAssignable, Category = "GAS|Attributes")
	FOnMaxMentalChangedSignature OnMaxMentalChanged;

	UPROPERTY(BlueprintAssignable, Category = "Player|Growth")
	FOnPlayerStatLevelChangedSignature OnPlayerLevelChanged;

	// --- 채팅 델리게이트 및 함수 ---
	UPROPERTY(BlueprintAssignable, Category = "DG|Chat")
	FOnChatMessageReceivedSignature OnChatMessageReceived;

	UPROPERTY(BlueprintAssignable, Category = "DG|Chat")
	FOnChatFocusRequestedSignature OnChatFocusRequested;

	UFUNCTION(BlueprintCallable, Category = "DG|Chat")
	void SendChatMessage(const FString& Message);

	void RequestChatFocus();

protected:
	UFUNCTION()
	void OnPlayerChatMessageReceivedCallback(const FString& SenderName, const FString& Message);

	// 캐싱된 프로젝트 전용 어트리뷰트 셋 (편의를 위함)
	UDG_AttributeSet* GetDGAttributeSet();


#pragma region Enemy Status Widget
public:
	// --- 적 관련 델리게이트 ---
	UPROPERTY(BlueprintAssignable, Category = "GAS|Enemy Attributes")
	FOnEnemyHealthChangedSignature OnEnemyHealthChanged;

	UPROPERTY(BlueprintAssignable, Category = "GAS|Enemy Attributes")
	FOnEnemyGroggyChangedSignature OnEnemyGroggyChanged;

	UPROPERTY(BlueprintAssignable, Category = "GAS|Enemy Attributes")
	FOnEnemyTargetSetSignature OnEnemyTargetSet;

	UPROPERTY(BlueprintAssignable, Category = "GAS|Enemy Attributes")
	FOnEnemyTargetClearedSignature OnEnemyTargetCleared;

protected:
	UPROPERTY()
	TObjectPtr<UAbilitySystemComponent> CurrentEnemyASC;
	UPROPERTY()
	TObjectPtr<UDG_AttributeSet> CurrentEnemyAS;

	// 이전 바인딩 해제를 위한 델리게이트 핸들
	FDelegateHandle EnemyHealthChangedDelegateHandle;
	FDelegateHandle EnemyMaxHealthChangedDelegateHandle;
	FDelegateHandle EnemyGroggyChangedDelegateHandle;
	FDelegateHandle EnemyMaxGroggyChangedDelegateHandle;

public:
	// 우선순위에 의해 결정된 최종 적 타겟팅 적용
	void SetEnemyTarget(class UAbilitySystemComponent* InEnemyASC, class UAttributeSet* InEnemyAS, const FString& EnemyName);

	// 명시적인 적 등록 함수
	UFUNCTION(BlueprintCallable, Category = "GAS|Enemy Attributes")
	void NotifyBossEncountered(AActor* BossActor);

	UFUNCTION(BlueprintCallable, Category = "GAS|Enemy Attributes")
	void NotifyTargetChanged(AActor* TargetActor);

	UFUNCTION(BlueprintCallable, Category = "GAS|Enemy Attributes")
	void NotifyEnemyDamaged(AActor* DamagedEnemy);

	UFUNCTION(BlueprintCallable, Category = "GAS|Enemy Attributes")
	void ClearCurrentEnemyTarget();

	// 상태 기반 적 업데이트 로직 (우선순위 결정)
	void RefreshEnemyTargetPriority();

	// 현재 추적 중인 대상들
	TWeakObjectPtr<AActor> CurrentBossEnemy;
	TWeakObjectPtr<AActor> CurrentLockedTarget;
	TWeakObjectPtr<AActor> LastDamagedEnemy;

	// 마지막 피해를 입힌 시간 기록 (타이머용)
	float LastDamageTime = 0.f;
#pragma endregion

#pragma region Minimap
public:
	// --- 미니맵 델리게이트 ---
	UPROPERTY(BlueprintAssignable, Category = "Minimap")
	FOnMinimapMarkerUpdatedSignature OnMarkerAdded;

	UPROPERTY(BlueprintAssignable, Category = "Minimap")
	FOnMinimapMarkerUpdatedSignature OnMarkerRemoved;

protected:
	// --- 미니맵 핸들러 ---
	UFUNCTION()
	void HandleMarkerRegistered(UDGMinimapMarkerComponent* Marker);

	UFUNCTION()
	void HandleMarkerUnregistered(UDGMinimapMarkerComponent* Marker);

#pragma endregion


#pragma region Party
public:
	UPROPERTY(BlueprintAssignable, Category = "Party")
	FOnPartyMemberChangedSignature OnPartyMemberJoined;

	UPROPERTY(BlueprintAssignable, Category = "Party")
	FOnPartyMemberChangedSignature OnPartyMemberLeft;

protected:
	// --- 파티 핸들러 ---
	UFUNCTION()
	void HandlePartyMemberJoined(ADG_PlayerState* NewMemberPS);
	UFUNCTION()
	void HandlePartyMemberLeft(ADG_PlayerState* LeavingMemberPS);

	// (추후 시스템 로직에서 호출해줄 헬퍼 함수도 만들 수 있습니다)
	// void AddPartyMember(ADG_PlayerState* MemberPS) { OnPartyMemberJoined.Broadcast(MemberPS); }
#pragma endregion


#pragma region Skill Info

public:
	// 스킬이 등록될 때 1번 불림
	UPROPERTY(BlueprintAssignable, Category = "GAS|Skills")
	FOnSkillInfoSetSignature OnSkillInfoSet;

	// 콤보 단계 등 아이콘이 바뀌어야 할 때 불림
	UPROPERTY(BlueprintAssignable, Category = "GAS|Skills")
	FOnSkillIconUpdatedSignature OnSkillIconUpdated;

	// 쿨타임이 돌기 시작할 때 불림 (TimeRemaining이 0이면 쿨타임 종료)
	UPROPERTY(BlueprintAssignable, Category = "GAS|Skills")
	FOnSkillCooldownChangedSignature OnSkillCooldownChanged;

protected:
	// 쿨타임 태그 이벤트 처리용
	void OnCooldownTagChanged(const FGameplayTag InCooldownTag, int32 NewCount);

	// 콤보 갱신 이벤트 처리용
	UFUNCTION()
	void OnSkillComboStepChanged(FGameplayTag SkillTag, int32 NewStepIndex);

	// 레벨 갱신 이벤트 처리용
	UFUNCTION()
	void OnPlayerLevelChangedCallback(int32 NewLevel);

#pragma region Skill Info
};