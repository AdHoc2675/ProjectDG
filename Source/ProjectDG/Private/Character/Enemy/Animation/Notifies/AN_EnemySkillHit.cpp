// Fill out your copyright notice in the Description page of Project Settings.

#include "Character/Enemy/Animation/Notifies/AN_EnemySkillHit.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "Components/SkeletalMeshComponent.h"
#include "Core/DG_GameplayTags.h"
#include "GameFramework/Actor.h"

UAN_EnemySkillHit::UAN_EnemySkillHit()
{
	SkillHitEventTag = DGGameplayTags::Event_Attack_HitCheck;
	StepIndex = INDEX_NONE;

#if WITH_EDITORONLY_DATA
	NotifyColor = FColor(255, 80, 80);
#endif
}

FString UAN_EnemySkillHit::GetNotifyName_Implementation() const
{
	if (StepIndex >= 0)
	{
		return FString::Printf(TEXT("EnemySkillHit Step %d"), StepIndex);
	}

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

	// GA_EnemySkillBase에서 StepIndex로 사용.
	// INDEX_NONE(-1)이면 AN 순서 기반 자동 진행.
	Payload.EventMagnitude = static_cast<float>(StepIndex);

	UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(
		OwnerActor,
		SkillHitEventTag,
		Payload
	);
}