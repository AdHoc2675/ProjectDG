// Fill out your copyright notice in the Description page of Project Settings.


#include "Components/Inventory/DGInventoryComponent.h"
#include "GameplayEffect.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemGlobals.h" 
#include "GAS/Attributes/DG_AttributeSet.h"
#include "Item/DGItemDefinition.h"
#include "Item/DGItemInstance.h"

// Sets default values for this component's properties
UDGInventoryComponent::UDGInventoryComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = false;

	// ...
}

UDGItemInstance* UDGInventoryComponent::CraftItem(UDGItemDefinition* ItemToCraft, int32 CrafterLevel)
{
	return nullptr;
}

bool UDGInventoryComponent::EnhanceItem(UDGItemInstance* TargetItem)
{
	return false;
}

void UDGInventoryComponent::EquipItem(UDGItemInstance* ItemToEquip)
{
	if (!ItemToEquip || !ItemToEquip->ItemDef) return;

	// [네트워크 분기] 클라이언트인 경우 서버로 요청을 보냄
	if (!GetOwner()->HasAuthority())
	{
		// 1. 내가 클릭한 아이템이 내 인벤토리 배열에서 몇 번째(Index)인지 찾음
		int32 FoundIndex = InventoryEquipmentItems.Find(ItemToEquip);

		if (FoundIndex != INDEX_NONE)
		{
			// 2. 서버에게 n번째 아이템 착용에 대한 승인을 요청
			ServerEquipItem(FoundIndex);
		}

		// 클라이언트의 즉각적인 UI 갱신을 위해 아이템 배열 자리만 이동시켜줌 (클라이언트 예측)
		EDGEquipmentType ClientEquipType = ItemToEquip->ItemDef->EquipmentType;

		// 클라이언트가 이미 장착 중인 아이템이 있다면 장비 해제 및 인벤토리로 이동
		if (EquippedItems.Contains(ClientEquipType) && EquippedItems[ClientEquipType] != nullptr)
		{
			UDGItemInstance* OldItem = EquippedItems[ClientEquipType];
			EquippedItems.Remove(ClientEquipType);
			InventoryEquipmentItems.Add(OldItem);
		}
		InventoryEquipmentItems.Remove(ItemToEquip);
		EquippedItems.Add(ClientEquipType, ItemToEquip);

		return; // 클라이언트는 "스탯 부여"를 하지 않고 여기서 종료
	}


	// ==========================================
	// 아래부터는 무조건 서버(Authority)에서만 실행
	// ==========================================

	EDGEquipmentType EquipType = ItemToEquip->ItemDef->EquipmentType;

	// 이미 동일 부위에 장비가 있다면 벗기기
	if (EquippedItems.Contains(EquipType) && EquippedItems[EquipType] != nullptr)
	{
		UnequipItem(EquipType);
	}

	// 서버에서의 인벤토리 이동 처리
	InventoryEquipmentItems.Remove(ItemToEquip);
	EquippedItems.Add(EquipType, ItemToEquip);

	// 3. 서버 권한으로 GAS 스탯 적용
	UAbilitySystemComponent* ASC = UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(GetOwner());
	if (ASC)
	{
		// 임시 GE 버프(NewObject)를 생성하지 말고, 베이스 스탯 수치를 직접 더해주는 방식
		auto AddModifier = [&ASC](FGameplayAttribute Attribute, float Value)
			{
				if (Value == 0.f) return;

				// 해당 속성의 Base(기본값)를 가져와서 무기 깡스탯만큼을 더하고 돌려줌
				float CurrentBase = ASC->GetNumericAttributeBase(Attribute);
				ASC->SetNumericAttributeBase(Attribute, CurrentBase + Value);
			};

		// 아이템의 스탯들을 기본 스탯에 적용
		AddModifier(UDG_AttributeSet::GetMaxHealthAttribute(), ItemToEquip->HPValue);
		AddModifier(UDG_AttributeSet::GetAttackPowerAttribute(), ItemToEquip->AttackValue);
		AddModifier(UDG_AttributeSet::GetDefenseAttribute(), ItemToEquip->DefenseValue);
		AddModifier(UDG_AttributeSet::GetMainStatAttribute(), ItemToEquip->MainStatValue);

		UE_LOG(LogTemp, Log, TEXT("[DGInventoryComponent Server] [%s] 장착 완료. 스탯 증가 서버 반영됨."), *ItemToEquip->ItemDef->ItemName.ToString());
	}
}

// Server RPC 구현부 (클라이언트가 EquipItem을 부르면 서버에서 이게 대신 실행됨)
void UDGInventoryComponent::ServerEquipItem_Implementation(int32 ItemIndex)
{
	// 서버 안에 들고있는 배열의 인덱스가 유효한지 안전 검사
	if (InventoryEquipmentItems.IsValidIndex(ItemIndex))
	{
		UDGItemInstance* ServerSideItem = InventoryEquipmentItems[ItemIndex];
		if (ServerSideItem)
		{
			// "서버" 본인이 가지고 있는 UObject를 넘겨서 장착 로직 실행
			EquipItem(ServerSideItem);
		}
	}
}


void UDGInventoryComponent::UnequipItem(EDGEquipmentType SlotType)
{
	if (!EquippedItems.Contains(SlotType)) return;

	UDGItemInstance* ItemToUnequip = EquippedItems[SlotType];
	if (!ItemToUnequip) return;

	// 클라이언트인 경우 서버로 요청을 보냄
	if (!GetOwner()->HasAuthority())
	{
		ServerUnequipItem(SlotType);

		// 클라이언트 예측 처리 (배열만 갱신)
		EquippedItems.Remove(SlotType);
		InventoryEquipmentItems.Add(ItemToUnequip);
		return;
	}

	// ==========================================
	// 아래부터는 무조건 서버(Authority)에서만 실행
	// ==========================================


// 장착된 아이템의 스탯 원상복구 (더해진 Base Value 차감)
	UAbilitySystemComponent* ASC = UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(GetOwner());
	if (ASC)
	{
		auto RemoveModifier = [&ASC](FGameplayAttribute Attribute, float Value)
			{
				if (Value == 0.f) return;

				// 방금 벗은 옷의 스탯만큼 차감
				float CurrentBase = ASC->GetNumericAttributeBase(Attribute);
				ASC->SetNumericAttributeBase(Attribute, CurrentBase - Value);
			};

		// 뺐던 아이템의 스탯 복구
		RemoveModifier(UDG_AttributeSet::GetMaxHealthAttribute(), ItemToUnequip->HPValue);
		RemoveModifier(UDG_AttributeSet::GetAttackPowerAttribute(), ItemToUnequip->AttackValue);
		RemoveModifier(UDG_AttributeSet::GetDefenseAttribute(), ItemToUnequip->DefenseValue);
		RemoveModifier(UDG_AttributeSet::GetMainStatAttribute(), ItemToUnequip->MainStatValue);

		// 최대 체력이 깎였을 때, 만약 현재 HP가 낮아진 MaxHP보다 높다면 상한에 맞춰줌
		float CurrentMaxHP = ASC->GetNumericAttributeBase(UDG_AttributeSet::GetMaxHealthAttribute());
		float CurrentHP = ASC->GetNumericAttributeBase(UDG_AttributeSet::GetHealthAttribute());
		if (CurrentHP > CurrentMaxHP)
		{
			ASC->SetNumericAttributeBase(UDG_AttributeSet::GetHealthAttribute(), CurrentMaxHP);
		}
	}

	// 장착 슬롯에서 인벤토리로 이동
	EquippedItems.Remove(SlotType);
	InventoryEquipmentItems.Add(ItemToUnequip);

	UE_LOG(LogTemp, Log, TEXT("[DGInventoryComponent Server] [%s] 해제 완료. 스탯 감소 서버 반영됨."), *ItemToUnequip->ItemDef->ItemName.ToString());
}

// Server RPC 구현부
void UDGInventoryComponent::ServerUnequipItem_Implementation(EDGEquipmentType SlotType)
{
	UnequipItem(SlotType);
}

UDGItemInstance* UDGInventoryComponent::GetEquippedItem(EDGEquipmentType SlotType) const
{
	return nullptr;
}
