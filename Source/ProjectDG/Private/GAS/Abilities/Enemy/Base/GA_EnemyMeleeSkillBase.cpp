#include "GAS/Abilities/Enemy/Base/GA_EnemyMeleeSkillBase.h"

#include "Character/Enemy/Data/EnemySkillData.h"
#include "Core/DG_Debug.h"

UGA_EnemyMeleeSkillBase::UGA_EnemyMeleeSkillBase()
{
}

void UGA_EnemyMeleeSkillBase::ActivateAbility(
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

	if (!CanStartEnemyMeleeSkill())
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

	OnEnemyMeleeSkillCommitted();

	if (!PlaySkillMontageFromData(TEXT("EnemyMeleeSkillMontageTask")))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}
}

bool UGA_EnemyMeleeSkillBase::CanStartEnemyMeleeSkill() const
{
	return true;
}

void UGA_EnemyMeleeSkillBase::OnEnemyMeleeSkillCommitted()
{
}

void UGA_EnemyMeleeSkillBase::OnSkillMontageStarted()
{
	Super::OnSkillMontageStarted();
}

void UGA_EnemyMeleeSkillBase::OnSkillMontageCompleted()
{
	Super::OnSkillMontageCompleted();
}

void UGA_EnemyMeleeSkillBase::OnSkillMontageInterrupted()
{
	Super::OnSkillMontageInterrupted();
}

void UGA_EnemyMeleeSkillBase::OnSkillMontageCancelled()
{
	Super::OnSkillMontageCancelled();
}

void UGA_EnemyMeleeSkillBase::OnEnemySkillFinished(bool bWasCancelled)
{
	Super::OnEnemySkillFinished(bWasCancelled);
}

void UGA_EnemyMeleeSkillBase::HandleEnemySkillHitCheckEvent(const FGameplayEventData& Payload)
{
	// EnemySkillBase에서 HitShape 기반 판정/디버그/데미지 적용 처리
	Super::HandleEnemySkillHitCheckEvent(Payload);

	UEnemySkillData* CurrentSkillData = GetEnemySkillData();
	if (!CurrentSkillData)
	{
		return;
	}

	
}