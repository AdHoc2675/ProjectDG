#include "UI/Widget/Minimap/DGMinimapMarkerWidget.h"
#include "Components/UI/DGMinimapMarkerComponent.h"

// _Implementation 접미사 제거
void UDGMinimapMarkerWidget::SetupMarker(UDGMinimapMarkerComponent* MarkerData)
{
	// C++에서 확실하게 변수에 할당을 먼저 수행
	OwnerMarkerComp = MarkerData;

	// 부가적인 처리가 필요하면 블루프린트 이벤트 호출
	ReceiveSetupMarker();
}