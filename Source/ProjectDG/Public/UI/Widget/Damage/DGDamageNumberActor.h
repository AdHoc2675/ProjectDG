// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "DGDamageNumberActor.generated.h"

class UWidgetComponent;

UCLASS()
class PROJECTDG_API ADGDamageNumberActor : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ADGDamageNumberActor();

	/** 풀에서 꺼내져 실제로 데미지를 표시할 때 호출 */
	UFUNCTION(BlueprintCallable, Category = "Damage")
	void ShowDamage(float DamageAmount, bool bIsCritical);

	/** 데미지 애니메이션이 끝난 뒤 풀로 되돌아가기 위해 호출 (위젯 BP에서 애니메이션 종료 시 호출하세요) */
	UFUNCTION(BlueprintCallable, Category = "Damage")
	void ReturnToPool();

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UWidgetComponent> DamageWidgetComponent;
};
