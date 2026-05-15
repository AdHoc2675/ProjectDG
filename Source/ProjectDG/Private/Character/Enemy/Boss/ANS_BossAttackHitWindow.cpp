// Fill out your copyright notice in the Description page of Project Settings.

#include "Character/Enemy/Boss/ANS_BossAttackHitWindow.h"

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

UANS_BossAttackHitWindow::UANS_BossAttackHitWindow()
{
#if WITH_EDITORONLY_DATA
	NotifyColor = FColor(255, 140, 0);
#endif
}

FString UANS_BossAttackHitWindow::GetNotifyName_Implementation() const
{
	return TEXT("BossAttackHitWindow");
}

void UANS_BossAttackHitWindow::NotifyBegin(
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

	if (bLogWindowLifecycle)
	{
		Debug::Print(TEXT("[ANS_BossAttackHitWindow] Begin"), FColor::Orange);
	}
}

void UANS_BossAttackHitWindow::NotifyTick(
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

void UANS_BossAttackHitWindow::NotifyEnd(
	USkeletalMeshComponent* MeshComp,
	UAnimSequenceBase* Animation,
	const FAnimNotifyEventReference& EventReference
)
{
	Super::NotifyEnd(MeshComp, Animation, EventReference);

	RuntimeDataMap.Remove(MeshComp);

	if (bLogWindowLifecycle)
	{
		Debug::Print(TEXT("[ANS_BossAttackHitWindow] End"), FColor::Orange);
	}
}

void UANS_BossAttackHitWindow::InitializeRuntimeData(USkeletalMeshComponent* MeshComp)
{
	if (!MeshComp)
	{
		return;
	}

	FBossAttackHitWindowRuntimeData& RuntimeData = RuntimeDataMap.FindOrAdd(MeshComp);
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

void UANS_BossAttackHitWindow::TraceBodySockets(USkeletalMeshComponent* MeshComp)
{
	if (!MeshComp)
	{
		return;
	}

	FBossAttackHitWindowRuntimeData* RuntimeDataPtr = RuntimeDataMap.Find(MeshComp);
	if (!RuntimeDataPtr)
	{
		InitializeRuntimeData(MeshComp);
		RuntimeDataPtr = RuntimeDataMap.Find(MeshComp);
		if (!RuntimeDataPtr)
		{
			return;
		}
	}

	FBossAttackHitWindowRuntimeData& RuntimeData = *RuntimeDataPtr;

	AActor* OwnerActor = MeshComp->GetOwner();
	UWorld* World = OwnerActor ? OwnerActor->GetWorld() : nullptr;
	if (!OwnerActor || !World)
	{
		return;
	}

	FCollisionQueryParams QueryParams(SCENE_QUERY_STAT(BossAttackHitWindowTrace), false, OwnerActor);
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
			Debug::Print(FString::Printf(TEXT("[BossHit] %s"), *GetNameSafe(HitActor)), FColor::Cyan);

			SendHitEvent(OwnerActor, HitActor);
		}

		if (bEnableDebugDraw)
		{
			DrawTraceDebug(World, Start, End, FColor::Orange);
		}

		*PreviousLocationPtr = CurrentLocation;
	}
}

void UANS_BossAttackHitWindow::SendHitEvent(AActor* OwnerActor, AActor* HitActor) const
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

bool UANS_BossAttackHitWindow::ShouldIgnoreHitActor(AActor* OwnerActor, AActor* HitActor) const
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

bool UANS_BossAttackHitWindow::AreActorsOnSameTeam(AActor* FirstActor, AActor* SecondActor) const
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

void UANS_BossAttackHitWindow::DrawTraceDebug(UWorld* World, const FVector& Start, const FVector& End,
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
