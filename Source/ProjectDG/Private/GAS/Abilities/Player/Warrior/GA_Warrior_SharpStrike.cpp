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

        // Debug::Print(FString::Printf(
        //       TEXT("[SharpStrikeGA] ActivateAbility Avatar=%s Authority=%s LocalRole=%d CurrentCombo=%d Time=%.3f"),
        //       *GetNameSafe(DebugAvatarActor),
        //       DebugAvatarActor && DebugAvatarActor->HasAuthority() ? TEXT("true") : TEXT("false"),
        //       DebugAvatarActor ? static_cast<int32>(DebugAvatarActor->GetLocalRole()) : -1,
        //       CurrentComboIndex,
        //       DebugAvatarActor && DebugAvatarActor->GetWorld() ? DebugAvatarActor->GetWorld()->GetTimeSeconds() : -1.f
        // ), FColor::Green);
        
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
                
                // AActor* InfunctionDebugAvatarActor = GetAvatarActorFromActorInfo();
                // Debug::Print(FString::Printf(
                //         TEXT("[SharpStrikeGA] InputPressedTask Ready Avatar=%s Authority=%s LocalRole=%d Time=%.3f"),
                //         *GetNameSafe(InfunctionDebugAvatarActor),
                //         InfunctionDebugAvatarActor && InfunctionDebugAvatarActor->HasAuthority() ? TEXT("true") : TEXT("false"),
                //         InfunctionDebugAvatarActor ? static_cast<int32>(InfunctionDebugAvatarActor->GetLocalRole()) : -1,
                //         InfunctionDebugAvatarActor && InfunctionDebugAvatarActor->GetWorld() ? InfunctionDebugAvatarActor->GetWorld()->GetTimeSeconds() : -1.f
                // ), FColor::Cyan);
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
        HitActorsThisCombo.Reset();
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

void UGA_Warrior_SharpStrike::TryJumpToNextComboSection()
{
        if (!bComboInputBuffered)
        {
                // AActor* DebugAvatarActor = GetAvatarActorFromActorInfo();
                // Debug::Print(FString::Printf(
                //         TEXT("[SharpStrikeGA] Branch SKIPPED: no buffered input Avatar=%s Authority=%s Combo=%d WindowOpen=%s Held=%sTime=%.3f"),
                //         *GetNameSafe(DebugAvatarActor),
                //         DebugAvatarActor && DebugAvatarActor->HasAuthority() ? TEXT("true") : TEXT("false"),
                //         CurrentComboIndex,
                //         bComboInputWindowOpen ? TEXT("true") : TEXT("false"),
                //         IsSharpStrikeInputHeld() ? TEXT("true") : TEXT("false"),
                //         DebugAvatarActor && DebugAvatarActor->GetWorld() ? DebugAvatarActor->GetWorld()->GetTimeSeconds() : -1.f
                // ), FColor::Red);
                
                return;
        }

        bComboInputBuffered = false;

        if (CurrentComboIndex == 1)
        {
                CurrentComboIndex = 2;
                HitActorsThisCombo.Reset();
                
                // Debug::Print(FString::Printf(TEXT("[SharpStrike Reset HitActors] Jump to Combo=%d"),CurrentComboIndex), FColor::Green);
                
                MontageJumpToSection(Combo2SectionName);

                // Debug::Print(TEXT("[GA_Warrior_SharpStrike] Jump to Combo_2."), FColor::Green);
                return;
        }

        if (CurrentComboIndex == 2)
        {
                CurrentComboIndex = 3;
                HitActorsThisCombo.Reset();
                
                // Debug::Print(FString::Printf(TEXT("[SharpStrike Reset HitActors] Jump to Combo=%d"),CurrentComboIndex), FColor::Green);
                
                MontageJumpToSection(Combo3SectionName);

                // Debug::Print(TEXT("[GA_Warrior_SharpStrike] Jump to Combo_3."), FColor::Green);
                return;
        }
        if (CurrentComboIndex == 3)
        {
                // if (!IsSharpStrikeInputHeld())
                // {
                //         return;
                // }

                CurrentComboIndex = 1;
                HitActorsThisCombo.Reset();
                
                // Debug::Print(FString::Printf(TEXT("[SharpStrike Reset HitActors] Jump to Combo=%d"),CurrentComboIndex), FColor::Green);
                
                MontageJumpToSection(Combo1SectionName);
                return;
        }

        // Debug::Print(TEXT("[GA_Warrior_SharpStrike] ComboBranch ignored. Already at final combo."),FColor::Silver);
}

void UGA_Warrior_SharpStrike::PlaySharpStrikeMontageFromStart()
{
        if (!SharpStrikeMontage)
        {
                K2_EndAbility();
                return;
        }

        MontageTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(this,TEXT("SharpStrikeMontageTask"),
                SharpStrikeMontage, SharpStrikePlayRate, Combo1SectionName);

        if (!MontageTask)
        {
                K2_EndAbility();
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

        // AActor* DebugAvatarActor = GetAvatarActorFromActorInfo();
        // Debug::Print(FString::Printf(
        //       TEXT("[SharpStrikeGA] ComboWindow OPEN Avatar=%s Authority=%s Combo=%d Buffered=%s Held=%s Time=%.3f"),
        //       *GetNameSafe(DebugAvatarActor),
        //       DebugAvatarActor && DebugAvatarActor->HasAuthority() ? TEXT("true") : TEXT("false"),
        //       CurrentComboIndex,
        //       bComboInputBuffered ? TEXT("true") : TEXT("false"),
        //       IsSharpStrikeInputHeld() ? TEXT("true") : TEXT("false"),
        //       DebugAvatarActor && DebugAvatarActor->GetWorld() ? DebugAvatarActor->GetWorld()->GetTimeSeconds() : -1.f
        // ), FColor::Cyan);

        TryBufferComboInputFromHeldState();
  }

  void UGA_Warrior_SharpStrike::OnComboInputWindowClosed(FGameplayEventData Payload)
  {
        bComboInputWindowOpen = false;

        // AActor* DebugAvatarActor = GetAvatarActorFromActorInfo();
        // Debug::Print(FString::Printf(
        //       TEXT("[SharpStrikeGA] ComboWindow CLOSE Avatar=%s Authority=%s Combo=%d Buffered=%s Held=%s Time=%.3f"),
        //       *GetNameSafe(DebugAvatarActor),
        //       DebugAvatarActor && DebugAvatarActor->HasAuthority() ? TEXT("true") : TEXT("false"),
        //       CurrentComboIndex,
        //       bComboInputBuffered ? TEXT("true") : TEXT("false"),
        //       IsSharpStrikeInputHeld() ? TEXT("true") : TEXT("false"),
        //       DebugAvatarActor && DebugAvatarActor->GetWorld() ? DebugAvatarActor->GetWorld()->GetTimeSeconds() : -1.f
        // ), FColor::Silver);
  }

  void UGA_Warrior_SharpStrike::OnComboBranch(FGameplayEventData Payload)
  {
        //Debug::Print(TEXT("[GA_Warrior_SharpStrike] Combo branch."), FColor::Yellow);

        TryJumpToNextComboSection();
  }

  void UGA_Warrior_SharpStrike::OnMontageCompleted()
  {
        if (IsSharpStrikeInputHeld())
        {
                ResetComboState();
                PlaySharpStrikeMontageFromStart();
                return;
        }
        
        K2_EndAbility();
  }

  void UGA_Warrior_SharpStrike::OnMontageInterrupted()
  {
        K2_EndAbility();
  }

  void UGA_Warrior_SharpStrike::OnMontageBlendOut()
  {
        // Normal montage completion can trigger BlendOut before Completed.
        // Do not end the ability here because SharpStrike may restart while the key is held.
  }

  void UGA_Warrior_SharpStrike::OnMontageCancelled()
  {
        K2_EndAbility();
  }

void UGA_Warrior_SharpStrike::OnSharpStrikeInputPressed(FGameplayEventData Payload)
{
        // AActor* DebugAvatarActor = GetAvatarActorFromActorInfo();

        // Debug::Print(FString::Printf(
        //         TEXT("[SharpStrikeGA] InputEvent RECEIVED Avatar=%s Authority=%s Event=%s Combo=%d WindowOpen=%s BufferedBefore=%s Held=%s Time=%.3f"),
        //         *GetNameSafe(DebugAvatarActor),
        //         DebugAvatarActor && DebugAvatarActor->HasAuthority() ? TEXT("true") : TEXT("false"),
        //         *Payload.EventTag.ToString(),
        //         CurrentComboIndex,
        //         bComboInputWindowOpen ? TEXT("true") : TEXT("false"),
        //         bComboInputBuffered ? TEXT("true") : TEXT("false"),
        //         IsSharpStrikeInputHeld() ? TEXT("true") : TEXT("false"),
        //         DebugAvatarActor && DebugAvatarActor->GetWorld() ? DebugAvatarActor->GetWorld()->GetTimeSeconds() : -1.f
        // ), bComboInputWindowOpen ? FColor::Green : FColor::Red);
        
        if (!bComboInputWindowOpen)
        {
                return;
        }

        bComboInputBuffered = true;
        
        // Debug::Print(FString::Printf(
        //         TEXT("[SharpStrikeGA] InputEvent BUFFERED Combo=%d"),
        //         CurrentComboIndex
        // ), FColor::Green);
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
        
        if (HitActorsThisCombo.Contains(TargetActor))
        {
                return;
        }

        HitActorsThisCombo.Add(TargetActor);

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