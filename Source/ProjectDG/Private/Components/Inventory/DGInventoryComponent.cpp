// Fill out your copyright notice in the Description page of Project Settings.


#include "Components/Inventory/DGInventoryComponent.h"


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
