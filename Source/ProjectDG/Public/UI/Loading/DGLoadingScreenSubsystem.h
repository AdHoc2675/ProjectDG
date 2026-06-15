#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Engine/DataTable.h"
#include "DGLoadingScreenSubsystem.generated.h"

class UDGLoadingScreenWidget;
class UDataTable;

UCLASS()
class PROJECTDG_API UDGLoadingScreenSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	UFUNCTION(BlueprintCallable, Category = "LoadingScreen")
	void ShowLoadingScreen();

	UFUNCTION(BlueprintCallable, Category = "LoadingScreen")
	void HideLoadingScreen();

	// 월드 파티션 맵 내에서 목적지로 텔레포트하고 로딩을 처리합니다.
	UFUNCTION(BlueprintCallable, Category = "LoadingScreen")
	void TeleportWithLoadingScreen(AActor* TargetActor, const FTransform& DestTransform);

	// ShowLoadingScreen() 호출 후, 스트리밍 완료 시 자동으로 HideLoadingScreen()을 호출하는 타이머를 시작합니다.
	UFUNCTION(BlueprintCallable, Category = "LoadingScreen")
	void StartStreamingCheck();

	// 스트리밍 완료 후 로딩 화면을 유지할 추가 시간 (초). 오브젝트 스폰 등을 기다리기 위함.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LoadingScreen")
	float PostStreamingDelay = 3.5f;

	UFUNCTION(BlueprintCallable, Category = "LoadingScreen")
	void SetLoadingWidgetClass(TSubclassOf<UDGLoadingScreenWidget> InWidgetClass);

	UFUNCTION(BlueprintCallable, Category = "LoadingScreen")
	void SetTipsDataTable(UDataTable* InDataTable);

private:
	void OnPreLoadMap(const FString& MapName);
	void OnPostLoadMap(UWorld* LoadedWorld);

	void CheckStreamingStatus();
	void ResetState();

	FText GetRandomTipText() const;

private:
	UPROPERTY()
	TSubclassOf<UDGLoadingScreenWidget> LoadingWidgetClass;

	UPROPERTY()
	TObjectPtr<UDataTable> TipsDataTable;

	UPROPERTY()
	TObjectPtr<UDGLoadingScreenWidget> ActiveLoadingWidget;

	FTimerHandle StreamingCheckTimerHandle;
	FTimerHandle HideDelayTimerHandle;

	/** 레벨 전환(ClientTravel) 진행 중 여부 */
	bool bIsLevelTransitioning = false;
};
