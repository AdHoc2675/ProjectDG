#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "DGWaypoint.generated.h"

class UStaticMeshComponent;
class UDGMinimapMarkerComponent;
class ACharacter;

UCLASS()
class PROJECTDG_API ADGWaypoint : public AActor
{
	GENERATED_BODY()

public:
	ADGWaypoint();

protected:
	virtual void BeginPlay() override;

	// 미니맵/풀맵 마커 데이터 컴포넌트
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UDGMinimapMarkerComponent> MinimapMarkerComp;

	// 순간이동 후 위치를 미세조정하기 위한 오프셋 (웨이포인트 정중앙에 끼지 않도록)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Waypoint")
	FVector TeleportOffset = FVector(500.f, 0.f, 100.f);

private:
	// 마커 컴포넌트에서 클릭 방송이 올 때 실행될 콜백 함수
	UFUNCTION()
	void HandleMarkerClicked(UDGMinimapMarkerComponent* ClickedMarker);

protected:
	// 실제 텔레포트 요청을 처리하는 함수 (블루프린트에서 부가적인 이펙트 등 추가 구현 가능)
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Waypoint")
	void ExecuteTeleport(ACharacter* LocalPlayerCharacter);
};