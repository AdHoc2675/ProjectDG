// Fill out your copyright notice in the Description page of Project Settings.

#include "Character/Enemy/Animation/Notifies/AN_EnemySkillHit.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "Components/SkeletalMeshComponent.h"
#include "Core/DG_GameplayTags.h"
#include "GameFramework/Actor.h"

UAN_EnemySkillHit::UAN_EnemySkillHit()
{
	SkillHitEventTag = DGGameplayTags::Event_Attack_HitCheck;

#if WITH_EDITORONLY_DATA
	NotifyColor = FColor(255, 80, 80);
#endif
}

FString UAN_EnemySkillHit::GetNotifyName_Implementation() const
{
	return TEXT("EnemySkillHit");
}

void UAN_EnemySkillHit::Notify(
	USkeletalMeshComponent* MeshComp,
	UAnimSequenceBase* Animation,
	const FAnimNotifyEventReference& EventReference
)
{
	Super::Notify(MeshComp, Animation, EventReference);

	SendEnemySkillHitEvent(MeshComp);
}

void UAN_EnemySkillHit::SendEnemySkillHitEvent(USkeletalMeshComponent* MeshComp) const
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

	UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(
		OwnerActor,
		SkillHitEventTag,
		Payload
	);
}