// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "PlayerCharacterMovementData.generated.h"

class UAnimMontage;

/**
 * 
 */
UCLASS(BlueprintType)
class PROJECTDG_API UPlayerCharacterMovementData : public UDataAsset
{
	GENERATED_BODY()
	
public:
    /**
     * 기본 이동 속도 -> 이동 속도는 스킬이나 버프/디버프의 영향을 받을 수 있으므로 추후 AttributeSet에서 관리
     * CharacterMovementComponent::MaxWalkSpeed에 적용된다.
     */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Movement", meta = (ClampMin = "0.0"))
    float WalkSpeed = 500.f;

    /**
     * 질주 중 이동 속도. -> 이동 속도는 스킬이나 버프/디버프의 영향을 받을 수 있으므로 추후 AttributeSet에서 관리
     * bIsSprinting 상태일 때 CharacterMovementComponent::MaxWalkSpeed에 적용된다.
     */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Movement", meta = (ClampMin = "0.0"))
    float SprintSpeed = 800.f;

    /**
     * 이동 방향으로 캐릭터가 회전하는 속도.
     * CharacterMovementComponent::RotationRate에 적용된다.
     */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Movement")
    FRotator RotationRate = FRotator(0.f, 720.f, 0.f);

    /**
     * 점프 초기 속도.
     * CharacterMovementComponent::JumpZVelocity에 적용된다.
     */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Movement", meta = (ClampMin = "0.0"))
    float JumpZVelocity = 700.f;

    /**
     * 공중 제어력.
     * CharacterMovementComponent::AirControl에 적용된다.
     */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Movement", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float AirControl = 0.35f;

    /**
     * 회피 시 LaunchCharacter에 곱해질 힘.
     */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Dodge", meta = (ClampMin = "0.0"))
    float DodgeStrength = 900.f;

    /**
     * 회피 상태 유지 시간.
     * 이 시간이 지나면 bIsDodging을 false로 되돌린다.
     */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Dodge", meta = (ClampMin = "0.0"))
    float DodgeDuration = 0.25f;

    /**
     * 회피 1회 사용 시 소모할 스태미나.
     * 현재는 PlayerCharacterBase에서 직접 차감하고,
     * 추후 GE_Cost_DodgeStamina 같은 GameplayEffect로 대체할 예정.
     */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Dodge", meta = (ClampMin = "0.0"))
    float DodgeStaminaCost = 5.f;

    /**
     * 질주 중 초당 소모할 스태미나.
     * 현재는 PlayerCharacterBase에서 직접 차감하고,
     * 추후 GE_Cost_SprintStaminaTick 같은 GameplayEffect로 대체할 예정.
     */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Sprint", meta = (ClampMin = "0.0"))
    float SprintStaminaCostPerSecond = 15.f;
	
	
};
