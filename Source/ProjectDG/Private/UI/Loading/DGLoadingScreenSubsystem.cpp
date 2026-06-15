#include "UI/Loading/DGLoadingScreenSubsystem.h"
#include "Engine/GameInstance.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"
#include "UI/Loading/DGLoadingScreenWidget.h"
#include "UI/Loading/DGLoadingTipRow.h"
#include "WorldPartition/WorldPartitionSubsystem.h"

DEFINE_LOG_CATEGORY_STATIC(LogDGLoadingScreen, Log, All);

void UDGLoadingScreenSubsystem::Initialize(
    FSubsystemCollectionBase &Collection) {
  Super::Initialize(Collection);

  FCoreUObjectDelegates::PreLoadMap.AddUObject(
      this, &UDGLoadingScreenSubsystem::OnPreLoadMap);
  FCoreUObjectDelegates::PostLoadMapWithWorld.AddUObject(
      this, &UDGLoadingScreenSubsystem::OnPostLoadMap);
}

void UDGLoadingScreenSubsystem::Deinitialize() {
  FCoreUObjectDelegates::PreLoadMap.RemoveAll(this);
  FCoreUObjectDelegates::PostLoadMapWithWorld.RemoveAll(this);

  Super::Deinitialize();
}

void UDGLoadingScreenSubsystem::SetLoadingWidgetClass(
    TSubclassOf<UDGLoadingScreenWidget> InWidgetClass) {
  LoadingWidgetClass = InWidgetClass;
}

void UDGLoadingScreenSubsystem::SetTipsDataTable(UDataTable *InDataTable) {
  TipsDataTable = InDataTable;
}

FText UDGLoadingScreenSubsystem::GetRandomTipText() const {
  if (!TipsDataTable) {
    return FText::GetEmpty();
  }

  TArray<FDGLoadingTipRow *> AllRows;
  TipsDataTable->GetAllRows<FDGLoadingTipRow>(TEXT("LoadingScreenSubsystem"),
                                              AllRows);

  if (AllRows.Num() > 0) {
    int32 RandomIndex = FMath::RandRange(0, AllRows.Num() - 1);
    return AllRows[RandomIndex]->TipText;
  }

  return FText::GetEmpty();
}

bool UDGLoadingScreenSubsystem::IsLoadingScreenVisible() const {
  return ActiveLoadingWidget && ActiveLoadingWidget->IsInViewport();
}

void UDGLoadingScreenSubsystem::ShowLoadingScreen() {
  // 위젯이 유효하고 실제로 뷰포트에 있는지 확인 (레벨 전환 후 댕글링 포인터 방어)
  if (ActiveLoadingWidget && ActiveLoadingWidget->IsInViewport()) {
    UE_LOG(LogDGLoadingScreen, Log,
           TEXT("Loading screen is already visible. Skipping."));
    return;
  }

  // 이전 레벨에서 남은 댕글링 참조 정리
  if (ActiveLoadingWidget) {
    UE_LOG(LogDGLoadingScreen, Warning,
           TEXT("Clearing stale ActiveLoadingWidget reference from previous level."));
    ActiveLoadingWidget = nullptr;
  }

  if (!LoadingWidgetClass) {
    // Fallback: DefaultGame.ini의 DirectoriesToAlwaysCook에 의해 쿠킹이 보장됨
    FSoftClassPath LoadingWidgetPath(
        TEXT("/Game/__ProjectDG/__BP/UI/Loading_Screens/"
             "WBP_LoadingScreen.WBP_LoadingScreen_C"));
    UClass *LoadedClass =
        LoadingWidgetPath.TryLoadClass<UDGLoadingScreenWidget>();
    if (LoadedClass) {
      LoadingWidgetClass = LoadedClass;
    }
  }

  if (!TipsDataTable) {
    // Fallback: DefaultGame.ini의 DirectoriesToAlwaysCook에 의해 쿠킹이 보장됨
    FSoftObjectPath DataTablePath(
        TEXT("/Game/__ProjectDG/__BP/UI/Loading_Screens/"
             "DT_LoadingTips.DT_LoadingTips"));
    UDataTable *LoadedTable = Cast<UDataTable>(DataTablePath.TryLoad());
    if (LoadedTable) {
      TipsDataTable = LoadedTable;
    }
  }

  if (!LoadingWidgetClass) {
    UE_LOG(LogDGLoadingScreen, Warning,
           TEXT("LoadingWidgetClass is not set. Cannot show loading screen."));
    return;
  }

  UWorld *World = GetWorld();
  if (!World) {
    UE_LOG(LogDGLoadingScreen, Warning,
           TEXT("World is null. Cannot show loading screen."));
    return;
  }

  ActiveLoadingWidget =
      CreateWidget<UDGLoadingScreenWidget>(World, LoadingWidgetClass);
  if (ActiveLoadingWidget) {
    ActiveLoadingWidget->SetTipText(GetRandomTipText());
    ActiveLoadingWidget->AddToViewport(9999); // High Z-order to render on top
    UE_LOG(LogDGLoadingScreen, Log,
           TEXT("Loading screen displayed successfully. World: %s"),
           *World->GetName());
  } else {
    UE_LOG(LogDGLoadingScreen, Error,
           TEXT("Failed to create loading screen widget."));
  }
}

void UDGLoadingScreenSubsystem::HideLoadingScreen() {
  bPendingLoadingScreen = false;

  if (ActiveLoadingWidget) {
    ActiveLoadingWidget->RemoveFromParent();
    ActiveLoadingWidget = nullptr;
    UE_LOG(LogDGLoadingScreen, Log, TEXT("Loading screen hidden."));
  }

  UWorld *World = GetWorld();
  if (World) {
    World->GetTimerManager().ClearTimer(StreamingCheckTimerHandle);
    World->GetTimerManager().ClearTimer(HideDelayTimerHandle);
  }
}

void UDGLoadingScreenSubsystem::StartStreamingCheck() {
  UWorld *World = GetWorld();
  if (World) {
    World->GetTimerManager().SetTimer(
        StreamingCheckTimerHandle, this,
        &UDGLoadingScreenSubsystem::CheckStreamingStatus,
        0.1f, // Check every 0.1 seconds
        true);
  }
}

void UDGLoadingScreenSubsystem::TeleportWithLoadingScreen(
    AActor *TargetActor, const FTransform &DestTransform) {
  if (!TargetActor)
    return;

  // 1. Show Loading Screen
  ShowLoadingScreen();

  // 2. Teleport the Actor
  TargetActor->SetActorTransform(DestTransform);

  // 3. Wait for world partition streaming
  StartStreamingCheck();
}

void UDGLoadingScreenSubsystem::CheckStreamingStatus() {
  UWorld *World = GetWorld();
  if (!World)
    return;

  UWorldPartitionSubsystem *WPSubsystem =
      World->GetSubsystem<UWorldPartitionSubsystem>();
  if (!WPSubsystem) {
    // Not a world partition world, hide immediately
    UE_LOG(
        LogDGLoadingScreen, Log,
        TEXT("Not a World Partition map. Hiding loading screen immediately."));
    HideLoadingScreen();
    return;
  }

  // Check if the world partition streaming is completed
  bool bIsStreamingDone = WPSubsystem->IsStreamingCompleted();

  UE_LOG(LogDGLoadingScreen, Log,
         TEXT("Checking Streaming Status... IsCompleted: %s"),
         bIsStreamingDone ? TEXT("True") : TEXT("False"));

  if (bIsStreamingDone) {
    UE_LOG(LogDGLoadingScreen, Log,
           TEXT("Streaming completed. Hiding loading screen after %.1f seconds."),
           PostStreamingDelay);

    // 스트리밍 체크 타이머 중지
    World->GetTimerManager().ClearTimer(StreamingCheckTimerHandle);

    // PostStreamingDelay만큼 대기 후 Hide
    World->GetTimerManager().SetTimer(
        HideDelayTimerHandle, this,
        &UDGLoadingScreenSubsystem::HideLoadingScreen,
        PostStreamingDelay, false);
  }
}

void UDGLoadingScreenSubsystem::OnPreLoadMap(const FString &MapName) {
  UE_LOG(LogDGLoadingScreen, Log,
         TEXT("OnPreLoadMap: %s — cleaning up previous level state."), *MapName);

  // 현재 로딩 화면이 떠있었다면, 새 맵에서도 다시 띄워야 함을 기억
  if (ActiveLoadingWidget || bPendingLoadingScreen) {
    bPendingLoadingScreen = true;
    UE_LOG(LogDGLoadingScreen, Log,
           TEXT("Loading screen was active. Will re-show after new map loads."));
  }

  // 레벨 전환 시 기존 위젯과 타이머가 이전 World와 함께 소멸되므로 참조를 정리
  ActiveLoadingWidget = nullptr;
  StreamingCheckTimerHandle.Invalidate();
  HideDelayTimerHandle.Invalidate();
}

void UDGLoadingScreenSubsystem::OnPostLoadMap(UWorld *LoadedWorld) {
  UE_LOG(LogDGLoadingScreen, Log,
         TEXT("OnPostLoadMap: %s — bPendingLoadingScreen=%s"),
         LoadedWorld ? *LoadedWorld->GetName() : TEXT("null"),
         bPendingLoadingScreen ? TEXT("true") : TEXT("false"));

  if (bPendingLoadingScreen) {
    bPendingLoadingScreen = false;
    ShowLoadingScreen();
    StartStreamingCheck();
  }
}
