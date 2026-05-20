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

	// 화면에 표시될 이름 (예: 장검)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item Data")
	FText ItemName;

	// 무기인지 방어구인지 구분
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item Data")
	EDGEquipmentType EquipmentType;

	// 아이콘 이미지
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item Data")
	TObjectPtr<class UTexture2D> ItemIcon;

	// 장착했을 때 보여줄 외형 메시
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item Data")
	TObjectPtr<class USkeletalMesh> ItemMesh;
};