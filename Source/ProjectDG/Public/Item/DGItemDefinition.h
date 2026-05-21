#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Item/DG_ItemTypes.h"
#include "DGItemDefinition.generated.h"

UCLASS(BlueprintType)
class PROJECTDG_API UDGItemDefinition : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	// 고유 ID 
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item Data")
	FName ItemID;

	// 화면에 표시될 이름
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item Data")
	FText ItemName;

	// 아이템 타입 (장비, 소모품, 재료)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item Data")
	EDGItemType ItemType;

	// 무기인지 방어구인지 구분 (장비 타입일 때만 유효)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item Data", meta = (EditCondition = "ItemType == EDGItemType::Equipment", EditConditionHides))
	EDGEquipmentType EquipmentType;

	// 아이콘 이미지
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item Data")
	TObjectPtr<class UTexture2D> ItemIcon;

};