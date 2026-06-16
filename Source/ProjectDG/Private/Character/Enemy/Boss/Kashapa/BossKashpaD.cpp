// Fill out your copyright notice in the Description page of Project Settings.

#include "Character/Enemy/Boss/Kashapa/BossKashpaD.h"

#include "AbilitySystemComponent.h"
#include "Character/Enemy/Boss/Data/BossCharacterClassData.h"
#include "Character/Enemy/Data/BossSkillSetData.h"
#include "Character/Enemy/Data/EnemySkillData.h"
#include "Components/SkeletalMeshComponent.h"
#include "Core/DG_GameplayTags.h"
#include "GAS/Attributes/DG_BossAttributeSet.h"
#include "GameplayAbilitySpec.h"
#include "Materials/MaterialInterface.h"

namespace
{
	bool HasActiveEnemySkillDataAbility(const UAbilitySystemComponent* ASC)
	{
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
}

ABossKashapaD::ABossKashapaD()
{
	PrimaryActorTick.bCanEverTick = false;
}

void ABossKashapaD::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	// CurrentPhaseIndex는 ABossCharacterBase로 이동했으므로
	// 여기서 DOREPLIFETIME 하면 자식 클래스 중복 멤버 기준으로 빌드 에러가 난다.
	// 외형 동기화 Replication은 추후 별도 정리.
}

void ABossKashapaD::BeginPlay()
{
	Super::BeginPlay();

	if (!HasAuthority() && CurrentPhaseIndex > 0)
	{
		OnRep_CurrentPhaseIndex();
	}
}

void ABossKashapaD::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);

	if (HasAuthority() && CurrentPhaseIndex <= 0)
	{
		ApplyPhaseDataByIndex(InitialPhaseIndex);
	}
}

bool ABossKashapaD::SetKashapaPhase(int32 NewPhaseIndex)
{
	if (!HasAuthority())
	{
		return false;
	}

	return ApplyPhaseDataByIndex(NewPhaseIndex);
}

bool ABossKashapaD::ApplyPendingPhaseChangeFromNotify(int32 ExpectedPhaseIndex)
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
			TEXT("[BossKashapaD] ApplyPendingPhaseChangeFromNotify Failed. No pending phase. CurrentPhase=%d"),
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
			TEXT("[BossKashapaD] ApplyPendingPhaseChangeFromNotify Failed. Pending=%d Expected=%d"),
			PhaseIndexToApply,
			ExpectedPhaseIndex
		);

		return false;
	}

	UE_LOG(
		LogTemp,
		Warning,
		TEXT("[BossKashapaD] ApplyPendingPhaseChangeFromNotify Start. CurrentPhase=%d PendingPhase=%d"),
		CurrentPhaseIndex,
		PhaseIndexToApply
	);

	const bool bApplied = ApplyPhaseDataByIndex(PhaseIndexToApply);

	UE_LOG(
		LogTemp,
		Warning,
		TEXT("[BossKashapaD] ApplyPendingPhaseChangeFromNotify Finished. Applied=%s CurrentPhase=%d"),
		bApplied ? TEXT("true") : TEXT("false"),
		CurrentPhaseIndex
	);

	return bApplied;
}

void ABossKashapaD::UpdateHealthPhaseTags(float HealthRatio)
{
	if (!HasAuthority() || !BossClassData || IsDead())
	{
		return;
	}

	const int32 CurrentPhase = CurrentPhaseIndex > 0
		? CurrentPhaseIndex
		: FMath::RoundToInt(GetBossAttributeSet() ? GetBossAttributeSet()->GetCurrentPhase() : 1.f);

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

	if (bHasPendingPhaseChange && PendingPhaseIndex == NextPhaseData->PhaseIndex)
	{
		return;
	}

	bHasPendingPhaseChange = true;
	PendingPhaseIndex = NextPhaseData->PhaseIndex;

	UE_LOG(
		LogTemp,
		Warning,
		TEXT("[BossKashapaD] Phase change pending only. CurrentPhase=%d PendingPhase=%d HealthRatio=%.3f"),
		CurrentPhase,
		PendingPhaseIndex,
		HealthRatio
	);

	ForceNetUpdate();

	// 중요:
	// 여기서 ApplyPhaseDataByIndex 호출 금지.
	// AN_BossPhaseApply 전까지 1페 SkeletalMesh / Material / AnimClass 유지.
}

void ABossKashapaD::OnRep_CurrentPhaseIndex()
{
	const FBossPhaseData* PhaseData = FindKashapaPhaseDataByIndex(CurrentPhaseIndex);
	if (!PhaseData)
	{
		return;
	}

	CurrentPhaseSkillSetData = PhaseData->SkillSetData;
	ApplyPhaseVisual(*PhaseData);
}

const FBossPhaseData* ABossKashapaD::FindKashapaPhaseDataByIndex(int32 PhaseIndex) const
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

bool ABossKashapaD::ApplyPhaseDataByIndex(int32 PhaseIndex)
{
	const FBossPhaseData* PhaseData = FindKashapaPhaseDataByIndex(PhaseIndex);
	if (!PhaseData)
	{
		UE_LOG(
			LogTemp,
			Warning,
			TEXT("[BossKashapaD] ApplyPhaseDataByIndex Failed. PhaseIndex=%d"),
			PhaseIndex
		);

		return false;
	}

	CurrentPhaseIndex = PhaseIndex;
	CurrentPhaseSkillSetData = PhaseData->SkillSetData;

	bHasPendingPhaseChange = false;
	PendingPhaseIndex = INDEX_NONE;

	ApplyPhaseVisual(*PhaseData);
	ApplyPhaseTags(*PhaseData);
	GrantSkillSetAbilities(CurrentPhaseSkillSetData);

	UE_LOG(
		LogTemp,
		Warning,
		TEXT("[BossKashapaD] Phase Applied. PhaseIndex=%d SkillSet=%s SkillCount=%d"),
		CurrentPhaseIndex,
		CurrentPhaseSkillSetData ? *CurrentPhaseSkillSetData->GetName() : TEXT("None"),
		CurrentPhaseSkillSetData ? CurrentPhaseSkillSetData->Skills.Num() : 0
	);

	ForceNetUpdate();

	return true;
}

void ABossKashapaD::ApplyPhaseVisual(const FBossPhaseData& PhaseData)
{
	USkeletalMeshComponent* MeshComp = GetMesh();
	if (!MeshComp)
	{
		return;
	}

	if (PhaseData.SkeletalMesh)
	{
		MeshComp->SetSkeletalMesh(PhaseData.SkeletalMesh);
	}

	if (PhaseData.AnimClass)
	{
		MeshComp->SetAnimInstanceClass(PhaseData.AnimClass);
	}

	MeshComp->EmptyOverrideMaterials();

	for (int32 MaterialIndex = 0; MaterialIndex < PhaseData.OverrideMaterials.Num(); ++MaterialIndex)
	{
		UMaterialInterface* Material = PhaseData.OverrideMaterials[MaterialIndex].Get();
		if (!Material)
		{
			continue;
		}

		MeshComp->SetMaterial(MaterialIndex, Material);
	}
}

void ABossKashapaD::ApplyPhaseTags(const FBossPhaseData& PhaseData)
{
	UAbilitySystemComponent* ASC = GetCharacterAbilitySystemComponent();
	if (!ASC)
	{
		return;
	}

	ASC->RemoveLooseGameplayTag(DGGameplayTags::State_Boss_Phase_1, 1);
	ASC->RemoveLooseGameplayTag(DGGameplayTags::State_Boss_Phase_2, 1);
	ASC->RemoveLooseGameplayTag(DGGameplayTags::State_Boss_Phase_3, 1);

	if (PhaseData.PhaseTag.IsValid())
	{
		ASC->AddLooseGameplayTag(
			PhaseData.PhaseTag,
			1,
			EGameplayTagReplicationState::TagOnly
		);
	}

	if (UDG_BossAttributeSet* BossAttrSet = GetBossAttributeSet())
	{
		BossAttrSet->SetCurrentPhase(static_cast<float>(PhaseData.PhaseIndex));
	}
}

void ABossKashapaD::GrantSkillSetAbilities(UBossSkillSetData* SkillSetData)
{
	if (!HasAuthority() || !SkillSetData)
	{
		return;
	}

	UAbilitySystemComponent* ASC = GetCharacterAbilitySystemComponent();
	if (!ASC)
	{
		return;
	}

	for (const TObjectPtr<UEnemySkillData>& SkillDataPtr : SkillSetData->Skills)
	{
		UEnemySkillData* SkillData = SkillDataPtr.Get();
		if (!SkillData || !SkillData->AbilityClass)
		{
			continue;
		}

		if (HasGrantedSkillDataAbility(SkillData))
		{
			continue;
		}

		FGameplayAbilitySpec AbilitySpec(
			SkillData->AbilityClass,
			1,
			INDEX_NONE,
			SkillData
		);

		ASC->GiveAbility(AbilitySpec);
	}
}

bool ABossKashapaD::HasGrantedSkillDataAbility(UEnemySkillData* SkillData)
{
	if (!SkillData || !SkillData->AbilityClass)
	{
		return false;
	}

	UAbilitySystemComponent* ASC = GetCharacterAbilitySystemComponent();
	if (!ASC)
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

const TArray<TObjectPtr<UEnemySkillData>>& ABossKashapaD::GetAttackSkillDataList() const
{
	static const TArray<TObjectPtr<UEnemySkillData>> EmptySkills;

	if (bHasPendingPhaseChange)
	{
		UE_LOG(
			LogTemp,
			Warning,
			TEXT("[BossKashapaD] GetAttackSkillDataList called while pending phase exists. CurrentPhase=%d PendingPhase=%d. Phase is NOT applied here."),
			CurrentPhaseIndex,
			PendingPhaseIndex
		);
	}
	

	if (!CurrentPhaseSkillSetData)
	{
		UE_LOG(
			LogTemp,
			Warning,
			TEXT("[BossKashapaD] GetAttackSkillDataList Failed. CurrentPhaseSkillSetData is null. CurrentPhase=%d"),
			CurrentPhaseIndex
		);

		return EmptySkills;
	}

	UE_LOG(
		LogTemp,
		Warning,
		TEXT("[BossKashapaD] GetAttackSkillDataList Phase=%d SkillSet=%s SkillCount=%d"),
		CurrentPhaseIndex,
		*CurrentPhaseSkillSetData->GetName(),
		CurrentPhaseSkillSetData->Skills.Num()
	);

	return CurrentPhaseSkillSetData->Skills;
}

UEnemySkillData* ABossKashapaD::GetRandomAttackSkillData() const
{
	const TArray<TObjectPtr<UEnemySkillData>>& Skills = GetAttackSkillDataList();
	if (Skills.Num() == 0)
	{
		return nullptr;
	}

	const int32 RandomIndex = FMath::RandRange(0, Skills.Num() - 1);
	return Skills[RandomIndex].Get();
}