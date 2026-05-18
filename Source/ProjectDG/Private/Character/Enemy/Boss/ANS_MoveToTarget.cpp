// Fill out your copyright notice in the Description page of Project Settings.

#include "Character/Enemy/Boss/ANS_MoveToTarget.h"
#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "GameFramework/Pawn.h"
#include "Components/SkeletalMeshComponent.h"
#include "Navigation/PathFollowingComponent.h"

UANS_MoveToTarget::UANS_MoveToTarget()
{
}

FString UANS_MoveToTarget::GetNotifyName_Implementation() const
{
	return TEXT("Move To Target");
}

void UANS_MoveToTarget::NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration, const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyBegin(MeshComp, Animation, TotalDuration, EventReference);

	if (!MeshComp || !MeshComp->GetOwner())
	{
		return;
	}

	APawn* OwnerPawn = Cast<APawn>(MeshComp->GetOwner());
	if (!OwnerPawn)
	{
		return;
	}

	AAIController* AIController = Cast<AAIController>(OwnerPawn->GetController());
	if (!AIController)
	{
		return;
	}

	UBlackboardComponent* Blackboard = AIController->GetBlackboardComponent();
	if (!Blackboard)
	{
		return;
	}

	AActor* TargetActor = Cast<AActor>(Blackboard->GetValueAsObject(TargetKeyName));
	if (!TargetActor)
	{
		GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Red, TEXT("[ANS_MoveToTarget] Failed: TargetActor is NULL. Check TargetKeyName."));
		return;
	}

	FAIMoveRequest MoveReq;
	MoveReq.SetGoalActor(TargetActor);
	MoveReq.SetAcceptanceRadius(AcceptanceRadius);
	MoveReq.SetUsePathfinding(bUsePathfinding);
	MoveReq.SetAllowPartialPath(true);
	
	FPathFollowingRequestResult Result = AIController->MoveTo(MoveReq);

	FString ResultStr = TEXT("Unknown");
	switch (Result.Code)
	{
		case EPathFollowingRequestResult::Failed: ResultStr = TEXT("Failed"); break;
		case EPathFollowingRequestResult::AlreadyAtGoal: ResultStr = TEXT("AlreadyAtGoal"); break;
		case EPathFollowingRequestResult::RequestSuccessful: ResultStr = TEXT("RequestSuccessful"); break;
	}

	GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Yellow, FString::Printf(TEXT("[ANS_MoveToTarget] MoveTo Result: %s"), *ResultStr));
}

void UANS_MoveToTarget::NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyEnd(MeshComp, Animation, EventReference);

	if (!MeshComp || !MeshComp->GetOwner())
	{
		return;
	}

	APawn* OwnerPawn = Cast<APawn>(MeshComp->GetOwner());
	if (!OwnerPawn)
	{
		return;
	}

	AAIController* AIController = Cast<AAIController>(OwnerPawn->GetController());
	if (AIController)
	{
		// 구간이 끝나면 이동 중지
		AIController->StopMovement();
	}
}
