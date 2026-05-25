// Fill out your copyright notice in the Description page of Project Settings.

#include "GAS/Abilities/Base/GA_TargetMontageSkillBase.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"
#include "Character/Player/Data/PlayerSkillData.h"
#include "Core/DG_GameplayTags.h"

UGA_TargetMontageSkillBase::UGA_TargetMontageSkillBase()
{
      InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
      NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::LocalPredicted;
}

void UGA_TargetMontageSkillBase::ActivateAbility(
      const FGameplayAbilitySpecHandle Handle,
      const FGameplayAbilityActorInfo* ActorInfo,
      const FGameplayAbilityActivationInfo ActivationInfo,
      const FGameplayEventData* TriggerEventData
)
{
        ResetTargetMontageState();
        
        if (ActorInfo && ActorInfo->IsLocallyControlled())
        {
                if (!AcquireLocalTargetAndSendTargetData())
                {
                        EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
                        return;
                }

                ContinueTargetMontageAbility();
                return;
        }

        if (HasAuthorityAvatar())
        {
                WaitForRemoteTargetData();
                return;
        }

        EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
}

void UGA_TargetMontageSkillBase::EndAbility(
      const FGameplayAbilitySpecHandle Handle,
      const FGameplayAbilityActorInfo* ActorInfo,
      const FGameplayAbilityActivationInfo ActivationInfo,
      bool bReplicateEndAbility,
      bool bWasCancelled
)
{
      ResetTargetMontageState();

      MontageTask = nullptr;
      AttackHitWindowBeginTask = nullptr;
      AttackHitTask = nullptr;

      Super::EndAbility(
              Handle,
              ActorInfo,
              ActivationInfo,
              bReplicateEndAbility,
              bWasCancelled
      );
}

void UGA_TargetMontageSkillBase::ResetTargetMontageState()
{
      bEndingTargetMontageAbility = false;
      bWaitingForRemoteTargetData = false;
      CurrentTargetResult = FDGSkillTargetResult();
      HitActors.Reset();
}

void UGA_TargetMontageSkillBase::StartTargetMontageEventTasks()
{
        AttackHitWindowBeginTask = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(
                this,
                DGGameplayTags::Event_Attack_HitWindow_Begin.GetTag(),
                nullptr,
                false,
                true
        );

        if (AttackHitWindowBeginTask)
        {
                AttackHitWindowBeginTask->EventReceived.AddDynamic(
                        this,
                        &UGA_TargetMontageSkillBase::OnAttackHitWindowBegin
                );
                AttackHitWindowBeginTask->ReadyForActivation();
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
                AttackHitTask->EventReceived.AddDynamic(
                        this,
                        &UGA_TargetMontageSkillBase::OnAttackHit
                );
                AttackHitTask->ReadyForActivation();
        }
}

void UGA_TargetMontageSkillBase::PlayTargetSkillMontage()
{
      UAnimMontage* SkillMontage = GetSkillMontage();
      if (!SkillMontage)
      {
              EndTargetMontageAbility(true);
              return;
      }

      MontageTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(
              this,
              TEXT("TargetMontageSkillTask"),
              SkillMontage,
              MontagePlayRate
      );

      if (!MontageTask)
      {
              EndTargetMontageAbility(true);
              return;
      }

      MontageTask->OnCompleted.AddDynamic(this, &UGA_TargetMontageSkillBase::OnMontageCompleted);
      MontageTask->OnInterrupted.AddDynamic(this, &UGA_TargetMontageSkillBase::OnMontageInterrupted);
      MontageTask->OnBlendOut.AddDynamic(this, &UGA_TargetMontageSkillBase::OnMontageBlendOut);
      MontageTask->OnCancelled.AddDynamic(this, &UGA_TargetMontageSkillBase::OnMontageCancelled);

      MontageTask->ReadyForActivation();
}

void UGA_TargetMontageSkillBase::ExecuteTargetSkill(AActor* TargetActor, const FGameplayEventData& Payload)
{
      if (!HasAuthorityAvatar())
      {
              return;
      }

      if (!TargetActor)
      {
              return;
      }

      ApplyDamageToTarget(
              TargetActor,
              0.f,
              GetSkillDamageMultiplier(),
              GetSkillTag(),
              CurrentTargetResult.AimPoint,
              CurrentTargetResult.bHasTarget
      );

      ApplyStatusEffectToTarget(TargetActor);
}

bool UGA_TargetMontageSkillBase::IsCurrentTargetStillValid() const
{
      AActor* TargetActor = CurrentTargetResult.TargetActor;
      if (!CurrentTargetResult.bHasTarget || !TargetActor)
      {
              return false;
      }

      if (!IsValidSkillTarget(TargetActor))
      {
              return false;
      }

      const AActor* AvatarActor = GetAvatarActorFromAbility();
      if (!AvatarActor)
      {
              return false;
      }

      const float SkillRange = GetSkillRange();
      if (SkillRange <= 0.f)
      {
              return true;
      }

      const float DistanceSq = FVector::DistSquared2D(
              AvatarActor->GetActorLocation(),
              TargetActor->GetActorLocation()
      );

      return DistanceSq <= FMath::Square(SkillRange);
}

bool UGA_TargetMontageSkillBase::IsHitActorAcceptable(AActor* HitActor) const
{
      if (!HitActor)
      {
              return false;
      }

      if (!IsCurrentTargetStillValid())
      {
              return false;
      }

      if (bRequireHitTargetMatchesAcquiredTarget && HitActor != CurrentTargetResult.TargetActor)
      {
              return false;
      }

      if (HitActors.Contains(HitActor))
      {
              return false;
      }

      return true;
}

void UGA_TargetMontageSkillBase::FaceCurrentTarget()
{
      AActor* AvatarActor = GetAvatarActorFromAbility();
      AActor* TargetActor = CurrentTargetResult.TargetActor;

      if (!AvatarActor || !TargetActor)
      {
              return;
      }

      FVector Direction = TargetActor->GetActorLocation() - AvatarActor->GetActorLocation();
      Direction.Z = 0.f;

      if (Direction.Normalize())
      {
              AvatarActor->SetActorRotation(Direction.Rotation());
      }
}

void UGA_TargetMontageSkillBase::ApplyStatusEffectToTarget(AActor* TargetActor) const
{
      if (!HasAuthorityAvatar())
      {
              return;
      }

      if (!TargetActor)
      {
              return;
      }

      const UPlayerSkillData* Data = GetPlayerSkillData();
      if (!Data || !Data->StatusEffect)
      {
              return;
      }

      UAbilitySystemComponent* SourceASC = GetAbilitySystemComponentFromActorInfo();
      UAbilitySystemComponent* TargetASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(TargetActor);

      if (!SourceASC || !TargetASC)
      {
              return;
      }

      FGameplayEffectContextHandle EffectContext = SourceASC->MakeEffectContext();
      EffectContext.AddSourceObject(GetAvatarActorFromAbility());

      FGameplayEffectSpecHandle SpecHandle = SourceASC->MakeOutgoingSpec(
              Data->StatusEffect,
              GetAbilityLevel(),
              EffectContext
      );

      if (!SpecHandle.IsValid() || !SpecHandle.Data.IsValid())
      {
              return;
      }

      SourceASC->ApplyGameplayEffectSpecToTarget(
              *SpecHandle.Data.Get(),
              TargetASC
      );
}

void UGA_TargetMontageSkillBase::EndTargetMontageAbility(bool bWasCancelled)
{
      if (bEndingTargetMontageAbility)
      {
              return;
      }

      bEndingTargetMontageAbility = true;
      K2_EndAbility();
}

bool UGA_TargetMontageSkillBase::AcquireLocalTargetAndSendTargetData()
{
        if (!TryAcquireSkillTarget(CurrentTargetResult))
        {
                return false;
        }

        if (!IsCurrentTargetStillValid())
        {
                return false;
        }

        if (!HasAuthorityAvatar())
        {
                const FGameplayAbilityTargetDataHandle TargetDataHandle =
                        MakeTargetDataFromTargetResult(CurrentTargetResult);

                SendTargetDataToServer(TargetDataHandle);
        }

        return true;
}

void UGA_TargetMontageSkillBase::WaitForRemoteTargetData()
{
        UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo();
        if (!ASC)
        {
                EndTargetMontageAbility(true);
                return;
        }

        bWaitingForRemoteTargetData = true;

        const FGameplayAbilitySpecHandle SpecHandle = GetCurrentAbilitySpecHandle();
        const FPredictionKey ActivationPredictionKey = GetCurrentActivationInfo().GetActivationPredictionKey();

        ASC->AbilityTargetDataSetDelegate(SpecHandle, ActivationPredictionKey).AddUObject(
                this,
                &UGA_TargetMontageSkillBase::OnTargetDataReadyCallback
        );

        if (!ASC->CallReplicatedTargetDataDelegatesIfSet(SpecHandle, ActivationPredictionKey))
        {
                return;
        }
}

void UGA_TargetMontageSkillBase::ContinueTargetMontageAbility()
{
        if (!IsCurrentTargetStillValid())
        {
                EndTargetMontageAbility(true);
                return;
        }

        if (!CommitAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo))
        {
                EndTargetMontageAbility(true);
                return;
        }

        if (!GetSkillMontage())
        {
                EndTargetMontageAbility(true);
                return;
        }

        if (bFaceTargetOnActivate)
        {
                FaceCurrentTarget();
        }

        StartTargetMontageEventTasks();
        PlayTargetSkillMontage();
}

void UGA_TargetMontageSkillBase::SendTargetDataToServer(const FGameplayAbilityTargetDataHandle& TargetDataHandle)
{
        UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo();
        if (!ASC)
        {
                return;
        }

        FScopedPredictionWindow ScopedPrediction(ASC, true);

        ASC->ServerSetReplicatedTargetData(
                GetCurrentAbilitySpecHandle(),
                GetCurrentActivationInfo().GetActivationPredictionKey(),
                TargetDataHandle,
                FGameplayTag(),
                ASC->ScopedPredictionKey
        );
}

void UGA_TargetMontageSkillBase::OnTargetDataReadyCallback(const FGameplayAbilityTargetDataHandle& TargetDataHandle,
        FGameplayTag ActivationTag)
{
        UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo();
        if (!ASC)
        {
                EndTargetMontageAbility(true);
                return;
        }

        bWaitingForRemoteTargetData = false;

        const bool bMadeTargetResult = TryMakeTargetResultFromTargetData(
                TargetDataHandle,
                CurrentTargetResult
        );

        ASC->ConsumeClientReplicatedTargetData(
                GetCurrentAbilitySpecHandle(),
                GetCurrentActivationInfo().GetActivationPredictionKey()
        );

        if (!bMadeTargetResult)
        {
                EndTargetMontageAbility(true);
                return;
        }

        ContinueTargetMontageAbility();
}

void UGA_TargetMontageSkillBase::OnAttackHitWindowBegin(FGameplayEventData Payload)
{
      HitActors.Reset();
}

void UGA_TargetMontageSkillBase::OnAttackHit(FGameplayEventData Payload)
{
      if (!HasAuthorityAvatar())
      {
              return;
      }

      AActor* HitActor = const_cast<AActor*>(Payload.Target.Get());
      if (!IsHitActorAcceptable(HitActor))
      {
              return;
      }

      HitActors.Add(HitActor);

      ExecuteTargetSkill(HitActor, Payload);
}

void UGA_TargetMontageSkillBase::OnMontageCompleted()
{
      EndTargetMontageAbility(false);
}

void UGA_TargetMontageSkillBase::OnMontageInterrupted()
{
      EndTargetMontageAbility(true);
}

void UGA_TargetMontageSkillBase::OnMontageBlendOut()
{
      EndTargetMontageAbility(false);
}

void UGA_TargetMontageSkillBase::OnMontageCancelled()
{
      EndTargetMontageAbility(true);
}



