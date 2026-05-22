// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/Player/Animation/Notifies/ANS_SendGameplayEventWindow.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/Actor.h"

UANS_SendGameplayEventWindow::UANS_SendGameplayEventWindow()
{
#if WITH_EDITORONLY_DATA
	NotifyColor = FColor(80, 160, 255);
#endif
}

FString UANS_SendGameplayEventWindow::GetNotifyName_Implementation() const
{
	return EventTag.IsValid()
			? FString::Printf(TEXT("Send GameplayEvent: %s"), *EventTag.ToString())
			: TEXT("Send GameplayEvent");
}

void UANS_SendGameplayEventWindow::NotifyBegin(
	  USkeletalMeshComponent* MeshComp,
	  UAnimSequenceBase* Animation,
	  float TotalDuration,
	  const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyBegin(MeshComp, Animation, TotalDuration, EventReference);

	AActor* OwnerActor = MeshComp ? MeshComp->GetOwner() : nullptr;
	if (!OwnerActor || !EventTag.IsValid())
	{
		return;
	}

	if (bServerOnly && !OwnerActor->HasAuthority())
	{
		return;
	}

	FGameplayEventData Payload;
	Payload.EventTag = EventTag;
	Payload.Instigator = OwnerActor;
	Payload.Target = OwnerActor;
	Payload.EventMagnitude = TotalDuration;

	UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(OwnerActor, EventTag, Payload);
}


