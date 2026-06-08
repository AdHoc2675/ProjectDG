#include "AI/Services/BTService_UpdateEnemyCombatState.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "AIController.h"
#include "AI/Controller/EnemyAIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Character/Enemy/Data/EnemySkillData.h"
#include "Character/Enemy/Field/FieldEnemyBase.h"
#include "Character/Enemy/Field/Data/FieldCharacterClassData.h"
#include "GameFramework/Actor.h"
#include "GameFramework/Pawn.h"

namespace
{
	bool DoesTargetMatchSkillTags(AActor* TargetActor, const UEnemySkillData* SkillData)
	{
		if (!TargetActor || !SkillData)
		{
			return false;
		}

		if (SkillData->TargetRequiredTags.IsEmpty())
		{
			return true;
		}

		UAbilitySystemComponent* TargetASC =
			UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(TargetActor);

		if (!TargetASC)
		{
			return false;
		}

		FGameplayTagContainer OwnedTags;
		TargetASC->GetOwnedGameplayTags(OwnedTags);

		return OwnedTags.HasAll(SkillData->TargetRequiredTags);
	}

	bool CanUseAnyEnemySkillAtDistance(
		const UFieldCharacterClassData* FieldClassData,
		AActor* TargetActor,
		float DistanceToTarget
	)
	{
		if (!FieldClassData || !TargetActor)
		{
			return false;
		}

		for (const TObjectPtr<UEnemySkillData>& SkillDataPtr : FieldClassData->AttackSkills)
		{
			const UEnemySkillData* SkillData = SkillDataPtr.Get();
			if (!SkillData || !SkillData->AbilityClass)
			{
				continue;
			}

			if (SkillData->SelectionWeight <= 0.f)
			{
				continue;
			}

			if (DistanceToTarget < SkillData->MinRange || DistanceToTarget > SkillData->MaxRange)
			{
				continue;
			}

			if (!DoesTargetMatchSkillTags(TargetActor, SkillData))
			{
				continue;
			}

			return true;
		}

		return false;
	}
}

UBTService_UpdateEnemyCombatState::UBTService_UpdateEnemyCombatState()
{
	NodeName = TEXT("Update Enemy Combat State");

	Interval = 0.2f;
	RandomDeviation = 0.05f;
	bNotifyTick = true;

	TargetActorKey.AddObjectFilter(
		this,
		GET_MEMBER_NAME_CHECKED(UBTService_UpdateEnemyCombatState, TargetActorKey),
		AActor::StaticClass()
	);

	TargetLocationKey.AddVectorFilter(
		this,
		GET_MEMBER_NAME_CHECKED(UBTService_UpdateEnemyCombatState, TargetLocationKey)
	);

	TargetDistanceKey.AddFloatFilter(
		this,
		GET_MEMBER_NAME_CHECKED(UBTService_UpdateEnemyCombatState, TargetDistanceKey)
	);

	CanAttackKey.AddBoolFilter(
		this,
		GET_MEMBER_NAME_CHECKED(UBTService_UpdateEnemyCombatState, CanAttackKey)
	);
}

void UBTService_UpdateEnemyCombatState::TickNode(
	UBehaviorTreeComponent& OwnerComp,
	uint8* NodeMemory,
	float DeltaSeconds
)
{
	Super::TickNode(OwnerComp, NodeMemory, DeltaSeconds);

	AAIController* AIController = OwnerComp.GetAIOwner();
	UBlackboardComponent* BlackboardComp = OwnerComp.GetBlackboardComponent();

	if (!AIController || !BlackboardComp)
	{
		return;
	}

	AEnemyAIController* EnemyAIController = Cast<AEnemyAIController>(AIController);
	if (EnemyAIController && !EnemyAIController->IsAIStoppedByDeath())
	{
		EnemyAIController->RefreshTargetFromPerception();
	}

	APawn* ControlledPawn = AIController->GetPawn();
	if (!ControlledPawn)
	{
		if (TargetDistanceKey.SelectedKeyName != NAME_None)
		{
			BlackboardComp->SetValueAsFloat(TargetDistanceKey.SelectedKeyName, 0.f);
		}

		if (CanAttackKey.SelectedKeyName != NAME_None)
		{
			BlackboardComp->SetValueAsBool(CanAttackKey.SelectedKeyName, false);
		}

		return;
	}

	AActor* TargetActor = Cast<AActor>(
		BlackboardComp->GetValueAsObject(TargetActorKey.SelectedKeyName)
	);

	if (!IsValid(TargetActor))
	{
		if (TargetDistanceKey.SelectedKeyName != NAME_None)
		{
			BlackboardComp->SetValueAsFloat(TargetDistanceKey.SelectedKeyName, 0.f);
		}

		if (CanAttackKey.SelectedKeyName != NAME_None)
		{
			BlackboardComp->SetValueAsBool(CanAttackKey.SelectedKeyName, false);
		}

		return;
	}

	const FVector TargetLocation = TargetActor->GetActorLocation();
	const float TargetDistance = FVector::Dist(
		ControlledPawn->GetActorLocation(),
		TargetLocation
	);

	if (TargetLocationKey.SelectedKeyName != NAME_None)
	{
		BlackboardComp->SetValueAsVector(TargetLocationKey.SelectedKeyName, TargetLocation);
	}

	if (TargetDistanceKey.SelectedKeyName != NAME_None)
	{
		BlackboardComp->SetValueAsFloat(TargetDistanceKey.SelectedKeyName, TargetDistance);
	}

	bool bCanAttack = false;

	if (const AFieldEnemyBase* FieldEnemy = Cast<AFieldEnemyBase>(ControlledPawn))
	{
		const UFieldCharacterClassData* FieldClassData = FieldEnemy->GetFieldClassData();

		bCanAttack = CanUseAnyEnemySkillAtDistance(
			FieldClassData,
			TargetActor,
			TargetDistance
		);
	}

	if (CanAttackKey.SelectedKeyName != NAME_None)
	{
		BlackboardComp->SetValueAsBool(CanAttackKey.SelectedKeyName, bCanAttack);
	}
}