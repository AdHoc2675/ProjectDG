#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "DGMinimapSubsystem.generated.h"

class UDGMinimapMarkerComponent;

// 마커가 추가/삭제될 때 Controller에게 알리기 위한 델리게이트
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnMinimapMarkerChangedSignature, UDGMinimapMarkerComponent*, Marker);

UCLASS()
class PROJECTDG_API UDGMinimapSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	void RegisterMarker(UDGMinimapMarkerComponent* Marker);
	void UnregisterMarker(UDGMinimapMarkerComponent* Marker);

	const TArray<UDGMinimapMarkerComponent*>& GetActiveMarkers() const { return ActiveMarkers; }

	// 데이터가 변경될 때 방송될 이벤트
	UPROPERTY(BlueprintAssignable, Category = "Minimap")
	FOnMinimapMarkerChangedSignature OnMarkerRegistered;

	UPROPERTY(BlueprintAssignable, Category = "Minimap")
	FOnMinimapMarkerChangedSignature OnMarkerUnregistered;

private:
	UPROPERTY()
	TArray<UDGMinimapMarkerComponent*> ActiveMarkers;
};