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
 * - BossCharacterClassData.PhaseDataList 기반 페이즈 적용
 * - Phase별 SkeletalMesh / AnimClass / Materials 교체
 * - 현재 SkillSetData 캐시
 * - 현재 Phase SkillSetData 기반 보스 스킬 선택/부여
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
	UBossSkillSetData* GetCurrentSkillSetData() const { return CurrentSkillSetData; }

	virtual const TArray<TObjectPtr<UEnemySkillData>>& GetAttackSkillDataList() const override;
	virtual UEnemySkillData* GetRandomAttackSkillData() const override;

protected:
	virtual void BeginPlay() override;
	virtual void PossessedBy(AController* NewController) override;
	virtual void UpdateHealthPhaseTags(float HealthRatio) override;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Kashapa|Phase")
	int32 InitialPhaseIndex = 1;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, ReplicatedUsing = OnRep_CurrentPhaseIndex, Category = "Kashapa|Phase")
	int32 CurrentPhaseIndex = 0;

	UPROPERTY(Transient, BlueprintReadOnly, Category = "Kashapa|Skill")
	TObjectPtr<UBossSkillSetData> CurrentSkillSetData = nullptr;

	UFUNCTION()
	void OnRep_CurrentPhaseIndex();

	const FBossPhaseData* FindPhaseDataByIndex(int32 PhaseIndex) const;

	bool ApplyPhaseDataByIndex(int32 PhaseIndex);
	void ApplyPhaseVisual(const FBossPhaseData& PhaseData);
	void ApplyPhaseTags(const FBossPhaseData& PhaseData);
	void GrantSkillSetAbilities(UBossSkillSetData* SkillSetData);

	bool HasGrantedSkillDataAbility(UEnemySkillData* SkillData);
};