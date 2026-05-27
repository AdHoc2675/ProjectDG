#include "AI/Tasks/BTTask_UseRandomSkill.h"
#include "Data/SkillDataAsset.h"
#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemBlueprintLibrary.h"

UBTTask_UseRandomSkill::UBTTask_UseRandomSkill()
{
	NodeName = "Use Random Skill (GAS)";
	bNotifyTaskFinished = true;
}

EBTNodeResult::Type UBTTask_UseRandomSkill::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	AAIController* AIController = OwnerComp.GetAIOwner();
	UBlackboardComponent* BlackboardComp = OwnerComp.GetBlackboardComponent();

	if (!AIController || !BlackboardComp)
		return EBTNodeResult::Failed;

	APawn* ControlledPawn = AIController->GetPawn();
	if (!ControlledPawn)
		return EBTNodeResult::Failed;

	// 타겟과 ASC 가져오기
	AActor* TargetActor = Cast<AActor>(BlackboardComp->GetValueAsObject(TargetKey.SelectedKeyName));
	UAbilitySystemComponent* ASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(ControlledPawn);

	if (!ASC || !TargetActor)
		return EBTNodeResult::Failed;

	// 거리 계산
	float DistanceToTarget = ControlledPawn->GetDistanceTo(TargetActor);

	// 이전 스킬 가져오기
	USkillDataAsset* LastUsedSkill = Cast<USkillDataAsset>(BlackboardComp->GetValueAsObject(LastUsedSkillKey.SelectedKeyName));

	// 1. 거리 필터링 및 중복 제외
	TArray<USkillDataAsset*> ValidSkills;
	float TotalWeight = 0.f;

	for (USkillDataAsset* SkillAsset : AvailableSkills)
	{
		if (!SkillAsset || !SkillAsset->AbilityClass)
			continue;

		// 중복 방지
		if (SkillAsset == LastUsedSkill)
			continue;

		// 거리 제약
		if (DistanceToTarget >= SkillAsset->MinRange && DistanceToTarget <= SkillAsset->MaxRange)
		{
			// (선택사항) 여기서 ASC->CanActivateAbility 등을 추가로 체크할 수도 있습니다.
			
			ValidSkills.Add(SkillAsset);
			TotalWeight += SkillAsset->SelectionWeight;
		}
	}

	if (ValidSkills.Num() == 0)
	{
		// 조건에 맞는 스킬이 없으면 실패 (BT가 다른 행동을 하도록 유도)
		return EBTNodeResult::Failed;
	}

	// 2. 가중치 기반 랜덤 선택
	float RandomValue = FMath::FRandRange(0.f, TotalWeight);
	USkillDataAsset* SelectedSkill = nullptr;

	float CurrentWeightSum = 0.f;
	for (USkillDataAsset* SkillAsset : ValidSkills)
	{
		CurrentWeightSum += SkillAsset->SelectionWeight;
		if (RandomValue <= CurrentWeightSum)
		{
			SelectedSkill = SkillAsset;
			break;
		}
	}

	// 오차로 인해 못 뽑았을 경우 대비
	if (!SelectedSkill)
	{
		SelectedSkill = ValidSkills.Last();
	}

	// 3. 블랙보드 업데이트
	BlackboardComp->SetValueAsObject(LastUsedSkillKey.SelectedKeyName, SelectedSkill);

	// 4. 어빌리티 시전 및 델리게이트 바인딩
	if (ASC->TryActivateAbilityByClass(SelectedSkill->AbilityClass))
	{
		// 시전된 어빌리티를 찾기 위해 (다수 동시 실행 대비)
		// 단순 구현을 위해 ASC의 모든 활성화된 어빌리티를 검사하는 방식은 비효율적일 수 있으므로
		// AbilityEnded 델리게이트를 통해 종료 시점을 캐치합니다.
		
		CachedOwnerComp = &OwnerComp;
		CachedASC = ASC;

		AbilityEndedDelegateHandle = ASC->OnAbilityEnded.AddUObject(this, &UBTTask_UseRandomSkill::OnAbilityEnded);
		
		return EBTNodeResult::InProgress;
	}

	return EBTNodeResult::Failed;
}

void UBTTask_UseRandomSkill::OnAbilityEnded(const FAbilityEndedData& AbilityEndedData)
{
	// 종료된 어빌리티가 우리가 실행한 스킬 클래스인지 확인 (더 엄격하게 하려면 핸들이나 인스턴스로 확인)
	// 하지만 일반적으로 보스급 AI는 한 번에 하나의 공격 GA만 실행하므로 단순화 가능합니다.
	if (CachedASC.IsValid() && CachedOwnerComp.IsValid())
	{
		// 바인딩 해제
		CachedASC->OnAbilityEnded.Remove(AbilityEndedDelegateHandle);
		
		// 노드 완료 처리
		FinishLatentTask(*CachedOwnerComp.Get(), EBTNodeResult::Succeeded);
	}
}

void UBTTask_UseRandomSkill::OnTaskFinished(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, EBTNodeResult::Type TaskResult)
{
	Super::OnTaskFinished(OwnerComp, NodeMemory, TaskResult);

	// 강제 취소되었을 경우 바인딩 확실히 해제
	if (CachedASC.IsValid())
	{
		CachedASC->OnAbilityEnded.Remove(AbilityEndedDelegateHandle);
	}
}

FString UBTTask_UseRandomSkill::GetStaticDescription() const
{
	return FString::Printf(TEXT("랜덤 스킬 시전 (거리/가중치 반영)"));
}
