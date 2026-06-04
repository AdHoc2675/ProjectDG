// Fill out your copyright notice in the Description page of Project Settings.

#include "Character/Player/Animation/Notifies/ANS_GameplayCueWindow.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "Components/SkeletalMeshComponent.h"
#include "Core/DG_GameplayTags.h"
#include "GameFramework/Actor.h"

UANS_GameplayCueWindow::UANS_GameplayCueWindow()
{
      BeginEventTag = DGGameplayTags::Event_Weapon_Blade_BasicTrail_Begin;
      EndEventTag = DGGameplayTags::Event_Weapon_Blade_BasicTrail_End;

#if WITH_EDITORONLY_DATA
      NotifyColor = FColor(80, 200, 255);
#endif
}

FString UANS_GameplayCueWindow::GetNotifyName_Implementation() const
{
      if (BeginEventTag.IsValid() && EndEventTag.IsValid())
      {
              return FString::Printf(
                      TEXT("GameplayCueWindow: %s"),
                      *BeginEventTag.ToString()
              );
      }

      return TEXT("GameplayCueWindow");
}

void UANS_GameplayCueWindow::NotifyBegin(
      USkeletalMeshComponent* MeshComp,
      UAnimSequenceBase* Animation,
      float TotalDuration,
      const FAnimNotifyEventReference& EventReference
)
{
      Super::NotifyBegin(MeshComp, Animation, TotalDuration, EventReference);

      SendGameplayCueWindowEvent(MeshComp, BeginEventTag, TotalDuration);
}

void UANS_GameplayCueWindow::NotifyEnd(
      USkeletalMeshComponent* MeshComp,
      UAnimSequenceBase* Animation,
      const FAnimNotifyEventReference& EventReference
)
{
      Super::NotifyEnd(MeshComp, Animation, EventReference);

      SendGameplayCueWindowEvent(MeshComp, EndEventTag, 0.f);
}

void UANS_GameplayCueWindow::SendGameplayCueWindowEvent(
      USkeletalMeshComponent* MeshComp,
      const FGameplayTag& EventTag,
      float EventMagnitude
) const
{
      if (!MeshComp || !EventTag.IsValid())
      {
              return;
      }

      AActor* OwnerActor = MeshComp->GetOwner();
      if (!OwnerActor)
      {
              return;
      }

      FGameplayEventData Payload;
      Payload.EventTag = EventTag;
      Payload.Instigator = OwnerActor;
      Payload.Target = OwnerActor;
      Payload.EventMagnitude = EventMagnitude;

      UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(
              OwnerActor,
              EventTag,
              Payload
      );
}




