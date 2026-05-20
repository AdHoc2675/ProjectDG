#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Item/DG_ItemTypes.h"
#include "DGItemInstance.generated.h"

class UDGItemDefinition;

/**
 * 유저 인벤토리에 존재하는 실제 아이템 객체
 * 강화 수치, 아이템 레벨, 고유 난수옵션을 가집니다.
 */
UCLASS(BlueprintType, Blueprintable)
class PROJECTDG_API UDGItemInstance : public UObject
{
	GENERATED_BODY()

public:
	// 원본 데이터 참조
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Item Instance")
	TObjectPtr<UDGItemDefinition> ItemDef;

	// 인스턴스 고유 값들
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Item Instance")
	EDGItemGrade Grade;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Item Instance")
	int32 ItemLevel;

	// 부여된 보조 옵션 4개
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Item Instance")
	TArray<FDGSubOptionInstanceData> SubOptions;
};