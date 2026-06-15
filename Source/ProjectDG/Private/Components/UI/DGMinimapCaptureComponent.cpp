// Fill out your copyright notice in the Description page of Project Settings.


#include "Components/UI/DGMinimapCaptureComponent.h"
#include "Components/SceneCaptureComponent2D.h"
#include "Engine/TextureRenderTarget2D.h"
#include "GameFramework/Actor.h"

UDGMinimapCaptureComponent::UDGMinimapCaptureComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = true;
	bWantsInitializeComponent = true;
}

FDGMinimapScreenPosition UDGMinimapCaptureComponent::WorldToScreenPosition(const FVector& WorldLocation) const
{
	FDGMinimapScreenPosition Result;
	AActor* Owner = GetOwner();
	if (!Owner || CaptureRadius <= KINDA_SMALL_NUMBER) return Result;

	// 내 위치 기준의 상대 벡터 구하기
	const FVector Offset = WorldLocation - Owner->GetActorLocation();

	// 반경 대비 비율화 (-1.0 ~ 1.0)
	const float NormalizedX = Offset.X / CaptureRadius;
	const float NormalizedY = Offset.Y / CaptureRadius;

	const float DistSq = NormalizedX * NormalizedX + NormalizedY * NormalizedY;
	Result.bIsInRange = (DistSq <= 1.f);

	// 언리얼의 X(앞), Y(오른쪽) -> UI의 U(우측), V(아래) 방위 매핑
	// X(앞)은 미니맵에서 위(-Y 방향), Y(오른쪽)은 미니맵에서 우측(+X 방향)
	Result.ScreenPosition.X = (NormalizedY * 0.5f) + 0.5f;
	Result.ScreenPosition.Y = (-NormalizedX * 0.5f) + 0.5f;

	return Result;
}

void UDGMinimapCaptureComponent::BeginPlay()
{
	Super::BeginPlay();
	InitializeCaptureCamera();
	if (AActor* Owner = GetOwner())
	{
		LastCaptureLocation = Owner->GetActorLocation();
	}
}

void UDGMinimapCaptureComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (CaptureComponent)
	{
		CaptureComponent->DestroyComponent();
		CaptureComponent = nullptr;
	}
	Super::EndPlay(EndPlayReason);
}

void UDGMinimapCaptureComponent::TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	TimeSinceLastCapture += DeltaTime;
	if (TimeSinceLastCapture < CaptureUpdateInterval) return;

	TimeSinceLastCapture = 0.f;
	TryUpdateCapture();
}

void UDGMinimapCaptureComponent::InitializeComponent()
{
	Super::InitializeComponent();
	CreateRenderTarget();
}

void UDGMinimapCaptureComponent::InitializeCaptureCamera()
{
	AActor* Owner = GetOwner();
	if (!Owner) return;

	CaptureComponent = NewObject<USceneCaptureComponent2D>(Owner);
	if (!CaptureComponent) return;

	CaptureComponent->RegisterComponent();
	CaptureComponent->AttachToComponent(Owner->GetRootComponent(), FAttachmentTransformRules::KeepRelativeTransform);

	// 직교 투영으로 세팅 (왜곡 없이 위에서 평면으로 찍음)
	CaptureComponent->ProjectionType = ECameraProjectionMode::Orthographic;
	CaptureComponent->OrthoWidth = CaptureRadius * 2.f;

	CaptureComponent->TextureTarget = RenderTarget;
	CaptureComponent->SetRelativeLocation(FVector(0.f, 0.f, CaptureHeight));
	CaptureComponent->SetRelativeRotation(FRotator(-90.f, 0.f, 0.f));

	CaptureComponent->CaptureSource = SCS_BaseColor;

	CaptureComponent->bCaptureEveryFrame = false;
	CaptureComponent->bCaptureOnMovement = false;

	CaptureComponent->CaptureScene();
}

void UDGMinimapCaptureComponent::CreateRenderTarget()
{
	RenderTarget = NewObject<UTextureRenderTarget2D>(this);
	if (!RenderTarget) return;
	RenderTarget->InitAutoFormat(RenderTargetResolution, RenderTargetResolution);
	RenderTarget->UpdateResourceImmediate(true);
}

bool UDGMinimapCaptureComponent::TryUpdateCapture()
{
	if (!CaptureComponent) return false;
	AActor* Owner = GetOwner();
	if (!Owner) return false;

	const FVector CurrentLocation = Owner->GetActorLocation();
	const float DistanceMoved = FVector::Dist2D(CurrentLocation, LastCaptureLocation);

	//if (DistanceMoved < RecaptureDistanceThreshold) return false;

	CaptureComponent->SetWorldLocation(FVector(CurrentLocation.X, CurrentLocation.Y, CurrentLocation.Z + CaptureHeight));
	CaptureComponent->SetWorldRotation(FRotator(-90.f, 0.f, 0.f)); // 항시 정동남북 방향으로 고정
	CaptureComponent->CaptureScene();
	LastCaptureLocation = CurrentLocation;

	return true;
}
