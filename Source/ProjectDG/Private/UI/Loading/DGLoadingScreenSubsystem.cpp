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

void UDGLoadingScreenSubsystem::ShowLoadingScreen() {
  // 레벨 전환 중 파괴된 위젯 포인터를 안전하게 처리
  if (ActiveLoadingWidget && IsValid(ActiveLoadingWidget) &&
      ActiveLoadingWidget->IsInViewport()) {
    return; // Already showing
  }

  // 이전 위젯이 무효화된 경우 포인터 정리
  ActiveLoadingWidget = nullptr;

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
    return;
  }

  ActiveLoadingWidget =
      CreateWidget<UDGLoadingScreenWidget>(World, LoadingWidgetClass);
  if (ActiveLoadingWidget) {
    ActiveLoadingWidget->SetTipText(GetRandomTipText());
    ActiveLoadingWidget->AddToViewport(9999); // High Z-order to render on top
  }
}

void UDGLoadingScreenSubsystem::HideLoadingScreen() {
  if (ActiveLoadingWidget) {
    ActiveLoadingWidget->RemoveFromParent();
    ActiveLoadingWidget = nullptr;
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
         TEXT("OnPreLoadMap: %s — showing loading screen"), *MapName);

  bIsLevelTransitioning = true;

  // 기존 타이머 정리 (이전 월드의 타이머는 곧 무효화됨)
  UWorld *World = GetWorld();
  if (World) {
    World->GetTimerManager().ClearTimer(StreamingCheckTimerHandle);
    World->GetTimerManager().ClearTimer(HideDelayTimerHandle);
  }

  // 이전 위젯 포인터 정리 (곧 파괴될 위젯)
  ActiveLoadingWidget = nullptr;

  // 새 로딩 화면 표시
  ShowLoadingScreen();
}

void UDGLoadingScreenSubsystem::OnPostLoadMap(UWorld *LoadedWorld) {
  UE_LOG(LogDGLoadingScreen, Log,
         TEXT("OnPostLoadMap — resetting state and starting streaming check"));

  // 레벨 전환 중에 위젯이 파괴되었을 수 있으므로 상태 리셋
  ResetState();

  if (bIsLevelTransitioning) {
    bIsLevelTransitioning = false;

    // 새 월드에서 로딩 화면을 다시 생성
    ShowLoadingScreen();

    // 스트리밍 완료를 기다린 후 자동으로 숨김
    StartStreamingCheck();
  }
}

void UDGLoadingScreenSubsystem::ResetState() {
  // 위젯 포인터 정리 (레벨 전환으로 파괴되었을 수 있음)
  if (ActiveLoadingWidget && !IsValid(ActiveLoadingWidget)) {
    ActiveLoadingWidget = nullptr;
  }

  // 타이머 핸들 무효화 (이전 월드의 타이머는 이미 파괴됨)
  StreamingCheckTimerHandle.Invalidate();
  HideDelayTimerHandle.Invalidate();
}
