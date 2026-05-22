#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Animation/AnimNotifies/AnimNotifyState.h"
#include "ANS_SendGameplayEventWindow.generated.h"

/**
 * AnimNotifyState 구간 시작 시 GameplayEvent를 보낸다.
 *
 * EventMagnitude에는 NotifyState의 TotalDuration이 들어간다.
 * GA에서는 이 값을 RootMotionMoveToForce Duration으로 사용할 수 있다.
 */
UCLASS()
class PROJECTDG_API UANS_SendGameplayEventWindow : public UAnimNotifyState
{
	GENERATED_BODY()

public:
	UANS_SendGameplayEventWindow();

	virtual FString GetNotifyName_Implementation() const override;

	virtual void NotifyBegin(
			USkeletalMeshComponent* MeshComp,
			UAnimSequenceBase* Animation,
			float TotalDuration,
			const FAnimNotifyEventReference& EventReference
	) override;

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "GameplayEvent")
	FGameplayTag EventTag;

	/**
	 * true면 서버에서만 이벤트를 보낸다.
	 * 이동 예측용 이벤트라면 false 유지 권장.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "GameplayEvent")
	bool bServerOnly = false;
};
