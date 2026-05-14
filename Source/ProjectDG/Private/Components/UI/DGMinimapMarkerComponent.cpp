#include "Components/UI/DGMinimapMarkerComponent.h"
#include "UI/Widget/Minimap/DGMinimapSubsystem.h"

UDGMinimapMarkerComponent::UDGMinimapMarkerComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UDGMinimapMarkerComponent::BeginPlay()
{
	Super::BeginPlay();

	// 나 자신을 MinimapSubsystem에 등록
	if (UWorld* World = GetWorld())
	{
		if (UDGMinimapSubsystem* MinimapSubsystem = World->GetSubsystem<UDGMinimapSubsystem>())
		{
			MinimapSubsystem->RegisterMarker(this);
		}
	}
}

void UDGMinimapMarkerComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);

	// 액터가 사라지면 MinimapSubsystem에서도 삭제
	if (UWorld* World = GetWorld())
	{
		if (UDGMinimapSubsystem* MinimapSubsystem = World->GetSubsystem<UDGMinimapSubsystem>())
		{
			MinimapSubsystem->UnregisterMarker(this);
		}
	}
}