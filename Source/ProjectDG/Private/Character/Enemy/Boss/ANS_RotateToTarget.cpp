// Fill out your copyright notice in the Description page of Project Settings.

#include "Character/Enemy/Boss/ANS_RotateToTarget.h"
#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "GameFramework/Pawn.h"
#include "Components/SkeletalMeshComponent.h"

UANS_RotateToTarget::UANS_RotateToTarget()
{
}

FString UANS_RotateToTarget::GetNotifyName_Implementation() const
{
	return TEXT("Rotate To Target");
}

void UANS_RotateToTarget::NotifyTick(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float FrameDeltaTime, const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyTick(MeshComp, Animation, FrameDeltaTime, EventReference);

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
		return;
	}

	// 위치 기반으로 회전값 계산
	FVector PawnLocation = OwnerPawn->GetActorLocation();
	FVector TargetLocation = TargetActor->GetActorLocation();

	FRotator LookAtRotation = UKismetMathLibrary::FindLookAtRotation(PawnLocation, TargetLocation);
	FRotator CurrentRotation = OwnerPawn->GetActorRotation();

	// 회전은 Z축(Yaw)으로만 수행하도록 고정
	LookAtRotation.Pitch = CurrentRotation.Pitch;
	LookAtRotation.Roll = CurrentRotation.Roll;

	// RInterpTo를 사용하여 부드럽게 회전
	FRotator NewRotation = FMath::RInterpTo(CurrentRotation, LookAtRotation, FrameDeltaTime, RotationSpeed);
	OwnerPawn->SetActorRotation(NewRotation);
}
