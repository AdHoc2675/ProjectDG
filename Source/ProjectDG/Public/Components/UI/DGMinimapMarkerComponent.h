#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "DGMinimapMarkerComponent.generated.h"

// 미니맵에 표시될 마커의 종류 (색상이나 아이콘을 구분하기 위함)
UENUM(BlueprintType)
enum class EMinimapMarkerType : uint8
{
	Player UMETA(DisplayName = "Player"),
	Enemy UMETA(DisplayName = "Enemy"),
	Boss UMETA(DisplayName = "Boss"),
	Waypoint UMETA(DisplayName = "Waypoint"),
	Quest UMETA(DisplayName = "Quest")
};

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class PROJECTDG_API UDGMinimapMarkerComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UDGMinimapMarkerComponent();

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

public:
	// 마커 종류
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Minimap")
	EMinimapMarkerType MarkerType = EMinimapMarkerType::Enemy;

	// UI 텍스처 (기본 타입을 안 따르고 개별 아이콘을 쓰고 싶을 때 사용)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Minimap")
	TObjectPtr<UTexture2D> OverrideIcon;

	// 미니맵에서 내 회전 방향을 추적할지 여부
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Minimap")
	bool bTrackRotation = false;
};