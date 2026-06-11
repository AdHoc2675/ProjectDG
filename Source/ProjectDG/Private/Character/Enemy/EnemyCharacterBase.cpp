#include "Character/Enemy/EnemyCharacterBase.h"

#include "AI/Controller/AIControllerBase.h"
#include "AbilitySystemComponent.h"
#include "Abilities/GameplayAbility.h"
#include "Animation/AnimInstance.h"
#include "Animation/AnimMontage.h"

#include "AttributeSet.h"
#include "Data/Attribute/DT_Attribute.h"
#include "Engine/DataTable.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"


#include "Core/DG_GameplayTags.h"

#include "GAS/Attributes/DG_AttributeSet.h"
#include "GAS/Attributes/DG_EnemyAttributeSet.h"

#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "Components/UI/DGMinimapMarkerComponent.h"
#include "Core/DG_Debug.h"

#include "System/DGDamageNumberPoolSubsystem.h"
#include "UI/Widget/Damage/DGDamageNumberActor.h"

#include "UObject/ConstructorHelpers.h"

AEnemyCharacterBase::AEnemyCharacterBase()
{
	AbilitySystemComponent = CreateDefaultSubobject<UAbilitySystemComponent>(
		TEXT("AbilitySystemComponent"));

	AttributeSet = CreateDefaultSubobject<UDG_AttributeSet>(TEXT("AttributeSet"));
	EnemyAttributeSet = CreateDefaultSubobject<UDG_EnemyAttributeSet>(TEXT("EnemyAttributeSet"));

	// 드롭 컴포넌트 생성
	LootDropComponent2 = CreateDefaultSubobject<UDGLootDropComponent>(TEXT("LootDropComponent"));

	// 미니맵 마커 생성 및 기본 타입 설정
	MinimapMarkerComponent = CreateDefaultSubobject<UDGMinimapMarkerComponent>(TEXT("MinimapMarkerComponent"));
	MinimapMarkerComponent->MarkerType = EMinimapMarkerType::Enemy;

	// 서버에서도 소켓 기반 트레이스가 정상 작동하도록 애니메이션 본을 항상 갱신하게 설정
	if (USkeletalMeshComponent* MeshComp = GetMesh())
	{
		MeshComp->VisibilityBasedAnimTickOption = EVisibilityBasedAnimTickOption::AlwaysTickPoseAndRefreshBones;
	}

	static ConstructorHelpers::FClassFinder<ADGDamageNumberActor> DamageActorClassFinder(
		TEXT("/Game/__ProjectDG/__BP/UI/Damage/BP_DGDamageNumberActor.BP_DGDamageNumberActor_C"));

	if (DamageActorClassFinder.Succeeded())
	{
		DamageNumberClass = DamageActorClassFinder.Class;
	}
}

void AEnemyCharacterBase::BeginPlay()
{
	ABaseCharacter::BeginPlay();

	InitializeEnemyAbilitySystem();
}

void AEnemyCharacterBase::PossessedBy(AController* NewController)
{
	ABaseCharacter::PossessedBy(NewController);

	InitializeEnemyAbilitySystem();

	if (HasAuthority())
	{
		GrantDefaultAbilities();
		ApplyAttributeRowFromDataTable();
		ApplyDefaultEffects();
		
		if (AttributeSet)
		{
			Debug::Print(FString::Printf(
				TEXT("[EnemyAttribute] After GE. Health=%.1f MaxHealth=%.1f AttackPower=%.1f Defense=%.1f"),
				AttributeSet->GetHealth(),
				AttributeSet->GetMaxHealth(),
				AttributeSet->GetAttackPower(),
				AttributeSet->GetDefense()
			));
		}
		
		BindEnemyAttributeDelegatesOnce();
	}
}

void AEnemyCharacterBase::InitializeEnemyAbilitySystem()
{
	if (!AbilitySystemComponent)
	{
		return;
	}

	/**
	 * Enemy 구조:
	 * - OwnerActor  = this
	 * - AvatarActor = this
	 *
	 * 즉 캐릭터 본체가 ASC를 직접 들고 있으므로
	 * 둘 다 자기 자신으로 초기화한다.
	 */
	AbilitySystemComponent->InitAbilityActorInfo(this, this);
}

UAbilitySystemComponent* AEnemyCharacterBase::GetCharacterAbilitySystemComponent() const
{
	return AbilitySystemComponent;
}

const UAttributeSet* AEnemyCharacterBase::GetCharacterAttributeSet() const
{
	return AttributeSet;
}

bool AEnemyCharacterBase::HasGrantedAbilityClass(TSubclassOf<UGameplayAbility> AbilityClass) const
{
	if (!AbilitySystemComponent || !AbilityClass)
	{
		return false;
	}

	for (const FGameplayAbilitySpec& Spec : AbilitySystemComponent->GetActivatableAbilities())
	{
		if (Spec.Ability && Spec.Ability->GetClass() == AbilityClass)
		{
			return true;
		}
	}

	return false;
}

void AEnemyCharacterBase::GrantDefaultAbilities()
{
	if (!HasAuthority() || !AbilitySystemComponent)
	{
		return;
	}

	for (const auto& AbilityClass : DefaultAbilities)
	{
		if (!AbilityClass)
		{
			continue;
		}

		if (HasGrantedAbilityClass(AbilityClass))
		{
			continue;
		}

		AbilitySystemComponent->GiveAbility(FGameplayAbilitySpec(AbilityClass, 1));
	}
}

void AEnemyCharacterBase::ApplyDefaultEffects()
{
	if (!HasAuthority() || !AbilitySystemComponent)
	{
		return;
	}

	if (bDefaultEffectsApplied)
	{
		return;
	}

	FGameplayEffectContextHandle Context =
		AbilitySystemComponent->MakeEffectContext();
	Context.AddSourceObject(this);

	for (const auto& EffectClass : DefaultEffects)
	{
		if (!EffectClass)
		{
			continue;
		}

		FGameplayEffectSpecHandle Spec =
			AbilitySystemComponent->MakeOutgoingSpec(EffectClass, 1.f, Context);

		if (Spec.IsValid())
		{
			AbilitySystemComponent->ApplyGameplayEffectSpecToSelf(*Spec.Data.Get());
		}
	}

	bDefaultEffectsApplied = true;
}

void AEnemyCharacterBase::ApplyAttributeRowFromDataTable()
{
	if (!HasAuthority() || !AbilitySystemComponent || !AttributeSet)
	{
		Debug::Print(TEXT("[EnemyAttribute] Skip: Authority/ASC/AttributeSet invalid"));
		return;
	}

	if (bAttributeRowApplied)
	{
		Debug::Print(TEXT("[EnemyAttribute] Skip: already applied"));
		return;
	}

	if (!AttributeDataTable)
	{
		Debug::Print(TEXT("[EnemyAttribute] Skip: AttributeDataTable is null"));
		return;
	}

	const FGameplayTag SourceTag = GetAttributeSourceTag();
	const FName RowName = ResolveAttributeRowNameFromTag(SourceTag);

	Debug::Print(FString::Printf(
		TEXT("[EnemyAttribute] SourceTag=%s RowName=%s"),
		*SourceTag.ToString(),
		*RowName.ToString()
	));

	if (RowName.IsNone())
	{
		Debug::Print(TEXT("[EnemyAttribute] Skip: RowName is none"));
		return;
	}

	const FDT_Attribute* AttributeRow = AttributeDataTable->FindRow<FDT_Attribute>(
		RowName,
		TEXT("AEnemyCharacterBase::ApplyAttributeRowFromDataTable")
	);

	if (!AttributeRow)
	{
		Debug::Print(FString::Printf(
			TEXT("[EnemyAttribute] Row not found. RowName=%s"),
			*RowName.ToString()
		));
		return;
	}

	Debug::Print(FString::Printf(
		TEXT("[EnemyAttribute] Row found. Health=%.1f MaxHealth=%.1f AttackPower=%.1f Defense=%.1f"),
		AttributeRow->Health,
		AttributeRow->MaxHealth,
		AttributeRow->AttackPower,
		AttributeRow->Defense
	));

	ApplyAttributeRowToAttributeSet(*AttributeRow);

	Debug::Print(FString::Printf(
		TEXT("[EnemyAttribute] Applied. ASC Health=%.1f MaxHealth=%.1f"),
		AttributeSet->GetHealth(),
		AttributeSet->GetMaxHealth()
	));

	bAttributeRowApplied = true;
}

FGameplayTag AEnemyCharacterBase::GetAttributeSourceTag() const
{
	return DGGameplayTags::Team_Enemy;
}

FName AEnemyCharacterBase::ResolveAttributeRowNameFromTag(const FGameplayTag& SourceTag) const
{
	if (!SourceTag.IsValid())
	{
		return NAME_None;
	}

	const FString TagString = SourceTag.ToString();

	FString LeftString;
	FString RightString;

	if (TagString.Split(TEXT("."), &LeftString, &RightString, ESearchCase::IgnoreCase, ESearchDir::FromEnd))
	{
		return FName(*RightString);
	}

	return FName(*TagString);
}

void AEnemyCharacterBase::ApplyAttributeRowToAttributeSet(const FDT_Attribute& AttributeRow) const
{
	if (!AbilitySystemComponent)
	{
		return;
	}

	AbilitySystemComponent->SetNumericAttributeBase(UDG_AttributeSet::GetMaxHealthAttribute(), AttributeRow.MaxHealth);
	AbilitySystemComponent->SetNumericAttributeBase(
		UDG_AttributeSet::GetHealthAttribute(),
		FMath::Clamp(AttributeRow.Health, 0.f, AttributeRow.MaxHealth)
	);

	AbilitySystemComponent->SetNumericAttributeBase(UDG_AttributeSet::GetMaxMentalAttribute(), AttributeRow.MaxMental);
	AbilitySystemComponent->SetNumericAttributeBase(
		UDG_AttributeSet::GetMentalAttribute(),
		FMath::Clamp(AttributeRow.Mental, 0.f, AttributeRow.MaxMental)
	);

	AbilitySystemComponent->SetNumericAttributeBase(UDG_AttributeSet::GetMaxStaminaAttribute(), AttributeRow.MaxStamina);
	AbilitySystemComponent->SetNumericAttributeBase(
		UDG_AttributeSet::GetStaminaAttribute(),
		FMath::Clamp(AttributeRow.Stamina, 0.f, AttributeRow.MaxStamina)
	);

	AbilitySystemComponent->SetNumericAttributeBase(UDG_AttributeSet::GetMainStatAttribute(), AttributeRow.MainStat);
	AbilitySystemComponent->SetNumericAttributeBase(UDG_AttributeSet::GetAttackPowerAttribute(), AttributeRow.AttackPower);
	AbilitySystemComponent->SetNumericAttributeBase(UDG_AttributeSet::GetDefenseAttribute(), AttributeRow.Defense);
	AbilitySystemComponent->SetNumericAttributeBase(UDG_AttributeSet::GetHealthCoefficientAttribute(), AttributeRow.HealthCoefficient);
	AbilitySystemComponent->SetNumericAttributeBase(UDG_AttributeSet::GetDefenseCoefficientAttribute(), AttributeRow.DefenseCoefficient);
	AbilitySystemComponent->SetNumericAttributeBase(UDG_AttributeSet::GetCriticalRateAttribute(), AttributeRow.CriticalRate);
	AbilitySystemComponent->SetNumericAttributeBase(UDG_AttributeSet::GetCriticalDamageAttribute(), AttributeRow.CriticalDamage);
	AbilitySystemComponent->SetNumericAttributeBase(UDG_AttributeSet::GetMoveSpeedAttribute(), AttributeRow.MoveSpeed);
	AbilitySystemComponent->SetNumericAttributeBase(UDG_AttributeSet::GetAttackSpeedAttribute(), AttributeRow.AttackSpeed);
	AbilitySystemComponent->SetNumericAttributeBase(UDG_AttributeSet::GetGroggyDamageAttribute(), AttributeRow.GroggyDamage);
	AbilitySystemComponent->SetNumericAttributeBase(UDG_AttributeSet::GetFinalDamageIncreaseAttribute(), AttributeRow.FinalDamageIncrease);
	AbilitySystemComponent->SetNumericAttributeBase(UDG_AttributeSet::GetDamageReductionAttribute(), AttributeRow.DamageReduction);
	AbilitySystemComponent->SetNumericAttributeBase(UDG_AttributeSet::GetCooldownReductionAttribute(), AttributeRow.CooldownReduction);
	AbilitySystemComponent->SetNumericAttributeBase(UDG_AttributeSet::GetMentalRecoveryIncreaseAttribute(), AttributeRow.MentalRecoveryIncrease);
	AbilitySystemComponent->SetNumericAttributeBase(UDG_AttributeSet::GetLifeStealAttribute(), AttributeRow.LifeSteal);
	AbilitySystemComponent->SetNumericAttributeBase(UDG_AttributeSet::GetGroggyDamageIncreaseRateAttribute(), AttributeRow.GroggyDamageIncreaseRate);

	if (UCharacterMovementComponent* MovementComp = GetCharacterMovement())
	{
		MovementComp->MaxWalkSpeed = AttributeRow.MoveSpeed;
	}
}

void AEnemyCharacterBase::BindEnemyAttributeDelegatesOnce()
{
	if (!HasAuthority() || !AbilitySystemComponent)
	{
		return;
	}

	if (!bHealthDeathDelegateBound)
	{
		AbilitySystemComponent
			->GetGameplayAttributeValueChangeDelegate(UDG_AttributeSet::GetHealthAttribute())
			.AddUObject(this, &AEnemyCharacterBase::OnHealthChanged_DeathCheck);

		bHealthDeathDelegateBound = true;
	}
}

void AEnemyCharacterBase::HandleDeath()
{
	if (HasAuthority())
	{
		if (AbilitySystemComponent)
		{
			AbilitySystemComponent->AddLooseGameplayTag(
				DGGameplayTags::State_Enemy_Dead,
				1,
				EGameplayTagReplicationState::TagOnly);
		}

		if (AAIControllerBase* AIController = Cast<AAIControllerBase>(GetController()))
		{
			AIController->StopAIOnDeath();
		}
	}

	if (HasAuthority())
	{
		MulticastPlayDeathMontage();
	}

	if (HasAuthority() && LootDropComponent2)
	{
		LootDropComponent2->ProcessDrop(GetActorLocation());
	}

	Super::HandleDeath();
}

void AEnemyCharacterBase::MulticastPlayDeathMontage_Implementation()
{
	// 사망 시 플레이어가 시체를 통과할 수 있도록 Pawn과의 충돌을 무시(Ignore)로 변경합니다.
	if (UCapsuleComponent* Capsule = GetCapsuleComponent())
	{
		Capsule->SetCollisionResponseToChannel(ECC_Pawn, ECR_Ignore);
	}

	if (USkeletalMeshComponent* MeshComp = GetMesh())
	{
		MeshComp->SetCollisionResponseToChannel(ECC_Pawn, ECR_Ignore);
	}

	// 사망과 동시에 미니맵에서 마커 지우기
	if (MinimapMarkerComponent)
	{
		MinimapMarkerComponent->UnregisterFromSubsystem();
	}

	if (!DeathMontage)
	{
		return;
	}

	if (USkeletalMeshComponent* MeshComp = GetMesh())
	{
		if (UAnimInstance* AnimInstance = MeshComp->GetAnimInstance())
		{
			AnimInstance->Montage_Play(DeathMontage);
		}
	}
}

void AEnemyCharacterBase::Multicast_SpawnAOETelegraph_Implementation(
	UNiagaraSystem* VFX,
	FVector Location,
	FName ScaleParamName,
	float Scale)
{
	if (!VFX)
	{
		return;
	}

	if (AOETelegraphComponent)
	{
		AOETelegraphComponent->DeactivateImmediate();
		AOETelegraphComponent = nullptr;
	}

	AOETelegraphComponent = UNiagaraFunctionLibrary::SpawnSystemAtLocation(this, VFX, Location);

	if (AOETelegraphComponent && ScaleParamName != NAME_None)
	{
		AOETelegraphComponent->SetVariableFloat(ScaleParamName, Scale);
	}
}

void AEnemyCharacterBase::Multicast_SpawnAttachedDirectionalTelegraph_Implementation(
	UNiagaraSystem* VFX,
	FRotator RelativeRotation,
	float Length,
	float Width,
	FName LengthParamName,
	FName WidthParamName)
{
	if (!VFX)
	{
		return;
	}

	if (AOETelegraphComponent)
	{
		AOETelegraphComponent->DeactivateImmediate();
		AOETelegraphComponent = nullptr;
	}

	if (UCapsuleComponent* Capsule = GetCapsuleComponent())
	{
		// 캡슐의 바닥(발 밑)에 맞추기 위해 Z 오프셋 계산
		FVector Offset = FVector(0.f, 0.f, -Capsule->GetScaledCapsuleHalfHeight());

		AOETelegraphComponent = UNiagaraFunctionLibrary::SpawnSystemAttached(
			VFX,
			Capsule,
			NAME_None,
			Offset,
			RelativeRotation,
			EAttachLocation::KeepRelativeOffset,
			true
		);

		if (AOETelegraphComponent)
		{
			if (LengthParamName != NAME_None)
			{
				AOETelegraphComponent->SetVariableFloat(LengthParamName, Length);
			}

			if (WidthParamName != NAME_None)
			{
				AOETelegraphComponent->SetVariableFloat(WidthParamName, Width);
			}
		}
	}
}

void AEnemyCharacterBase::Multicast_DestroyAOETelegraph_Implementation()
{
	if (AOETelegraphComponent)
	{
		AOETelegraphComponent->DeactivateImmediate();
		AOETelegraphComponent = nullptr;
	}
}

void AEnemyCharacterBase::Multicast_ShowDamageNumber_Implementation(
	float DamageAmount,
	bool bIsCritical,
	AActor* DamageInstigator)
{
	//UE_LOG(LogTemp, Warning, TEXT("[DamageNumber] Multicast called! Damage: %.1f"), DamageAmount);

	if (!DamageNumberClass)
	{
		UE_LOG(LogTemp, Error,
			TEXT("[DamageNumber] DamageNumberClass is NULL! (적 블루프린트에서 DamageNumberClass를 설정했는지 확인하세요)"));
		return;
	}

	if (DamageAmount <= 0.f)
	{
		return;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	if (UDGDamageNumberPoolSubsystem* PoolSubsystem = World->GetSubsystem<UDGDamageNumberPoolSubsystem>())
	{
		const float CapsuleHalfHeight = GetCapsuleComponent()
			? GetCapsuleComponent()->GetScaledCapsuleHalfHeight()
			: 50.f;

		FVector SpawnLoc = GetActorLocation() + FVector(
			FMath::RandRange(-40.f, 40.f),
			FMath::RandRange(-40.f, 40.f),
			0.f);

		const float BottomZ = GetActorLocation().Z - CapsuleHalfHeight;
		const float TargetHeight = FMath::Max(CapsuleHalfHeight, FMath::RandRange(50.f, 100.f));
		SpawnLoc.Z = BottomZ + TargetHeight;

		if (ADGDamageNumberActor* DmgActor = PoolSubsystem->AcquireDamageNumber(DamageNumberClass, SpawnLoc))
		{
			//UE_LOG(LogTemp, Warning, TEXT("[DamageNumber] Actor Acquired! Calling ShowDamage."));
			DmgActor->ShowDamage(DamageAmount, bIsCritical);
		}
	}
}

void AEnemyCharacterBase::OnHealthChanged_DeathCheck(const FOnAttributeChangeData& Data)
{
	if (!HasAuthority())
	{
		return;
	}

	if (bIsDead)
	{
		return;
	}

	if (Data.NewValue <= 0.f)
	{
		Die();
	}
}