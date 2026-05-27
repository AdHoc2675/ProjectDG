// Fill out your copyright notice in the Description page of Project Settings.

#include "GAS/Abilities/Player/Archer/GA_Archer_AimedArrow.h"

#include "Character/BaseCharacter.h"
#include "Character/Player/PlayerCharacterBase.h"
#include "Components/Targeting/LockOnComponent.h"
#include "Core/DG_GameplayTags.h"
#include "Core/DG_Debug.h"

UGA_Archer_AimedArrow::UGA_Archer_AimedArrow()
{
	AbilityTags.AddTag(DGGameplayTags::Skill_Archer_AimedArrow);
	ActivationOwnedTags.AddTag(DGGameplayTags::State_Skill_Archer_AimedArrow_Active);
}

void UGA_Archer_AimedArrow::ExecuteChargedSkill(int32 ChargeLevel, float ChargeTime)
{
	if (!HasAuthorityAvatar())
	{
		return;
	}

	AActor* TargetActor = nullptr;
	FVector AimPoint = FVector::ZeroVector;

	if (!TryAcquireAimedTarget(TargetActor, AimPoint))
	{
		return;
	}

	ApplyDamageToTarget(
		TargetActor,
		0.f,
		GetAimedArrowDamageMultiplier(TargetActor, ChargeLevel),
		GetSkillTag(),
		AimPoint,
		true
	);
	
	Debug::Print(FString::Printf(
	TEXT("[Archer_AimedArrow] Damage Applied. Target=%s Level=%d"),
	*GetNameSafe(TargetActor),
	ChargeLevel
));

	// 후속 작업:
	// - 그로기 대상이면 추가 피해 +20%
	// - GroggyDamage 10 적용
	// - 필요 시 State.Groggy / Attribute / CombatComponent 파이프라인 확인 후 연결
}

bool UGA_Archer_AimedArrow::TryAcquireAimedTarget(AActor*& OutTargetActor, FVector& OutAimPoint) const
{
	OutTargetActor = nullptr;
	OutAimPoint = FVector::ZeroVector;

	const APlayerCharacterBase* PlayerCharacter = GetAvatarPlayerCharacter();
	if (!PlayerCharacter)
	{
		return false;
	}

	ULockOnComponent* LockOnComponent = PlayerCharacter->GetLockOnComponent();
	if (!LockOnComponent)
	{
		return false;
	}

	const float SkillRange = GetSkillRange();

	FLockOnTargetResult LockOnResult;
	if (LockOnComponent->TryGetLockedTargetResult(LockOnResult))
	{
		if (IsAimedTargetValid(LockOnResult.TargetActor))
		{
			OutTargetActor = LockOnResult.TargetActor;
			OutAimPoint = LockOnResult.AimPoint;
			return true;
		}
	}

	FGameplayTagContainer RequiredTags;
	RequiredTags.AddTag(DGGameplayTags::Team_Enemy.GetTag());

	if (LockOnComponent->FindBestTargetByTags(RequiredTags, SkillRange, LockOnResult))
	{
		if (IsAimedTargetValid(LockOnResult.TargetActor))
		{
			OutTargetActor = LockOnResult.TargetActor;
			OutAimPoint = LockOnResult.AimPoint;
			return true;
		}
	}

	return false;
}

bool UGA_Archer_AimedArrow::IsAimedTargetValid(AActor* TargetActor) const
{
	if (!TargetActor)
	{
		return false;
	}

	const AActor* AvatarActor = GetAvatarActorFromAbility();
	if (!AvatarActor || AvatarActor == TargetActor)
	{
		return false;
	}

	const ABaseCharacter* TargetCharacter = Cast<ABaseCharacter>(TargetActor);
	if (!TargetCharacter || TargetCharacter->IsDead())
	{
		return false;
	}

	if (!TargetCharacter->HasTeamTag(DGGameplayTags::Team_Enemy.GetTag()))
	{
		return false;
	}

	const float SkillRange = GetSkillRange();
	if (SkillRange <= 0.f)
	{
		return true;
	}

	const float DistanceSq = FVector::DistSquared2D(
		AvatarActor->GetActorLocation(),
		TargetActor->GetActorLocation()
	);

	return DistanceSq <= FMath::Square(SkillRange);
}

float UGA_Archer_AimedArrow::GetAimedArrowDamageMultiplier(
	AActor* TargetActor,
	int32 ChargeLevel
) const
{
	float DamageMultiplier = GetSkillDamageMultiplier();

	// 후속 작업:
	// - TargetActor가 그로기 상태인지 확인
	// - 그로기 상태라면 DamageMultiplier *= 1.2f
	// - 차지 단계별 데미지 계수가 필요하면 여기에서 ChargeLevel 기준으로 보정

	return DamageMultiplier;
}