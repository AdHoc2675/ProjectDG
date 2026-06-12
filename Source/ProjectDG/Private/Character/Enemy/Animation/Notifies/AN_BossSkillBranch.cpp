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

	Debug::Print(
		FString::Printf(TEXT("[AN_BossSkillBranch] Notify Fired Step=%d"), BranchStepIndex),
		FColor::Cyan
	);

	if (!MeshComp)
	{
		Debug::Print(TEXT("[AN_BossSkillBranch] MeshComp Invalid"), FColor::Red);
		return;
	}

	AActor* OwnerActor = MeshComp->GetOwner();
	if (!OwnerActor)
	{
		Debug::Print(TEXT("[AN_BossSkillBranch] OwnerActor Invalid"), FColor::Red);
		return;
	}

	if (!BranchEventTag.IsValid())
	{
		Debug::Print(TEXT("[AN_BossSkillBranch] BranchEventTag Invalid"), FColor::Red);
		return;
	}

	UAbilitySystemComponent* ASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(OwnerActor);
	if (!ASC)
	{
		Debug::Print(
			FString::Printf(
				TEXT("[AN_BossSkillBranch] ASC Invalid Owner=%s"),
				*OwnerActor->GetName()
			),
			FColor::Red
		);
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

	Debug::Print(
		FString::Printf(
			TEXT("[AN_BossSkillBranch] Event Sent Tag=%s Step=%d Owner=%s"),
			*BranchEventTag.ToString(),
			BranchStepIndex,
			*OwnerActor->GetName()
		),
		FColor::Green
	);
}

FString UAN_BossSkillBranch::GetNotifyName_Implementation() const
{
	return FString::Printf(TEXT("BossSkillBranch_%d"), BranchStepIndex);
}