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

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnMapMarkerClickedSignature, UDGMinimapMarkerComponent*, ClickedMarker);

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

	// 이 마커가 '나(로컬 플레이어)'의 것인지 '남(파티원 등)'의 것인지 판별하는 유틸 함수
	UFUNCTION(BlueprintPure, Category = "Minimap")
	bool IsLocalPlayerMarker() const;

	/* Visibility Config */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Map|Visibility")
	bool bShowOnMinimap = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Map|Visibility")
	bool bShowOnFullMap = true;

	/* Interaction (주로 Fullmap에서 사용) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Map|Interaction")
	bool bIsInteractable = false;

	// 상호작용(클릭) 이벤트 브로드캐스터
	UPROPERTY(BlueprintAssignable, Category = "Map|Interaction")
	FOnMapMarkerClickedSignature OnMarkerClicked;

	// UI Widget에서 클릭되었을 때 호출할 함수
	UFUNCTION(BlueprintCallable, Category = "Map|Interaction")
	void NotifyClicked();
};