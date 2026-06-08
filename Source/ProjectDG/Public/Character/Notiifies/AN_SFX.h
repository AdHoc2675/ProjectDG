#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotify.h"
#include "GameplayTagContainer.h"
#include "AN_SFX.generated.h"

/**
 * 스킬 SFX 출력용 공통 AnimNotify.
 *
 * 이 Notify는 직접 Sound를 출력하지 않고,
 * CueEventTag를 GameplayEvent로 전송한다.
 *
 * 실제 SFX 출력은 PlayerSkillBase / EnemySkillBase가
 * 현재 SkillData의 SFX 필드를 읽어서 처리한다.
 */
UCLASS()
class PROJECTDG_API UAN_SFX : public UAnimNotify
{
	GENERATED_BODY()

public:
	UAN_SFX();

	virtual void Notify(
		USkeletalMeshComponent* MeshComp,
		UAnimSequenceBase* Animation,
		const FAnimNotifyEventReference& EventReference
	) override;

	virtual FString GetNotifyName_Implementation() const override;

protected:
	/** Event.Skill.SFX.* 계열 GameplayEvent 태그 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "SkillSFX", meta = (Categories = "Event.Skill.SFX"))
	FGameplayTag CueEventTag;
};