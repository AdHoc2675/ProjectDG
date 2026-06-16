// Fill out your copyright notice in the Description page of Project Settings.

#include "Character/Enemy/Animation/Notifies/AN_BossSkillBranch.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Core/DG_Debug.h"
#include "GameplayEffectTypes.h"

UAN_BossSkillBranch::UAN_BossSkillBranch()
{
	BranchEventTag = FGameplayTag::RequestGameplayTag(
		FName(TEXT("Event.Enemy.SkillBranch")),
		false
	);
}

void UAN_BossSkillBranch::Notify(
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

	if (!BranchEventTag.IsValid())
	{
		
		return;
	}

	UAbilitySystemComponent* ASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(OwnerActor);
	if (!ASC)
	{
		
		return;
	}

	FGameplayEventData Payload;
	Payload.EventTag = BranchEventTag;
	Payload.Instigator = OwnerActor;
	Payload.Target = OwnerActor;
	Payload.EventMagnitude = static_cast<float>(BranchStepIndex);

	ASC->HandleGameplayEvent(
		BranchEventTag,
		&Payload
	);

	
}

FString UAN_BossSkillBranch::GetNotifyName_Implementation() const
{
	return FString::Printf(TEXT("BossSkillBranch_%d"), BranchStepIndex);
}