#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Item/DG_ItemTypes.h" 
#include "DGInventoryComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnEquipmentChanged, EDGEquipmentType, SlotType, class UDGItemDefinition*, EquippedItemDef);

// 아이템 습득 시 UI(오버레이 등)에 알리기 위한 델리게이트
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnItemLootedSignature, class UDGItemDefinition*, ItemDef, int32, Quantity, EDGItemGrade, Grade);

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

	// 아이템 습득 시 호출할 함수 (드롭 액터에서 호출, 서버 진입점)
	UFUNCTION(BlueprintCallable, Category = "DG|Inventory")
	void AddItem(UDGItemDefinition* NewItemDef, int32 Quantity, EDGItemGrade Grade = EDGItemGrade::Hero);

	// 미리 세팅된 아이템 인스턴스를 직접 습득할 때 호출할 함수
	UFUNCTION(BlueprintCallable, Category = "DG|Inventory")
	void AddItemByInstance(UDGItemInstance* NewItemInstance);

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


	// 서버가 클라이언트에게 "로컬 인벤토리에 아이템 추가"를 지시하는 RPC
	UFUNCTION(Client, Reliable)
	void ClientAddItem(UDGItemDefinition* NewItemDef, int32 Quantity, EDGItemGrade Grade);

	// 커스텀 스탯이 있는 인스턴스를 클라이언트에게 추가하라고 지시하는 RPC
	UFUNCTION(Client, Reliable)
	void ClientAddItemByInstance(UDGItemDefinition* ItemDef, int32 Quantity, EDGItemGrade Grade, float HP, float Atk, float Def, float Main, const TArray<FDGSubOptionInstanceData>& SubOptions);

	// 실제로 UObject를 생성하고 배열에 넣는 공통 내부 로직
	void InternalAddItemConfig(UDGItemDefinition* NewItemDef, int32 Quantity, EDGItemGrade Grade);


#pragma region 장비 장착/해제 관련
public:
public:
	// 아이템 획득 이벤트
	UPROPERTY(BlueprintAssignable, Category = "DG|Inventory")
	FOnItemLootedSignature OnItemLooted;

	// 장착 슬롯에 아이템이 변경될 때 발생하는 이벤트 (장착/해제 모두 사용)
	UPROPERTY(BlueprintAssignable, Category = "DG|Inventory")
	FOnEquipmentChanged OnEquipmentChanged;

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

	// 다른 클라이언트들에게 외형 변경을 알리기 위한 Multicast RPC
	UFUNCTION(NetMulticast, Reliable)
	void MulticastEquipmentChanged(EDGEquipmentType SlotType, UDGItemDefinition* EquippedItemDef);

	// 스탯이 초기화(레벨업 등) 되었을 때, 현재 장착 중인 아이템의 스탯을 다시 더해주는 함수
	void ReapplyEquippedItemStats();

protected:
	// [추가] 현재 부위별로 장착 중인 아이템 관리 Map
	UPROPERTY(EditAnywhere, Instanced, Category = "DG|Inventory")
	TMap<EDGEquipmentType, TObjectPtr<UDGItemInstance>> EquippedItems;
#pragma endregion 장비 장착/해제 관련
};