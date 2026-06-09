#pragma once

#include "CoreMinimal.h"
#include "GAS/Abilities/Enemy/Base/GA_EnemyMeleeSkillBase.h"
#include "GA_EnemyDashMeleeSkillBase.generated.h"

class AActor;

/**
 * UGA_EnemyDashMeleeSkillBase
 *
 * 돌진형 근접 몬스터/보스 스킬 Base.
 *
 * 1차 기준:
 * - 실제 이동은 Montage RootMotion / 애니메이션 이동 기준
 * - 이 Base는 돌진 시작 전 타겟 방향 회전만 담당
 * - 타격 판정은 UGA_EnemySkillBase의 HitShape 기반 공통 판정 사용
 *
 * 추천 SkillData 설정:
 * - HitShape = PathBoxSweep 또는 ForwardBox
 * - HitOrigin = Self
 * - BoxExtent = 돌진 중 타격 박스
 * - ForwardOffset = 0 또는 전방 보정값
 */
UCLASS(Abstract)
class PROJECTDG_API UGA_EnemyDashMeleeSkillBase : public UGA_EnemyMeleeSkillBase
{
	GENERATED_BODY()

public:
	UGA_EnemyDashMeleeSkillBase();

protected:
	/** 돌진 시작 시 타겟 방향으로 회전할지 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "EnemyDash|Target")
	bool bFaceTargetOnDashStart = true;

	/** 기본 타겟 Blackboard Key */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "EnemyDash|Blackboard")
	FName TargetActorBlackboardKeyName = TEXT("TargetActor");

	/** 보조 타겟 Blackboard Key */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "EnemyDash|Blackboard")
	FName FallbackTargetActorBlackboardKeyName = TEXT("ObjectKey");

	/** Dash 스킬 시작 전 추가 검증 */
	virtual bool CanStartEnemyMeleeSkill() const override;

	/** Commit 성공 후 Montage 재생 직전 */
	virtual void OnEnemyMeleeSkillCommitted() override;

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

	/** Blackboard 기준 돌진 타겟 획득 */
	virtual AActor* ResolveDashTargetActor() const;

	/** 돌진 시작 전 타겟 방향 회전 */
	virtual bool FaceDashTarget();
};