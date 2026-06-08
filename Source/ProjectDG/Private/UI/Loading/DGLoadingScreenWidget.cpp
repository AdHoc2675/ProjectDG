#include "UI/Loading/DGLoadingScreenWidget.h"
#include "Components/TextBlock.h"

void UDGLoadingScreenWidget::SetTipText(const FText& InTipText)
{
	if (TipTextBlock)
	{
		TipTextBlock->SetText(InTipText);
	}
}
