// Fill out your copyright notice in the Description page of Project Settings.

#include "Character/Enemy/Animation/Notifies/AN_SkillIndicator.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "Components/SkeletalMeshComponent.h"
#include "Core/DG_Debug.h"
#include "Core/DG_GameplayTags.h"
#include "GameFramework/Actor.h"

UAN_SkillIndicator::UAN_SkillIndicator()
{
	EventTag = DGGameplayTags::Event_Boss_Indicator;
	StepIndex = INDEX_NONE;
	bPrintDebug = false;
}

void UAN_SkillIndicator::Notify(
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

	// StepIndex 전달용.
	// GA_EnemySkillBase에서 Payload.EventMagnitude를 int32로 변환해서 사용.
	Payload.EventMagnitude = static_cast<float>(StepIndex);

	UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(
		OwnerActor,
		EventTag,
		Payload
	);

}

FString UAN_SkillIndicator::GetNotifyName_Implementation() const
{
	return FString::Printf(
		TEXT("SkillIndicator Step %d"),
		StepIndex
	);
}