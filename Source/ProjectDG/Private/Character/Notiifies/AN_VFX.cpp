#include "Character/Notiifies/AN_VFX.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/Actor.h"

UAN_VFX::UAN_VFX()
{
#if WITH_EDITORONLY_DATA
	NotifyColor = FColor(80, 180, 255);
#endif
}

void UAN_VFX::Notify(
	USkeletalMeshComponent* MeshComp,
	UAnimSequenceBase* Animation,
	const FAnimNotifyEventReference& EventReference
)
{
	Super::Notify(MeshComp, Animation, EventReference);

	if (!MeshComp || !CueEventTag.IsValid())
	{
		return;
	}

	AActor* OwnerActor = MeshComp->GetOwner();
	if (!OwnerActor)
	{
		return;
	}

	FGameplayEventData Payload;
	Payload.EventTag = CueEventTag;
	Payload.Instigator = OwnerActor;
	Payload.Target = OwnerActor;
	Payload.OptionalObject = Animation;

	UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(
		OwnerActor,
		CueEventTag,
		Payload
	);
}

FString UAN_VFX::GetNotifyName_Implementation() const
{
	if (CueEventTag.IsValid())
	{
		return FString::Printf(
			TEXT("Skill VFX: %s"),
			*CueEventTag.ToString()
		);
	}

	return TEXT("Skill VFX");
}