#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Item/DG_ItemTypes.h" 
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


#pragma region 장비 장착/해제 관련
public:
	// 장비 장착 함수
	UFUNCTION(BlueprintCallable, Category = "DG|Inventory")
	void EquipItem(UDGItemInstance* ItemToEquip);

	// 클라이언트가 서버에 장착을 요청하는 Server RPC
	UFUNCTION(Server, Reliable)
	void ServerEquipItem(int32 ItemIndex);

	// 장비 해제 함수
	UFUNCTION(BlueprintCallable, Category = "DG|Inventory")
	void UnequipItem(EDGEquipmentType SlotType);

	// 클라이언트가 서버에 해제를 요청하는 Server RPC
	UFUNCTION(Server, Reliable)
	void ServerUnequipItem(EDGEquipmentType SlotType);

	// 장착된 아이템 정보 가져오기
	UFUNCTION(BlueprintCallable, Category = "DG|Inventory")
	UDGItemInstance* GetEquippedItem(EDGEquipmentType SlotType) const;

protected:
	// [추가] 현재 부위별로 장착 중인 아이템 관리 Map
	UPROPERTY(EditAnywhere, Instanced, Category = "DG|Inventory")
	TMap<EDGEquipmentType, TObjectPtr<UDGItemInstance>> EquippedItems;
#pragma endregion 장비 장착/해제 관련
};