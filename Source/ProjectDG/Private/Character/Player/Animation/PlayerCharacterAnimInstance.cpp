// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/Player/Animation/PlayerCharacterAnimInstance.h"
#include "Character/Player/PlayerCharacterBase.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "AbilitySystemComponent.h"
#include "Core/DG_GameplayTags.h"

void UPlayerCharacterAnimInstance::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();
	
	// 소유한 폰을 PlayerCharacterBase로 캐스팅하여 저장
	PlayerCharacter = Cast<APlayerCharacterBase>(TryGetPawnOwner());
	if (PlayerCharacter)
	{
		PlayerMovement = PlayerCharacter->GetCharacterMovement();
	}
}

void UPlayerCharacterAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeUpdateAnimation(DeltaSeconds);
	
	// 캐릭터 참조가 유효하지 않으면 재시도
	if (!PlayerCharacter)
	{
		PlayerCharacter = Cast<APlayerCharacterBase>(TryGetPawnOwner());
		if (PlayerCharacter)
		{
			PlayerMovement = PlayerCharacter->GetCharacterMovement();
		}
	}

	if (!PlayerCharacter || !PlayerMovement) return;

	// 이동 속도 업데이트 (Z축 제외 수평 속도)
	GroundSpeed = PlayerCharacter->GetVelocity().Size2D();
	
	// 가속 여부 업데이트
	bIsAccelerating = PlayerMovement->GetCurrentAcceleration().Size() > 0.f;

	// 공중 상태 업데이트
	bIsFalling = PlayerMovement->IsFalling();

	// blendspace 달리기 변수 업데이트
	const FVector WorldVelocity = PlayerCharacter->GetVelocity();
	const FVector LocalVelocity =
	PlayerCharacter->GetActorTransform().InverseTransformVectorNoScale(WorldVelocity);

	const float MaxReferenceSpeed = FMath::Max(PlayerMovement->GetMaxSpeed(), 1.f);

	MoveForward = FMath::Clamp(LocalVelocity.X / MaxReferenceSpeed, -1.f, 1.f);
	MoveRight = FMath::Clamp(LocalVelocity.Y / MaxReferenceSpeed, -1.f, 1.f);
	
	MeleeTwist = GetCurveValue(TEXT("meleetwist"));

	const float SafeAngleForFullBias = FMath::Max(MeleeTwistAngleForFullBias, 1.f);
	NormalizedMeleeTwist = FMath::Clamp(MeleeTwist / SafeAngleForFullBias, -1.f, 1.f);

	SmoothedMeleeTwist = FMath::FInterpTo(
			SmoothedMeleeTwist,
			NormalizedMeleeTwist,
			DeltaSeconds,
			MeleeTwistInterpSpeed
	);

	FinalMoveForward = MoveForward;
	FinalMoveRight = FMath::Clamp(
			MoveRight + (SmoothedMeleeTwist * MeleeTwistScale),
			-1.f,
			1.f
	);
	
	float TargetRunBlendSpacePlayRate = 1.f;

	if (UAbilitySystemComponent* ASC = PlayerCharacter->GetCharacterAbilitySystemComponent())
	{
		const bool bIsSharpStrikeActive =
				ASC->HasMatchingGameplayTag(DGGameplayTags::State_Skill_Warrior_SharpStrike_Active.GetTag());

		if (bIsSharpStrikeActive)
		{
			TargetRunBlendSpacePlayRate = SharpStrikeRunBlendSpacePlayRate;
		}
	}

	CurrentRunBlendSpacePlayRate = FMath::FInterpTo(
			CurrentRunBlendSpacePlayRate,
			TargetRunBlendSpacePlayRate,
			DeltaSeconds,
			RunBlendSpacePlayRateInterpSpeed
	);

	RunBlendSpacePlayRate = CurrentRunBlendSpacePlayRate;
	
	if (FMath::Abs(MeleeTwist) > KINDA_SMALL_NUMBER)
	{
		UE_LOG(LogTemp, Log, TEXT("MeleeTwist=%.2f Normalized=%.2f FinalMoveRight=%.2f"),
				MeleeTwist,
				NormalizedMeleeTwist,
				FinalMoveRight);
	}
}
