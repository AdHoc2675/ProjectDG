#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "DGMinimapCaptureComponent.generated.h"

class USceneCaptureComponent2D;
class UTextureRenderTarget2D;

// 미니맵 상의 0.0 ~ 1.0 비율 위치 및 표시 여부
USTRUCT(BlueprintType)
struct FDGMinimapScreenPosition
{
	GENERATED_BODY()

	UPROPERTY()
	FVector2D ScreenPosition = FVector2D::ZeroVector;

	UPROPERTY()
	bool bIsInRange = false;
};

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class PROJECTDG_API UDGMinimapCaptureComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UDGMinimapCaptureComponent();

	UTextureRenderTarget2D* GetRenderTarget() const { return RenderTarget; }
	float GetCaptureRadius() const { return CaptureRadius; }

	// 월드좌표 -> 미니맵 0~1 비율 좌표로 변환
	FDGMinimapScreenPosition WorldToScreenPosition(const FVector& WorldLocation) const;

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	virtual void InitializeComponent() override;

	UPROPERTY(EditAnywhere, Category = "Capture")
	float CaptureHeight = 10000.f; // 카메라가 떠있는 높이

	UPROPERTY(EditAnywhere, Category = "Capture")
	float CaptureRadius = 10000.f; // 캡처 반경 (넓을수록 축소됨)

	UPROPERTY(EditAnywhere, Category = "Capture")
	int32 RenderTargetResolution = 512;

	UPROPERTY(EditAnywhere, Category = "Capture")
	float CaptureUpdateInterval = 0.05f;

	UPROPERTY(EditAnywhere, Category = "Capture")
	float RecaptureDistanceThreshold = 50.f;

private:
	void InitializeCaptureCamera();
	void CreateRenderTarget();
	bool TryUpdateCapture();

	UPROPERTY()
	TObjectPtr<USceneCaptureComponent2D> CaptureComponent;

	UPROPERTY()
	TObjectPtr<UTextureRenderTarget2D> RenderTarget;

	float TimeSinceLastCapture = 0.f;
	FVector LastCaptureLocation = FVector::ZeroVector;
};