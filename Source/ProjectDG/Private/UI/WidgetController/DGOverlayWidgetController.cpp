#include "UI/WidgetController/DGOverlayWidgetController.h"
#include "GAS/Attributes/DG_AttributeSet.h"
#include "AbilitySystemComponent.h"

#include "UI/Widget/Minimap/DGMinimapSubsystem.h"
#include "Components/UI/DGMinimapMarkerComponent.h"

#include "GameFramework/DG_GameState.h"
#include "GameFramework/DG_PlayerState.h"
#include "GAS/Attributes/DG_EnemyAttributeSet.h"


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
		OnMentalChanged.Broadcast(DGAS->GetMental());
		OnMaxMentalChanged.Broadcast(DGAS->GetMaxMental());
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

	// 파티원 UI가 초기화될 때 이미 방에 접속해 있는 기존 파티원(PlayerState)들을 긁어와서 목록에 추가
	if (UWorld* World = GetWorld())
	{
		if (ADG_GameState* GameState = World->GetGameState<ADG_GameState>())
		{
			// GameState가 관리하는 모든 PlayerState 배열 순회
			for (APlayerState* PS : GameState->PlayerArray)
			{
				if (ADG_PlayerState* DGPS = Cast<ADG_PlayerState>(PS))
				{
					// 본인 체크 등 로직이 이미 HandlePartyMemberJoined에 잘 구현되어 있으므로 이를 재활용하여 호출
					HandlePartyMemberJoined(DGPS);
				}
			}
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

		AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(DGAS->GetMentalAttribute()).AddLambda(
			[this](const FOnAttributeChangeData& Data)
			{
				OnMentalChanged.Broadcast(Data.NewValue);
			}
		);

		AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(DGAS->GetMaxMentalAttribute()).AddLambda(
			[this](const FOnAttributeChangeData& Data)
			{
				OnMaxMentalChanged.Broadcast(Data.NewValue);
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

	// 파티(GameState) 합류/탈퇴 델리게이트 바인딩
	if (UWorld* World = GetWorld())
	{
		if (ADG_GameState* GameState = World->GetGameState<ADG_GameState>())
		{
			GameState->OnPlayerJoinedDelegate.AddDynamic(this, &UDGOverlayWidgetController::HandlePartyMemberJoined);
			GameState->OnPlayerLeftDelegate.AddDynamic(this, &UDGOverlayWidgetController::HandlePartyMemberLeft);
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
	ClearCurrentEnemyTarget();

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

	// 대상 ASC에서 그로기를 담당하는 UDG_EnemyAttributeSet 직접 찾기 (주로 2번째 배열 등에 존재)
	UDG_EnemyAttributeSet* GroggyAS = nullptr;
	for (UAttributeSet* AS : CurrentEnemyASC->GetSpawnedAttributes())
	{
		if (UDG_EnemyAttributeSet* EnemyTypeAS = Cast<UDG_EnemyAttributeSet>(AS))
		{
			GroggyAS = EnemyTypeAS;
			UE_LOG(LogTemp, Log, TEXT("[DGOverlayWidgetController] Found Groggy AttributeSet for enemy: %s"), *EnemyName);
			break;
		}
	}

	// 찾은 그로기 AttributeSet으로 이벤트 연결
	if (GroggyAS)
	{
		EnemyGroggyChangedDelegateHandle = CurrentEnemyASC->GetGameplayAttributeValueChangeDelegate(GroggyAS->GetGroggyGaugeAttribute()).AddLambda(
			[this, GroggyAS](const FOnAttributeChangeData& Data)
			{
				OnEnemyGroggyChanged.Broadcast(Data.NewValue, GroggyAS->GetMaxGroggyGauge());
			}
		);
		EnemyMaxGroggyChangedDelegateHandle = CurrentEnemyASC->GetGameplayAttributeValueChangeDelegate(GroggyAS->GetMaxGroggyGaugeAttribute()).AddLambda(
			[this, GroggyAS](const FOnAttributeChangeData& Data)
			{
				OnEnemyGroggyChanged.Broadcast(GroggyAS->GetGroggyGauge(), Data.NewValue);
			}
		);
	}

	// 임시로 MaxHealth 1000당 1줄, 최소 1줄로 계산하도록 처리 (필요 시 수정)
	float CurrentMaxHealth = CurrentEnemyAS->GetMaxHealth();
	int32 CalculatedMaxBars = 1;

	OnEnemyTargetSet.Broadcast(EnemyName, CalculatedMaxBars);

	// MaxHealth에 따라 UI에서 보여줄 체력바 줄 수 계산 (예시 로직, 필요에 따라 조정)
	if (CurrentMaxHealth > 0.0f)
	{
		CalculatedMaxBars = FMath::Max(1, FMath::FloorToInt(CurrentMaxHealth / 1000.0f));
	}

	// 타겟 설정 방송 (이름과 계산된 최대 줄 수)
	OnEnemyTargetSet.Broadcast(EnemyName, CalculatedMaxBars);

	// 즉시 초기값 방송하여 UI를 띄움
	OnEnemyHealthChanged.Broadcast(CurrentEnemyAS->GetHealth(), CurrentEnemyAS->GetMaxHealth());

	if (GroggyAS)
	{
		OnEnemyGroggyChanged.Broadcast(GroggyAS->GetGroggyGauge(), GroggyAS->GetMaxGroggyGauge());
	}
}

void UDGOverlayWidgetController::NotifyBossEncountered(AActor* BossActor)
{
	CurrentBossEnemy = BossActor;
	RefreshEnemyTargetPriority();
}

void UDGOverlayWidgetController::NotifyTargetChanged(AActor* TargetActor)
{
	CurrentLockedTarget = TargetActor;
	RefreshEnemyTargetPriority();
}

void UDGOverlayWidgetController::NotifyEnemyDamaged(AActor* DamagedEnemy)
{
	LastDamagedEnemy = DamagedEnemy;
	if (UWorld* World = GetWorld())
	{
		LastDamageTime = World->GetTimeSeconds();
	}

	RefreshEnemyTargetPriority();
}

void UDGOverlayWidgetController::ClearCurrentEnemyTarget()
{
	// 델리게이트 해제
	if (CurrentEnemyASC && CurrentEnemyAS)
	{
		CurrentEnemyASC->GetGameplayAttributeValueChangeDelegate(CurrentEnemyAS->GetHealthAttribute()).Remove(EnemyHealthChangedDelegateHandle);
		CurrentEnemyASC->GetGameplayAttributeValueChangeDelegate(CurrentEnemyAS->GetMaxHealthAttribute()).Remove(EnemyMaxHealthChangedDelegateHandle);

		if (UDG_EnemyAttributeSet* GroggyAS = Cast<UDG_EnemyAttributeSet>(CurrentEnemyAS))
		{
			CurrentEnemyASC->GetGameplayAttributeValueChangeDelegate(GroggyAS->GetGroggyGaugeAttribute()).Remove(EnemyGroggyChangedDelegateHandle);
			CurrentEnemyASC->GetGameplayAttributeValueChangeDelegate(GroggyAS->GetMaxGroggyGaugeAttribute()).Remove(EnemyMaxGroggyChangedDelegateHandle);
		}
	}

	CurrentEnemyASC = nullptr;
	CurrentEnemyAS = nullptr;

	// 타겟 초기화 방송
	OnEnemyTargetCleared.Broadcast();
}


void UDGOverlayWidgetController::RefreshEnemyTargetPriority()
{
	AActor* TargetToShow = nullptr;

	// 1. 보스 우선 표시
	if (CurrentBossEnemy.IsValid())
	{
		TargetToShow = CurrentBossEnemy.Get();
	}
	// 2. 보스가 없다면 내가 락온한 적 표시
	else if (CurrentLockedTarget.IsValid())
	{
		TargetToShow = CurrentLockedTarget.Get();
	}
	// 3. 그것도 없다면 최근에 때린 적 (단, 맞은지 너무 오래됐다면 패스)
	else if (LastDamagedEnemy.IsValid())
	{
		UWorld* World = GetWorld();
		if (World && (World->GetTimeSeconds() - LastDamageTime < 10.0f)) // 10초 이내에 때렸던 타겟만
		{
			TargetToShow = LastDamagedEnemy.Get();
		}
	}

	if (TargetToShow)
	{
		// 대상의 ASC 및 스탯 찾기
		UAbilitySystemComponent* TargetASC = TargetToShow->GetComponentByClass<UAbilitySystemComponent>();
		if (TargetASC)
		{
			// 보통 배열 첫 번째가 주인공 스탯
			const UAttributeSet* TargetAS = TargetASC->GetSpawnedAttributes().Num() > 0 ? TargetASC->GetSpawnedAttributes()[0] : nullptr;

			// 대상의 이름을 알 수 있는 함수(GetName 등)를 가져옵니다. 필요에 따라 형변환 가능 (예: AEnemyCharacterBase)
			FString UnitName = TargetToShow->GetName();

			SetEnemyTarget(TargetASC, const_cast<UAttributeSet*>(TargetAS), UnitName);
		}
	}
	// 보여줄 대상이 없다면 초기화
	else
	{
		ClearCurrentEnemyTarget();
	}
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

void UDGOverlayWidgetController::HandlePartyMemberJoined(ADG_PlayerState* NewMemberPS)
{
	if (!NewMemberPS) return;

	// 내 PlayerState면 파티원 리스트에는 시각적으로 추가하지 않음
	if (NewMemberPS == PlayerState)
	{
		UE_LOG(LogTemp, Log, TEXT("[DGOverlayWidgetController] 본인(로컬 플레이어)이 월드에 참가했습니다: %s"), *NewMemberPS->GetPlayerName());
		return;
	}

	// 파티원 합류 로그 출력
	UE_LOG(LogTemp, Log, TEXT("[DGOverlayWidgetController] 새로운 파티원이 참가했습니다: %s"), *NewMemberPS->GetPlayerName());

	// View(DGPartyListWidget)에게 새로운 파티원이 왔다고 방송
	OnPartyMemberJoined.Broadcast(NewMemberPS);
}

void UDGOverlayWidgetController::HandlePartyMemberLeft(ADG_PlayerState* LeavingMemberPS)
{
	if (!LeavingMemberPS) return;

	if (LeavingMemberPS == PlayerState)
	{
		UE_LOG(LogTemp, Log, TEXT("[DGOverlayWidgetController] 본인(로컬 플레이어)이 월드에서 퇴장했습니다: %s"), *LeavingMemberPS->GetPlayerName());
		return;
	}

	// 파티원 퇴장 로그 출력
	UE_LOG(LogTemp, Log, TEXT("[DGOverlayWidgetController] 파티원이 탈퇴(퇴장)했습니다: %s"), *LeavingMemberPS->GetPlayerName());

	// View(DGPartyListWidget)에게 파티원이 나갔다고 방송
	OnPartyMemberLeft.Broadcast(LeavingMemberPS);
}
