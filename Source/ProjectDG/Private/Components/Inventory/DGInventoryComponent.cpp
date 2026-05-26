// Fill out your copyright notice in the Description page of Project Settings.


#include "Components/Inventory/DGInventoryComponent.h"
#include "GameplayEffect.h"
#include "AbilitySystemComponent.h"
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

	EDGEquipmentType EquipType = ItemToEquip->ItemDef->EquipmentType;

	// 1. 이미 해당 부위에 장비가 있다면 먼저 장비 해제
	if (EquippedItems.Contains(EquipType) && EquippedItems[EquipType] != nullptr)
	{
		UnequipItem(EquipType);
	}

	// 2. 인벤토리에서 제거 후 장착 Map에 추가
	InventoryEquipmentItems.Remove(ItemToEquip);
	EquippedItems.Add(EquipType, ItemToEquip);

	// 3. GAS 스탯 적용 (동적 GameplayEffect 생성)
	UAbilitySystemComponent* ASC = GetOwner()->FindComponentByClass<UAbilitySystemComponent>();
	if (ASC)
	{
		// 런타임에 휘발성(Transient) 버프를 하나 생성
		UGameplayEffect* EquipEffect = NewObject<UGameplayEffect>(GetTransientPackage(), FName(TEXT("EquipStats")));
		EquipEffect->DurationPolicy = EGameplayEffectDurationType::Infinite; // 무한 지속(벗을 때까지)

		// 람다 함수로 스탯 더하기
		auto AddModifier = [&EquipEffect](FGameplayAttribute Attribute, float Value)
			{
				if (Value == 0.f) return;
				FGameplayModifierInfo ModInfo;
				ModInfo.Attribute = Attribute;
				ModInfo.ModifierOp = EGameplayModOp::Additive; // 더하기 연산
				ModInfo.ModifierMagnitude = FScalableFloat(Value);
				EquipEffect->Modifiers.Add(ModInfo);
			};

		// 아이템의 스탯들을 버프에 추가 (주스탯/공/방/체력 등)
		AddModifier(UDG_AttributeSet::GetMaxHealthAttribute(), ItemToEquip->HPValue);
		AddModifier(UDG_AttributeSet::GetAttackPowerAttribute(), ItemToEquip->AttackValue);
		AddModifier(UDG_AttributeSet::GetDefenseAttribute(), ItemToEquip->DefenseValue);
		AddModifier(UDG_AttributeSet::GetMainStatAttribute(), ItemToEquip->MainStatValue);

		// TODO: SubOptions 순회하며 추가 옵션들도 AddModifier로 더해줄 수 있음

		// 주인공의 ASC에 적용 후, 반환된 추적 번호를 아이템에 저장
		FActiveGameplayEffectHandle Handle = ASC->ApplyGameplayEffectToSelf(EquipEffect, 1.f, ASC->MakeEffectContext());
		ItemToEquip->EquipStatHandle = Handle;

		UE_LOG(LogTemp, Log, TEXT("[DGInventoryComponent] [%s] 장착 완료. 스탯 증가 적용됨."), *ItemToEquip->ItemDef->ItemName.ToString());
	}
}

void UDGInventoryComponent::UnequipItem(EDGEquipmentType SlotType)
{
	if (!EquippedItems.Contains(SlotType)) return;

	UDGItemInstance* ItemToUnequip = EquippedItems[SlotType];
	if (!ItemToUnequip) return;

	// 1. 장착된 버프 제거 (스탯 원상복구)
	UAbilitySystemComponent* ASC = GetOwner()->FindComponentByClass<UAbilitySystemComponent>();
	if (ASC && ItemToUnequip->EquipStatHandle.IsValid())
	{
		ASC->RemoveActiveGameplayEffect(ItemToUnequip->EquipStatHandle);
		ItemToUnequip->EquipStatHandle.Invalidate();
	}

	// 2. 장착 슬롯에서 인벤토리로 이동
	EquippedItems.Remove(SlotType);
	InventoryEquipmentItems.Add(ItemToUnequip);

	UE_LOG(LogTemp, Log, TEXT("[%s] 해제 완료. 스탯 감소 복구됨."), *ItemToUnequip->ItemDef->ItemName.ToString());
}

UDGItemInstance* UDGInventoryComponent::GetEquippedItem(EDGEquipmentType SlotType) const
{
	return nullptr;
}
