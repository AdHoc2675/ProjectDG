// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/Player/Animation/Notifies/ANS_ComboInputWindow.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "Components/SkeletalMeshComponent.h"
#include "Core/DG_GameplayTags.h"
#include "GameFramework/Actor.h"

UANS_ComboInputWindow::UANS_ComboInputWindow()
{
	WindowOpenEventTag = DGGameplayTags::Event_Combo_InputWindow_Open;
	WindowCloseEventTag = DGGameplayTags::Event_Combo_InputWindow_Close;
	
	#if WITH_EDITORONLY_DATA
	NotifyColor = FColor(80, 180, 255);
	#endif
}

FString UANS_ComboInputWindow::GetNotifyName_Implementation() const
{
	return TEXT("ComboInputWindow");
}

void UANS_ComboInputWindow::NotifyBegin(USkeletalMeshComponent* MeshComp,UAnimSequenceBase* Animation,float TotalDuration,const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyBegin(MeshComp, Animation, TotalDuration, EventReference);

	SendComboInputWindowEvent(MeshComp, WindowOpenEventTag, TotalDuration);
}

void UANS_ComboInputWindow::NotifyEnd(USkeletalMeshComponent* MeshComp,UAnimSequenceBase* Animation,const FAnimNotifyEventReference& EventReference)
{
  	Super::NotifyEnd(MeshComp, Animation, EventReference);

  	SendComboInputWindowEvent(MeshComp, WindowCloseEventTag, 0.f);
}

void UANS_ComboInputWindow::SendComboInputWindowEvent(USkeletalMeshComponent* MeshComp,const FGameplayTag& EventTag,float EventMagnitude) const
{
  	if (!MeshComp || !EventTag.IsValid())
  	{
  		return;
  	}

  	AActor* OwnerActor = MeshComp->GetOwner();
  	if (!OwnerActor)
  	{
  		return;
  	}

  	FGameplayEventData Payload;
  	Payload.EventTag = EventTag;
  	Payload.Instigator = OwnerActor;
  	Payload.Target = OwnerActor;
  	Payload.EventMagnitude = EventMagnitude;

  	UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(OwnerActor, EventTag, Payload);
}