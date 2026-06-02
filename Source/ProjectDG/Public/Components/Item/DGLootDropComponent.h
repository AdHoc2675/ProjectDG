#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Item/DG_ItemTypes.h"
#include "DGLootDropComponent.generated.h"

class UDGItemDefinition;
class ADGLootItemActor;

// 개별 드롭 테이블 행 구조체
USTRUCT(BlueprintType)
struct FDGLootDropInfo
{
	GENERATED_BODY()

	// 드롭할 아이템 원본 데이터
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<UDGItemDefinition> ItemDef;

	// 드롭될 아이템의 표시 등급
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EDGItemGrade Grade = EDGItemGrade::Hero;

	// 몇 개가 떨어질 것인지 (최소/최대)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 MinQuantity = 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 MaxQuantity = 1;

	// 드롭 확률 (0.0 ~ 100.0)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0", ClampMax = "100.0"))
	float DropChance = 100.f;
};


UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent, PrioritizeCategories = "DG"))
class PROJECTDG_API UDGLootDropComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UDGLootDropComponent();

	// 몬스터 사망 시 외부(액터)에서 이 함수를 호출
	UFUNCTION(BlueprintCallable, Category = "DG|Loot")
	void ProcessDrop(const FVector& DropLocation);

protected:
	UPROPERTY(EditDefaultsOnly, Category = "DG|Loot")
	TSubclassOf<ADGLootItemActor> LootItemClass;

	// 이 몬스터(또는 스포너)가 가지는 고유 드롭 리스트
	// (추후 UFieldCharacterClassData 와 같은 DataAsset에서 가져올 수도 있습니다)
	UPROPERTY(EditAnywhere, Category = "DG|Loot")
	TArray<FDGLootDropInfo> DropTable;
};