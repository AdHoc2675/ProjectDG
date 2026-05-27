#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "GameplayAbilitySpec.h"
#include "SkillDataAsset.generated.h"

class UGameplayAbility;

/**
 * 스킬의 정보, 거리 조건 및 가중치를 정의하는 데이터 에셋입니다.
 */
UCLASS(BlueprintType, Blueprintable)
class PROJECTDG_API USkillDataAsset : public UDataAsset
{
	GENERATED_BODY()

public:
	// 스킬 이름
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Skill")
	FName SkillName;

	// 실행할 Gameplay Ability 클래스
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Skill")
	TSubclassOf<UGameplayAbility> AbilityClass;

	// 이 스킬을 사용할 수 있는 타겟과의 최소 거리
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Skill|Condition")
	float MinRange = 0.f;

	// 이 스킬을 사용할 수 있는 타겟과의 최대 거리
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Skill|Condition")
	float MaxRange = 500.f;

	// 스킬이 랜덤하게 선택될 때의 가중치 (값이 클수록 선택될 확률이 높음)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Skill|Condition")
	float SelectionWeight = 1.f;
};
