#include "GAS/Abilities/Enemy/Base/GA_EnemySkillBase.h"

#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "Character/Enemy/EnemyCharacterBase.h"
#include "Character/Enemy/Data/EnemySkillData.h"
#include "Components/Combat/CombatComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Core/DG_Debug.h"
#include "Core/DG_Struct.h"
#include "Core/DG_GameplayTags.h"
#include "DrawDebugHelpers.h"
#include "Engine/HitResult.h"
#include "Engine/OverlapResult.h"
#include "Engine/World.h"

UGA_EnemySkillBase::UGA_EnemySkillBase()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::ServerOnly;
}

void UGA_EnemySkillBase::EndAbility(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	bool bReplicateEndAbility,
	bool bWasCancelled
)
{
	UnregisterEnemySkillHitCheckEvent();
	ResetEnemySkillRuntimeHitState();

	if (MontageTask)
	{
		MontageTask->OnCompleted.RemoveAll(this);
		MontageTask->OnBlendOut.RemoveAll(this);
		MontageTask->OnInterrupted.RemoveAll(this);
		MontageTask->OnCancelled.RemoveAll(this);
		MontageTask = nullptr;
	}

	bIsFinishingEnemySkill = false;

	Super::EndAbility(
		Handle,
		ActorInfo,
		ActivationInfo,
		bReplicateEndAbility,
		bWasCancelled
	);
}

UEnemySkillData* UGA_EnemySkillBase::GetEnemySkillData() const
{
	if (SkillData)
	{
		return SkillData;
	}

	return Cast<UEnemySkillData>(GetCurrentSourceObject());
}

AEnemyCharacterBase* UGA_EnemySkillBase::GetEnemyCharacterFromActorInfo() const
{
	return Cast<AEnemyCharacterBase>(GetAvatarActorFromActorInfo());
}

UCombatComponent* UGA_EnemySkillBase::GetEnemyCombatComponent() const
{
	AEnemyCharacterBase* EnemyCharacter = GetEnemyCharacterFromActorInfo();
	if (!EnemyCharacter)
	{
		return nullptr;
	}

	return EnemyCharacter->GetCombatComponent();
}

bool UGA_EnemySkillBase::ApplyDamageToTarget(
	AActor* TargetActor,
	const FVector& HitLocation,
	bool bHasHitLocation
) const
{
	AEnemyCharacterBase* EnemyCharacter = GetEnemyCharacterFromActorInfo();
	if (!EnemyCharacter || !EnemyCharacter->HasAuthority())
	{
		return false;
	}

	if (!TargetActor)
	{
		return false;
	}

	UEnemySkillData* CurrentSkillData = GetEnemySkillData();
	if (!CurrentSkillData)
	{
		return false;
	}

	UCombatComponent* CombatComponent = GetEnemyCombatComponent();
	if (!CombatComponent)
	{
		return false;
	}

	FDGDamageRequest DamageRequest;
	DamageRequest.SourceActor = EnemyCharacter;
	DamageRequest.TargetActor = TargetActor;
	DamageRequest.BaseDamage = CurrentSkillData->BaseDamage;
	DamageRequest.DamageMultiplier = CurrentSkillData->DamageMultiplier;
	DamageRequest.GroggyDamage = CurrentSkillData->GroggyDamage;
	DamageRequest.DamageTypeTag = CurrentSkillData->DamageTypeTag;
	DamageRequest.SourceTag = CurrentSkillData->SkillTag;
	DamageRequest.HitLocation = HitLocation;
	DamageRequest.bHasHitLocation = bHasHitLocation;

	const FDGDamageResult DamageResult =
		CombatComponent->ApplyDamageRequest(DamageRequest);

	return DamageResult.bSuccess;
}

void UGA_EnemySkillBase::ApplyDamageToTargets(const TArray<AActor*>& TargetActors) const
{
	for (AActor* TargetActor : TargetActors)
	{
		if (!TargetActor)
		{
			continue;
		}

		const FVector HitLocation = TargetActor->GetActorLocation();

		const bool bDamageApplied = ApplyDamageToTarget(
			TargetActor,
			HitLocation,
			true
		);

		
	}
}

bool UGA_EnemySkillBase::CanPlaySkillMontageFromData() const
{
	const UEnemySkillData* CurrentSkillData = GetEnemySkillData();
	if (!CurrentSkillData)
	{
		return false;
	}

	return CurrentSkillData->Montage != nullptr;
}

bool UGA_EnemySkillBase::PlaySkillMontageFromData(
	FName TaskInstanceName,
	FName StartSectionName,
	bool bStopWhenAbilityEnds
)
{
	UEnemySkillData* CurrentSkillData = GetEnemySkillData();
	if (!CurrentSkillData)
	{
		return false;
	}

	if (!CurrentSkillData->Montage)
	{
		return false;
	}

	if (MontageTask)
	{
		MontageTask->OnCompleted.RemoveAll(this);
		MontageTask->OnBlendOut.RemoveAll(this);
		MontageTask->OnInterrupted.RemoveAll(this);
		MontageTask->OnCancelled.RemoveAll(this);
		MontageTask = nullptr;
	}

	MontageTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(
		this,
		TaskInstanceName,
		CurrentSkillData->Montage,
		CurrentSkillData->PlayRate,
		StartSectionName,
		bStopWhenAbilityEnds
	);

	if (!MontageTask)
	{
		return false;
	}

	MontageTask->OnCompleted.AddDynamic(this, &UGA_EnemySkillBase::HandleSkillMontageCompleted);
	MontageTask->OnBlendOut.AddDynamic(this, &UGA_EnemySkillBase::HandleSkillMontageBlendOut);
	MontageTask->OnInterrupted.AddDynamic(this, &UGA_EnemySkillBase::HandleSkillMontageInterrupted);
	MontageTask->OnCancelled.AddDynamic(this, &UGA_EnemySkillBase::HandleSkillMontageCancelled);

	OnSkillMontageStarted();

	MontageTask->ReadyForActivation();

	return true;
}

void UGA_EnemySkillBase::FinishEnemySkill(bool bWasCancelled)
{
	if (bIsFinishingEnemySkill)
	{
		return;
	}

	bIsFinishingEnemySkill = true;

	OnEnemySkillFinished(bWasCancelled);

	EndAbility(
		CurrentSpecHandle,
		CurrentActorInfo,
		CurrentActivationInfo,
		true,
		bWasCancelled
	);
}

void UGA_EnemySkillBase::RegisterEnemySkillHitCheckEvent()
{
	if (SkillHitCheckEventTask)
	{
		return;
	}

	ResetEnemySkillRuntimeHitState();

	SkillHitCheckEventTask = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(
		this,
		DGGameplayTags::Event_Attack_HitCheck,
		nullptr,
		false,
		true
	);

	if (!SkillHitCheckEventTask)
	{
		return;
	}

	SkillHitCheckEventTask->EventReceived.AddDynamic(
		this,
		&UGA_EnemySkillBase::OnEnemySkillHitCheckEvent
	);

	SkillHitCheckEventTask->ReadyForActivation();
}

void UGA_EnemySkillBase::UnregisterEnemySkillHitCheckEvent()
{
	if (!SkillHitCheckEventTask)
	{
		return;
	}

	SkillHitCheckEventTask->EndTask();
	SkillHitCheckEventTask = nullptr;
}

void UGA_EnemySkillBase::OnEnemySkillHitCheckEvent(FGameplayEventData Payload)
{
	HandleEnemySkillHitCheckEvent(Payload);
}

void UGA_EnemySkillBase::HandleEnemySkillHitCheckEvent(const FGameplayEventData& Payload)
{
	UEnemySkillData* CurrentSkillData = GetEnemySkillData();
	if (!CurrentSkillData)
	{
		return;
	}

	

	TArray<AActor*> HitActors;
	if (!CollectEnemySkillTargetsFromData(Payload, CurrentSkillData, HitActors))
	{
		DrawEnemySkillHitDebug(Payload, CurrentSkillData, HitActors);
		return;
	}

	DrawEnemySkillHitDebug(Payload, CurrentSkillData, HitActors);
	ApplyDamageToTargets(HitActors);
}

void UGA_EnemySkillBase::ResetEnemySkillRuntimeHitState()
{
	bHasLastPathSweepCenter = false;
	bHasLastPathSweepDebugSegment = false;
	LastPathSweepCenter = FVector::ZeroVector;
	LastPathSweepDebugStart = FVector::ZeroVector;
	LastPathSweepDebugEnd = FVector::ZeroVector;
}

bool UGA_EnemySkillBase::CollectEnemySkillTargetsFromData(
	const FGameplayEventData& Payload,
	const UEnemySkillData* CurrentSkillData,
	TArray<AActor*>& OutTargetActors
)
{
	OutTargetActors.Reset();
	bHasLastPathSweepDebugSegment = false;

	if (!CurrentSkillData)
	{
		return false;
	}

	switch (CurrentSkillData->HitShape)
	{
	case EDGEnemySkillHitShape::AcquiredTarget:
		CollectAcquiredTargetFromSkillData(Payload, CurrentSkillData, OutTargetActors);
		break;

	case EDGEnemySkillHitShape::ForwardBox:
		CollectForwardBoxTargetsFromSkillData(CurrentSkillData, OutTargetActors);
		break;

	case EDGEnemySkillHitShape::Radius:
		CollectRadiusTargetsFromSkillData(Payload, CurrentSkillData, OutTargetActors);
		break;

	case EDGEnemySkillHitShape::PathBoxSweep:
		CollectPathBoxSweepTargetsFromSkillData(CurrentSkillData, OutTargetActors);
		break;

	case EDGEnemySkillHitShape::SocketSweep:
		break;

	case EDGEnemySkillHitShape::Projectile:
		break;

	case EDGEnemySkillHitShape::None:
	default:
		break;
	}

	return OutTargetActors.Num() > 0;
}

void UGA_EnemySkillBase::CollectAcquiredTargetFromSkillData(
	const FGameplayEventData& Payload,
	const UEnemySkillData* CurrentSkillData,
	TArray<AActor*>& OutTargetActors
) const
{
	TSet<AActor*> UniqueActors;

	AActor* TargetActor = const_cast<AActor*>(Payload.Target.Get());
	AddTargetIfValid(TargetActor, CurrentSkillData, OutTargetActors, UniqueActors);
}

void UGA_EnemySkillBase::CollectForwardBoxTargetsFromSkillData(
	const UEnemySkillData* CurrentSkillData,
	TArray<AActor*>& OutTargetActors
) const
{
	AActor* AvatarActor = GetAvatarActorFromActorInfo();
	UWorld* World = AvatarActor ? AvatarActor->GetWorld() : nullptr;

	if (!World || !AvatarActor || !CurrentSkillData)
	{
		return;
	}

	const FVector Center =
		AvatarActor->GetActorLocation()
		+ AvatarActor->GetActorForwardVector() * CurrentSkillData->ForwardOffset;

	const FQuat BoxQuat = AvatarActor->GetActorQuat();
	const FCollisionShape BoxShape = FCollisionShape::MakeBox(CurrentSkillData->BoxExtent);

	FCollisionObjectQueryParams ObjectQueryParams;
	ObjectQueryParams.AddObjectTypesToQuery(ECC_Pawn);

	FCollisionQueryParams QueryParams(SCENE_QUERY_STAT(EnemySkillForwardBox), false);
	QueryParams.AddIgnoredActor(AvatarActor);

	TArray<FOverlapResult> OverlapResults;
	const bool bHit = World->OverlapMultiByObjectType(
		OverlapResults,
		Center,
		BoxQuat,
		ObjectQueryParams,
		BoxShape,
		QueryParams
	);

	if (!bHit)
	{
		return;
	}

	TSet<AActor*> UniqueActors;

	for (const FOverlapResult& Result : OverlapResults)
	{
		AddTargetIfValid(
			Result.GetActor(),
			CurrentSkillData,
			OutTargetActors,
			UniqueActors
		);
	}
}

void UGA_EnemySkillBase::CollectRadiusTargetsFromSkillData(
	const FGameplayEventData& Payload,
	const UEnemySkillData* CurrentSkillData,
	TArray<AActor*>& OutTargetActors
) const
{
	AActor* AvatarActor = GetAvatarActorFromActorInfo();
	UWorld* World = AvatarActor ? AvatarActor->GetWorld() : nullptr;

	if (!World || !AvatarActor || !CurrentSkillData)
	{
		return;
	}

	FVector Center = FVector::ZeroVector;
	if (!ResolveSkillHitCenter(Payload, CurrentSkillData, Center))
	{
		return;
	}

	const FCollisionShape SphereShape = FCollisionShape::MakeSphere(CurrentSkillData->Radius);

	FCollisionObjectQueryParams ObjectQueryParams;
	ObjectQueryParams.AddObjectTypesToQuery(ECC_Pawn);

	FCollisionQueryParams QueryParams(SCENE_QUERY_STAT(EnemySkillRadius), false);
	QueryParams.AddIgnoredActor(AvatarActor);

	TArray<FOverlapResult> OverlapResults;
	const bool bHit = World->OverlapMultiByObjectType(
		OverlapResults,
		Center,
		FQuat::Identity,
		ObjectQueryParams,
		SphereShape,
		QueryParams
	);

	if (!bHit)
	{
		return;
	}

	TSet<AActor*> UniqueActors;

	for (const FOverlapResult& Result : OverlapResults)
	{
		AddTargetIfValid(
			Result.GetActor(),
			CurrentSkillData,
			OutTargetActors,
			UniqueActors
		);
	}
}

void UGA_EnemySkillBase::CollectPathBoxSweepTargetsFromSkillData(
	const UEnemySkillData* CurrentSkillData,
	TArray<AActor*>& OutTargetActors
)
{
	AActor* AvatarActor = GetAvatarActorFromActorInfo();
	UWorld* World = AvatarActor ? AvatarActor->GetWorld() : nullptr;

	if (!World || !AvatarActor || !CurrentSkillData)
	{
		return;
	}

	FVector CurrentCenter = FVector::ZeroVector;
	if (!ResolvePathBoxSweepCenter(CurrentSkillData, CurrentCenter))
	{
		return;
	}

	const FVector StartCenter = bHasLastPathSweepCenter ? LastPathSweepCenter : CurrentCenter;
	const FVector EndCenter = CurrentCenter;

	LastPathSweepDebugStart = StartCenter;
	LastPathSweepDebugEnd = EndCenter;
	bHasLastPathSweepDebugSegment = true;

	const FQuat BoxQuat = AvatarActor->GetActorQuat();
	const FCollisionShape BoxShape = FCollisionShape::MakeBox(CurrentSkillData->BoxExtent);

	FCollisionObjectQueryParams ObjectQueryParams;
	ObjectQueryParams.AddObjectTypesToQuery(ECC_Pawn);

	FCollisionQueryParams QueryParams(SCENE_QUERY_STAT(EnemySkillPathBoxSweep), false);
	QueryParams.AddIgnoredActor(AvatarActor);

	TSet<AActor*> UniqueActors;

	if (!bHasLastPathSweepCenter)
	{
		TArray<FOverlapResult> OverlapResults;
		const bool bHit = World->OverlapMultiByObjectType(
			OverlapResults,
			CurrentCenter,
			BoxQuat,
			ObjectQueryParams,
			BoxShape,
			QueryParams
		);

		if (bHit)
		{
			for (const FOverlapResult& Result : OverlapResults)
			{
				AddTargetIfValid(
					Result.GetActor(),
					CurrentSkillData,
					OutTargetActors,
					UniqueActors
				);
			}
		}
	}
	else
	{
		TArray<FHitResult> SweepResults;
		const bool bHit = World->SweepMultiByObjectType(
			SweepResults,
			StartCenter,
			EndCenter,
			BoxQuat,
			ObjectQueryParams,
			BoxShape,
			QueryParams
		);

		if (bHit)
		{
			for (const FHitResult& Result : SweepResults)
			{
				AddTargetIfValid(
					Result.GetActor(),
					CurrentSkillData,
					OutTargetActors,
					UniqueActors
				);
			}
		}
	}

	LastPathSweepCenter = CurrentCenter;
	bHasLastPathSweepCenter = true;
}

bool UGA_EnemySkillBase::DoesActorMatchSkillTargetTags(
	AActor* TargetActor,
	const UEnemySkillData* CurrentSkillData
) const
{
	if (!TargetActor || !CurrentSkillData)
	{
		return false;
	}

	if (TargetActor == GetAvatarActorFromActorInfo())
	{
		return false;
	}

	if (CurrentSkillData->TargetRequiredTags.IsEmpty())
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

	return OwnedTags.HasAll(CurrentSkillData->TargetRequiredTags);
}

void UGA_EnemySkillBase::AddTargetIfValid(
	AActor* CandidateActor,
	const UEnemySkillData* CurrentSkillData,
	TArray<AActor*>& OutTargetActors,
	TSet<AActor*>& UniqueActors
) const
{
	if (!CandidateActor || !CurrentSkillData)
	{
		return;
	}

	if (UniqueActors.Contains(CandidateActor))
	{
		return;
	}

	if (!DoesActorMatchSkillTargetTags(CandidateActor, CurrentSkillData))
	{
		return;
	}

	UniqueActors.Add(CandidateActor);
	OutTargetActors.Add(CandidateActor);

	if (CurrentSkillData->MaxHitTargets > 0 &&
		OutTargetActors.Num() >= CurrentSkillData->MaxHitTargets)
	{
		return;
	}
}

bool UGA_EnemySkillBase::ResolveSkillHitCenter(
	const FGameplayEventData& Payload,
	const UEnemySkillData* CurrentSkillData,
	FVector& OutCenter
) const
{
	AActor* AvatarActor = GetAvatarActorFromActorInfo();
	if (!AvatarActor || !CurrentSkillData)
	{
		return false;
	}

	switch (CurrentSkillData->HitOrigin)
	{
	case EDGEnemySkillHitOrigin::Self:
		OutCenter = AvatarActor->GetActorLocation();
		return true;

	case EDGEnemySkillHitOrigin::Target:
	{
		AActor* TargetActor = const_cast<AActor*>(Payload.Target.Get());
		if (!TargetActor)
		{
			return false;
		}

		OutCenter = TargetActor->GetActorLocation();
		return true;
	}

	case EDGEnemySkillHitOrigin::Socket:
	{
		const AEnemyCharacterBase* EnemyCharacter = GetEnemyCharacterFromActorInfo();
		const USkeletalMeshComponent* MeshComp = EnemyCharacter ? EnemyCharacter->GetMesh() : nullptr;

		if (!MeshComp || CurrentSkillData->TraceSocketNames.Num() == 0)
		{
			return false;
		}

		OutCenter = MeshComp->GetSocketLocation(CurrentSkillData->TraceSocketNames[0]);
		return true;
	}

	case EDGEnemySkillHitOrigin::World:
	default:
		return false;
	}
}

bool UGA_EnemySkillBase::ResolvePathBoxSweepCenter(
	const UEnemySkillData* CurrentSkillData,
	FVector& OutCenter
) const
{
	AActor* AvatarActor = GetAvatarActorFromActorInfo();
	if (!AvatarActor || !CurrentSkillData)
	{
		return false;
	}

	// 돌진 / 이동 중 자기 몸 주변 박스는 ForwardOffset을 0으로 두면 몸 중심 박스가 됨.
	OutCenter =
		AvatarActor->GetActorLocation()
		+ AvatarActor->GetActorForwardVector() * CurrentSkillData->ForwardOffset;

	return true;
}

void UGA_EnemySkillBase::DrawEnemySkillHitDebug(
	const FGameplayEventData& Payload,
	const UEnemySkillData* CurrentSkillData,
	const TArray<AActor*>& HitActors
) const
{
	if (!CurrentSkillData || !CurrentSkillData->bDrawHitDebug)
	{
		return;
	}

	AActor* AvatarActor = GetAvatarActorFromActorInfo();
	UWorld* World = AvatarActor ? AvatarActor->GetWorld() : nullptr;

	if (!World || !AvatarActor)
	{
		return;
	}

	const FColor DebugColor = HitActors.Num() > 0 ? FColor::Green : FColor::Red;
	const float DebugDuration = 1.5f;

	switch (CurrentSkillData->HitShape)
	{
	case EDGEnemySkillHitShape::ForwardBox:
	{
		const FVector Center =
			AvatarActor->GetActorLocation()
			+ AvatarActor->GetActorForwardVector() * CurrentSkillData->ForwardOffset;

		DrawDebugBox(
			World,
			Center,
			CurrentSkillData->BoxExtent,
			AvatarActor->GetActorQuat(),
			DebugColor,
			false,
			DebugDuration
		);

		break;
	}

	case EDGEnemySkillHitShape::Radius:
	{
		FVector Center = FVector::ZeroVector;
		if (!ResolveSkillHitCenter(Payload, CurrentSkillData, Center))
		{
			Center = AvatarActor->GetActorLocation();
		}

		DrawDebugSphere(
			World,
			Center,
			CurrentSkillData->Radius,
			32,
			DebugColor,
			false,
			DebugDuration
		);

		break;
	}

	case EDGEnemySkillHitShape::PathBoxSweep:
	{
		if (bHasLastPathSweepDebugSegment)
		{
			DrawDebugBox(
				World,
				LastPathSweepDebugStart,
				CurrentSkillData->BoxExtent,
				AvatarActor->GetActorQuat(),
				DebugColor,
				false,
				DebugDuration
			);

			DrawDebugBox(
				World,
				LastPathSweepDebugEnd,
				CurrentSkillData->BoxExtent,
				AvatarActor->GetActorQuat(),
				DebugColor,
				false,
				DebugDuration
			);

			DrawDebugLine(
				World,
				LastPathSweepDebugStart,
				LastPathSweepDebugEnd,
				DebugColor,
				false,
				DebugDuration,
				0,
				2.f
			);
		}

		break;
	}

	case EDGEnemySkillHitShape::AcquiredTarget:
	{
		AActor* TargetActor = const_cast<AActor*>(Payload.Target.Get());
		if (TargetActor)
		{
			DrawDebugSphere(
				World,
				TargetActor->GetActorLocation(),
				50.f,
				16,
				DebugColor,
				false,
				DebugDuration
			);
		}

		break;
	}

	case EDGEnemySkillHitShape::SocketSweep:
	case EDGEnemySkillHitShape::Projectile:
	case EDGEnemySkillHitShape::None:
	default:
		break;
	}
}

void UGA_EnemySkillBase::HandleSkillMontageCompleted()
{
	OnSkillMontageCompleted();

	FinishEnemySkill(false);
}

void UGA_EnemySkillBase::HandleSkillMontageBlendOut()
{
	OnSkillMontageBlendOut();

	// BlendOut은 기본적으로 Ability 종료 조건으로 보지 않는다.
	// 후딜레이, 장판 유지, 패턴 후처리 등이 있을 수 있기 때문.
}

void UGA_EnemySkillBase::HandleSkillMontageInterrupted()
{
	OnSkillMontageInterrupted();

	FinishEnemySkill(true);
}

void UGA_EnemySkillBase::HandleSkillMontageCancelled()
{
	OnSkillMontageCancelled();

	FinishEnemySkill(true);
}

void UGA_EnemySkillBase::OnSkillMontageStarted()
{
}

void UGA_EnemySkillBase::OnSkillMontageCompleted()
{
}

void UGA_EnemySkillBase::OnSkillMontageBlendOut()
{
}

void UGA_EnemySkillBase::OnSkillMontageInterrupted()
{
}

void UGA_EnemySkillBase::OnSkillMontageCancelled()
{
}

void UGA_EnemySkillBase::OnEnemySkillFinished(bool bWasCancelled)
{
}