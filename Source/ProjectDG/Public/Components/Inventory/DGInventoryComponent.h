#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "DGInventoryComponent.generated.h"

class UDGItemInstance;
class UDGItemDefinition;

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent, PrioritizeCategories="DG"))
class PROJECTDG_API UDGInventoryComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UDGInventoryComponent();

	// 아이템 제작 요청 함수
	UFUNCTION(BlueprintCallable, Category = "DG|Inventory")
	UDGItemInstance* CraftItem(UDGItemDefinition* ItemToCraft, int32 CrafterLevel);

	// 아이템 강화 요청 함수
	UFUNCTION(BlueprintCallable, Category = "DG|Inventory")
	bool EnhanceItem(UDGItemInstance* TargetItem);

protected:
	// 플레이어가 소유한 장비 인스턴스들의 리스트
	UPROPERTY(EditAnywhere, Instanced, Category = "DG|Inventory")
	TArray<TObjectPtr<UDGItemInstance>> InventoryItems;
};