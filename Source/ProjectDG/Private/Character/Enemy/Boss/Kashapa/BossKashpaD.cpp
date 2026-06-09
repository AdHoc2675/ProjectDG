// Fill out your copyright notice in the Description page of Project Settings.

#include "Character/Enemy/Boss/Kashapa/BossKashpaD.h"

#include "AbilitySystemComponent.h"
#include "Character/Enemy/Boss/Data/BossCharacterClassData.h"
#include "Character/Enemy/Data/BossSkillData.h"
#include "Character/Enemy/Data/BossSkillSetData.h"
#include "Components/SkeletalMeshComponent.h"
#include "Core/DG_GameplayTags.h"
#include "GAS/Attributes/DG_BossAttributeSet.h"
#include "GameplayAbilitySpec.h"
#include "Materials/MaterialInterface.h"
#include "Net/UnrealNetwork.h"

ABossKashapaD::ABossKashapaD()
{
	PrimaryActorTick.bCanEverTick = false;
}

void ABossKashapaD::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ABossKashapaD, CurrentPhaseIndex);
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

	ApplyPhaseDataByIndex(NextPhaseData->PhaseIndex);
}



void ABossKashapaD::OnRep_CurrentPhaseIndex()
{
	const FBossPhaseData* PhaseData = FindPhaseDataByIndex(CurrentPhaseIndex);
	if (!PhaseData)
	{
		return;
	}

	CurrentSkillSetData = PhaseData->SkillSetData;
	ApplyPhaseVisual(*PhaseData);
}

const FBossPhaseData* ABossKashapaD::FindPhaseDataByIndex(int32 PhaseIndex) const
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
	const FBossPhaseData* PhaseData = FindPhaseDataByIndex(PhaseIndex);
	if (!PhaseData)
	{
		return false;
	}

	CurrentPhaseIndex = PhaseIndex;
	CurrentSkillSetData = PhaseData->SkillSetData;

	ApplyPhaseVisual(*PhaseData);
	ApplyPhaseTags(*PhaseData);
	GrantSkillSetAbilities(PhaseData->SkillSetData);

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

	for (const TObjectPtr<UBossSkillData>& SkillDataPtr : SkillSetData->Skills)
	{
		UBossSkillData* SkillData = SkillDataPtr.Get();
		if (!SkillData || !SkillData->AbilityClass)
		{
			continue;
		}

		if (HasGrantedSkillDataAbility(SkillData))
		{
			continue;
		}

		FGameplayAbilitySpec AbilitySpec(SkillData->AbilityClass, 1, INDEX_NONE, SkillData);
		ASC->GiveAbility(AbilitySpec);
	}
}

bool ABossKashapaD::HasGrantedSkillDataAbility(UBossSkillData* SkillData)
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