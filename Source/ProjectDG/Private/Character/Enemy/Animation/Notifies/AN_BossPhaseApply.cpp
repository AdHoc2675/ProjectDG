// Fill out your copyright notice in the Description page of Project Settings.

#include "Character/Enemy/Animation/Notifies/AN_BossPhaseApply.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "Animation/AnimSequenceBase.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/Actor.h"

UAN_BossPhaseApply::UAN_BossPhaseApply()
{
	PhaseApplyEventTag = FGameplayTag::RequestGameplayTag(
		FName(TEXT("Event.Boss.PhaseApply")),
		false
	);

	TargetPhaseIndex = 2;
}

void UAN_BossPhaseApply::Notify(
	USkeletalMeshComponent* MeshComp,
	UAnimSequenceBase* Animation,
	const FAnimNotifyEventReference& EventReference
)
{
	Super::Notify(
		MeshComp,
		Animation,
		EventReference
	);

	if (!MeshComp)
	{
		UE_LOG(
			LogTemp,
			Warning,
			TEXT("[AN_BossPhaseApply] MeshComp Invalid")
		);
		return;
	}

	AActor* OwnerActor = MeshComp->GetOwner();
	if (!OwnerActor)
	{
		UE_LOG(
			LogTemp,
			Warning,
			TEXT("[AN_BossPhaseApply] OwnerActor Invalid")
		);
		return;
	}

	// 중요:
	// PhaseTransition GA는 ServerOnly이고 Pending Phase 적용도 서버에서만 해야 한다.
	// 클라에서도 Multicast Montage가 재생되면서 Notify가 발동할 수 있으므로,
	// 클라에서는 이벤트를 보내지 않는다.
	if (!OwnerActor->HasAuthority())
	{
		UE_LOG(
			LogTemp,
			Verbose,
			TEXT("[AN_BossPhaseApply] Ignored on client. Owner=%s"),
			*OwnerActor->GetName()
		);
		return;
	}

	if (!PhaseApplyEventTag.IsValid())
	{
		PhaseApplyEventTag = FGameplayTag::RequestGameplayTag(
			FName(TEXT("Event.Boss.PhaseApply")),
			false
		);
	}

	if (!PhaseApplyEventTag.IsValid())
	{
		UE_LOG(
			LogTemp,
			Warning,
			TEXT("[AN_BossPhaseApply] PhaseApplyEventTag Invalid")
		);
		return;
	}

	FGameplayEventData Payload;
	Payload.EventTag = PhaseApplyEventTag;
	Payload.Instigator = OwnerActor;
	Payload.Target = OwnerActor;
	Payload.EventMagnitude = static_cast<float>(TargetPhaseIndex);

	UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(
		OwnerActor,
		PhaseApplyEventTag,
		Payload
	);

	UE_LOG(
		LogTemp,
		Warning,
		TEXT("[AN_BossPhaseApply] Event Sent Tag=%s TargetPhase=%d Owner=%s"),
		*PhaseApplyEventTag.ToString(),
		TargetPhaseIndex,
		*OwnerActor->GetName()
	);
}

FString UAN_BossPhaseApply::GetNotifyName_Implementation() const
{
	return TEXT("Boss Phase Apply");
}