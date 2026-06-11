#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Item/DG_ItemTypes.h"
#include "DGLootDropComponent.generated.h"

class UDGItemDefinition;
class ADGLootItemActor;
class UDGItemInstance;

// 개별 드롭 테이블 행 구조체
USTRUCT(BlueprintType)
struct FDGLootDropInfo
{
	GENERATED_BODY()

	// 드롭할 아이템 인스턴스 (스탯, 수량, 등급 등을 자유롭게 세팅 가능)
	UPROPERTY(EditAnywhere, Instanced, BlueprintReadWrite)
	TObjectPtr<UDGItemInstance> DropItemInstance;

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

	// 즉시 획득 보상 반환용 Getter
	int32 GetRewardExp() const { return RewardExp; }
	int32 GetMinRewardGold() const { return MinRewardGold; }
	int32 GetMaxRewardGold() const { return MaxRewardGold; }

protected:
	/** 사망 시 지급할 경험치 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DG|Loot")
	int32 RewardExp = 50;

	/** 사망 시 지급할 최소 골드 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DG|Loot")
	int32 MinRewardGold = 10;

	/** 사망 시 지급할 최대 골드 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DG|Loot")
	int32 MaxRewardGold = 30;

	UPROPERTY(EditDefaultsOnly, Category = "DG|Loot")
	TSubclassOf<ADGLootItemActor> LootItemClass;

	// 이 몬스터(또는 스포너)가 가지는 고유 드롭 리스트
	// (추후 UFieldCharacterClassData 와 같은 DataAsset에서 가져올 수도 있습니다)
	UPROPERTY(EditAnywhere, Category = "DG|Loot")
	TArray<FDGLootDropInfo> DropTable;
};