// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Widget/Damage/DGDamageNumberActor.h"
#include "Components/WidgetComponent.h"
#include "UI/Widget/Damage/DGDamageNumberWidget.h"
#include "System/DGDamageNumberPoolSubsystem.h"

// Sets default values
ADGDamageNumberActor::ADGDamageNumberActor()
{
	PrimaryActorTick.bCanEverTick = false;

	// 네트워크 복제 끄기 (로컬 클라이언트 시각용)
	bReplicates = false;

	DamageWidgetComponent = CreateDefaultSubobject<UWidgetComponent>(TEXT("DamageWidgetComponent"));
	RootComponent = DamageWidgetComponent;

	// UI가 항상 정면을 바라보게 하려면 Screen 모드 사용
	DamageWidgetComponent->SetWidgetSpace(EWidgetSpace::Screen);
	DamageWidgetComponent->SetDrawSize(FVector2D(200.f, 100.f));

	// 물리, 충돌 끄기 최적화
	DamageWidgetComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);

}

void ADGDamageNumberActor::ShowDamage(float DamageAmount, bool bIsCritical)
{
	SetActorHiddenInGame(false);

	//UE_LOG(LogTemp, Warning, TEXT("[DamageActor] ShowDamage called."));

	if (DamageWidgetComponent)
	{
		UUserWidget* RawWidget = DamageWidgetComponent->GetWidget();
		if (!RawWidget)
		{
			UE_LOG(LogTemp, Error, TEXT("[DamageActor] RawWidget is NULL! (액터 BP의 WidgetComponent에서 Widget Class를 할당했는지 확인하세요)"));
			return;
		}

		if (UDGDamageNumberWidget* DamageWidget = Cast<UDGDamageNumberWidget>(RawWidget))
		{
			//UE_LOG(LogTemp, Warning, TEXT("[DamageActor] Casting Success! Calling PlayDamageAnimation."));
			DamageWidget->PlayDamageAnimation(DamageAmount, bIsCritical, this);
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("[DamageActor] Cast Failed! (위젯이 UDGDamageNumberWidget을 상속받았는지 확인하세요)"));
		}
	}
}

void ADGDamageNumberActor::ReturnToPool()
{
	// 화면에서 숨김 처리
	SetActorHiddenInGame(true);

	if (UWorld* World = GetWorld())
	{
		if (UDGDamageNumberPoolSubsystem* PoolSubsystem = World->GetSubsystem<UDGDamageNumberPoolSubsystem>())
		{
			PoolSubsystem->ReturnDamageNumber(this);
		}
	}
}
