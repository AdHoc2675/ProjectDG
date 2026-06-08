// Fill out your copyright notice in the Description page of Project Settings.

#include "Character/Player/Animation/Notifies/AN_SendGameplayEvent.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/Actor.h"

UAN_SendGameplayEvent::UAN_SendGameplayEvent()
{
#if WITH_EDITORONLY_DATA
	NotifyColor = FColor(80, 200, 120);
#endif
}

FString UAN_SendGameplayEvent::GetNotifyName_Implementation() const
{
	return EventTag.IsValid()
			? FString::Printf(TEXT("Send GameplayEvent: %s"), *EventTag.ToString())
			: TEXT("Send GameplayEvent");
}

void UAN_SendGameplayEvent::Notify(
	  USkeletalMeshComponent* MeshComp,
	  UAnimSequenceBase* Animation,
	  const FAnimNotifyEventReference& EventReference
)
{
	Super::Notify(MeshComp, Animation, EventReference);

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

	UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(
			OwnerActor,
			EventTag,
			Payload
	);
}




