#pragma once

#include "CoreMinimal.h"
#include "Character/Player/Animation/PlayerCharacterAnimInstance.h"
#include "GameplayTagContainer.h"
#include "AssassinAnimInstance.generated.h"

/**
 * 암살자 전용 AnimInstance.
 * 공통 이동 / meleetwist 값은 UPlayerCharacterAnimInstance에서 계산하고,
 * 암살자 이동 가능 공격 스킬의 상체 블렌딩 조건만 여기서 확장한다.
 */
UCLASS()
class PROJECTDG_API UAssassinAnimInstance : public UPlayerCharacterAnimInstance
{
	GENERATED_BODY()

public:
	virtual void NativeUpdateAnimation(float DeltaSeconds) override;

protected:
	UPROPERTY(BlueprintReadOnly, Category = "Assassin|MovingAttack")
	bool bIsAssassinMovingAttackActive = false;

	UPROPERTY(BlueprintReadOnly, Category = "Assassin|MovingAttack")
	bool bUseAssassinMovingAttackUpperBody = false;
	
	UPROPERTY(BlueprintReadOnly, Category = "Assassin|MovingAttack")
	bool bIsAssassinMeleeTwistCorrectionActive = false;

	UPROPERTY(BlueprintReadOnly, Category = "Assassin|MovingAttack")
	bool bUseAssassinMeleeTwistCorrection = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Assassin|MovingAttack")
	float AssassinMovingAttackThreshold = 10.f;
	
	UPROPERTY(BlueprintReadOnly, Category = "Assassin|MovingAttack")
	float AssassinMovingAttackUpperBodyAlpha = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Assassin|MovingAttack")
	float AssassinMovingAttackUpperBodyBlendInterpSpeed = 12.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Assassin|MovingAttack")
	FGameplayTagContainer MovingAttackStateTags;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Assassin|MovingAttack")
	FGameplayTagContainer MeleeTwistCorrectionStateTags;
	
	UPROPERTY(BlueprintReadOnly, Category = "Assassin|MovingAttack")
	float MovingAttackFullBodyLockCurveValue = 0.f;

	UPROPERTY(BlueprintReadOnly, Category = "Assassin|MovingAttack")
	bool bIsMovingAttackFullBodyLockedByCurve = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Assassin|MovingAttack")
	FName MovingAttackFullBodyLockCurveName = TEXT("MovingAttackFullBodyLock");

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Assassin|MovingAttack")
	float MovingAttackFullBodyLockThreshold = 0.5f;
};