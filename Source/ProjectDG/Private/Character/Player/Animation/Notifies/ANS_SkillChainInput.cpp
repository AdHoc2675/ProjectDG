// Fill out your copyright notice in the Description page of Project Settings.

#include "Character/Player/Animation/Notifies/ANS_SkillChainInput.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "Components/SkeletalMeshComponent.h"
#include "Core/DG_GameplayTags.h"
#include "GameFramework/Actor.h"

UANS_SkillChainInput::UANS_SkillChainInput()
{
	OpenEventTag = DGGameplayTags::Event_Skill_ChainInput_Open;
	CloseEventTag = DGGameplayTags::Event_Skill_ChainInput_Close;

#if WITH_EDITORONLY_DATA
	NotifyColor = FColor(120, 220, 120);
#endif
}

void UANS_SkillChainInput::NotifyBegin(
	USkeletalMeshComponent* MeshComp,
	UAnimSequenceBase* Animation,
	float TotalDuration,
	const FAnimNotifyEventReference& EventReference
)
{
	Super::NotifyBegin(MeshComp, Animation, TotalDuration, EventReference);

	SendSkillChainInputEvent(MeshComp, OpenEventTag);
}

void UANS_SkillChainInput::NotifyEnd(
	USkeletalMeshComponent* MeshComp,
	UAnimSequenceBase* Animation,
	const FAnimNotifyEventReference& EventReference
)
{
	Super::NotifyEnd(MeshComp, Animation, EventReference);

	SendSkillChainInputEvent(MeshComp, CloseEventTag);
}

void UANS_SkillChainInput::SendSkillChainInputEvent(
	USkeletalMeshComponent* MeshComp,
	FGameplayTag EventTag
) const
{
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

FString UANS_SkillChainInput::GetNotifyName_Implementation() const
{
	return TEXT("SkillChainInput");
}