#include "UI/WidgetController/DGOverlayWidgetController.h"
#include "GAS/Attributes/DG_AttributeSet.h"
#include "AbilitySystemComponent.h"

#include "UI/Widget/Minimap/DGMinimapSubsystem.h"
#include "Components/UI/DGMinimapMarkerComponent.h"

void UDGOverlayWidgetController::BroadcastInitialValues()
{
	UDG_AttributeSet* DGAS = GetDGAttributeSet();

	if (DGAS)
	{
		// 처음 UI가 생성될 때 현재 스탯을 한번 뿌려줌
		OnHealthChanged.Broadcast(DGAS->GetHealth());
		OnMaxHealthChanged.Broadcast(DGAS->GetMaxHealth());
		OnStaminaChanged.Broadcast(DGAS->GetStamina());
		OnMaxStaminaChanged.Broadcast(DGAS->GetMaxStamina());

		UE_LOG(LogTemp, Log, TEXT("[DGOverlayWidgetController] BroadcastInitialValues called. Health: %f, MaxHealth: %f, Stamina: %f, MaxStamina: %f"),
			DGAS->GetHealth(), DGAS->GetMaxHealth(), DGAS->GetStamina(), DGAS->GetMaxStamina());
	}

	// 미니맵 초기 마커 
	if (PlayerController)
	{
		if (UDGMinimapSubsystem* MinimapSubsystem = PlayerController->GetWorld()->GetSubsystem<UDGMinimapSubsystem>())
		{
			for (UDGMinimapMarkerComponent* Marker : MinimapSubsystem->GetActiveMarkers())
			{
				OnMarkerAdded.Broadcast(Marker);
			}

			UE_LOG(LogTemp, Log, TEXT("[DGOverlayWidgetController] BroadcastInitialValues called. Initial Minimap Markers Count: %d"), MinimapSubsystem->GetActiveMarkers().Num());
		}
	}

}

void UDGOverlayWidgetController::BindCallbacksToDependencies()
{
	UDG_AttributeSet* DGAS = GetDGAttributeSet();
	if (AbilitySystemComponent && DGAS)
	{
		// 체력 변경 시 내부 람다를 호출해 등록된 OnHealthChanged를 Broadcast함
		AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(DGAS->GetHealthAttribute()).AddLambda(
			[this](const FOnAttributeChangeData& Data)
			{
				OnHealthChanged.Broadcast(Data.NewValue);
			}
		);

		AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(DGAS->GetMaxHealthAttribute()).AddLambda(
			[this](const FOnAttributeChangeData& Data)
			{
				OnMaxHealthChanged.Broadcast(Data.NewValue);
			}
		);

		AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(DGAS->GetStaminaAttribute()).AddLambda(
			[this](const FOnAttributeChangeData& Data)
			{
				OnStaminaChanged.Broadcast(Data.NewValue);
			}
		);

		AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(DGAS->GetMaxStaminaAttribute()).AddLambda(
			[this](const FOnAttributeChangeData& Data)
			{
				OnMaxStaminaChanged.Broadcast(Data.NewValue);
			}
		);
	}


	// 미니맵 서브시스템 델리게이트 바인딩
	if (PlayerController)
	{
		if (UDGMinimapSubsystem* MinimapSubsystem = PlayerController->GetWorld()->GetSubsystem<UDGMinimapSubsystem>())
		{
			MinimapSubsystem->OnMarkerRegistered.AddDynamic(this, &UDGOverlayWidgetController::HandleMarkerRegistered);
			MinimapSubsystem->OnMarkerUnregistered.AddDynamic(this, &UDGOverlayWidgetController::HandleMarkerUnregistered);
		}
	}
}

UDG_AttributeSet* UDGOverlayWidgetController::GetDGAttributeSet()
{
	return Cast<UDG_AttributeSet>(AttributeSet);
}


void UDGOverlayWidgetController::SetEnemyTarget(UAbilitySystemComponent* InEnemyASC, UAttributeSet* InEnemyAS, const FString& EnemyName)
{
	if (!InEnemyASC || !InEnemyAS) return;

	UDG_AttributeSet* EnemyDGAS = Cast<UDG_AttributeSet>(InEnemyAS);
	if (!EnemyDGAS) return;

	// 기존에 타겟팅하던 적이 있다면 델리게이트 해제 (메모리 누수 및 오작동 방지)
	if (CurrentEnemyASC && CurrentEnemyAS)
	{
		CurrentEnemyASC->GetGameplayAttributeValueChangeDelegate(CurrentEnemyAS->GetHealthAttribute()).Remove(EnemyHealthChangedDelegateHandle);
		CurrentEnemyASC->GetGameplayAttributeValueChangeDelegate(CurrentEnemyAS->GetMaxHealthAttribute()).Remove(EnemyMaxHealthChangedDelegateHandle);
	}

	// 새 타겟 설정
	CurrentEnemyASC = InEnemyASC;
	CurrentEnemyAS = EnemyDGAS;

	// 새 타겟의 이벤트 바인딩
	EnemyHealthChangedDelegateHandle = CurrentEnemyASC->GetGameplayAttributeValueChangeDelegate(CurrentEnemyAS->GetHealthAttribute()).AddLambda(
		[this, EnemyDGAS](const FOnAttributeChangeData& Data)
		{
			OnEnemyHealthChanged.Broadcast(Data.NewValue, EnemyDGAS->GetMaxHealth());
		}
	);

	EnemyMaxHealthChangedDelegateHandle = CurrentEnemyASC->GetGameplayAttributeValueChangeDelegate(CurrentEnemyAS->GetMaxHealthAttribute()).AddLambda(
		[this, EnemyDGAS](const FOnAttributeChangeData& Data)
		{
			OnEnemyHealthChanged.Broadcast(EnemyDGAS->GetHealth(), Data.NewValue);
		}
	);

	// 즉시 초기값 방송하여 UI를 띄움
	OnEnemyHealthChanged.Broadcast(CurrentEnemyAS->GetHealth(), CurrentEnemyAS->GetMaxHealth());
}

// 마커가 새로 태어났을 때  -> UI로 토스
void UDGOverlayWidgetController::HandleMarkerRegistered(UDGMinimapMarkerComponent* Marker)
{
	OnMarkerAdded.Broadcast(Marker);
}

// 마커가 죽거나 파괴됐을 때 -> UI로 토스
void UDGOverlayWidgetController::HandleMarkerUnregistered(UDGMinimapMarkerComponent* Marker)
{
	OnMarkerRemoved.Broadcast(Marker);
}