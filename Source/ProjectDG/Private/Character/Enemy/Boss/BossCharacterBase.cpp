// Fill out your copyright notice in the Description page of Project Settings.

#include "Character/Enemy/Boss/BossCharacterBase.h"

#include "AbilitySystemComponent.h"
#include "Abilities/GameplayAbility.h"
#include "AI/Controller/BossAIController.h"
#include "Character/Enemy/Boss/Data/BossCharacterClassData.h"
#include "Character/Enemy/Data/BossSkillSetData.h"
#include "Character/Enemy/Data/EnemySkillData.h"
#include "Components/UI/DGMinimapMarkerComponent.h"
#include "Core/DG_GameplayTags.h"
#include "GAS/Attributes/DG_AttributeSet.h"
#include "GAS/Attributes/DG_BossAttributeSet.h"
#include "GAS/Attributes/DG_EnemyAttributeSet.h"
#include "GameplayAbilitySpec.h"
#include "Animation/AnimInstance.h"
#include "Animation/AnimMontage.h"
#include "Components/SkeletalMeshComponent.h"

namespace
{
	bool HasGrantedEnemySkillDataAbility(
		const UAbilitySystemComponent* ASC,
		const UEnemySkillData* SkillData
	)
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

	bool HasGrantedBossAbilityClass(
		const UAbilitySystemComponent* ASC,
		const TSubclassOf<UGameplayAbility> AbilityClass
	)
	{
		if (!ASC || !AbilityClass)
		{
			return false;
		}

		for (const FGameplayAbilitySpec& Spec : ASC->GetActivatableAbilities())
		{
			if (!Spec.Ability)
			{
				continue;
			}

			if (Spec.Ability->GetClass() == AbilityClass)
			{
				return true;
			}
		}

		return false;
	}
}

ABossCharacterBase::ABossCharacterBase()
{
	BossAttributeSet = CreateDefaultSubobject<UDG_BossAttributeSet>(TEXT("BossAttributeSet"));
	
	AIControllerClass = ABossAIController::StaticClass();
	AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;

	if (MinimapMarkerComponent)
	{
		MinimapMarkerComponent->MarkerType = EMinimapMarkerType::Boss;
	}
}

// Boss Class Data 초기 태그 설정
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
		if (!bBossHealthPhaseDelegateBound)
		{
			AbilitySystemComponent
				->GetGameplayAttributeValueChangeDelegate(UDG_AttributeSet::GetHealthAttribute())
				.AddUObject(this, &ABossCharacterBase::OnHealthChanged);

			bBossHealthPhaseDelegateBound = true;
		}

		if (!bGroggyDelegateBound)
		{
			AbilitySystemComponent
				->GetGameplayAttributeValueChangeDelegate(UDG_EnemyAttributeSet::GetGroggyGaugeAttribute())
				.AddUObject(this, &ABossCharacterBase::OnGroggyGaugeChanged);

			bGroggyDelegateBound = true;
		}
	}
}

void ABossCharacterBase::BeginPlay()
{
	Super::BeginPlay();

	if (!bHasInitialBossTransform)
	{
		InitialBossTransform = GetActorTransform();
		bHasInitialBossTransform = true;

		UE_LOG(
			LogTemp,
			Warning,
			TEXT("[BossCharacterBase] InitialBossTransform saved. Owner=%s Location=%s Rotation=%s"),
			*GetName(),
			*InitialBossTransform.GetLocation().ToString(),
			*InitialBossTransform.GetRotation().Rotator().ToString()
		);
	}
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
			if (!bBossHealthPhaseDelegateBound)
			{
				AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(
					UDG_AttributeSet::GetHealthAttribute()
				).AddUObject(this, &ABossCharacterBase::OnHealthChanged);

				bBossHealthPhaseDelegateBound = true;
			}

			if (!bGroggyDelegateBound)
			{
				AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(
					UDG_EnemyAttributeSet::GetGroggyGaugeAttribute()
				).AddUObject(this, &ABossCharacterBase::OnGroggyGaugeChanged);

				bGroggyDelegateBound = true;
			}
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

	if (bBossDataEffectsApplied)
	{
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
				FGameplayEffectSpecHandle Spec = AbilitySystemComponent->MakeOutgoingSpec(
					EffectClass,
					1.f,
					Context
				);

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
				FGameplayEffectSpecHandle Spec = AbilitySystemComponent->MakeOutgoingSpec(
					EffectClass,
					1.f,
					Context
				);

				if (Spec.IsValid())
				{
					AbilitySystemComponent->ApplyGameplayEffectSpecToSelf(*Spec.Data.Get());
				}
			}
		}
	}

	bBossDataEffectsApplied = true;
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
			FGameplayEffectSpecHandle Spec = AbilitySystemComponent->MakeOutgoingSpec(
				EffectClass,
				1.f,
				Context
			);

			if (Spec.IsValid())
			{
				AbilitySystemComponent->ApplyGameplayEffectSpecToSelf(*Spec.Data.Get());
			}
		}
	}

	if (BossAttributeSet)
	{
		UE_LOG(
			LogTemp,
			Log,
			TEXT("[BossCharacterBase] Boss special effects applied. Phase=%.2f Threshold=%.2f Rage=%.2f/100.00 DR=%.2f"),
			BossAttributeSet->GetCurrentPhase(),
			BossAttributeSet->GetPhaseThreshold(),
			BossAttributeSet->GetRageGauge(),
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

	const float MaxHealth = AbilitySystemComponent->GetNumericAttribute(
		UDG_AttributeSet::GetMaxHealthAttribute()
	);

	if (MaxHealth <= 0.f)
	{
		return;
	}

	const float Ratio = Data.NewValue / MaxHealth;
	UpdateHealthPhaseTags(Ratio);
}

void ABossCharacterBase::OnGroggyGaugeChanged(const FOnAttributeChangeData& Data)
{
	if (!HasAuthority())
	{
		UE_LOG(
			LogTemp,
			Warning,
			TEXT("[BossGroggy] GaugeChanged ignored. NoAuthority Owner=%s Old=%.2f New=%.2f"),
			*GetName(),
			Data.OldValue,
			Data.NewValue
		);
		return;
	}

	if (!AbilitySystemComponent)
	{
		UE_LOG(
			LogTemp,
			Warning,
			TEXT("[BossGroggy] GaugeChanged failed. ASC is null Owner=%s Old=%.2f New=%.2f"),
			*GetName(),
			Data.OldValue,
			Data.NewValue
		);
		return;
	}

	if (!EnemyAttributeSet)
	{
		UE_LOG(
			LogTemp,
			Warning,
			TEXT("[BossGroggy] GaugeChanged failed. EnemyAttributeSet is null Owner=%s Old=%.2f New=%.2f"),
			*GetName(),
			Data.OldValue,
			Data.NewValue
		);
		return;
	}

	const float MaxGroggyGauge = EnemyAttributeSet->GetMaxGroggyGauge();

	UE_LOG(
		LogTemp,
		Warning,
		TEXT("[BossGroggy] GaugeChanged Owner=%s Old=%.2f New=%.2f Max=%.2f HasGroggyTag=%d IsDead=%d"),
		*GetName(),
		Data.OldValue,
		Data.NewValue,
		MaxGroggyGauge,
		AbilitySystemComponent->HasMatchingGameplayTag(DGGameplayTags::State_Boss_Groggy) ? 1 : 0,
		IsDead() ? 1 : 0
	);

	if (AbilitySystemComponent->HasMatchingGameplayTag(DGGameplayTags::State_Boss_Groggy))
	{
		UE_LOG(
			LogTemp,
			Warning,
			TEXT("[BossGroggy] Event skipped. Already groggy Owner=%s"),
			*GetName()
		);
		return;
	}

	if (IsDead())
	{
		UE_LOG(
			LogTemp,
			Warning,
			TEXT("[BossGroggy] Event skipped. Boss is dead Owner=%s"),
			*GetName()
		);
		return;
	}

	if (MaxGroggyGauge <= 0.f)
	{
		UE_LOG(
			LogTemp,
			Warning,
			TEXT("[BossGroggy] Event skipped. Invalid MaxGroggyGauge Owner=%s Max=%.2f"),
			*GetName(),
			MaxGroggyGauge
		);
		return;
	}

	if (Data.NewValue < MaxGroggyGauge)
	{
		UE_LOG(
			LogTemp,
			Warning,
			TEXT("[BossGroggy] Event skipped. Gauge not full Owner=%s New=%.2f Max=%.2f"),
			*GetName(),
			Data.NewValue,
			MaxGroggyGauge
		);
		return;
	}

	FGameplayEventData Payload;
	Payload.EventTag = DGGameplayTags::Event_Boss_Groggy;
	Payload.Instigator = this;
	Payload.Target = this;

	const int32 TriggeredAbilities = AbilitySystemComponent->HandleGameplayEvent(
		DGGameplayTags::Event_Boss_Groggy,
		&Payload
	);

	UE_LOG(
		LogTemp,
		Warning,
		TEXT("[BossGroggy] Event Sent Owner=%s Tag=%s TriggeredAbilities=%d New=%.2f Max=%.2f"),
		*GetName(),
		*DGGameplayTags::Event_Boss_Groggy.GetTag().ToString(),
		TriggeredAbilities,
		Data.NewValue,
		MaxGroggyGauge
	);
}

void ABossCharacterBase::UpdateHealthPhaseTags(float HealthRatio)
{
	if (!HasAuthority() || !BossClassData || !AbilitySystemComponent || !BossAttributeSet)
	{
		return;
	}

	const int32 CurrentPhase = FMath::RoundToInt(BossAttributeSet->GetCurrentPhase());

	const FBossPhaseData* NextPhaseData = nullptr;

	for (const FBossPhaseData& PhaseData : BossClassData->PhaseDataList)
	{
		if (PhaseData.PhaseIndex <= CurrentPhase)
		{
			continue;
		}

		if (!PhaseData.PhaseTag.IsValid())
		{
			continue;
		}

		if (HealthRatio > PhaseData.HealthRatioThreshold)
		{
			continue;
		}

		if (!NextPhaseData || PhaseData.PhaseIndex > NextPhaseData->PhaseIndex)
		{
			NextPhaseData = &PhaseData;
		}
	}

	if (!NextPhaseData)
	{
		return;
	}

	// 중요:
	// GA 실행 중에는 PhaseTag / CurrentPhase / SkillSet을 즉시 바꾸지 않는다.
	// 현재 GA가 정상 종료된 뒤 다음 스킬 선택 시점에 TryApplyPendingPhaseChange()에서 적용된다.
	RequestPhaseChange(*NextPhaseData);
}

void ABossCharacterBase::GrantDefaultAbilities()
{
	Super::GrantDefaultAbilities();

	if (!HasAuthority() || !AbilitySystemComponent || !BossClassData)
	{
		return;
	}
	
	if (BossClassData->GroggyAbilityClass)
	{
		if (!HasGrantedBossAbilityClass(AbilitySystemComponent, BossClassData->GroggyAbilityClass))
		{
			FGameplayAbilitySpec GroggyAbilitySpec(
				BossClassData->GroggyAbilityClass,
				1,
				INDEX_NONE,
				BossClassData
			);

			AbilitySystemComponent->GiveAbility(GroggyAbilitySpec);
		}
	}

	for (const FBossPhaseData& PhaseData : BossClassData->PhaseDataList)
	{
		UBossSkillSetData* SkillSetData = PhaseData.SkillSetData;
		if (!SkillSetData)
		{
			continue;
		}

		for (const TObjectPtr<UEnemySkillData>& SkillDataPtr : SkillSetData->Skills)
		{
			UEnemySkillData* SkillData = SkillDataPtr.Get();
			if (!SkillData || !SkillData->AbilityClass)
			{
				continue;
			}

			if (HasGrantedEnemySkillDataAbility(AbilitySystemComponent, SkillData))
			{
				continue;
			}

			FGameplayAbilitySpec AbilitySpec(
				SkillData->AbilityClass,
				1,
				INDEX_NONE,
				SkillData
			);

			AbilitySystemComponent->GiveAbility(AbilitySpec);
		}
	}
}

const TArray<TObjectPtr<UEnemySkillData>>& ABossCharacterBase::GetAttackSkillDataList() const
{
	static const TArray<TObjectPtr<UEnemySkillData>> EmptySkills;

	if (!BossClassData)
	{
		return EmptySkills;
	}

	if (bHasPendingPhaseChange)
	{
		UE_LOG(
			LogTemp,
			Warning,
			TEXT("[BossCharacterBase] GetAttackSkillDataList called while pending phase exists. CurrentPhase=%d PendingPhase=%d. Phase is NOT applied here."),
			CurrentPhaseIndex,
			PendingPhaseIndex
		);
	}

	if (CurrentPhaseSkillSetData)
	{
		UE_LOG(
			LogTemp,
			Warning,
			TEXT("[BossCharacterBase] GetAttackSkillDataList CurrentPhaseIndex=%d SkillSet=%s SkillCount=%d"),
			CurrentPhaseIndex,
			*CurrentPhaseSkillSetData->GetName(),
			CurrentPhaseSkillSetData->Skills.Num()
		);

		return CurrentPhaseSkillSetData->Skills;
	} 

	// 초기 상태 보정:
	// CurrentPhaseSkillSetData가 아직 세팅되지 않았다면 AttributeSet의 CurrentPhase 기준으로 1회 캐싱.
	const int32 AttributePhase =
		BossAttributeSet
			? FMath::RoundToInt(BossAttributeSet->GetCurrentPhase())
			: 1;

	const FBossPhaseData* PhaseData = FindPhaseDataByIndex(AttributePhase);
	if (!PhaseData || !PhaseData->SkillSetData)
	{
		UE_LOG(
			LogTemp,
			Warning,
			TEXT("[BossCharacterBase] GetAttackSkillDataList Failed. Phase=%d SkillSet=None"),
			AttributePhase
		);

		return EmptySkills;
	}

	ABossCharacterBase* MutableThis = const_cast<ABossCharacterBase*>(this);
	MutableThis->CurrentPhaseIndex = PhaseData->PhaseIndex;
	MutableThis->CurrentPhaseSkillSetData = PhaseData->SkillSetData;

	UE_LOG(
		LogTemp,
		Warning,
		TEXT("[BossCharacterBase] GetAttackSkillDataList InitPhase=%d SkillSet=%s SkillCount=%d"),
		MutableThis->CurrentPhaseIndex,
		*MutableThis->CurrentPhaseSkillSetData->GetName(),
		MutableThis->CurrentPhaseSkillSetData->Skills.Num()
	);

	return MutableThis->CurrentPhaseSkillSetData->Skills;
}

UEnemySkillData* ABossCharacterBase::GetRandomAttackSkillData() const
{
	const TArray<TObjectPtr<UEnemySkillData>>& Skills = GetAttackSkillDataList();
	if (Skills.Num() == 0)
	{
		return nullptr;
	}

	const int32 RandomIndex = FMath::RandRange(0, Skills.Num() - 1);
	return Skills[RandomIndex].Get();
}

FGameplayTag ABossCharacterBase::GetAttributeSourceTag() const
{
	if (BossClassData && BossClassData->BossTag.IsValid())
	{
		return BossClassData->BossTag;
	}

	return DGGameplayTags::Team_Enemy_Boss;
}

void ABossCharacterBase::Multicast_PlayBossMontage_Implementation(
	UAnimMontage* Montage,
	float PlayRate,
	FName StartSectionName
)
{
	// 서버는 GA에서 이미 Montage_Play를 직접 실행한다.
	// 여기서는 클라이언트 보스 Mesh에만 재생시킨다.
	if (HasAuthority())
	{
		return;
	}

	if (!Montage)
	{
		UE_LOG(
			LogTemp,
			Warning,
			TEXT("[BossCharacterBase] Multicast_PlayBossMontage failed. Montage null. Boss=%s"),
			*GetNameSafe(this)
		);
		return;
	}

	USkeletalMeshComponent* MeshComp = GetMesh();
	if (!MeshComp)
	{
		return;
	}

	UAnimInstance* AnimInstance = MeshComp->GetAnimInstance();
	if (!AnimInstance)
	{
		UE_LOG(
			LogTemp,
			Warning,
			TEXT("[BossCharacterBase] Multicast_PlayBossMontage failed. AnimInstance null. Boss=%s Montage=%s"),
			*GetNameSafe(this),
			*GetNameSafe(Montage)
		);
		return;
	}

	const float SafePlayRate = FMath::Max(PlayRate, 0.01f);

	const float PlayedDuration = AnimInstance->Montage_Play(
		Montage,
		SafePlayRate
	);

	if (PlayedDuration <= 0.0f)
	{
		UE_LOG(
			LogTemp,
			Warning,
			TEXT("[BossCharacterBase] Multicast_PlayBossMontage Montage_Play failed. Boss=%s Montage=%s AnimInstance=%s"),
			*GetNameSafe(this),
			*GetNameSafe(Montage),
			*GetNameSafe(AnimInstance)
		);
		return;
	}

	if (StartSectionName != NAME_None)
	{
		AnimInstance->Montage_JumpToSection(
			StartSectionName,
			Montage
		);
	}

	UE_LOG(
		LogTemp,
		Warning,
		TEXT("[BossCharacterBase] Multicast_PlayBossMontage played on client. Boss=%s Montage=%s Section=%s PlayRate=%.2f AnimInstance=%s"),
		*GetNameSafe(this),
		*GetNameSafe(Montage),
		StartSectionName != NAME_None ? *StartSectionName.ToString() : TEXT("None"),
		SafePlayRate,
		*GetNameSafe(AnimInstance)
	);
}

bool ABossCharacterBase::HasActiveEnemySkillAbility() const
{
	const UAbilitySystemComponent* ASC = GetAbilitySystemComponent();
	if (!ASC)
	{
		return false;
	}

	for (const FGameplayAbilitySpec& Spec : ASC->GetActivatableAbilities())
	{
		if (!Spec.IsActive())
		{
			continue;
		}

		if (Cast<UEnemySkillData>(Spec.SourceObject.Get()))
		{
			return true;
		}
	}

	return false;
}

const FBossPhaseData* ABossCharacterBase::FindPhaseDataByIndex(int32 PhaseIndex) const
{
	if (!BossClassData)
	{
		return nullptr;
	}

	for (const FBossPhaseData& PhaseData : BossClassData->PhaseDataList)
	{
		if (PhaseData.PhaseIndex == PhaseIndex)
		{
			return &PhaseData;
		}
	}

	return nullptr;
}

void ABossCharacterBase::RequestPhaseChange(const FBossPhaseData& PhaseData)
{
	const int32 AttributeCurrentPhase =
		BossAttributeSet
			? FMath::RoundToInt(BossAttributeSet->GetCurrentPhase())
			: CurrentPhaseIndex;

	if (AttributeCurrentPhase >= PhaseData.PhaseIndex)
	{
		return;
	}

	if (bHasPendingPhaseChange && PendingPhaseIndex == PhaseData.PhaseIndex)
	{
		return;
	}

	bHasPendingPhaseChange = true;
	PendingPhaseIndex = PhaseData.PhaseIndex;

	UE_LOG(
		LogTemp,
		Warning,
		TEXT("[BossCharacterBase] Phase change pending only. CurrentPhase=%d PendingPhase=%d"),
		AttributeCurrentPhase,
		PendingPhaseIndex
	);

	ForceNetUpdate();

	// 중요:
	// 컷신 구조에서는 여기서 ApplyPhaseChange를 호출하지 않는다.
	// 실제 적용은 GA_Boss_Kashapa_PhaseTransition이
	// Event.Boss.PhaseApply를 받은 시점에만 수행한다.
}

void ABossCharacterBase::ApplyPhaseChange(const FBossPhaseData& PhaseData)
{
	if (!HasAuthority() || !AbilitySystemComponent || !BossAttributeSet)
	{
		return;
	}

	AbilitySystemComponent->RemoveLooseGameplayTag(DGGameplayTags::State_Boss_Phase_1, 1);
	AbilitySystemComponent->RemoveLooseGameplayTag(DGGameplayTags::State_Boss_Phase_2, 1);
	AbilitySystemComponent->RemoveLooseGameplayTag(DGGameplayTags::State_Boss_Phase_3, 1);

	if (PhaseData.PhaseTag.IsValid())
	{
		AbilitySystemComponent->AddLooseGameplayTag(
			PhaseData.PhaseTag,
			1,
			EGameplayTagReplicationState::TagOnly
		);
	}

	BossAttributeSet->SetCurrentPhase(static_cast<float>(PhaseData.PhaseIndex));

	CurrentPhaseIndex = PhaseData.PhaseIndex;
	CurrentPhaseSkillSetData = PhaseData.SkillSetData;

	bHasPendingPhaseChange = false;
	PendingPhaseIndex = INDEX_NONE;

	UE_LOG(
		LogTemp,
		Warning,
		TEXT("[BossCharacterBase] Phase changed. PhaseIndex=%d PhaseTag=%s SkillSet=%s SkillCount=%d"),
		CurrentPhaseIndex,
		PhaseData.PhaseTag.IsValid() ? *PhaseData.PhaseTag.ToString() : TEXT("None"),
		CurrentPhaseSkillSetData ? *CurrentPhaseSkillSetData->GetName() : TEXT("None"),
		CurrentPhaseSkillSetData ? CurrentPhaseSkillSetData->Skills.Num() : 0
	);

	// Mesh / AnimClass / Material 변경 로직이 기존에 별도 함수로 있다면
	// 여기서 호출해야 GA 종료 이후 안전하게 외형 전환된다.
}

bool ABossCharacterBase::ApplyPendingPhaseChangeFromNotify(int32 ExpectedPhaseIndex)
{
	if (!HasAuthority())
	{
		return false;
	}

	if (!bHasPendingPhaseChange || PendingPhaseIndex == INDEX_NONE)
	{
		UE_LOG(
			LogTemp,
			Warning,
			TEXT("[BossCharacterBase] ApplyPendingPhaseChangeFromNotify Failed. No pending phase. CurrentPhase=%d"),
			CurrentPhaseIndex
		);

		return false;
	}

	const int32 PhaseIndexToApply = PendingPhaseIndex;

	if (ExpectedPhaseIndex != INDEX_NONE && PhaseIndexToApply != ExpectedPhaseIndex)
	{
		UE_LOG(
			LogTemp,
			Warning,
			TEXT("[BossCharacterBase] ApplyPendingPhaseChangeFromNotify Failed. Pending=%d Expected=%d"),
			PhaseIndexToApply,
			ExpectedPhaseIndex
		);

		return false;
	}

	const FBossPhaseData* PendingPhaseData = FindPhaseDataByIndex(PhaseIndexToApply);
	if (!PendingPhaseData)
	{
		UE_LOG(
			LogTemp,
			Warning,
			TEXT("[BossCharacterBase] ApplyPendingPhaseChangeFromNotify Failed. PhaseData not found. Pending=%d"),
			PhaseIndexToApply
		);

		bHasPendingPhaseChange = false;
		PendingPhaseIndex = INDEX_NONE;
		return false;
	}

	UE_LOG(
		LogTemp,
		Warning,
		TEXT("[BossCharacterBase] ApplyPendingPhaseChangeFromNotify Start. CurrentPhase=%d PendingPhase=%d"),
		CurrentPhaseIndex,
		PhaseIndexToApply
	);

	ApplyPhaseChange(*PendingPhaseData);

	UE_LOG(
		LogTemp,
		Warning,
		TEXT("[BossCharacterBase] ApplyPendingPhaseChangeFromNotify Finished. CurrentPhase=%d"),
		CurrentPhaseIndex
	);

	return true;
}

bool ABossCharacterBase::MoveToInitialBossTransformForCutscene()
{
	if (!HasAuthority())
	{
		return false;
	}

	if (!bHasInitialBossTransform)
	{
		InitialBossTransform = GetActorTransform();
		bHasInitialBossTransform = true;

		UE_LOG(
			LogTemp,
			Warning,
			TEXT("[BossCharacterBase] InitialBossTransform was missing. Saved current transform instead. Owner=%s"),
			*GetName()
		);

		return true;
	}

	SetActorLocationAndRotation(
		InitialBossTransform.GetLocation(),
		InitialBossTransform.GetRotation().Rotator(),
		false,
		nullptr,
		ETeleportType::TeleportPhysics
	);

	ForceNetUpdate();

	UE_LOG(
		LogTemp,
		Warning,
		TEXT("[BossCharacterBase] Moved to InitialBossTransform for cutscene. Owner=%s Location=%s Rotation=%s"),
		*GetName(),
		*InitialBossTransform.GetLocation().ToString(),
		*InitialBossTransform.GetRotation().Rotator().ToString()
	);

	return true;
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