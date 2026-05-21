// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "DG_HUD.generated.h"

class UUserWidget;
class UAbilitySystemComponent;
class UAttributeSet;
class UDGOverlayWidgetController;
struct FWidgetControllerParams;
class UDGUserWidget;

class UDGInventoryWidgetController;
class UDGInventoryWidget;

/*
설명
- 게임의 HUD를 전체적으로 관리하는 역할
- UMG 위젯을 화면에 띄우는 역할만 하고, 실제 데이터 업데이트(예: "체력이 깎였다") 이벤트는 컨트롤러 인스턴스가 관장
- HUD는 플레이어마다 존재하므로, 플레이어마다 다른 정보를 띄울 수 있음 (예: "체력이 깎였다" 이벤트는 해당 플레이어에게만 띄워짐)

- GAS의 GetGameplayAttributeValueChangeDelegate() 등을 활용해 체력, 정신력 등이 변경될 때 델리게이트를 호출
*/

UCLASS()
class PROJECTDG_API ADG_HUD : public AHUD
{
	GENERATED_BODY()

#pragma region Core
public:
	// 캐릭터 쪽에서 ASC 초기화가 끝난 후 호출할 함수
	void SetupPlayerUI(class UAbilitySystemComponent* ASC, class UDG_AttributeSet* AttributeSet);

protected:
	virtual void BeginPlay() override;

private:
	// 공통 입력 상태 관리 헬퍼 함수
	void UpdateInputMode();
#pragma endregion Core


#pragma region Overlay
public:
	// HUD 초기화 함수 (PlayerController, PlayerState, AbilitySystemComponent 연동)
	void InitOverlay(APlayerController* PC, APlayerState* PS, UAbilitySystemComponent* ASC, UAttributeSet* AS);

	// 여러 번 호출해도 한 번 만들어진 컨트롤러를 리턴 (싱글턴 느낌)
	UDGOverlayWidgetController* GetOverlayWidgetController(const FWidgetControllerParams& WCParams);

private:
	// 메인 HUD 위젯 클래스 (블루프린트에서 할당)
	UPROPERTY(EditDefaultsOnly, Category = "DG|UI|Overlay")
	TSubclassOf<UDGUserWidget> OverlayWidgetClass;

	// 화면에 띄워질 메인 위젯 인스턴스
	UPROPERTY()
	TObjectPtr<UDGUserWidget> OverlayWidget;

	// 컨트롤러 클래스 및 인스턴스
	UPROPERTY(EditDefaultsOnly, Category = "DG|UI|Overlay")
	TSubclassOf<UDGOverlayWidgetController> OverlayWidgetControllerClass;

	UPROPERTY()
	TObjectPtr<UDGOverlayWidgetController> OverlayWidgetController;
#pragma endregion Overlay


#pragma region FullMap
public:
	void ToggleMapWidget();

private:
	UPROPERTY(EditDefaultsOnly, Category = "DG|UI|Map")
	TSubclassOf<class UDGUserWidget> FullMapWidgetClass;

	UPROPERTY()
	TObjectPtr<class UDGUserWidget> FullMapWidget;
	
	bool bIsMapOpen = false;
#pragma endregion FullMap


#pragma region Inventory
public:
	void ToggleInventoryWidget();

private:
	// 인벤토리 컨트롤러를 가져오거나 생성하는 헬퍼 함수
	UDGInventoryWidgetController* GetInventoryWidgetController(const FWidgetControllerParams& WCParams);

	UPROPERTY(EditDefaultsOnly, Category = "DG|UI|Inventory")
	TSubclassOf<class UDGUserWidget> InventoryWidgetClass;

	UPROPERTY()
	TObjectPtr<class UDGUserWidget> InventoryWidget;

	bool bIsInventoryOpen = false;

	// 인벤토리 컨트롤러 클래스 지정 속성
	UPROPERTY(EditAnywhere, Category = "DG|UI|Inventory")
	TSubclassOf<UDGInventoryWidgetController> InventoryWidgetControllerClass;

	// 인벤토리 컨트롤러 인스턴스
	UPROPERTY()
	TObjectPtr<UDGInventoryWidgetController> InventoryWidgetController;
#pragma endregion Inventory

};
