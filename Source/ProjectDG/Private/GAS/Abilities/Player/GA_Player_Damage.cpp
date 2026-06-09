// Fill out your copyright notice in the Description page of Project Settings.

#include "GAS/Abilities/Player/GA_Player_Damage.h"

#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Core/DG_GameplayTags.h"

UGA_Player_Damage::UGA_Player_Damage()
{
      InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
      NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::ServerInitiated;

      ActivationOwnedTags.AddTag(DGGameplayTags::State_Player_Damage);

      ActivationBlockedTags.AddTag(DGGameplayTags::State_Skill_Active);
      ActivationBlockedTags.AddTag(DGGameplayTags::State_Player_Dead);
      ActivationBlockedTags.AddTag(DGGameplayTags::State_Player_Damage);

      FAbilityTriggerData TriggerData;
      TriggerData.TriggerTag = DGGameplayTags::Event_Player_Damage;
      TriggerData.TriggerSource = EGameplayAbilityTriggerSource::GameplayEvent;
      AbilityTriggers.Add(TriggerData);
}

void UGA_Player_Damage::ActivateAbility(
      const FGameplayAbilitySpecHandle Handle,
      const FGameplayAbilityActorInfo* ActorInfo,
      const FGameplayAbilityActivationInfo ActivationInfo,
      const FGameplayEventData* TriggerEventData
)
{
      if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
      {
              EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
              return;
      }

      AActor* AvatarActor = ActorInfo ? ActorInfo->AvatarActor.Get() : nullptr;
      if (!AvatarActor)
      {
              EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
              return;
      }

      const EDGDamageDirection Direction = CalculateDamageDirection(AvatarActor, TriggerEventData);
      UAnimMontage* MontageToPlay = GetDamageMontageByDirection(Direction);
      if (!MontageToPlay)
      {
              EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
              return;
      }

      MontageTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(
              this,
              NAME_None,
              MontageToPlay,
              1.f,
              NAME_None,
              true
      );

      if (!MontageTask)
      {
              EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
              return;
      }

      MontageTask->OnCompleted.AddDynamic(this, &UGA_Player_Damage::OnMontageCompleted);
      MontageTask->OnInterrupted.AddDynamic(this, &UGA_Player_Damage::OnMontageInterrupted);
      MontageTask->OnCancelled.AddDynamic(this, &UGA_Player_Damage::OnMontageCancelled);
      MontageTask->ReadyForActivation();
}

EDGDamageDirection UGA_Player_Damage::CalculateDamageDirection(
      const AActor* AvatarActor,
      const FGameplayEventData* TriggerEventData
) const
{
      if (!AvatarActor || !TriggerEventData)
      {
              return EDGDamageDirection::Front;
      }

      const FHitResult* HitResult = TriggerEventData->ContextHandle.GetHitResult();
      if (!HitResult)
      {
              return EDGDamageDirection::Front;
      }

      const FVector ToDamageSource =
              (HitResult->ImpactPoint - AvatarActor->GetActorLocation()).GetSafeNormal2D();

      const float ForwardDot = FVector::DotProduct(
              AvatarActor->GetActorForwardVector(),
              ToDamageSource
      );

      const float RightDot = FVector::DotProduct(
              AvatarActor->GetActorRightVector(),
              ToDamageSource
      );

      if (FMath::Abs(ForwardDot) >= FMath::Abs(RightDot))
      {
              return ForwardDot >= 0.f
                      ? EDGDamageDirection::Front
                      : EDGDamageDirection::Back;
      }

      return RightDot >= 0.f
              ? EDGDamageDirection::Right
              : EDGDamageDirection::Left;
}

UAnimMontage* UGA_Player_Damage::GetDamageMontageByDirection(EDGDamageDirection Direction) const
{
      switch (Direction)
      {
      case EDGDamageDirection::Back:
              return DamageBackMontage;
      case EDGDamageDirection::Left:
              return DamageLeftMontage;
      case EDGDamageDirection::Right:
              return DamageRightMontage;
      case EDGDamageDirection::Front:
      default:
              return DamageFrontMontage;
      }
}

void UGA_Player_Damage::OnMontageCompleted()
{
      K2_EndAbility();
}

void UGA_Player_Damage::OnMontageInterrupted()
{
      K2_EndAbility();
}

void UGA_Player_Damage::OnMontageCancelled()
{
      K2_EndAbility();
}

void UGA_Player_Damage::EndAbility(
      const FGameplayAbilitySpecHandle Handle,
      const FGameplayAbilityActorInfo* ActorInfo,
      const FGameplayAbilityActivationInfo ActivationInfo,
      bool bReplicateEndAbility,
      bool bWasCancelled
)
{
      if (MontageTask)
      {
              MontageTask->EndTask();
              MontageTask = nullptr;
      }

      Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}




