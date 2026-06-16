// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Character/Enemy/Boss/BossCharacterBase.h"
#include "BossKashpaD.generated.h"

class UBossSkillSetData;
class UEnemySkillData;
struct FBossPhaseData;

/**
 * 새 카샤파 보스 전용 클래스.
 *
 * 기존 카샤파 작업물은 보존하고,
 * BP_Kashapa_D에서 이 클래스를 Parent로 사용한다.
 *
 * 역할:
 * - BossCharacterClassData.PhaseDataList 기반 카샤파 전용 페이즈 적용
 * - Phase별 SkeletalMesh / AnimClass / Materials 교체
 * - 부모 ABossCharacterBase의 CurrentPhaseSkillSetData 기반 보스 스킬 선택
 */
UCLASS()
class PROJECTDG_API ABossKashapaD : public ABossCharacterBase
{
	GENERATED_BODY()

public:
	ABossKashapaD();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UFUNCTION(BlueprintCallable, Category = "Kashapa|Phase")
	bool SetKashapaPhase(int32 NewPhaseIndex);

	UFUNCTION(BlueprintCallable, Category = "Kashapa|Phase")
	int32 GetCurrentPhaseIndex() const { return CurrentPhaseIndex; }

	UFUNCTION(BlueprintCallable, Category = "Kashapa|Skill")
	UBossSkillSetData* GetCurrentSkillSetData() const { return CurrentPhaseSkillSetData; }

	virtual const TArray<TObjectPtr<UEnemySkillData>>& GetAttackSkillDataList() const override;
	virtual UEnemySkillData* GetRandomAttackSkillData() const override;

	// PhaseTransition GA의 AN_BossPhaseApply 수신 시점에서만 호출된다.
	// 여기서 2페 SkeletalMesh / AnimClass / Material / SkillSet이 실제 적용된다.
	virtual bool ApplyPendingPhaseChangeFromNotify(int32 ExpectedPhaseIndex = INDEX_NONE) override;
protected:
	virtual void BeginPlay() override;
	virtual void PossessedBy(AController* NewController) override;
	virtual void UpdateHealthPhaseTags(float HealthRatio) override;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Kashapa|Phase")
	int32 InitialPhaseIndex = 1;

	UFUNCTION()
	void OnRep_CurrentPhaseIndex();

	const FBossPhaseData* FindKashapaPhaseDataByIndex(int32 PhaseIndex) const;

	bool ApplyPhaseDataByIndex(int32 PhaseIndex);
	void ApplyPhaseVisual(const FBossPhaseData& PhaseData);
	void ApplyPhaseTags(const FBossPhaseData& PhaseData);
	void GrantSkillSetAbilities(UBossSkillSetData* SkillSetData);

	bool HasGrantedSkillDataAbility(UEnemySkillData* SkillData);
};