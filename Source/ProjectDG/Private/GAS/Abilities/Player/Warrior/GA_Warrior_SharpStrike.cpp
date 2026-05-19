// Fill out your copyright notice in the Description page of Project Settings.


#include "GAS/Abilities/Player/Warrior/GA_Warrior_SharpStrike.h"

#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"
#include "Character/Player/PlayerCharacterBase.h"
#include "Core/DG_GameplayTags.h"
#include "Core/DG_Debug.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "GameplayEffect.h"

UGA_Warrior_SharpStrike::UGA_Warrior_SharpStrike()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::LocalPredicted;

	// 태그 주가
	AbilityTags.AddTag(DGGameplayTags::Skill_Warrior_SharpStrike);
	ActivationOwnedTags.AddTag(DGGameplayTags::State_Skill_Warrior_SharpStrike_Active);
}

void UGA_Warrior_SharpStrike::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* OwnerInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
        AActor* DebugAvatarActor = GetAvatarActorFromActorInfo();
        
	if (!CommitAbility(Handle, OwnerInfo, ActivationInfo))
        {
                EndAbility(Handle, OwnerInfo, ActivationInfo, true, true);
                return;
        }

        if (!SharpStrikeMontage)
        {
                EndAbility(Handle, OwnerInfo, ActivationInfo, true, true);
                return;
        }

        bEndingSharpStrike = false;
        ResetComboState();

        ComboInputWindowOpenTask = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(
                this,
                DGGameplayTags::Event_Combo_InputWindow_Open.GetTag(),
                nullptr,
                false,
                true
        );

        if (ComboInputWindowOpenTask)
        {
                ComboInputWindowOpenTask->EventReceived.AddDynamic(this, &UGA_Warrior_SharpStrike::OnComboInputWindowOpened);
                ComboInputWindowOpenTask->ReadyForActivation();
        }

        ComboInputWindowCloseTask = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(
                this,
                DGGameplayTags::Event_Combo_InputWindow_Close.GetTag(),
                nullptr,
                false,
                true
        );

        if (ComboInputWindowCloseTask)
        {
                ComboInputWindowCloseTask->EventReceived.AddDynamic(this,&UGA_Warrior_SharpStrike::OnComboInputWindowClosed);
                ComboInputWindowCloseTask->ReadyForActivation();
        }
        
        AttackHitWindowBeginTask = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(
          this,
          DGGameplayTags::Event_Attack_HitWindow_Begin.GetTag(),
          nullptr,
          false,
          true);

        if (AttackHitWindowBeginTask)
        {
                AttackHitWindowBeginTask->EventReceived.AddDynamic(this, &UGA_Warrior_SharpStrike::OnAttackHitWindowBegin);
                AttackHitWindowBeginTask->ReadyForActivation();
        }

        ComboBranchTask = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(
                this,
                DGGameplayTags::Event_Combo_Branch.GetTag(),
                nullptr,
                false,
                true
        );

        if (ComboBranchTask)
        {
                ComboBranchTask->EventReceived.AddDynamic(this, &UGA_Warrior_SharpStrike::OnComboBranch);
                ComboBranchTask->ReadyForActivation();
        }
        
        SharpStrikeInputPressedTask = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(
        this,
        DGGameplayTags::Event_Input_Warrior_SharpStrike.GetTag(),
        nullptr,
        false,
        true);
        
        if (SharpStrikeInputPressedTask)
        {
                SharpStrikeInputPressedTask->EventReceived.AddDynamic(this, &UGA_Warrior_SharpStrike::OnSharpStrikeInputPressed);
                SharpStrikeInputPressedTask->ReadyForActivation();
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
                AttackHitTask->EventReceived.AddDynamic(this, &UGA_Warrior_SharpStrike::OnAttackHit);
                AttackHitTask->ReadyForActivation();
        }
        
        // Debug::Print(TEXT("[GA_Warrior_SharpStrike] SharpStrike activated."), FColor::Green);

        PlaySharpStrikeMontageFromStart();
}

void UGA_Warrior_SharpStrike::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, 
        const FGameplayAbilityActivationInfo ActivationInfo,bool bReplicateEndAbility, bool bWasCancelled)
{
	    ResetComboState();

	    Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

void UGA_Warrior_SharpStrike::ResetComboState()
{
        CurrentComboIndex = 1;
        bComboInputWindowOpen = false;
        bComboInputBuffered = false;
        HitActorsByCombo.Reset();
}

bool UGA_Warrior_SharpStrike::IsSharpStrikeInputHeld() const
{
        APlayerCharacterBase* PlayerCharacter = Cast<APlayerCharacterBase>(GetAvatarActorFromActorInfo());
        if (!PlayerCharacter)
        {
                return false;
        }

        return PlayerCharacter->IsSkillTagHeld(DGGameplayTags::Skill_Warrior_SharpStrike.GetTag());
}

void UGA_Warrior_SharpStrike::TryBufferComboInputFromHeldState()
{
        const bool bHeld = IsSharpStrikeInputHeld();

        if (bHeld)
        {
                bComboInputBuffered = true;
        }
}

void UGA_Warrior_SharpStrike::TryJumpToNextComboSection(int32 BranchComboIndex)
{
        if (BranchComboIndex != CurrentComboIndex)
        {
                Debug::Print(FString::Printf(
                        TEXT("[SharpStrikeGA] Branch combo differs from current state. BranchCombo=%d CurrentCombo=%d Buffered=%s Held=%s Authority=%s Avatar=%s"),
                        BranchComboIndex,
                        CurrentComboIndex,
                        bComboInputBuffered ? TEXT("true") : TEXT("false"),
                        IsSharpStrikeInputHeld() ? TEXT("true") : TEXT("false"),
                        GetAvatarActorFromActorInfo() && GetAvatarActorFromActorInfo()->HasAuthority() ? TEXT("true") :TEXT("false"),
                        *GetNameSafe(GetAvatarActorFromActorInfo())
                ), FColor::Orange);
        }
        
        if (BranchComboIndex < 1 || BranchComboIndex > 3)
        {
                return;
        }

        if (!bComboInputBuffered)
        {
                return;
        }

        bComboInputBuffered = false;

        if (BranchComboIndex == 1)
        {
                CurrentComboIndex = 2;

                MontageJumpToSection(Combo2SectionName);
                return;
        }

        if (BranchComboIndex == 2)
        {
                CurrentComboIndex = 3;

                MontageJumpToSection(Combo3SectionName);
                return;
        }

        if (BranchComboIndex == 3)
        {
                if (!IsSharpStrikeInputHeld())
                {
                        return;
                }

                CurrentComboIndex = 1;

                MontageJumpToSection(Combo1SectionName);
                return;
        }

        // Debug::Print(TEXT("[GA_Warrior_SharpStrike] ComboBranch ignored. Already at final combo."),FColor::Silver);
}

void UGA_Warrior_SharpStrike::PlaySharpStrikeMontageFromStart()
{
        if (!SharpStrikeMontage)
        {
                EndSharpStrikeAbility();
                return;
        }

        MontageTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(this,TEXT("SharpStrikeMontageTask"),
                SharpStrikeMontage, SharpStrikePlayRate, Combo1SectionName);

        if (!MontageTask)
        {
                EndSharpStrikeAbility();
                return;
        }

        MontageTask->OnCompleted.AddDynamic(this, &UGA_Warrior_SharpStrike::OnMontageCompleted);
        MontageTask->OnInterrupted.AddDynamic(this, &UGA_Warrior_SharpStrike::OnMontageInterrupted);
        MontageTask->OnBlendOut.AddDynamic(this, &UGA_Warrior_SharpStrike::OnMontageBlendOut);
        MontageTask->OnCancelled.AddDynamic(this, &UGA_Warrior_SharpStrike::OnMontageCancelled);

        MontageTask->ReadyForActivation();
}

void UGA_Warrior_SharpStrike::OnComboInputWindowOpened(FGameplayEventData Payload)
  {
        bComboInputWindowOpen = true;

        TryBufferComboInputFromHeldState();
  }

  void UGA_Warrior_SharpStrike::OnComboInputWindowClosed(FGameplayEventData Payload)
  {
        bComboInputWindowOpen = false;
  }

void UGA_Warrior_SharpStrike::OnAttackHitWindowBegin(FGameplayEventData Payload)
{
        const int32 HitWindowComboIndex = FMath::RoundToInt(Payload.EventMagnitude);

        if (HitWindowComboIndex < 1 || HitWindowComboIndex > 3)
        {
                Debug::Print(FString::Printf(
                        TEXT("[SharpStrikeGA] Invalid HitWindowComboIndex=%d"),
                        HitWindowComboIndex
                ), FColor::Red);
                return;
        }

        HitActorsByCombo.FindOrAdd(HitWindowComboIndex).Reset();

        Debug::Print(FString::Printf(
                TEXT("[SharpStrikeGA] HitWindow Begin. Reset HitActors. HitCombo=%d CurrentCombo=%d Authority=%s Avatar=%s"),
                HitWindowComboIndex,
                CurrentComboIndex,
                GetAvatarActorFromActorInfo() && GetAvatarActorFromActorInfo()->HasAuthority() ? TEXT("true") : TEXT("false"),
                *GetNameSafe(GetAvatarActorFromActorInfo())), FColor::Cyan);
}

void UGA_Warrior_SharpStrike::OnComboBranch(FGameplayEventData Payload)
  {
        const int32 BranchComboIndex = FMath::RoundToInt(Payload.EventMagnitude);

        TryJumpToNextComboSection(BranchComboIndex);
  }

void UGA_Warrior_SharpStrike::OnMontageCompleted()
{
        EndSharpStrikeAbility();
}

  void UGA_Warrior_SharpStrike::OnMontageInterrupted()
  {
        EndSharpStrikeAbility();
  }

  void UGA_Warrior_SharpStrike::OnMontageBlendOut()
  {
        EndSharpStrikeAbility();
  }

  void UGA_Warrior_SharpStrike::OnMontageCancelled()
  {
        EndSharpStrikeAbility();
  }

void UGA_Warrior_SharpStrike::OnSharpStrikeInputPressed(FGameplayEventData Payload)
{
        if (!bComboInputWindowOpen)
        {
                return;
        }

        bComboInputBuffered = true;
}

void UGA_Warrior_SharpStrike::EndSharpStrikeAbility()
{
        if (bEndingSharpStrike)
        {
                return;
        }

        bEndingSharpStrike = true;
        K2_EndAbility();
}

float UGA_Warrior_SharpStrike::GetCurrentComboDamage() const
{
        return ComboDamage;
}

void UGA_Warrior_SharpStrike::OnAttackHit(FGameplayEventData Payload)
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
        
        const int32 HitComboIndex = FMath::RoundToInt(Payload.EventMagnitude);
        if (HitComboIndex < 1 || HitComboIndex > 3)
        {
                Debug::Print(FString::Printf(
                        TEXT("[SharpStrikeGA] Invalid HitComboIndex=%d Target=%s"),
                        HitComboIndex,
                        *GetNameSafe(TargetActor)), FColor::Red);
                return;
        }

        TSet<TWeakObjectPtr<AActor>>& HitActorsForCombo = HitActorsByCombo.FindOrAdd(HitComboIndex);

        if (HitActorsForCombo.Contains(TargetActor))
        {
                return;
        }

        HitActorsForCombo.Add(TargetActor);
        
        if (HitComboIndex != CurrentComboIndex)
        {
                Debug::Print(FString::Printf(
                        TEXT("[SharpStrikeGA] Hit combo differs from current state. HitCombo=%d CurrentCombo=%d Target=%s"),
                        HitComboIndex,
                        CurrentComboIndex,
                        *GetNameSafe(TargetActor)), FColor::Orange);
        }

        if (!DamageEffectClass)
        {
                return;
        }

        UAbilitySystemComponent* SourceASC = GetAbilitySystemComponentFromActorInfo();
        UAbilitySystemComponent* TargetASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(TargetActor);

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
                GetCurrentComboDamage()
        );

        SourceASC->ApplyGameplayEffectSpecToTarget(*SpecHandle.Data.Get(), TargetASC);
}