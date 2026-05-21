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

	// (임시) 인벤토리 아이템 리스트 Getter
	UFUNCTION(BlueprintCallable, Category = "DG|Inventory")
	TArray<UDGItemInstance*> GetInventoryEquipmentItems() const { return InventoryEquipmentItems; }

	// (임시) 소모품 리스트 Getter
	UFUNCTION(BlueprintCallable, Category = "DG|Inventory")
	TArray<UDGItemInstance*> GetInventoryConsumableItems() const { return InventoryConsumableItems; }

	// (임시) 제작 재료 리스트 Getter
	UFUNCTION(BlueprintCallable, Category = "DG|Inventory")
	TArray<UDGItemInstance*> GetInventoryCraftingMaterialItems() const { return InventoryCraftingMaterialItems; }

protected:
	// 플레이어가 소유한 장비 인스턴스들의 리스트
	UPROPERTY(EditAnywhere, Instanced, Category = "DG|Inventory")
	TArray<TObjectPtr<UDGItemInstance>> InventoryEquipmentItems;

	// 플레이어가 소유한 소모품 인스턴스들의 리스트
	UPROPERTY(EditAnywhere, Instanced, Category = "DG|Inventory")
	TArray<TObjectPtr<UDGItemInstance>> InventoryConsumableItems;

	// 플레이어가 소유한 제작 재료 인스턴스들의 리스트
	UPROPERTY(EditAnywhere, Instanced, Category = "DG|Inventory")
	TArray<TObjectPtr<UDGItemInstance>> InventoryCraftingMaterialItems;
};