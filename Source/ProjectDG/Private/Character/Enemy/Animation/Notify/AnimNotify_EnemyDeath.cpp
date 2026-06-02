// Fill out your copyright notice in the Description page of Project Settings.

#include "Character/Enemy/Animation/Notify/AnimNotify_EnemyDeath.h"
#include "Character/Enemy/Field/FieldEnemyBase.h"

void UAnimNotify_EnemyDeath::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
	Super::Notify(MeshComp, Animation, EventReference);

	if (MeshComp && MeshComp->GetOwner())
	{
		if (AFieldEnemyBase* FieldEnemy = Cast<AFieldEnemyBase>(MeshComp->GetOwner()))
		{
			FieldEnemy->OnDeathAnimationFinished();
		}
	}
}
