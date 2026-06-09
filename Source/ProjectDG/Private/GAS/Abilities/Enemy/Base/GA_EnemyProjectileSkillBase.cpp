#include "GAS/Abilities/Enemy/Base/GA_EnemyProjectileSkillBase.h"

#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Character/Enemy/Data/EnemySkillData.h"
#include "Character/Enemy/EnemyCharacterBase.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "GameFramework/Pawn.h"

UGA_EnemyProjectileSkillBase::UGA_EnemyProjectileSkillBase()
{
}

void UGA_EnemyProjectileSkillBase::ActivateAbility(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData
)
{
	// InstancedPerActor 재사용 상황 대비
	bIsFinishingEnemySkill = false;

	UEnemySkillData* CurrentSkillData = GetEnemySkillData();
	if (!CurrentSkillData)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	if (!CurrentSkillData->Montage)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	if (!CurrentSkillData->ProjectileClass)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	if (!CanStartEnemyProjectileSkill())
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	RegisterEnemySkillHitCheckEvent();

	OnEnemyProjectileSkillCommitted();

	if (!PlaySkillMontageFromData(TEXT("EnemyProjectileSkillMontageTask")))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}
}

bool UGA_EnemyProjectileSkillBase::CanStartEnemyProjectileSkill() const
{
	return true;
}

void UGA_EnemyProjectileSkillBase::OnEnemyProjectileSkillCommitted()
{
}

void UGA_EnemyProjectileSkillBase::HandleEnemySkillHitCheckEvent(const FGameplayEventData& Payload)
{
	SpawnProjectileFromSkillData(Payload);
}

AActor* UGA_EnemyProjectileSkillBase::SpawnProjectileFromSkillData(const FGameplayEventData& Payload)
{
	AEnemyCharacterBase* EnemyCharacter = GetEnemyCharacterFromActorInfo();
	if (!EnemyCharacter || !EnemyCharacter->HasAuthority())
	{
		return nullptr;
	}

	UEnemySkillData* CurrentSkillData = GetEnemySkillData();
	if (!CurrentSkillData || !CurrentSkillData->ProjectileClass)
	{
		return nullptr;
	}

	UWorld* World = EnemyCharacter->GetWorld();
	if (!World)
	{
		return nullptr;
	}

	FTransform SpawnTransform;
	if (!ResolveProjectileSpawnTransform(Payload, SpawnTransform))
	{
		return nullptr;
	}

	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = EnemyCharacter;
	SpawnParams.Instigator = Cast<APawn>(EnemyCharacter);
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	return World->SpawnActor<AActor>(
		CurrentSkillData->ProjectileClass,
		SpawnTransform,
		SpawnParams
	);
}

bool UGA_EnemyProjectileSkillBase::ResolveProjectileSpawnTransform(
	const FGameplayEventData& Payload,
	FTransform& OutSpawnTransform
) const
{
	const AEnemyCharacterBase* EnemyCharacter = GetEnemyCharacterFromActorInfo();
	const UEnemySkillData* CurrentSkillData = GetEnemySkillData();

	if (!EnemyCharacter || !CurrentSkillData)
	{
		return false;
	}

	FVector SpawnLocation = FVector::ZeroVector;

	const USkeletalMeshComponent* MeshComp = EnemyCharacter->GetMesh();
	if (MeshComp && CurrentSkillData->TraceSocketNames.Num() > 0)
	{
		const FName SocketName = CurrentSkillData->TraceSocketNames[0];
		if (MeshComp->DoesSocketExist(SocketName))
		{
			SpawnLocation = MeshComp->GetSocketLocation(SocketName);
		}
	}

	if (SpawnLocation.IsNearlyZero())
	{
		SpawnLocation =
			EnemyCharacter->GetActorLocation()
			+ EnemyCharacter->GetActorForwardVector() * CurrentSkillData->ForwardOffset;
	}

	const FVector Direction = ResolveProjectileDirection(SpawnLocation, Payload);
	if (Direction.IsNearlyZero())
	{
		return false;
	}

	OutSpawnTransform = FTransform(Direction.Rotation(), SpawnLocation);
	return true;
}

AActor* UGA_EnemyProjectileSkillBase::ResolveProjectileTargetActor(const FGameplayEventData& Payload) const
{
	const AEnemyCharacterBase* EnemyCharacter = GetEnemyCharacterFromActorInfo();
	if (!EnemyCharacter)
	{
		return nullptr;
	}

	AActor* PayloadTarget = const_cast<AActor*>(Payload.Target.Get());
	if (PayloadTarget && PayloadTarget != EnemyCharacter)
	{
		return PayloadTarget;
	}

	const AAIController* AIController = Cast<AAIController>(EnemyCharacter->GetController());
	const UBlackboardComponent* BlackboardComp = AIController ? AIController->GetBlackboardComponent() : nullptr;
	if (!BlackboardComp)
	{
		return nullptr;
	}

	if (AActor* TargetActor = Cast<AActor>(BlackboardComp->GetValueAsObject(TEXT("TargetActor"))))
	{
		return TargetActor;
	}

	if (AActor* TargetActor = Cast<AActor>(BlackboardComp->GetValueAsObject(TEXT("ObjectKey"))))
	{
		return TargetActor;
	}

	return nullptr;
}

FVector UGA_EnemyProjectileSkillBase::ResolveProjectileDirection(
	const FVector& SpawnLocation,
	const FGameplayEventData& Payload
) const
{
	const AEnemyCharacterBase* EnemyCharacter = GetEnemyCharacterFromActorInfo();
	if (!EnemyCharacter)
	{
		return FVector::ZeroVector;
	}

	AActor* TargetActor = ResolveProjectileTargetActor(Payload);
	if (TargetActor)
	{
		const FVector TargetLocation = TargetActor->GetActorLocation();
		const FVector DirectionToTarget = (TargetLocation - SpawnLocation).GetSafeNormal();

		if (!DirectionToTarget.IsNearlyZero())
		{
			return DirectionToTarget;
		}
	}

	return EnemyCharacter->GetActorForwardVector();
}

void UGA_EnemyProjectileSkillBase::OnSkillMontageStarted()
{
	Super::OnSkillMontageStarted();
}

void UGA_EnemyProjectileSkillBase::OnSkillMontageCompleted()
{
	Super::OnSkillMontageCompleted();
}

void UGA_EnemyProjectileSkillBase::OnSkillMontageInterrupted()
{
	Super::OnSkillMontageInterrupted();
}

void UGA_EnemyProjectileSkillBase::OnSkillMontageCancelled()
{
	Super::OnSkillMontageCancelled();
}

void UGA_EnemyProjectileSkillBase::OnEnemySkillFinished(bool bWasCancelled)
{
	Super::OnEnemySkillFinished(bWasCancelled);
}