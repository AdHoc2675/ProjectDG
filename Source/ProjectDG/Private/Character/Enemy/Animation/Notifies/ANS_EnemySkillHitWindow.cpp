#include "Character/Enemy/Animation/Notifies/ANS_EnemySkillHitWindow.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "Components/SkeletalMeshComponent.h"
#include "Core/DG_GameplayTags.h"
#include "GameFramework/Actor.h"

UANS_EnemySkillHitWindow::UANS_EnemySkillHitWindow()
{
	SkillHitEventTag = DGGameplayTags::Event_Attack_HitCheck;

#if WITH_EDITORONLY_DATA
	NotifyColor = FColor(255, 120, 40);
#endif
}

FString UANS_EnemySkillHitWindow::GetNotifyName_Implementation() const
{
	return TEXT("EnemySkillHitWindow");
}

void UANS_EnemySkillHitWindow::NotifyTick(
	USkeletalMeshComponent* MeshComp,
	UAnimSequenceBase* Animation,
	float FrameDeltaTime,
	const FAnimNotifyEventReference& EventReference
)
{
	Super::NotifyTick(MeshComp, Animation, FrameDeltaTime, EventReference);

	SendEnemySkillHitEvent(MeshComp);
}

void UANS_EnemySkillHitWindow::SendEnemySkillHitEvent(USkeletalMeshComponent* MeshComp) const
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