#include "UI/Widget/Minimap/DGMinimapSubsystem.h"
#include "Components/UI/DGMinimapMarkerComponent.h"

void UDGMinimapSubsystem::RegisterMarker(UDGMinimapMarkerComponent* Marker)
{
	if (Marker && !ActiveMarkers.Contains(Marker))
	{
		ActiveMarkers.Add(Marker);
		OnMarkerRegistered.Broadcast(Marker);
	}
}

void UDGMinimapSubsystem::UnregisterMarker(UDGMinimapMarkerComponent* Marker)
{
	if (Marker && ActiveMarkers.Contains(Marker))
	{
		ActiveMarkers.Remove(Marker);
		OnMarkerUnregistered.Broadcast(Marker);
	}
}