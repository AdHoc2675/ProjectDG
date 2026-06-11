// Fill out your copyright notice in the Description page of Project Settings.

#include "Character/Enemy/Field/FieldEnemyBase.h"

#include "AIController.h"
#include "AI/Controller/AIControllerBase.h"
#include "BrainComponent.h"
#include "AbilitySystemComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Character/Enemy/Data/EnemySkillData.h"
#include "Character/Enemy/Field/Data/FieldCharacterClassData.h"
#include "Components/CapsuleComponent.h"
#include "Components/UI/DGMinimapMarkerComponent.h"
#include "Core/DG_GameplayTags.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GAS/Attributes/DG_AttributeSet.h"
#include "GAS/Attributes/DG_EnemyAttributeSet.h"
#include "GameplayAbilitySpec.h"
#include "System/FieldEnemyPoolSubsystem.h"

namespace
{
	bool HasGrantedSkillDataAbility(const UAbilitySystemComponent* ASC, const UEnemySkillData* SkillData)
	{
		if (!ASC || !SkillData || !SkillData->AbilityClass)
		{
			return false;
		}

		for (const FGameplayAbilitySpec& Spec : ASC->GetActivatableAbilities())
		{
			if (!Spec.Ability)
			{
				continue;
			}

			if (Spec.Ability->GetClass() != SkillData->AbilityClass)
			{
				continue;
			}

			if (Spec.SourceObject.Get() == SkillData)
			{
				return true;
			}
		}

		return false;
	}
}

AFieldEnemyBase::AFieldEnemyBase()
{
	// 기본값 초기화
	LeashDistance = 1500.f;
	PatrolRadius = 800.f;
	EnemyLevel = 1;
	RewardExp = 50;
	MinRewardGold = 10;
	MaxRewardGold = 30;
	bIsReturning = false;

	// 풀링되는 액터는 월드 파티션 그리드에 의해 언로드되지 않도록 공간 로딩을 끕니다.
#if WITH_EDITORONLY_DATA
	SetIsSpatiallyLoaded(false);
#endif

	// 스포너 등을 통해 동적으로 생성되었을 때도 AI가 빙의할 수 있도록 설정
	AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;
}

// Field Class Data에서 태그 초기화
void AFieldEnemyBase::InitializeFieldTagFromClassData()
{
	if (!HasAuthority())
	{
		return;
	}

	if (!FieldClassData)
	{
		return;
	}

	if (!FieldClassData->FieldTag.IsValid())
	{
		return;
	}

	FieldTag = FieldClassData->FieldTag;

	if (AbilitySystemComponent && !AbilitySystemComponent->HasMatchingGameplayTag(FieldTag))
	{
		AbilitySystemComponent->AddLooseGameplayTag(
			FieldTag,
			1,
			EGameplayTagReplicationState::TagOnly
		);
	}
}

FGameplayTag AFieldEnemyBase::GetAttributeSourceTag() const
{
	if (FieldClassData && FieldClassData->FieldTag.IsValid())
	{
		return FieldClassData->FieldTag;
	}

	return DGGameplayTags::Team_Enemy_Field;
}

void AFieldEnemyBase::GrantDefaultAbilities()
{
	// 부모 클래스의 DefaultAbilities 배열 부여
	Super::GrantDefaultAbilities();

	// DataAsset의 AttackSkills 배열도 ASC에 부여
	if (!HasAuthority() || !AbilitySystemComponent || !FieldClassData)
	{
		return;
	}

	for (const TObjectPtr<UEnemySkillData>& SkillDataPtr : FieldClassData->AttackSkills)
	{
		UEnemySkillData* SkillData = SkillDataPtr.Get();
		if (!SkillData || !SkillData->AbilityClass)
		{
			continue;
		}

		if (HasGrantedSkillDataAbility(AbilitySystemComponent, SkillData))
		{
			continue;
		}

		FGameplayAbilitySpec AbilitySpec(SkillData->AbilityClass, 1, INDEX_NONE, SkillData);
		AbilitySystemComponent->GiveAbility(AbilitySpec);
	}
}

void AFieldEnemyBase::ApplyDefaultEffects()
{
	if (!HasAuthority() || !AbilitySystemComponent)
	{
		return;
	}

	const bool bHasStartupEffects =
		FieldClassData && FieldClassData->StartupEffects.Num() > 0;
	const bool bHasEnemyStartupEffects =
		FieldClassData && FieldClassData->EnemyStartupEffects.Num() > 0;

	if (!FieldClassData || (!bHasStartupEffects && !bHasEnemyStartupEffects))
	{
		Super::ApplyDefaultEffects();
		return;
	}

	if (bFieldDataEffectsApplied)
	{
		return;
	}

	FGameplayEffectContextHandle Context =
		AbilitySystemComponent->MakeEffectContext();
	Context.AddSourceObject(this);

	if (bHasStartupEffects)
	{
		for (const auto& EffectClass : FieldClassData->StartupEffects)
		{
			if (EffectClass)
			{
				FGameplayEffectSpecHandle Spec =
					AbilitySystemComponent->MakeOutgoingSpec(EffectClass, 1.f, Context);
				if (Spec.IsValid())
				{
					AbilitySystemComponent->ApplyGameplayEffectSpecToSelf(*Spec.Data.Get());
				}
			}
		}
	}

	if (bHasEnemyStartupEffects)
	{
		for (const auto& EffectClass : FieldClassData->EnemyStartupEffects)
		{
			if (EffectClass)
			{
				FGameplayEffectSpecHandle Spec =
					AbilitySystemComponent->MakeOutgoingSpec(EffectClass, 1.f, Context);
				if (Spec.IsValid())
				{
					AbilitySystemComponent->ApplyGameplayEffectSpecToSelf(*Spec.Data.Get());
				}
			}
		}
	}

	bFieldDataEffectsApplied = true;
}

void AFieldEnemyBase::BeginPlay()
{
	Super::BeginPlay();

	// 스폰되었을 때의 최초 위치를 원점으로 기억
	SpawnOriginLocation = GetActorLocation();

	// BeginPlay 시점에 이미 Possess된 경우를 대비해 Blackboard 값 초기화 시도
	if (HasAuthority())
	{
		UpdateBlackboardValues();
	}
}

void AFieldEnemyBase::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);

	if (HasAuthority())
	{
		InitializeFieldTagFromClassData();

		if (AAIController* AIController = Cast<AAIController>(NewController))
		{
			if (FieldClassData && FieldClassData->BehaviorTree)
			{
				AIController->RunBehaviorTree(FieldClassData->BehaviorTree);
			}
		}

		UpdateBlackboardValues();

		if (AbilitySystemComponent && !bGroggyDelegateBound)
		{
			AbilitySystemComponent
				->GetGameplayAttributeValueChangeDelegate(UDG_EnemyAttributeSet::GetGroggyGaugeAttribute())
				.AddUObject(this, &AFieldEnemyBase::OnGroggyGaugeChanged);

			bGroggyDelegateBound = true;
		}
	}
}

void AFieldEnemyBase::OnGroggyGaugeChanged(const FOnAttributeChangeData& Data)
{
	if (!HasAuthority() || !AbilitySystemComponent || !EnemyAttributeSet)
	{
		return;
	}

	if (AbilitySystemComponent->HasMatchingGameplayTag(DGGameplayTags::State_Enemy_Groggy) ||
		IsDead())
	{
		return;
	}

	const float MaxGroggyGauge = EnemyAttributeSet->GetMaxGroggyGauge();
	if (MaxGroggyGauge <= 0.f)
	{
		return;
	}

	if (Data.NewValue < MaxGroggyGauge)
	{
		return;
	}

	FGameplayEventData Payload;
	AbilitySystemComponent->HandleGameplayEvent(DGGameplayTags::Event_Enemy_Groggy, &Payload);
}

void AFieldEnemyBase::StartReturnToOrigin()
{
	if (bIsReturning)
	{
		return;
	}

	bIsReturning = true;

	// 1. 귀환중 GameplayTag 부여
	if (AbilitySystemComponent)
	{
		AbilitySystemComponent->AddLooseGameplayTag(
			DGGameplayTags::State_Enemy_Returning,
			1,
			EGameplayTagReplicationState::TagOnly
		);
	}

	// 2. 귀환 중 플레이어 공격 무력화(무적) 및 빠른 체력 회복 GameplayEffect 적용
	if (HasAuthority() && AbilitySystemComponent && ReturningEffectClass)
	{
		FGameplayEffectContextHandle Context =
			AbilitySystemComponent->MakeEffectContext();
		Context.AddSourceObject(this);
		FGameplayEffectSpecHandle Spec = AbilitySystemComponent->MakeOutgoingSpec(
			ReturningEffectClass,
			1.f,
			Context
		);
		if (Spec.IsValid())
		{
			ReturningEffectHandle =
				AbilitySystemComponent->ApplyGameplayEffectSpecToSelf(*Spec.Data.Get());
		}
	}

	// 3. AI Blackboard 갱신
	if (AAIController* AIController = Cast<AAIController>(GetController()))
	{
		if (UBlackboardComponent* BBComp = AIController->GetBlackboardComponent())
		{
			BBComp->SetValueAsBool(IsReturningKeyName, true);
		}
	}
}

void AFieldEnemyBase::CompleteReturnToOrigin()
{
	if (!bIsReturning)
	{
		return;
	}

	bIsReturning = false;

	// 1. 귀환중 GameplayTag 제거
	if (AbilitySystemComponent)
	{
		AbilitySystemComponent->RemoveLooseGameplayTag(
			DGGameplayTags::State_Enemy_Returning,
			1
		);
	}

	// 2. 무적/체력 회복 버프(GameplayEffect) 제거
	if (HasAuthority() && AbilitySystemComponent &&
		ReturningEffectHandle.IsValid())
	{
		AbilitySystemComponent->RemoveActiveGameplayEffect(ReturningEffectHandle);
		ReturningEffectHandle.Invalidate();
	}

	// 3. AI Blackboard 갱신
	if (AAIController* AIController = Cast<AAIController>(GetController()))
	{
		if (UBlackboardComponent* BBComp = AIController->GetBlackboardComponent())
		{
			BBComp->SetValueAsBool(IsReturningKeyName, false);
		}
	}
}

void AFieldEnemyBase::UpdateBlackboardValues()
{
	if (AAIController* AIController = Cast<AAIController>(GetController()))
	{
		if (UBlackboardComponent* BBComp = AIController->GetBlackboardComponent())
		{
			BBComp->SetValueAsVector(SpawnOriginKeyName, SpawnOriginLocation);
			BBComp->SetValueAsFloat(PatrolRadiusKeyName, PatrolRadius);
			BBComp->SetValueAsFloat(LeashDistanceKeyName, LeashDistance);
			BBComp->SetValueAsBool(IsReturningKeyName, bIsReturning);
		}
	}
}

void AFieldEnemyBase::InitFromDataAsset(UFieldCharacterClassData* Data)
{
	if (!Data)
	{
		return;
	}

	FieldClassData = Data;
	EnemyLevel = Data->EnemyLevel;
	PatrolRadius = Data->PatrolRadius;
	LeashDistance = Data->LeashDistance;
	RewardExp = Data->RewardExp;
	MinRewardGold = Data->MinRewardGold;
	MaxRewardGold = Data->MaxRewardGold;
	ReturningEffectClass = Data->ReturningEffectClass;

	// Initialize tags and effects if already possessed
	if (HasAuthority())
	{
		InitializeFieldTagFromClassData();

		// 데이터 에셋이 세팅/변경되었으므로 어빌리티와 이펙트를 명시적으로 다시 부여
		GrantDefaultAbilities();
		ApplyDefaultEffects();

		// 만약 AIController가 빙의된 상태라면 즉시 BT 실행 (이때 Blackboard도 생성됨)
		if (AAIController* AIController = Cast<AAIController>(GetController()))
		{
			if (FieldClassData->BehaviorTree)
			{
				AIController->RunBehaviorTree(FieldClassData->BehaviorTree);
			}
		}

		// Blackboard needs to be updated with new patrol/leash stats
		// 반드시 BT가 실행되어 Blackboard가 만들어진 이후에 값을 세팅해야 합니다.
		UpdateBlackboardValues();
	}
}

void AFieldEnemyBase::OnSpawnedFromPool(const FVector& SpawnLocation)
{
	if (!HasAuthority())
	{
		return;
	}

	// 생존 상태로 초기화 (사망 플래그 및 태그 해제)
	bIsDead = false;
	if (AbilitySystemComponent)
	{
		AbilitySystemComponent->RemoveLooseGameplayTag(DGGameplayTags::State_Enemy_Dead, 99);

		// 체력을 MaxHealth로 복구 (Instant GE로 깎인 체력 복원)
		if (AttributeSet)
		{
			AbilitySystemComponent->SetNumericAttributeBase(
				UDG_AttributeSet::GetHealthAttribute(),
				AttributeSet->GetMaxHealth()
			);
		}
	}

	// 이전 데스 몽타주가 계속 루프 돌고 있었다면 강제 정지
	if (USkeletalMeshComponent* MeshComp = GetMesh())
	{
		if (UAnimInstance* AnimInstance = MeshComp->GetAnimInstance())
		{
			AnimInstance->StopAllMontages(0.0f);
		}
	}

	// 캡슐 절반 높이만큼 올려서 바닥에 박히는 현상 방지
	FVector AdjustedLocation = SpawnLocation;
	if (UCapsuleComponent* Capsule = GetCapsuleComponent())
	{
		AdjustedLocation.Z += Capsule->GetScaledCapsuleHalfHeight();
	}

	SetActorLocation(AdjustedLocation);
	SpawnOriginLocation = AdjustedLocation;
	bIsReturning = false;

	// 모든 클라이언트(및 서버)에 스폰되었음을 알림
	// (액터 가시성, 콜리전, 틱, 컴포넌트 콜리전 복구 포함)
	Multicast_OnSpawnedFromPool();

	// AI 재활성화 및 BT 실행
	if (AAIController* AIController = Cast<AAIController>(GetController()))
	{
		// AAIControllerBase의 사망 시 정지 플래그 및 감각 복구
		if (AAIControllerBase* AIBase = Cast<AAIControllerBase>(AIController))
		{
			AIBase->RestartAIFromPool();
		}

		AIController->GetBrainComponent()->RestartLogic();
		if (FieldClassData && FieldClassData->BehaviorTree)
		{
			AIController->RunBehaviorTree(FieldClassData->BehaviorTree);
		}
		UpdateBlackboardValues();
	}

	// (주의) ApplyDefaultEffects()를 여기서 다시 호출하면 Instant 형태의 스탯 증가 효과가
	// 매 스폰마다 중첩(Stacking)되므로 호출하지 않습니다. 스탯은 유지되고 체력만 위에서 리셋됩니다.
}

void AFieldEnemyBase::OnReturnedToPool()
{
	if (!HasAuthority())
	{
		return;
	}

	// 클라이언트(및 서버)에 풀 반환 알림
	Multicast_OnReturnedToPool();

	// AI 정지
	if (AAIController* AIController = Cast<AAIController>(GetController()))
	{
		if (AIController->GetBrainComponent())
		{
			AIController->GetBrainComponent()->StopLogic(TEXT("Returned to Pool"));
		}
	}

	// 기존 걸려있는 이펙트, 타이머 등을 정리해야 한다면 여기서 처리
	if (AbilitySystemComponent)
	{
		AbilitySystemComponent->RemoveAllGameplayCues();

		if (ReturningEffectHandle.IsValid())
		{
			AbilitySystemComponent->RemoveActiveGameplayEffect(ReturningEffectHandle);
			ReturningEffectHandle.Invalidate();
		}

		AbilitySystemComponent->RemoveLooseGameplayTag(DGGameplayTags::State_Enemy_Returning, 99);
	}
}

void AFieldEnemyBase::OnDeathAnimationFinished()
{
	if (!HasAuthority())
	{
		return;
	}

	if (UWorld* World = GetWorld())
	{
		if (UFieldEnemyPoolSubsystem* PoolSubsystem = World->GetSubsystem<UFieldEnemyPoolSubsystem>())
		{
			PoolSubsystem->ReturnEnemy(this);
		}
		else
		{
			Destroy(); // fallback
		}
	}
}

void AFieldEnemyBase::Multicast_OnSpawnedFromPool_Implementation()
{
	// 액터 레벨 상태 복구 (서버+클라이언트 모두에서 실행)
	SetActorHiddenInGame(false);
	SetActorEnableCollision(true);
	SetActorTickEnabled(true);

	// 이전 데스 몽타주가 계속 루프 돌고 있었다면 강제 정지 (클라이언트 필수)
	if (USkeletalMeshComponent* MeshComp = GetMesh())
	{
		if (UAnimInstance* AnimInstance = MeshComp->GetAnimInstance())
		{
			AnimInstance->StopAllMontages(0.0f);
		}
	}

	// 부활 시 미니맵에 다시 마커 등록
	if (MinimapMarkerComponent)
	{
		MinimapMarkerComponent->RegisterToSubsystem();
	}

	// 사망 시 완전히 껐던 컴포넌트들의 충돌 및 물리/이동 상태 복구
	if (UCapsuleComponent* Capsule = GetCapsuleComponent())
	{
		Capsule->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		Capsule->SetCollisionResponseToChannel(ECC_Pawn, ECR_Block);
	}

	if (USkeletalMeshComponent* MeshComp = GetMesh())
	{
		MeshComp->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		MeshComp->SetCollisionResponseToChannel(ECC_Pawn, ECR_Block);
	}

	if (UCharacterMovementComponent* MoveComp = GetCharacterMovement())
	{
		MoveComp->SetMovementMode(MOVE_Walking);
	}
}

void AFieldEnemyBase::Multicast_OnReturnedToPool_Implementation()
{
	// 풀로 돌아간 액터는 화면에서 숨기고 충돌을 완전히 해제 (보이지 않는 벽 방지)
	SetActorHiddenInGame(true);
	SetActorEnableCollision(false);
	SetActorTickEnabled(false);
}

UEnemySkillData* AFieldEnemyBase::GetRandomAttackSkillData() const
{
	if (!FieldClassData)
	{
		return nullptr;
	}

	const TArray<TObjectPtr<UEnemySkillData>>& Skills = FieldClassData->AttackSkills;
	if (Skills.Num() == 0)
	{
		return nullptr;
	}

	const int32 RandomIndex = FMath::RandRange(0, Skills.Num() - 1);
	return Skills[RandomIndex].Get();
}

TSubclassOf<UGameplayAbility> AFieldEnemyBase::GetRandomAttackAbilityClass() const
{
	const UEnemySkillData* SkillData = GetRandomAttackSkillData();
	if (!SkillData)
	{
		
		return nullptr;
	}

	return SkillData->AbilityClass;
}