// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/Player/Animation/Notifies/AN_ComboBranch.h"

#include "AbilitySystemBlueprintLibrary.h"
 #include "Components/SkeletalMeshComponent.h"
 #include "Core/DG_GameplayTags.h"
 #include "GameFramework/Actor.h"

UAN_ComboBranch::UAN_ComboBranch()
{
 	BranchEventTag = DGGameplayTags::Event_Combo_Branch;

#if WITH_EDITORONLY_DATA
 	NotifyColor = FColor(255, 180, 60);
#endif
}

FString UAN_ComboBranch::GetNotifyName_Implementation() const
{
	return TEXT("ComboBranch");
}

void UAN_ComboBranch::Notify(USkeletalMeshComponent* MeshComp,UAnimSequenceBase* Animation,const FAnimNotifyEventReference& EventReference)
{
 	Super::Notify(MeshComp, Animation, EventReference);

 	SendComboBranchEvent(MeshComp);
}

void UAN_ComboBranch::SendComboBranchEvent(USkeletalMeshComponent* MeshComp) const
{
 	if (!MeshComp || !BranchEventTag.IsValid())
 	{
 		return;
 	}

 	AActor* OwnerActor = MeshComp->GetOwner();
 	if (!OwnerActor)
 	{
 		return;
 	}

 	FGameplayEventData Payload;
 	Payload.EventTag = BranchEventTag;
 	Payload.Instigator = OwnerActor;
 	Payload.Target = OwnerActor;
	Payload.EventMagnitude = static_cast<float>(BranchComboIndex);

 	UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(OwnerActor, BranchEventTag, Payload);
}