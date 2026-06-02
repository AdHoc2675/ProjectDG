// Fill out your copyright notice in the Description page of Project Settings.

#include "Character/Enemy/Field/ANS_EnemyAOETelegraphWindow.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "Components/SkeletalMeshComponent.h"
#include "Core/DG_GameplayTags.h"
#include "GameFramework/Actor.h"

UANS_EnemyAOETelegraphWindow::UANS_EnemyAOETelegraphWindow()
{
#if WITH_EDITORONLY_DATA
	NotifyColor = FColor(255, 160, 60);
#endif
}

FString UANS_EnemyAOETelegraphWindow::GetNotifyName_Implementation() const
{
	return TEXT("EnemyAOETelegraphWindow");
}

void UANS_EnemyAOETelegraphWindow::NotifyBegin(
	USkeletalMeshComponent* MeshComp,
	UAnimSequenceBase* Animation,
	float TotalDuration,
	const FAnimNotifyEventReference& EventReference
)
{
	Super::NotifyBegin(MeshComp, Animation, TotalDuration, EventReference);
	SendTelegraphEvent(MeshComp, DGGameplayTags::Event_Enemy_AOE_Telegraph_Begin.GetTag());
}

void UANS_EnemyAOETelegraphWindow::NotifyEnd(
	USkeletalMeshComponent* MeshComp,
	UAnimSequenceBase* Animation,
	const FAnimNotifyEventReference& EventReference
)
{
	Super::NotifyEnd(MeshComp, Animation, EventReference);
	SendTelegraphEvent(MeshComp, DGGameplayTags::Event_Enemy_AOE_Telegraph_End.GetTag());
}

void UANS_EnemyAOETelegraphWindow::SendTelegraphEvent(USkeletalMeshComponent* MeshComp, const FGameplayTag& EventTag) const
{
	if (!MeshComp || !EventTag.IsValid())
	{
		return;
	}

	AActor* OwnerActor = MeshComp->GetOwner();
	if (!OwnerActor || !OwnerActor->HasAuthority())
	{
		return;
	}

	FGameplayEventData Payload;
	Payload.EventTag = EventTag;
	Payload.Instigator = OwnerActor;
	Payload.Target = OwnerActor;

	UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(OwnerActor, EventTag, Payload);
}
