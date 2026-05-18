// Fill out your copyright notice in the Description page of Project Settings.

#include "GAS/Abilities/Enemy/Boss/GA_Boss_Attack.h"

#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "Core/DG_GameplayTags.h"
#include "Core/DG_Debug.h"

UGA_Boss_Attack::UGA_Boss_Attack()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::ServerOnly;

	AbilityTags.AddTag(DGGameplayTags::Skill_Boss_Kashapa_Attack);
}

void UGA_Boss_Attack::ActivateAbility(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* OwnerInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData
)
{
	if (!CommitAbility(Handle, OwnerInfo, ActivationInfo))
	{
		EndAbility(Handle, OwnerInfo, ActivationInfo, true, true);
		return;
	}

	if (!AttackMontage)
	{
		Debug::Print(TEXT("[GA_Boss_Attack] AttackMontage is null."), FColor::Red);
		EndAbility(Handle, OwnerInfo, ActivationInfo, true, true);
		return;
	}

	AttackHitTask = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(
		this,
		DGGameplayTags::Event_Attack_Hit.GetTag(),
		nullptr,
		false,
		true
	);

	if (AttackHitTask)
	{
		AttackHitTask->EventReceived.AddDynamic(this, &UGA_Boss_Attack::OnAttackHit);
		AttackHitTask->ReadyForActivation();
	}

	MontageTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(
		this,
		TEXT("BossAttackMontageTask"),
		AttackMontage,
		PlayRate
	);

	if (!MontageTask)
	{
		EndAbility(Handle, OwnerInfo, ActivationInfo, true, true);
		return;
	}

	MontageTask->OnCompleted.AddDynamic(this, &UGA_Boss_Attack::OnMontageCompleted);
	MontageTask->OnInterrupted.AddDynamic(this, &UGA_Boss_Attack::OnMontageInterrupted);
	MontageTask->OnCancelled.AddDynamic(this, &UGA_Boss_Attack::OnMontageCancelled);

	MontageTask->ReadyForActivation();
}

void UGA_Boss_Attack::EndAbility(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	bool bReplicateEndAbility,
	bool bWasCancelled
)
{
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

void UGA_Boss_Attack::OnAttackHit(FGameplayEventData Payload)
{
	AActor* AvatarActor = GetAvatarActorFromActorInfo();
	if (!AvatarActor || !AvatarActor->HasAuthority())
	{
		return;
	}

	AActor* TargetActor = const_cast<AActor*>(Payload.Target.Get());
	if (!TargetActor || TargetActor == AvatarActor)
	{
		return;
	}

	if (!DamageEffectClass)
	{
		Debug::Print(TEXT("[GA_Boss_Attack] DamageEffectClass is null."), FColor::Red);
		return;
	}

	UAbilitySystemComponent* SourceASC = GetAbilitySystemComponentFromActorInfo();
	UAbilitySystemComponent* TargetASC =
		UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(TargetActor);

	if (!SourceASC || !TargetASC)
	{
		return;
	}

	const FGameplayEffectSpecHandle SpecHandle = MakeOutgoingGameplayEffectSpec(DamageEffectClass, GetAbilityLevel());
	if (!SpecHandle.IsValid())
	{
		return;
	}

	UAbilitySystemBlueprintLibrary::AssignTagSetByCallerMagnitude(
		SpecHandle,
		DGGameplayTags::Data_Damage.GetTag(),
		AttackDamage
	);

	SourceASC->ApplyGameplayEffectSpecToTarget(*SpecHandle.Data.Get(), TargetASC);

	Debug::Print(FString::Printf(
		TEXT("[GA_Boss_Attack] Hit %s for %.1f damage."),
		*GetNameSafe(TargetActor),
		AttackDamage
	), FColor::Orange);
}

void UGA_Boss_Attack::OnMontageCompleted()
{
	K2_EndAbility();
}

void UGA_Boss_Attack::OnMontageInterrupted()
{
	K2_EndAbility();
}

void UGA_Boss_Attack::OnMontageCancelled()
{
	K2_EndAbility();
}
