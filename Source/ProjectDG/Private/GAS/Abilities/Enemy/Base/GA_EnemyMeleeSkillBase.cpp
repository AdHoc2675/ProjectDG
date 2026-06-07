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
		Debug::Print(TEXT("[GA_EnemyMeleeSkillBase] Activate failed. SkillData is null."), FColor::Red);
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	if (!CurrentSkillData->Montage)
	{
		Debug::Print(TEXT("[GA_EnemyMeleeSkillBase] Activate failed. Montage is null."), FColor::Red);
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	if (!CanStartEnemyMeleeSkill())
	{
		Debug::Print(TEXT("[GA_EnemyMeleeSkillBase] Activate failed. CanStartEnemyMeleeSkill returned false."), FColor::Red);
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
	{
		Debug::Print(TEXT("[GA_EnemyMeleeSkillBase] Activate failed. CommitAbility failed."), FColor::Red);
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	RegisterEnemySkillHitCheckEvent();

	OnEnemyMeleeSkillCommitted();

	if (!PlaySkillMontageFromData(TEXT("EnemyMeleeSkillMontageTask")))
	{
		Debug::Print(TEXT("[GA_EnemyMeleeSkillBase] Activate failed. PlaySkillMontageFromData failed."), FColor::Red);
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
		Debug::Print(TEXT("[GA_EnemyMeleeSkillBase] HitCheck failed. SkillData is null."), FColor::Red);
		return;
	}

	Debug::Print(
		FString::Printf(
			TEXT("[GA_EnemyMeleeSkillBase] HitCheck Event Received. SkillData=%s HitShape=%d HitOrigin=%d EventMagnitude=%.2f"),
			*GetNameSafe(CurrentSkillData),
			static_cast<int32>(CurrentSkillData->HitShape),
			static_cast<int32>(CurrentSkillData->HitOrigin),
			Payload.EventMagnitude
		),
		FColor::Green
	);
}