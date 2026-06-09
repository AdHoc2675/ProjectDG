#include "GAS/Abilities/Enemy/Base/GA_EnemyAOESkillBase.h"

#include "Character/Enemy/Data/EnemySkillData.h"

UGA_EnemyAOESkillBase::UGA_EnemyAOESkillBase()
{
}

void UGA_EnemyAOESkillBase::ActivateAbility(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData
)
{
	// InstancedPerActor 재사용 상황 대비
	bIsFinishingEnemySkill = false;

	UEnemySkillData* CurrentSkillData = GetEnemySkillData();
	if (!CurrentSkillData)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	if (!CurrentSkillData->Montage)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	if (!CanStartEnemyAOESkill())
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	RegisterEnemySkillHitCheckEvent();

	OnEnemyAOESkillCommitted();

	if (!PlaySkillMontageFromData(TEXT("EnemyAOESkillMontageTask")))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}
}

bool UGA_EnemyAOESkillBase::CanStartEnemyAOESkill() const
{
	return true;
}

void UGA_EnemyAOESkillBase::OnEnemyAOESkillCommitted()
{
}

void UGA_EnemyAOESkillBase::HandleEnemySkillHitCheckEvent(const FGameplayEventData& Payload)
{
	// EnemySkillBase에서 HitShape 기반 판정 / 디버그 / 데미지 적용 처리
	Super::HandleEnemySkillHitCheckEvent(Payload);
}

void UGA_EnemyAOESkillBase::OnSkillMontageStarted()
{
	Super::OnSkillMontageStarted();
}

void UGA_EnemyAOESkillBase::OnSkillMontageCompleted()
{
	Super::OnSkillMontageCompleted();
}

void UGA_EnemyAOESkillBase::OnSkillMontageInterrupted()
{
	Super::OnSkillMontageInterrupted();
}

void UGA_EnemyAOESkillBase::OnSkillMontageCancelled()
{
	Super::OnSkillMontageCancelled();
}

void UGA_EnemyAOESkillBase::OnEnemySkillFinished(bool bWasCancelled)
{
	Super::OnEnemySkillFinished(bWasCancelled);
}