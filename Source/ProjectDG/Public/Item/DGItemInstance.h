#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Item/DG_ItemTypes.h"
#include "DGItemInstance.generated.h"

class UDGItemDefinition;

UCLASS(BlueprintType, Blueprintable, EditInlineNew, DefaultToInstanced)
class PROJECTDG_API UDGItemInstance : public UObject
{
	GENERATED_BODY()

public:
	// 개별 아이템 고유 ID (DB 저장 및 복제 구분용)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item Instance")
	int64 ItemUniqueID;

	// 원본 데이터 참조 (이름, 타입, 아이콘 등 접근용)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item Instance")
	TObjectPtr<UDGItemDefinition> ItemDef;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item Instance")
	EDGItemGrade Grade;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item Instance")
	int32 ItemLevel;

	// --- 생성 시 고정되는 주스탯 ---
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item Instance|MainStat")
	float MainStatValue = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item Instance|MainStat")
	float HPValue = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item Instance|MainStat")
	float AttackValue = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item Instance|MainStat")
	float DefenseValue = 0.f;

	// 부여된 보조 옵션 (최대 4개)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item Instance|SubOption")
	TArray<FDGSubOptionInstanceData> SubOptions;

	// 겹침 수량 (장비는 항상 1, 소모품/재료는 1 이상)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item Instance|Stack")
	int32 Quantity = 1;
};