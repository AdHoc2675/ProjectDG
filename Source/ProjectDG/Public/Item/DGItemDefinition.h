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

	// 장비할 메쉬 (이 부분이 캐릭터에게 전달되어 실루엣을 바꿈)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item Data", meta = (EditCondition = "ItemType == EDGItemType::Equipment", EditConditionHides))
	TSoftObjectPtr<USkeletalMesh> EquipmentMesh;

	// 아이템 설명 (UI에서 여러 줄로 표시될 수 있음)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item Data", meta = (MultiLine = true))
	FText ItemDescription;

	// 아이콘 이미지
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item Data")
	TObjectPtr<class UTexture2D> ItemIcon;

	// 아이템 획득(루팅) 시 재생될 사운드
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item Data|Sound")
	TObjectPtr<class USoundBase> PickupSound;

	// 장비 장착 시 재생될 사운드 (장비 전용)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item Data|Sound", meta = (EditCondition = "ItemType == EDGItemType::Equipment", EditConditionHides))
	TObjectPtr<class USoundBase> EquipSound;

	// 장비 해제 시 재생될 사운드 (장비 전용)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item Data|Sound", meta = (EditCondition = "ItemType == EDGItemType::Equipment", EditConditionHides))
	TObjectPtr<class USoundBase> UnequipSound;

};