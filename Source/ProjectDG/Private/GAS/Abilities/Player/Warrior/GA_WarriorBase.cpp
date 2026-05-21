// Fill out your copyright notice in the Description page of Project Settings.


#include "GAS/Abilities/Player/Warrior/GA_WarriorBase.h"

#include "Character/Player/PlayerCharacterBase.h"


bool UGA_WarriorBase::IsWarriorSkillInputHeld(FGameplayTag SkillTag) const
{
	const APlayerCharacterBase* PlayerCharacter = Cast<APlayerCharacterBase>(GetAvatarActorFromActorInfo());
	if (!PlayerCharacter)
	{
		return false;
	}

	return PlayerCharacter->IsSkillTagHeld(SkillTag);
}

bool UGA_WarriorBase::IsAuthorityAvatar() const
{
	const AActor* AvatarActor = GetAvatarActorFromActorInfo();
	return AvatarActor && AvatarActor->HasAuthority();
}

AActor* UGA_WarriorBase::GetPayloadTargetActor(const FGameplayEventData& Payload) const
{
	return const_cast<AActor*>(Payload.Target.Get());
}

FVector UGA_WarriorBase::GetPayloadHitLocationOrActorLocation(const FGameplayEventData& Payload,AActor* FallbackActor) const
{
	if (Payload.TargetData.Num() > 0)
	{
		return Payload.TargetData.Get(0)->GetEndPoint();
	}

	return FallbackActor ? FallbackActor->GetActorLocation() : FVector::ZeroVector;
}

