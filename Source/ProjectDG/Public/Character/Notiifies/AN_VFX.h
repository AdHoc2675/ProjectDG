#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotify.h"
#include "GameplayTagContainer.h"
#include "AN_VFX.generated.h"

/**
 * 스킬 VFX 출력용 공통 AnimNotify.
 *
 * 이 Notify는 직접 Niagara를 출력하지 않고,
 * CueEventTag를 GameplayEvent로 전송한다.
 *
 * 실제 VFX 출력은 PlayerSkillBase / EnemySkillBase가
 * 현재 SkillData의 VFX 필드를 읽어서 처리한다.
 */
UCLASS()
class PROJECTDG_API UAN_VFX : public UAnimNotify
{
	GENERATED_BODY()

public:
	UAN_VFX();

	virtual void Notify(
		USkeletalMeshComponent* MeshComp,
		UAnimSequenceBase* Animation,
		const FAnimNotifyEventReference& EventReference
	) override;

	virtual FString GetNotifyName_Implementation() const override;

protected:
	/** Event.Skill.VFX.* 계열 GameplayEvent 태그 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "SkillVFX", meta = (Categories = "Event.Skill.VFX"))
	FGameplayTag CueEventTag;
};