#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "DGLoadingTipRow.generated.h"

USTRUCT(BlueprintType)
struct PROJECTDG_API FDGLoadingTipRow : public FTableRowBase
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LoadingTip")
	FText TipText;
};
