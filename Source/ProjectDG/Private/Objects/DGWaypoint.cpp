#include "Objects/DGWaypoint.h"
#include "Components/UI/DGMinimapMarkerComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/Character.h"
#include "Character/Player/PlayerCharacterBase.h"
#include "UI/HUD/DG_HUD.h" 

ADGWaypoint::ADGWaypoint()
{
	PrimaryActorTick.bCanEverTick = false;

	MinimapMarkerComp = CreateDefaultSubobject<UDGMinimapMarkerComponent>(TEXT("MinimapMarkerComp"));

	// C++ 기본값 세팅: 웨이포인트용 마커이며 상호작용 가능함
	MinimapMarkerComp->MarkerType = EMinimapMarkerType::Waypoint;
	MinimapMarkerComp->bShowOnMinimap = true;
	MinimapMarkerComp->bShowOnFullMap = true;
	MinimapMarkerComp->bIsInteractable = true;
}

void ADGWaypoint::BeginPlay()
{
	Super::BeginPlay();

	// 델리게이트 구독: 클릭되면 HandleMarkerClicked 함수가 실행됨
	if (MinimapMarkerComp)
	{
		MinimapMarkerComp->OnMarkerClicked.AddDynamic(this, &ADGWaypoint::HandleMarkerClicked);
	}
}

void ADGWaypoint::HandleMarkerClicked(UDGMinimapMarkerComponent* ClickedMarker)
{
	// UI 클릭 이벤트는 '클라이언트'에서만 일어나므로, 로컬 플레이어 빙의 캐릭터를 찾음
	ACharacter* LocalPlayer = UGameplayStatics::GetPlayerCharacter(this, 0);

	if (LocalPlayer)
	{
		// 텔레포트 로직 실행
		ExecuteTeleport(LocalPlayer);
	}
}

void ADGWaypoint::ExecuteTeleport_Implementation(ACharacter* LocalPlayerCharacter)
{
	if (!LocalPlayerCharacter) return;

	FVector TargetLocation = GetActorLocation() + TeleportOffset;

	// -------------------------------------------------------------
	// [멀티플레이] 서버에게 이동 요청
	// -------------------------------------------------------------
	if (APlayerCharacterBase* DGPlayer = Cast<APlayerCharacterBase>(LocalPlayerCharacter))
	{
		// 이 코드는 클라이언트에서 불리더라도, Server RPC이므로 서버에게 패킷을 보내 서버에서 SetActorLocation이 실행
		DGPlayer->ServerTeleportToLocation(TargetLocation);
	}
	else
	{
		// 만약 캐스팅에 실패했거나 예외 상황일 경우 (안전용)
		LocalPlayerCharacter->SetActorLocation(TargetLocation);
	}

	// -------------------------------------------------------------
	// [텔레포트 후 HUD 조작] 풀맵 닫기 (로컬 UI 조작)
	// -------------------------------------------------------------
	APlayerController* PC = Cast<APlayerController>(LocalPlayerCharacter->GetController());
	if (PC)
	{
		if (ADG_HUD* HUD = Cast<ADG_HUD>(PC->GetHUD()))
		{
			HUD->ToggleMapWidget();
		}
	}
}