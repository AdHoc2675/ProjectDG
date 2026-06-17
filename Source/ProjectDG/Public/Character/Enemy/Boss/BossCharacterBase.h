// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemComponent.h"
#include "GameplayTagContainer.h"
#include "Character/Enemy/EnemyCharacterBase.h"
#include "BossCharacterBase.generated.h"

struct FOnAttributeChangeData;
struct FBossPhaseData;

class UBossCharacterClassData;
class UEnemySkillData;
class UBossSkillSetData;
class UDG_BossAttributeSet;
class UDG_EnemyAttributeSet;

/**
 * 
 */
UCLASS()
class PROJECTDG_API ABossCharacterBase : public AEnemyCharacterBase
{
	GENERATED_BODY()

public:
	ABossCharacterBase();

protected:
	// 보스 전용 데이터 에셋 (초기 스탯/GE/BT/컷신 등)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "BossCharacterBase|Data")
	TObjectPtr<UBossCharacterClassData> BossClassData = nullptr;

	// 보스 식별용 태그
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "BossCharacterBase|Team")
	FGameplayTag BossTag;

	// 보스 전용 AttributeSet
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "BossCharacterBase|ASC")
	TObjectPtr<UDG_BossAttributeSet> BossAttributeSet = nullptr;

	// 현재 적용된 페이즈 인덱스
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "BossCharacterBase|Phase")
	int32 CurrentPhaseIndex = INDEX_NONE;

	// 현재 페이즈에서 실제 BT가 선택할 SkillSet
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "BossCharacterBase|Phase")
	TObjectPtr<UBossSkillSetData> CurrentPhaseSkillSetData = nullptr;

	// GA 실행 중 페이즈 전환이 발생했을 때 즉시 적용하지 않고 대기시키는 플래그
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "BossCharacterBase|Phase")
	bool bHasPendingPhaseChange = false;

	// 대기 중인 페이즈 인덱스
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "BossCharacterBase|Phase")
	int32 PendingPhaseIndex = INDEX_NONE;

	// 컷신 기준 위치.
	// 보스가 전투 중 이동했더라도 PhaseTransition 컷신 시작 시 이 위치로 되돌린다.
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "BossCharacterBase|Cutscene")
	FTransform InitialBossTransform = FTransform::Identity;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "BossCharacterBase|Cutscene")
	bool bHasInitialBossTransform = false;

	// 보스 전용 스탯 GE 적용
	void ApplyBossSpecialEffects();

	// 보스 태그 초기화
	void InitializeBossTagFromClassData();

	// 보스는 DataAsset이 있으면 그 값을 우선 적용
	virtual void ApplyDefaultEffects() override;

	// DataAsset / Phase SkillSet 기반 Ability 부여
	virtual void GrantDefaultAbilities() override;

	// 소환 직후 보스 전용 스탯 적용
	virtual void PossessedBy(AController* NewController) override;

	// BT 시작 후 Blackboard 초기값 설정
	virtual void BeginPlay() override;

	// Health Attribute 변경 콜백
	void OnHealthChanged(const FOnAttributeChangeData& Data);

	// Groggy Attribute 변경 콜백
	void OnGroggyGaugeChanged(const FOnAttributeChangeData& Data);

	// Health 비율에 따라 Phase 전환 요청
	virtual void UpdateHealthPhaseTags(float HealthRatio);

	// 보스 사망 처리
	virtual void HandleDeath() override;

public:
	UFUNCTION(BlueprintCallable, Category = "BossCharacterBase|Data")
	UBossCharacterClassData* GetBossClassData() const
	{
		return BossClassData;
	}

	UFUNCTION(BlueprintCallable, Category = "BossCharacterBase|ASC")
	UDG_BossAttributeSet* GetBossAttributeSet() const
	{
		return BossAttributeSet;
	}

	UFUNCTION(BlueprintCallable, Category = "BossCharacterBase|Team")
	FGameplayTag GetBossTag() const
	{
		return BossTag;
	}
	
	virtual const TArray<TObjectPtr<UEnemySkillData>>& GetAttackSkillDataList() const;

	UFUNCTION(BlueprintCallable, Category = "BossCharacterBase|Skill")
	virtual UEnemySkillData* GetRandomAttackSkillData() const;

	UFUNCTION(BlueprintCallable, Category = "BossCharacterBase|Phase")
	bool HasPendingPhaseChange() const
	{
		return bHasPendingPhaseChange && PendingPhaseIndex != INDEX_NONE;
	}

	UFUNCTION(BlueprintCallable, Category = "BossCharacterBase|Phase")
	int32 GetPendingPhaseIndex() const
	{
		return PendingPhaseIndex;
	}

	// PhaseTransition GA의 AN_BossPhaseApply 수신 시점에서만 호출한다.
	// ActivateAbility 시점에서 호출하면 안 됨.
	virtual bool ApplyPendingPhaseChangeFromNotify(int32 ExpectedPhaseIndex = INDEX_NONE);

	// PhaseTransition 컷신 시작 시 보스를 최초 배치 위치로 되돌린다.
	// 카메라 연출은 이 위치 기준으로 제작한다.
	UFUNCTION(BlueprintCallable, Category = "BossCharacterBase|Cutscene")
	virtual bool MoveToInitialBossTransformForCutscene();

	UFUNCTION(BlueprintCallable, Category = "BossCharacterBase|Cutscene")
	FTransform GetInitialBossTransform() const
	{
		return InitialBossTransform;
	}

	virtual FGameplayTag GetAttributeSourceTag() const override;

private:
	bool bBossSpecialEffectsApplied = false;
	bool bBossDataEffectsApplied = false;
	bool bBossHealthPhaseDelegateBound = false;
	bool bGroggyDelegateBound = false;

private:
	// 현재 DA 기반 보스 스킬 GA가 실행 중인지 확인
	bool HasActiveEnemySkillAbility() const;

	// PhaseIndex 기준 PhaseData 검색
	const FBossPhaseData* FindPhaseDataByIndex(int32 PhaseIndex) const;

	// 페이즈 전환 요청.
	// 컷신 구조에서는 즉시 적용하지 않고 Pending만 설정한다.
	void RequestPhaseChange(const FBossPhaseData& PhaseData);

	// 실제 페이즈 적용. PhaseTag / CurrentPhase / SkillSet 변경
	// 카샤파 전용 외형 변경은 ABossKashapaD::ApplyPhaseDataByIndex 쪽에서 처리한다.
	void ApplyPhaseChange(const FBossPhaseData& PhaseData);
};