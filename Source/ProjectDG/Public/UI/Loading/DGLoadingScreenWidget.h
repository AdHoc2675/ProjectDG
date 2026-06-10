#pragma once

#include "Blueprint/UserWidget.h"
#include "CoreMinimal.h"
#include "DGLoadingScreenWidget.generated.h"

class UTextBlock;
class UImage;

UCLASS(Abstract)
class PROJECTDG_API UDGLoadingScreenWidget : public UUserWidget {
  GENERATED_BODY()

public:
  UFUNCTION(BlueprintCallable, Category = "LoadingScreen")
  void SetTipText(const FText &InTipText);

protected:
  virtual void NativeConstruct() override;
  virtual void NativeTick(const FGeometry &MyGeometry,
                          float InDeltaTime) override;

  UPROPERTY(meta = (BindWidget))
  TObjectPtr<UTextBlock> TipTextBlock;

  // 페이드 애니메이션 대상 이미지 (위젯 블루프린트에서 "Image_0" 이름으로
  // 바인딩)
  UPROPERTY(meta = (BindWidget))
  TObjectPtr<UImage> Image_0;

  // 페이드 속도 (초 단위 주기의 절반, 기본 1.5초 = 3초 주기)
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LoadingScreen|Fade")
  float FadeHalfCycleDuration = 2.5f;

  // 최소 투명도
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LoadingScreen|Fade")
  float MinOpacity = 0.3f;

  // 최대 투명도
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LoadingScreen|Fade")
  float MaxOpacity = 0.8f;

private:
  float FadeElapsedTime = 0.f;
};
