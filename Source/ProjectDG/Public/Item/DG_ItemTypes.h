#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "DG_ItemTypes.generated.h"

/**
 * 아이템 시스템 핵심 Enum
 */

UENUM(BlueprintType)
enum class EDGItemType : uint8
{
	Equipment	UMETA(DisplayName = "장비"),
	Consumable	UMETA(DisplayName = "소모품"),
	Material	UMETA(DisplayName = "재료")
};

UENUM(BlueprintType)
enum class EDGEquipmentType : uint8
{
	Weapon		UMETA(DisplayName = "무기"),
	Armor		UMETA(DisplayName = "방어구")
};

UENUM(BlueprintType)
enum class EDGItemGrade : uint8
{
	Normal		UMETA(DisplayName = "일반"),
	Hero		UMETA(DisplayName = "영웅"),
	Legendary	UMETA(DisplayName = "전설"),
	Ancient		UMETA(DisplayName = "고대")
};

UENUM(BlueprintType)
enum class EDGMaterialType : uint8
{
	Craft		UMETA(DisplayName = "제작"),
	Enhance		UMETA(DisplayName = "강화"),
	Reroll		UMETA(DisplayName = "재설정")
};


/**
 * 1. DT_ItemGrade : 장비 등급, 색상, 보조옵션 개수.
 */
USTRUCT(BlueprintType)
struct FDGItemGradeTableRow : public FTableRowBase
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString DisplayName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString ColorHex;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 SubOptionCount = 0;
};

/**
 * 2. DT_EquipmentMainStatByLevel : 아이템 레벨별 주스탯
 */
USTRUCT(BlueprintType)
struct FDGEquipmentMainStatTableRow : public FTableRowBase
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 ItemLevel = 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MainStat = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float HP = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float WeaponAttack = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float ArmorDefense = 0.f;
};

/**
 * 3. DT_EquipmentRecipe : 장비 제작 레시피
 */
USTRUCT(BlueprintType)
struct FDGEquipmentRecipeTableRow : public FTableRowBase
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EDGEquipmentType EquipmentType = EDGEquipmentType::Weapon;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 RequiredLevelMin = 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 RequiredLevelMax = 10;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName MaterialID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 MaterialCount = 4;
};

/**
 * 4. DT_ItemMaterial : 제작 / 강화 / 재설정 재료 정보
 */
USTRUCT(BlueprintType)
struct FDGItemMaterialTableRow : public FTableRowBase
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString DisplayName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EDGMaterialType Type = EDGMaterialType::Craft;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString LevelRange; // ex) "1~10"

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString Description;
};

/**
 * 보조옵션 런타임 인스턴스 전용 구조체
 * (인게임에서 실제 무기에 붙는 각각의 보조옵션 데이터)
 */
USTRUCT(BlueprintType)
struct FDGSubOptionInstanceData
{
	GENERATED_BODY()

public:
	// DT_SubOptionDefinition의 RowName과 매칭
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName SubOptionID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float BaseValue = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 EnhanceCount = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float EnhanceTotalValue = 0.f;
};