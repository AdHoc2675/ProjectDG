#include "GAS/Abilities/Enemy/Base/GA_EnemyDashMeleeSkillBase.h"

#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Character/Enemy/EnemyCharacterBase.h"
#include "GameFramework/Controller.h"

UGA_EnemyDashMeleeSkillBase::UGA_EnemyDashMeleeSkillBase()
{
}

bool UGA_EnemyDashMeleeSkillBase::CanStartEnemyMeleeSkill() const
{
	if (!Super::CanStartEnemyMeleeSkill())
	{
		return false;
	}

	return true;
}

void UGA_EnemyDashMeleeSkillBase::OnEnemyMeleeSkillCommitted()
{
	Super::OnEnemyMeleeSkillCommitted();

	if (bFaceTargetOnDashStart)
	{
		FaceDashTarget();
	}
}

void UGA_EnemyDashMeleeSkillBase::OnSkillMontageStarted()
{
	Super::OnSkillMontageStarted();
}

void UGA_EnemyDashMeleeSkillBase::OnSkillMontageCompleted()
{
	Super::OnSkillMontageCompleted();
}

void UGA_EnemyDashMeleeSkillBase::OnSkillMontageInterrupted()
{
	Super::OnSkillMontageInterrupted();
}

void UGA_EnemyDashMeleeSkillBase::OnSkillMontageCancelled()
{
	Super::OnSkillMontageCancelled();
}

void UGA_EnemyDashMeleeSkillBase::OnEnemySkillFinished(bool bWasCancelled)
{
	Super::OnEnemySkillFinished(bWasCancelled);
}

AActor* UGA_EnemyDashMeleeSkillBase::ResolveDashTargetActor() const
{
	const AEnemyCharacterBase* EnemyCharacter = GetEnemyCharacterFromActorInfo();
	if (!EnemyCharacter)
	{
		return nullptr;
	}

	const AAIController* AIController = Cast<AAIController>(EnemyCharacter->GetController());
	const UBlackboardComponent* BlackboardComp = AIController ? AIController->GetBlackboardComponent() : nullptr;
	if (!BlackboardComp)
	{
		return nullptr;
	}

	if (TargetActorBlackboardKeyName != NAME_None)
	{
		if (AActor* TargetActor = Cast<AActor>(
			BlackboardComp->GetValueAsObject(TargetActorBlackboardKeyName)
		))
		{
			return TargetActor;
		}
	}

	if (FallbackTargetActorBlackboardKeyName != NAME_None)
	{
		if (AActor* TargetActor = Cast<AActor>(
			BlackboardComp->GetValueAsObject(FallbackTargetActorBlackboardKeyName)
		))
		{
			return TargetActor;
		}
	}

	return nullptr;
}

bool UGA_EnemyDashMeleeSkillBase::FaceDashTarget()
{
	AEnemyCharacterBase* EnemyCharacter = GetEnemyCharacterFromActorInfo();
	AActor* TargetActor = ResolveDashTargetActor();

	if (!EnemyCharacter || !TargetActor)
	{
		return false;
	}

	FVector Direction = TargetActor->GetActorLocation() - EnemyCharacter->GetActorLocation();
	Direction.Z = 0.f;

	if (Direction.IsNearlyZero())
	{
		return false;
	}

	const FRotator TargetRotation = Direction.Rotation();

	if (AController* Controller = EnemyCharacter->GetController())
	{
		Controller->SetControlRotation(TargetRotation);
	}

	EnemyCharacter->SetActorRotation(TargetRotation);

	return true;
}