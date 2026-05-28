#include "Components/UI/DGMinimapMarkerComponent.h"
#include "UI/Widget/Minimap/DGMinimapSubsystem.h"

#include "GameFramework/Pawn.h"

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

bool UDGMinimapMarkerComponent::IsLocalPlayerMarker() const
{
	// 오너가 폰(Pawn)인지 확인
	if (APawn* OwnerPawn = Cast<APawn>(GetOwner()))
	{
		// 폰이 내 로컬 컨트롤러에 의해 조종되고 있는지 확인
		return OwnerPawn->IsLocallyControlled();
	}

	return false;
}

void UDGMinimapMarkerComponent::NotifyClicked()
{
	// 나를 구독하고 있는 대상(웨이포인트 등)에게 클릭 사실을 알림
	OnMarkerClicked.Broadcast(this);
}
