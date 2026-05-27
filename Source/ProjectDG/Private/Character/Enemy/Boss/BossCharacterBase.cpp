// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/Enemy/Boss/BossCharacterBase.h"

#include "AbilitySystemComponent.h"
#include "AIController.h"
#include "Animation/AnimInstance.h"
#include "Animation/AnimMontage.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "BrainComponent.h"
#include "Character/Enemy/Boss/Data/BossCharacterClassData.h"
#include "Core/DG_GameplayTags.h"
#include "GAS/Attributes/DG_AttributeSet.h"
#include "GAS/Attributes/DG_BossAttributeSet.h"
#include "GAS/Attributes/DG_EnemyAttributeSet.h"
#include "GameFramework/CharacterMovementComponent.h"

ABossCharacterBase::ABossCharacterBase()
{
	BossAttributeSet = CreateDefaultSubobject<UDG_BossAttributeSet>(TEXT("BossAttributeSet"));
	EnemyAttributeSet = CreateDefaultSubobject<UDG_EnemyAttributeSet>(TEXT("EnemyAttributeSet"));
}

//Boss Class Data 초기 태그 설정
void ABossCharacterBase::InitializeBossTagFromClassData()
{
	if (!HasAuthority())
	{
		return;
	}

	if (!BossClassData)
	{
		return;
	}

	if (!BossClassData->BossTag.IsValid())
	{
		return;
	}
	
	BossTag = BossClassData->BossTag;

	if (AbilitySystemComponent)
	{
		AbilitySystemComponent->AddLooseGameplayTag(BossTag, 1, EGameplayTagReplicationState::TagOnly);

		if (BossClassData->InitialPhaseTag.IsValid())
		{
			AbilitySystemComponent->AddLooseGameplayTag(BossClassData->InitialPhaseTag, 1, EGameplayTagReplicationState::TagOnly);
		}

		// UE_LOG(LogTemp, Warning, TEXT("[BossCharacterBase] InitializeBossTagFromClassData Success. NetMode=%d Name=%s"),
		// 	static_cast<int32>(GetNetMode()),
		// 	*GetName());
	}
	
}

void ABossCharacterBase::BeginPlay()
{
	Super::BeginPlay();
}

void ABossCharacterBase::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);

	if (HasAuthority())
	{
		InitializeBossTagFromClassData();
		ApplyBossSpecialEffects();

		if (AbilitySystemComponent)
		{
			AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(
				UDG_AttributeSet::GetHealthAttribute()
			).AddUObject(this, &ABossCharacterBase::OnHealthChanged);
			AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(
				UDG_EnemyAttributeSet::GetGroggyGaugeAttribute()
			).AddUObject(this, &ABossCharacterBase::OnGroggyGaugeChanged);
		}
	}
}

// 보스 전용 효과 적용 (보스 전용 AttributeSet과 연동된 GE)
void ABossCharacterBase::ApplyDefaultEffects()
{
	if (!HasAuthority() || !AbilitySystemComponent)
	{
		return;
	}

	const bool bHasStartupEffects = BossClassData && BossClassData->StartupEffects.Num() > 0;
	const bool bHasEnemyStartupEffects = BossClassData && BossClassData->EnemyStartupEffects.Num() > 0;

	if (!BossClassData || (!bHasStartupEffects && !bHasEnemyStartupEffects))
	{
		Super::ApplyDefaultEffects();
		return;
	}

	FGameplayEffectContextHandle Context = AbilitySystemComponent->MakeEffectContext();
	Context.AddSourceObject(this);

	// 기본 AttributeSet 초기화용 GE 적용
	if (bHasStartupEffects)
	{
		for (const auto& EffectClass : BossClassData->StartupEffects)
		{
			if (EffectClass)
			{
				FGameplayEffectSpecHandle Spec = AbilitySystemComponent->MakeOutgoingSpec(EffectClass, 1.f, Context);
				if (Spec.IsValid())
				{
					AbilitySystemComponent->ApplyGameplayEffectSpecToSelf(*Spec.Data.Get());
				}
			}
		}
	}

	// 적 공통 AttributeSet 초기화용 GE 적용
	if (bHasEnemyStartupEffects)
	{
		for (const auto& EffectClass : BossClassData->EnemyStartupEffects)
		{
			if (EffectClass)
			{
				FGameplayEffectSpecHandle Spec = AbilitySystemComponent->MakeOutgoingSpec(EffectClass, 1.f, Context);
				if (Spec.IsValid())
				{
					AbilitySystemComponent->ApplyGameplayEffectSpecToSelf(*Spec.Data.Get());
				}
			}
		}
	}
}

void ABossCharacterBase::ApplyBossSpecialEffects()
{
	if (bBossSpecialEffectsApplied)
	{
		return;
	}

	if (!BossClassData || BossClassData->BossStartupEffects.Num() == 0)
	{
		return;
	}

	if (!AbilitySystemComponent)
	{
		return;
	}

	// 보스 전용 AttributeSet에 대응하는 GE를 소환 시점에 적용
	FGameplayEffectContextHandle Context = AbilitySystemComponent->MakeEffectContext();
	Context.AddSourceObject(this);

	for (const auto& EffectClass : BossClassData->BossStartupEffects)
	{
		if (EffectClass)
		{
			FGameplayEffectSpecHandle Spec = AbilitySystemComponent->MakeOutgoingSpec(EffectClass, 1.f, Context);
			if (Spec.IsValid())
			{
				AbilitySystemComponent->ApplyGameplayEffectSpecToSelf(*Spec.Data.Get());
			}
		}
	}

	if (BossAttributeSet)
	{
		// 보스 전용 AttributeSet 값이 적용됐는지 확인용 로그
		UE_LOG(
			LogTemp,
			Log,
			TEXT("[BossCharacterBase] Boss special effects applied. Phase=%.2f Threshold=%.2f Rage=%.2f/%.2f DR=%.2f"),
			BossAttributeSet->GetCurrentPhase(),
			BossAttributeSet->GetPhaseThreshold(),
			BossAttributeSet->GetRageGauge(),
			BossAttributeSet->GetMaxRageGauge(),
			BossAttributeSet->GetDamageReduction()
		);
	}

	bBossSpecialEffectsApplied = true;
}

void ABossCharacterBase::OnHealthChanged(const FOnAttributeChangeData& Data)
{
	if (!AbilitySystemComponent || !BossAttributeSet)
	{
		return;
	}

	const float MaxHealth = AbilitySystemComponent->GetNumericAttribute(UDG_AttributeSet::GetMaxHealthAttribute());
	if (MaxHealth <= 0.f)
	{
		return;
	}

	const float Ratio = Data.NewValue / MaxHealth;
	UpdateHealthPhaseTags(Ratio);
}

void ABossCharacterBase::OnGroggyGaugeChanged(const FOnAttributeChangeData& Data)
{
	if (!HasAuthority() || !AbilitySystemComponent || !EnemyAttributeSet)
	{
		return;
	}

	if (bIsGroggy || IsDead())
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

	StartGroggy();
}

void ABossCharacterBase::StartGroggy()
{
	if (!HasAuthority() || bIsGroggy || !AbilitySystemComponent)
	{
		return;
	}

	bIsGroggy = true;

	AbilitySystemComponent->AddLooseGameplayTag(
		DGGameplayTags::State_Boss_Groggy,
		1,
		EGameplayTagReplicationState::TagOnly
	);

	AbilitySystemComponent->CancelAllAbilities();

	if (USkeletalMeshComponent* MeshComp = GetMesh())
	{
		if (UAnimInstance* AnimInstance = MeshComp->GetAnimInstance())
		{
			AnimInstance->StopAllMontages(0.1f);
		}
	}

	if (AAIController* AIController = Cast<AAIController>(GetController()))
	{
		AIController->StopMovement();

		if (UBrainComponent* BrainComp = AIController->GetBrainComponent())
		{
			BrainComp->StopLogic(TEXT("Groggy"));
		}
	}

	if (UCharacterMovementComponent* MoveComp = GetCharacterMovement())
	{
		MoveComp->StopMovementImmediately();
		MoveComp->DisableMovement();
	}

	AbilitySystemComponent->SetNumericAttributeBase(
		UDG_EnemyAttributeSet::GetGroggyGaugeAttribute(),
		0.f
	);

	if (GroggyMontage)
	{
		MulticastPlayGroggyMontage();
	}

	if (GroggyDuration > 0.f)
	{
		GetWorldTimerManager().SetTimer(
			GroggyTimerHandle,
			this,
			&ABossCharacterBase::EndGroggy,
			GroggyDuration,
			false
		);
	}
}

void ABossCharacterBase::EndGroggy()
{
	if (!bIsGroggy)
	{
		return;
	}

	bIsGroggy = false;

	GetWorldTimerManager().ClearTimer(GroggyTimerHandle);

	if (AbilitySystemComponent)
	{
		AbilitySystemComponent->RemoveLooseGameplayTag(DGGameplayTags::State_Boss_Groggy, 1);
	}

	if (UCharacterMovementComponent* MoveComp = GetCharacterMovement())
	{
		MoveComp->SetMovementMode(MOVE_Walking);
	}

	if (AAIController* AIController = Cast<AAIController>(GetController()))
	{
		if (UBrainComponent* BrainComp = AIController->GetBrainComponent())
		{
			BrainComp->RestartLogic();
		}
	}
}

void ABossCharacterBase::MulticastPlayGroggyMontage_Implementation()
{
	if (!GroggyMontage)
	{
		return;
	}

	if (USkeletalMeshComponent* MeshComp = GetMesh())
	{
		if (UAnimInstance* AnimInstance = MeshComp->GetAnimInstance())
		{
			AnimInstance->Montage_Play(GroggyMontage);
		}
	}
}

void ABossCharacterBase::UpdateHealthPhaseTags(float HealthRatio)
{
	if (!BossClassData || !AbilitySystemComponent || !BossAttributeSet)
	{
		return;
	}

	// PhaseEntries는 HealthRatioThreshold 내림차순 정렬 가정
	// (e.g. [{Phase2, 0.66}, {Phase3, 0.33}])
	// 인덱스 0 → Phase 2, 인덱스 1 → Phase 3, ...
	for (int32 i = 0; i < BossClassData->PhaseEntries.Num(); ++i)
	{
		const FBossPhaseEntry& Entry = BossClassData->PhaseEntries[i];

		const float EntryPhaseIndex = static_cast<float>(i + 2);

		if (!Entry.PhaseTag.IsValid())
		{
			continue;
		}

		// 단방향: 이미 해당 페이즈 이상이면 스킵
		if (BossAttributeSet->GetCurrentPhase() >= EntryPhaseIndex)
		{
			continue;
		}

		if (HealthRatio <= Entry.HealthRatioThreshold)
		{
			// 이전 페이즈 태그 제거 (i==0이면 InitialPhaseTag, 아니면 직전 Entry의 태그)
			if (i == 0)
			{
				if (BossClassData->InitialPhaseTag.IsValid())
				{
					AbilitySystemComponent->RemoveLooseGameplayTag(BossClassData->InitialPhaseTag, 1);
				}
			}
			else
			{
				const FGameplayTag& PrevTag = BossClassData->PhaseEntries[i - 1].PhaseTag;
				if (PrevTag.IsValid())
				{
					AbilitySystemComponent->RemoveLooseGameplayTag(PrevTag, 1);
				}
			}

			AbilitySystemComponent->AddLooseGameplayTag(Entry.PhaseTag, 1, EGameplayTagReplicationState::TagOnly);
			BossAttributeSet->SetCurrentPhase(EntryPhaseIndex);

			UE_LOG(LogTemp, Warning, TEXT("[BossCharacterBase] Phase transition → Phase %.0f (HealthRatio=%.2f)"),
				EntryPhaseIndex, HealthRatio);

			break; // 한 사이클에 하나의 페이즈만 전환
		}
	}
}

void ABossCharacterBase::HandleDeath()
{
	Super::HandleDeath();

	if (HasAuthority() && AbilitySystemComponent)
	{
		AbilitySystemComponent->AddLooseGameplayTag(
			DGGameplayTags::State_Boss_Dead,
			1,
			EGameplayTagReplicationState::TagOnly
		);
	}
}

