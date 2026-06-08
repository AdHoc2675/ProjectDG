// Fill out your copyright notice in the Description page of Project Settings.

#include "Character/Enemy/Field/ANS_FieldEnemyAttackHitWindow.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "Character/BaseCharacter.h"
#include "Components/Combat/CombatComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Core/DG_Debug.h"
#include "Core/DG_GameplayTags.h"
#include "Core/DG_Struct.h"
#include "DrawDebugHelpers.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"

UANS_FieldEnemyAttackHitWindow::UANS_FieldEnemyAttackHitWindow()
{
#if WITH_EDITORONLY_DATA
	NotifyColor = FColor(50, 205, 50); // 필드 몬스터는 녹색으로 구분
#endif
}

FString UANS_FieldEnemyAttackHitWindow::GetNotifyName_Implementation() const
{
	return TEXT("FieldEnemyAttackHitWindow");
}

void UANS_FieldEnemyAttackHitWindow::NotifyBegin(
	USkeletalMeshComponent* MeshComp,
	UAnimSequenceBase* Animation,
	float TotalDuration,
	const FAnimNotifyEventReference& EventReference
)
{
	Super::NotifyBegin(MeshComp, Animation, TotalDuration, EventReference);

	if (!MeshComp)
	{
		return;
	}

	InitializeRuntimeData(MeshComp);

	
}

void UANS_FieldEnemyAttackHitWindow::NotifyTick(
	USkeletalMeshComponent* MeshComp,
	UAnimSequenceBase* Animation,
	float FrameDeltaTime,
	const FAnimNotifyEventReference& EventReference
)
{
	Super::NotifyTick(MeshComp, Animation, FrameDeltaTime, EventReference);

	if (!MeshComp)
	{
		return;
	}

	TraceBodySockets(MeshComp);
}

void UANS_FieldEnemyAttackHitWindow::NotifyEnd(
	USkeletalMeshComponent* MeshComp,
	UAnimSequenceBase* Animation,
	const FAnimNotifyEventReference& EventReference
)
{
	Super::NotifyEnd(MeshComp, Animation, EventReference);

	RuntimeDataMap.Remove(MeshComp);

	
}

void UANS_FieldEnemyAttackHitWindow::InitializeRuntimeData(USkeletalMeshComponent* MeshComp)
{
	if (!MeshComp)
	{
		return;
	}

	FFieldEnemyAttackHitWindowRuntimeData& RuntimeData = RuntimeDataMap.FindOrAdd(MeshComp);
	RuntimeData.PreviousSocketLocations.Reset();
	RuntimeData.HitActors.Reset();

	for (const FName& SocketName : TraceSocketNames)
	{
		if (MeshComp->DoesSocketExist(SocketName))
		{
			RuntimeData.PreviousSocketLocations.Add(SocketName, MeshComp->GetSocketLocation(SocketName));
		}
	}
}

void UANS_FieldEnemyAttackHitWindow::TraceBodySockets(USkeletalMeshComponent* MeshComp)
{
	if (!MeshComp)
	{
		return;
	}

	FFieldEnemyAttackHitWindowRuntimeData* RuntimeDataPtr = RuntimeDataMap.Find(MeshComp);
	if (!RuntimeDataPtr)
	{
		InitializeRuntimeData(MeshComp);
		RuntimeDataPtr = RuntimeDataMap.Find(MeshComp);
		if (!RuntimeDataPtr)
		{
			return;
		}
	}

	FFieldEnemyAttackHitWindowRuntimeData& RuntimeData = *RuntimeDataPtr;

	AActor* OwnerActor = MeshComp->GetOwner();
	UWorld* World = OwnerActor ? OwnerActor->GetWorld() : nullptr;
	if (!OwnerActor || !World)
	{
		return;
	}

	FCollisionQueryParams QueryParams(SCENE_QUERY_STAT(FieldEnemyAttackHitWindowTrace), false, OwnerActor);
	QueryParams.AddIgnoredActor(OwnerActor);

	for (const FName& SocketName : TraceSocketNames)
	{
		if (!MeshComp->DoesSocketExist(SocketName))
		{
			continue;
		}

		const FVector CurrentLocation = MeshComp->GetSocketLocation(SocketName);
		FVector* PreviousLocationPtr = RuntimeData.PreviousSocketLocations.Find(SocketName);

		if (!PreviousLocationPtr)
		{
			RuntimeData.PreviousSocketLocations.Add(SocketName, CurrentLocation);
			continue;
		}

		const FVector Start = *PreviousLocationPtr;
		const FVector End = CurrentLocation;

		TArray<FHitResult> HitResults;
		World->SweepMultiByChannel(
			HitResults,
			Start,
			End,
			FQuat::Identity,
			TraceChannel,
			FCollisionShape::MakeSphere(TraceRadius),
			QueryParams
		);

		for (const FHitResult& HitResult : HitResults)
		{
			AActor* HitActor = HitResult.GetActor();
			if (!IsValid(HitActor) || HitActor == OwnerActor)
			{
				continue;
			}

			if (ShouldIgnoreHitActor(OwnerActor, HitActor))
			{
				continue;
			}

			if (RuntimeData.HitActors.Contains(HitActor))
			{
				continue;
			}

			RuntimeData.HitActors.Add(HitActor);

			DrawDebugSphere(World, HitResult.ImpactPoint, 20.f, 12, FColor::Cyan, false, 2.f);

			SendHitEvent(OwnerActor, HitActor);
		}

		if (bEnableDebugDraw)
		{
			DrawTraceDebug(World, Start, End, FColor::Green);
		}

		*PreviousLocationPtr = CurrentLocation;
	}
}

void UANS_FieldEnemyAttackHitWindow::SendHitEvent(AActor* OwnerActor, AActor* HitActor) const
{
	if (!OwnerActor || !OwnerActor->HasAuthority())
	{
		return;
	}

	ABaseCharacter* OwnerCharacter = Cast<ABaseCharacter>(OwnerActor);
	if (!OwnerCharacter)
	{
		return;
	}

	UCombatComponent* CombatComp = OwnerCharacter->GetCombatComponent();
	if (!CombatComp)
	{
		return;
	}

	FDGDamageRequest Request;
	Request.SourceActor = OwnerActor;
	Request.TargetActor = HitActor;
	Request.BaseDamage = AttackDamage;

	CombatComp->ApplyDamageRequest(Request);
}

bool UANS_FieldEnemyAttackHitWindow::ShouldIgnoreHitActor(AActor* OwnerActor, AActor* HitActor) const
{
	if (!OwnerActor || !HitActor)
	{
		return true;
	}

	UAbilitySystemComponent* HitASC =
		UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(HitActor);
	if (!HitASC)
	{
		return true;
	}

	if (bIgnoreSameTeam && AreActorsOnSameTeam(OwnerActor, HitActor))
	{
		return true;
	}

	return false;
}

bool UANS_FieldEnemyAttackHitWindow::AreActorsOnSameTeam(AActor* FirstActor, AActor* SecondActor) const
{
	if (!FirstActor || !SecondActor)
	{
		return false;
	}

	UAbilitySystemComponent* FirstASC =
		UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(FirstActor);
	UAbilitySystemComponent* SecondASC =
		UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(SecondActor);

	if (!FirstASC || !SecondASC)
	{
		return false;
	}

	const bool bBothPlayer =
		FirstASC->HasMatchingGameplayTag(DGGameplayTags::Team_Player.GetTag()) &&
		SecondASC->HasMatchingGameplayTag(DGGameplayTags::Team_Player.GetTag());

	const bool bBothEnemy =
		FirstASC->HasMatchingGameplayTag(DGGameplayTags::Team_Enemy.GetTag()) &&
		SecondASC->HasMatchingGameplayTag(DGGameplayTags::Team_Enemy.GetTag());

	const bool bBothObject =
		FirstASC->HasMatchingGameplayTag(DGGameplayTags::Team_Object.GetTag()) &&
		SecondASC->HasMatchingGameplayTag(DGGameplayTags::Team_Object.GetTag());

	return bBothPlayer || bBothEnemy || bBothObject;
}

void UANS_FieldEnemyAttackHitWindow::DrawTraceDebug(UWorld* World, const FVector& Start, const FVector& End,
	const FColor& Color) const
{
	if (!World)
	{
		return;
	}

	DrawDebugSphere(World, Start, TraceRadius, 12, Color, false, DebugDrawDuration);
	DrawDebugSphere(World, End, TraceRadius, 12, Color, false, DebugDrawDuration);
	DrawDebugLine(World, Start, End, Color, false, DebugDrawDuration, 0, 1.5f);
}
