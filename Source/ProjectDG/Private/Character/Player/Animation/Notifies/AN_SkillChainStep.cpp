// Fill out your copyright notice in the Description page of Project Settings.

#include "Character/Player/Animation/Notifies/AN_SkillChainStep.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "Components/SkeletalMeshComponent.h"
#include "Core/DG_GameplayTags.h"
#include "GameFramework/Actor.h"

UAN_SkillChainStep::UAN_SkillChainStep()
{
	EventTag = DGGameplayTags::Event_Skill_ChainStep;

#if WITH_EDITORONLY_DATA
	NotifyColor = FColor(80, 160, 255);
#endif
}

void UAN_SkillChainStep::Notify(
	USkeletalMeshComponent* MeshComp,
	UAnimSequenceBase* Animation,
	const FAnimNotifyEventReference& EventReference
)
{
	Super::Notify(MeshComp, Animation, EventReference);

	if (!MeshComp)
	{
		return;
	}

	AActor* OwnerActor = MeshComp->GetOwner();
	if (!OwnerActor)
	{
		return;
	}

	if (!EventTag.IsValid())
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

FString UAN_SkillChainStep::GetNotifyName_Implementation() const
{
	if (EventTag.IsValid())
	{
		return FString::Printf(TEXT("SkillChainStep: %s"), *EventTag.ToString());
	}

	return TEXT("SkillChainStep");
}