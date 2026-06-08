#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "DGLoadingScreenWidget.generated.h"

class UTextBlock;

UCLASS(Abstract)
class PROJECTDG_API UDGLoadingScreenWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "LoadingScreen")
	void SetTipText(const FText& InTipText);

protected:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> TipTextBlock;
};
