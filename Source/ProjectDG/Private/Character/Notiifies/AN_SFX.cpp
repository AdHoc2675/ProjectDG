#include "Character/Notiifies/AN_SFX.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/Actor.h"

UAN_SFX::UAN_SFX()
{
#if WITH_EDITORONLY_DATA
	NotifyColor = FColor(255, 180, 80);
#endif
}

void UAN_SFX::Notify(
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

FString UAN_SFX::GetNotifyName_Implementation() const
{
	if (CueEventTag.IsValid())
	{
		return FString::Printf(
			TEXT("Skill SFX: %s"),
			*CueEventTag.ToString()
		);
	}

	return TEXT("Skill SFX");
}