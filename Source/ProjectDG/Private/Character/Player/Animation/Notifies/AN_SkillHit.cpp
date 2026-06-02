// Fill out your copyright notice in the Description page of Project Settings.

#include "Character/Player/Animation/Notifies/AN_SkillHit.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "Components/SkeletalMeshComponent.h"
#include "Core/DG_GameplayTags.h"
#include "Core/DG_Debug.h"
#include "GameFramework/Actor.h"

UAN_SkillHit::UAN_SkillHit()
{
	SkillHitEventTag = DGGameplayTags::Event_Attack_HitCheck;

#if WITH_EDITORONLY_DATA
	NotifyColor = FColor(255, 120, 80);
#endif
}

FString UAN_SkillHit::GetNotifyName_Implementation() const
{
	return TEXT("SkillHit");
}

void UAN_SkillHit::Notify(
	USkeletalMeshComponent* MeshComp,
	UAnimSequenceBase* Animation,
	const FAnimNotifyEventReference& EventReference
)
{
	Super::Notify(MeshComp, Animation, EventReference);

	SendSkillHitEvent(MeshComp);
}

void UAN_SkillHit::SendSkillHitEvent(USkeletalMeshComponent* MeshComp) const
{
	if (!MeshComp || !SkillHitEventTag.IsValid())
	{
		return;
	}

	AActor* OwnerActor = MeshComp->GetOwner();
	if (!OwnerActor)
	{
		return;
	}

	FGameplayEventData Payload;
	Payload.EventTag = SkillHitEventTag;
	Payload.Instigator = OwnerActor;
	Payload.Target = OwnerActor;
	Payload.EventMagnitude = EventMagnitude;


	UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(OwnerActor, SkillHitEventTag, Payload);
}
