#pragma once

#include "CoreMinimal.h"
#include "GAS/Abilities/Enemy/Base/GA_EnemySkillBase.h"
#include "GA_EnemyMeleeSkillBase.generated.h"

/**
 * UGA_EnemyMeleeSkillBase
 *
 * 근접형 몬스터/보스 스킬 Base.
 *
 * 담당:
 * - EnemySkillData 기반 Melee 스킬 시작 흐름
 * - SkillData.Montage 재생
 * - AN_EnemySkillHit → Event.Attack.HitCheck 수신 시 EnemySkillBase 공통 판정 호출
 *
 * 주의:
 * - Montage 재생/종료 공통 처리는 UGA_EnemySkillBase가 담당한다.
 * - HitShape 기반 판정/디버그/데미지 적용도 UGA_EnemySkillBase가 담당한다.
 * - 이 클래스는 "근접 스킬 실행 흐름"과 Melee 전용 Hook만 담당한다.
 */
UCLASS(Abstract)
class PROJECTDG_API UGA_EnemyMeleeSkillBase : public UGA_EnemySkillBase
{
	GENERATED_BODY()

public:
	UGA_EnemyMeleeSkillBase();

	virtual void ActivateAbility(
		const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo,
		const FGameplayEventData* TriggerEventData
	) override;

protected:
	/** Melee 스킬 시작 전 추가 검증 Hook */
	virtual bool CanStartEnemyMeleeSkill() const;

	/** Commit 성공 후 Montage 재생 직전 Hook */
	virtual void OnEnemyMeleeSkillCommitted();

	/** Montage 시작 후 Hook */
	virtual void OnSkillMontageStarted() override;

	/** Montage 정상 완료 Hook */
	virtual void OnSkillMontageCompleted() override;

	/** Montage 중단 Hook */
	virtual void OnSkillMontageInterrupted() override;

	/** Montage 취소 Hook */
	virtual void OnSkillMontageCancelled() override;

	/** Ability 종료 직전 Hook */
	virtual void OnEnemySkillFinished(bool bWasCancelled) override;

	/** AN_EnemySkillHit 이벤트 수신 후 EnemySkillBase 공통 판정 처리 */
	virtual void HandleEnemySkillHitCheckEvent(const FGameplayEventData& Payload) override;
};