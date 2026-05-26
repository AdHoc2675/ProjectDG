// Fill out your copyright notice in the Description page of Project Settings.

#include "Character/Player/Animation/Notifies/ANS_AOEOverlapWindow.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "Components/SkeletalMeshComponent.h"
#include "Core/DG_GameplayTags.h"
#include "GameFramework/Actor.h"

UANS_AOEOverlapWindow::UANS_AOEOverlapWindow()
{
#if WITH_EDITORONLY_DATA
	NotifyColor = FColor(120, 220, 120);
#endif
}

FString UANS_AOEOverlapWindow::GetNotifyName_Implementation() const
{
	return TEXT("AOEOverlapWindow");
}

void UANS_AOEOverlapWindow::NotifyBegin(
	  USkeletalMeshComponent* MeshComp,
	  UAnimSequenceBase* Animation,
	  float TotalDuration,
	  const FAnimNotifyEventReference& EventReference
)
{
	Super::NotifyBegin(MeshComp, Animation, TotalDuration, EventReference);

	SendAOEWindowEvent(MeshComp, DGGameplayTags::Event_AOE_OverlapWindow_Begin.GetTag());
}

void UANS_AOEOverlapWindow::NotifyEnd(
	  USkeletalMeshComponent* MeshComp,
	  UAnimSequenceBase* Animation,
	  const FAnimNotifyEventReference& EventReference
)
{
	Super::NotifyEnd(MeshComp, Animation, EventReference);

	SendAOEWindowEvent(MeshComp, DGGameplayTags::Event_AOE_OverlapWindow_End.GetTag());
}

void UANS_AOEOverlapWindow::SendAOEWindowEvent(USkeletalMeshComponent* MeshComp, const FGameplayTag& EventTag) const
{
	if (!MeshComp || !EventTag.IsValid())
	{
		return;
	}

	AActor* OwnerActor = MeshComp->GetOwner();
	if (!OwnerActor || !OwnerActor->HasAuthority())
	{
		return;
	}

	FGameplayEventData Payload;
	Payload.EventTag = EventTag;
	Payload.Instigator = OwnerActor;
	Payload.Target = OwnerActor;

	UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(OwnerActor, EventTag, Payload);
}




