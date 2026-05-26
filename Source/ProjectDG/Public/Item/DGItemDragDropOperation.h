#pragma once

#include "CoreMinimal.h"
#include "Blueprint/DragDropOperation.h"
#include "DGItemDragDropOperation.generated.h"

class UDGItemInstance;
class UDGInventorySlotWidget;

UCLASS()
class PROJECTDG_API UDGItemDragDropOperation : public UDragDropOperation
{
	GENERATED_BODY()

public:
	// 마우스로 이동 중인 실제 아이템 데이터
	UPROPERTY(BlueprintReadWrite, Category = "DragDrop", meta = (ExposeOnSpawn = "true"))
	TObjectPtr<UDGItemInstance> DraggedItem;

	// 어디서 드래그를 시작했는지 원본 슬롯 (나중에 자리 교환 기능 등에 쓰임)
	UPROPERTY(BlueprintReadWrite, Category = "DragDrop", meta = (ExposeOnSpawn = "true"))
	TObjectPtr<UUserWidget> SourceWidget;
};