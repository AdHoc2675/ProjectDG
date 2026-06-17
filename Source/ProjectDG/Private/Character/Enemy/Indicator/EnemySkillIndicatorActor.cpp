// Fill out your copyright notice in the Description page of Project Settings.

#include "Character/Enemy/Indicator/EnemySkillIndicatorActor.h"

#include "Components/DecalComponent.h"
#include "Components/SceneComponent.h"
#include "Materials/MaterialInstanceDynamic.h"

AEnemySkillIndicatorActor::AEnemySkillIndicatorActor()
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = false;

	SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
	SetRootComponent(SceneRoot);

	PreviewDecalComponent = CreateDefaultSubobject<UDecalComponent>(TEXT("PreviewDecalComponent"));
	PreviewDecalComponent->SetupAttachment(SceneRoot);
	PreviewDecalComponent->SetRelativeRotation(FRotator(-90.0f, 0.0f, 0.0f));
	PreviewDecalComponent->SetRelativeScale3D(FVector::OneVector);
	PreviewDecalComponent->SortOrder = 0;

	FillDecalComponent = CreateDefaultSubobject<UDecalComponent>(TEXT("FillDecalComponent"));
	FillDecalComponent->SetupAttachment(SceneRoot);
	FillDecalComponent->SetRelativeRotation(FRotator(-90.0f, 0.0f, 0.0f));
	FillDecalComponent->SetRelativeScale3D(FVector::OneVector);
	FillDecalComponent->SortOrder = 1;

	SetDecalComponentsVisible(false);
}

void AEnemySkillIndicatorActor::BeginPlay()
{
	Super::BeginPlay();
}

void AEnemySkillIndicatorActor::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (!bIsRunning)
	{
		return;
	}

	if (TelegraphTime <= 0.0f)
	{
		SetFillAmount(1.0f);

		bIsRunning = false;
		SetActorTickEnabled(false);
		SetDecalComponentsVisible(false);

		Destroy();
		return;
	}

	ElapsedTime += DeltaSeconds;

	const float Alpha = FMath::Clamp(
		ElapsedTime / TelegraphTime,
		0.0f,
		1.0f
	);

	SetFillAmount(Alpha);

	if (Alpha >= 1.0f)
	{
		bIsRunning = false;
		SetActorTickEnabled(false);

		// 인디케이터는 텔레그래프가 끝나면 시각용 Actor이므로 제거한다.
		SetDecalComponentsVisible(false);
		Destroy();
	}
}

void AEnemySkillIndicatorActor::ConfigureFromSkillData(const UEnemySkillData* InSkillData)
{
	ResetRuntimeState();

	if (!InSkillData)
	{
		SetDecalComponentsVisible(false);
		return;
	}

	if (!InSkillData->bUseIndicator)
	{
		SetDecalComponentsVisible(false);
		return;
	}

	if (!InSkillData->IndicatorMaterialOverride)
	{
		SetDecalComponentsVisible(false);
		return;
	}

	CachedIndicatorShape = InSkillData->IndicatorShape;
	CachedRadius = InSkillData->Radius;
	CachedInnerRadius = InSkillData->InnerRadius;
	CachedSectorAngleDegrees = InSkillData->SectorAngleDegrees;
	TelegraphTime = InSkillData->IndicatorTelegraphTime;

	ConfigureDecalSizeFromSkillData(InSkillData);
	ConfigureMaterialFromSkillData(InSkillData);

	SetDecalComponentsVisible(true);
	SetFillAmount(0.0f);
}

void AEnemySkillIndicatorActor::StartIndicator()
{
	ElapsedTime = 0.0f;

	if (!PreviewMID || !FillMID)
	{
		SetActorTickEnabled(false);
		return;
	}

	if (TelegraphTime <= 0.0f)
	{
		SetFillAmount(1.0f);
		bIsRunning = false;
		SetActorTickEnabled(false);
		return;
	}

	bIsRunning = true;
	SetActorTickEnabled(true);
}

void AEnemySkillIndicatorActor::StopIndicator()
{
	bIsRunning = false;
	SetActorTickEnabled(false);
	SetDecalComponentsVisible(false);

	Destroy();
}

void AEnemySkillIndicatorActor::SetFillAmount(float InFillAmount)
{
	const float ClampedFillAmount = FMath::Clamp(InFillAmount, 0.0f, 1.0f);
	const float VisualFillAmount = MakeVisualFillAmount(ClampedFillAmount);

	if (FillMID)
	{
		FillMID->SetScalarParameterValue(TEXT("FillAmount"), VisualFillAmount);
	}

	// Preview는 항상 전체 범위를 보여줘야 하므로 FillAmount 1 고정.
	if (PreviewMID)
	{
		PreviewMID->SetScalarParameterValue(TEXT("FillAmount"), 1.0f);
	}
}

void AEnemySkillIndicatorActor::ResetRuntimeState()
{
	bIsRunning = false;
	ElapsedTime = 0.0f;
	TelegraphTime = 0.0f;

	CachedIndicatorShape = EDGEnemySkillIndicatorShape::None;
	CachedRadius = 0.0f;
	CachedInnerRadius = 0.0f;
	CachedSectorAngleDegrees = 360.0f;

	PreviewMID = nullptr;
	FillMID = nullptr;

	SetActorTickEnabled(false);
}

void AEnemySkillIndicatorActor::ConfigureDecalSizeFromSkillData(const UEnemySkillData* InSkillData)
{
	if (!InSkillData || !PreviewDecalComponent || !FillDecalComponent)
	{
		return;
	}

	PreviewDecalComponent->SetRelativeScale3D(FVector::OneVector);
	FillDecalComponent->SetRelativeScale3D(FVector::OneVector);

	FVector DecalSize(
		InSkillData->IndicatorProjectionDepth,
		100.0f,
		100.0f
	);

	switch (InSkillData->IndicatorShape)
	{
	case EDGEnemySkillIndicatorShape::Circle:
	case EDGEnemySkillIndicatorShape::Sector:
	case EDGEnemySkillIndicatorShape::SectorRing:
	case EDGEnemySkillIndicatorShape::Donut:
		{
			const float Radius = FMath::Max(InSkillData->Radius, 1.0f);

			// 기존 원형 계열은 이 값으로 이미 Debug와 동기화 확인됨.
			DecalSize = FVector(
				InSkillData->IndicatorProjectionDepth,
				Radius,
				Radius
			);

			break;
		}

	case EDGEnemySkillIndicatorShape::Box:
		{
			const FVector BoxExtent = InSkillData->BoxExtent;

			// 중요:
			// 이 프로젝트의 Decal/M_Indicator 기준에서는 BoxExtent를 그대로 넣어야
			// DrawDebugBox의 BoxExtent와 크기가 맞는다.
			// BoxExtent * 2 금지.
			DecalSize = FVector(
				InSkillData->IndicatorProjectionDepth,
				BoxExtent.Y,
				BoxExtent.X
			);

			break;
		}

	case EDGEnemySkillIndicatorShape::None:
	default:
		break;
	}

	PreviewDecalComponent->DecalSize = DecalSize;
	FillDecalComponent->DecalSize = DecalSize;
}

void AEnemySkillIndicatorActor::ConfigureMaterialFromSkillData(const UEnemySkillData* InSkillData)
{
	if (!InSkillData || !InSkillData->IndicatorMaterialOverride)
	{
		return;
	}

	PreviewMID = UMaterialInstanceDynamic::Create(
		InSkillData->IndicatorMaterialOverride,
		this
	);

	FillMID = UMaterialInstanceDynamic::Create(
		InSkillData->IndicatorMaterialOverride,
		this
	);

	if (!PreviewMID || !FillMID)
	{
		return;
	}

	PreviewDecalComponent->SetDecalMaterial(PreviewMID);
	FillDecalComponent->SetDecalMaterial(FillMID);

	ApplyCommonMaterialParameters(
		PreviewMID,
		InSkillData,
		InSkillData->IndicatorPreviewOpacity,
		1.0f
	);

	ApplyCommonMaterialParameters(
		FillMID,
		InSkillData,
		InSkillData->IndicatorFillOpacity,
		0.0f
	);
}

void AEnemySkillIndicatorActor::ApplyCommonMaterialParameters(
	UMaterialInstanceDynamic* InMID,
	const UEnemySkillData* InSkillData,
	float InOpacity,
	float InFillAmount
) const
{
	if (!InMID || !InSkillData)
	{
		return;
	}

	const float InnerRadiusRatio = CalculateInnerRadiusRatio();

	InMID->SetScalarParameterValue(TEXT("Opacity"), InOpacity);
	InMID->SetScalarParameterValue(TEXT("FillAmount"), InFillAmount);
	InMID->SetScalarParameterValue(TEXT("AngleDegrees"), InSkillData->SectorAngleDegrees);
	InMID->SetScalarParameterValue(TEXT("InnerRadiusRatio"), InnerRadiusRatio);
}

float AEnemySkillIndicatorActor::MakeVisualFillAmount(float InFillAmount) const
{
	const float ClampedFillAmount = FMath::Clamp(InFillAmount, 0.0f, 1.0f);

	const bool bIsRingShape =
		CachedIndicatorShape == EDGEnemySkillIndicatorShape::Donut ||
		CachedIndicatorShape == EDGEnemySkillIndicatorShape::SectorRing;

	if (!bIsRingShape)
	{
		return ClampedFillAmount;
	}

	const float InnerRadiusRatio = CalculateInnerRadiusRatio();

	return FMath::Lerp(
		InnerRadiusRatio,
		1.0f,
		ClampedFillAmount
	);
}

float AEnemySkillIndicatorActor::CalculateInnerRadiusRatio() const
{
	if (CachedRadius <= 0.0f)
	{
		return 0.0f;
	}

	return FMath::Clamp(
		CachedInnerRadius / CachedRadius,
		0.0f,
		1.0f
	);
}

void AEnemySkillIndicatorActor::SetDecalComponentsVisible(bool bVisible)
{
	if (PreviewDecalComponent)
	{
		PreviewDecalComponent->SetVisibility(bVisible);
	}

	if (FillDecalComponent)
	{
		FillDecalComponent->SetVisibility(bVisible);
	}
}