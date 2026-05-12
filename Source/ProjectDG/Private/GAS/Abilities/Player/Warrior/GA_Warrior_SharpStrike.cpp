// Fill out your copyright notice in the Description page of Project Settings.


#include "GAS/Abilities/Player/Warrior/GA_Warrior_SharpStrike.h"

#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"
#include "Character/Player/PlayerCharacterBase.h"
#include "Core/DG_GameplayTags.h"
#include "Core/DG_Debug.h"

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
	if (!CommitAbility(Handle, OwnerInfo, ActivationInfo))
        {
                EndAbility(Handle, OwnerInfo, ActivationInfo, true, true);
                return;
        }

        if (!SharpStrikeFullBodyMontage || !SharpStrikeUpperBodyMontage)
        {
                Debug::Print(TEXT("[GA_Warrior_SharpStrike] SharpStrikeMontage is null."), FColor::Red);
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
        
        
        Debug::Print(TEXT("[GA_Warrior_SharpStrike] SharpStrike activated."), FColor::Green);

        PlaySharpStrikeMontageFromStart();
}

void UGA_Warrior_SharpStrike::EndAbility(
        const FGameplayAbilitySpecHandle Handle,
        const FGameplayAbilityActorInfo* ActorInfo,
        const FGameplayAbilityActivationInfo ActivationInfo,
        bool bReplicateEndAbility,
        bool bWasCancelled
  )
{
	    ResetComboState();

	    Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

void UGA_Warrior_SharpStrike::ResetComboState()
{
        CurrentComboIndex = 1;
        bComboInputWindowOpen = false;
        bComboInputBuffered = false;
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
        if (IsSharpStrikeInputHeld())
        {
                bComboInputBuffered = true;

                Debug::Print(TEXT("[GA_Warrior_SharpStrike] Combo input buffered by held input."), FColor::Cyan);
        }
  }

  void UGA_Warrior_SharpStrike::TryJumpToNextComboSection()
  {
        if (!bComboInputBuffered)
        {
                Debug::Print(TEXT("[GA_Warrior_SharpStrike] ComboBranch reached, but no buffered input."),
  FColor::Silver);
                return;
        }

        bComboInputBuffered = false;

        if (CurrentComboIndex == 1)
        {
                CurrentComboIndex = 2;
                MontageJumpToSection(Combo2SectionName);

                Debug::Print(TEXT("[GA_Warrior_SharpStrike] Jump to Combo_2."), FColor::Green);
                return;
        }

        if (CurrentComboIndex == 2)
        {
                CurrentComboIndex = 3;
                MontageJumpToSection(Combo3SectionName);

                Debug::Print(TEXT("[GA_Warrior_SharpStrike] Jump to Combo_3."), FColor::Green);
                return;
        }

        Debug::Print(TEXT("[GA_Warrior_SharpStrike] ComboBranch ignored. Already at final combo."),FColor::Silver);
  }

void UGA_Warrior_SharpStrike::PlaySharpStrikeMontageFromStart()
{
        UAnimMontage* MontageToPlay = GetSharpStrikeMontageToPlay();
        if (!MontageToPlay)
        {
                Debug::Print(TEXT("[GA_Warrior_SharpStrike] MontageToPlay is null."), FColor::Red);
                K2_EndAbility();
                return;
        }

        MontageTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(
                this,
                TEXT("SharpStrikeMontageTask"),
                MontageToPlay,
                SharpStrikePlayRate,
                Combo1SectionName
        );

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

UAnimMontage* UGA_Warrior_SharpStrike::GetSharpStrikeMontageToPlay() const
{
        APlayerCharacterBase* PlayerCharacter = Cast<APlayerCharacterBase>(GetAvatarActorFromActorInfo());
        if (!PlayerCharacter)
        {
                return SharpStrikeFullBodyMontage;
        }

        const bool bIsMoving = PlayerCharacter->GetVelocity().Size2D() > MovingMontageThreshold;
        return bIsMoving ? SharpStrikeUpperBodyMontage : SharpStrikeFullBodyMontage;
}

void UGA_Warrior_SharpStrike::OnComboInputWindowOpened(FGameplayEventData Payload)
  {
        bComboInputWindowOpen = true;

        Debug::Print(TEXT("[GA_Warrior_SharpStrike] Combo input window opened."), FColor::Cyan);

        TryBufferComboInputFromHeldState();
  }

  void UGA_Warrior_SharpStrike::OnComboInputWindowClosed(FGameplayEventData Payload)
  {
        bComboInputWindowOpen = false;

        Debug::Print(TEXT("[GA_Warrior_SharpStrike] Combo input window closed."), FColor::Cyan);
  }

  void UGA_Warrior_SharpStrike::OnComboBranch(FGameplayEventData Payload)
  {
        Debug::Print(TEXT("[GA_Warrior_SharpStrike] Combo branch."), FColor::Yellow);

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