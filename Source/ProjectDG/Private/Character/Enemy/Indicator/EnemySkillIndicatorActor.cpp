// Fill out your copyright notice in the Description page of Project Settings.

#include "Character/Enemy/Indicator/EnemySkillIndicatorActor.h"

#include "Character/Enemy/Data/EnemySkillData.h"
#include "Components/DecalComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Materials/MaterialInterface.h"

AEnemySkillIndicatorActor::AEnemySkillIndicatorActor()
{
	PrimaryActorTick.bCanEverTick = true;

	bReplicates = false;

	DecalComponent = CreateDefaultSubobject<UDecalComponent>(TEXT("DecalComponent"));
	SetRootComponent(DecalComponent);

	if (DecalComponent)
	{
		// 데칼은 바닥을 향해 투영하기 위해 -90도 회전해서 사용한다.
		DecalComponent->SetRelativeRotation(FRotator(-90.f, 0.f, 0.f));
		DecalComponent->DecalSize = FVector(512.f, 512.f, 512.f);
	}
}

void AEnemySkillIndicatorActor::BeginPlay()
{
	Super::BeginPlay();
}

void AEnemySkillIndicatorActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (!bIsPlayingIndicator)
	{
		return;
	}

	IndicatorElapsedTime += DeltaTime;

	const float Alpha = IndicatorDuration > 0.f
		? FMath::Clamp(IndicatorElapsedTime / IndicatorDuration, 0.f, 1.f)
		: 1.f;

	SetFillAmount(Alpha);

	if (Alpha >= 1.f)
	{
		bIsPlayingIndicator = false;

		if (bAutoDestroyOnFillComplete)
		{
			Destroy();
		}
	}
}

void AEnemySkillIndicatorActor::ConfigureFromSkillData(const UEnemySkillData* SkillData)
{
	if (!SkillData || !DecalComponent)
	{
		return;
	}

	// SpawnTransform 쪽에서 이미 IndicatorZOffset을 적용하고 있으므로,
	// 여기서는 추가 위치 보정을 하지 않는다.
	// 중복 적용하면 인디케이터가 바닥에서 떠 보일 수 있다.

	IndicatorDuration = FMath::Max(SkillData->IndicatorTelegraphTime, 0.01f);

	ApplyMaterialFromSkillData(SkillData);
	ApplyDecalSizeFromSkillData(SkillData);
	ApplyMaterialParametersFromSkillData(SkillData);

	SetFillAmount(0.f);
}

void AEnemySkillIndicatorActor::StartIndicator(float OverrideDuration)
{
	IndicatorDuration = OverrideDuration > 0.f
		? OverrideDuration
		: FMath::Max(IndicatorDuration, 0.01f);

	IndicatorElapsedTime = 0.f;
	bIsPlayingIndicator = true;

	SetFillAmount(0.f);
}

void AEnemySkillIndicatorActor::StopIndicator(bool bDestroyActor)
{
	bIsPlayingIndicator = false;

	if (bDestroyActor)
	{
		Destroy();
	}
}

void AEnemySkillIndicatorActor::SetFillAmount(float NewFillAmount)
{
	CurrentFillAmount = FMath::Clamp(NewFillAmount, 0.f, 1.f);

	if (DynamicIndicatorMaterial)
	{
		DynamicIndicatorMaterial->SetScalarParameterValue(
			FillAmountParameterName,
			CurrentFillAmount
		);
	}
}

void AEnemySkillIndicatorActor::ApplyDecalSizeFromSkillData(const UEnemySkillData* SkillData)
{
	if (!SkillData || !DecalComponent)
	{
		return;
	}

	const float ProjectionDepth = FMath::Max(SkillData->IndicatorProjectionDepth, 1.f);

	switch (SkillData->IndicatorShape)
	{
	case EDGEnemySkillIndicatorShape::Circle:
	case EDGEnemySkillIndicatorShape::Sector:
	case EDGEnemySkillIndicatorShape::SectorRing:
	case EDGEnemySkillIndicatorShape::Donut:
		{
			const float RadiusSize = FMath::Max(SkillData->Radius, 1.f);

			DecalComponent->DecalSize = FVector(
				ProjectionDepth,
				RadiusSize,
				RadiusSize
			);

			break;
		}

	case EDGEnemySkillIndicatorShape::Box:
	{
		const float Length = FMath::Max(SkillData->BoxExtent.X * 2.f, 1.f);
		const float Width = FMath::Max(SkillData->BoxExtent.Y * 2.f, 1.f);

		DecalComponent->DecalSize = FVector(
			ProjectionDepth,
			Width,
			Length
		);

		break;
	}

	case EDGEnemySkillIndicatorShape::None:
	default:
		break;
	}
}

void AEnemySkillIndicatorActor::ApplyMaterialFromSkillData(const UEnemySkillData* SkillData)
{
	if (!SkillData || !DecalComponent)
	{
		return;
	}

	UMaterialInterface* SourceMaterial = SkillData->IndicatorMaterialOverride.Get();

	if (!SourceMaterial)
	{
		SourceMaterial = DecalComponent->GetDecalMaterial();
	}

	if (!SourceMaterial)
	{
		return;
	}

	DynamicIndicatorMaterial = UMaterialInstanceDynamic::Create(SourceMaterial, this);
	if (DynamicIndicatorMaterial)
	{
		DecalComponent->SetDecalMaterial(DynamicIndicatorMaterial);
	}
}

void AEnemySkillIndicatorActor::ApplyMaterialParametersFromSkillData(const UEnemySkillData* SkillData)
{
	if (!SkillData || !DynamicIndicatorMaterial)
	{
		return;
	}

	DynamicIndicatorMaterial->SetScalarParameterValue(
		OpacityParameterName,
		FMath::Clamp(SkillData->IndicatorOpacity, 0.f, 1.f)
	);

	DynamicIndicatorMaterial->SetScalarParameterValue(
		AngleDegreesParameterName,
		FMath::Clamp(SkillData->SectorAngleDegrees, 1.f, 360.f)
	);

	const float InnerRadiusRatio = SkillData->Radius > 0.f
		? FMath::Clamp(SkillData->InnerRadius / SkillData->Radius, 0.f, 1.f)
		: 0.f;

	DynamicIndicatorMaterial->SetScalarParameterValue(
		InnerRadiusRatioParameterName,
		InnerRadiusRatio
	);
}