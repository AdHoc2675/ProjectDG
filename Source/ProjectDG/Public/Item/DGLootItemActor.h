// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "DG_ItemTypes.h"
#include "DGLootItemActor.generated.h"

class USphereComponent;
class UNiagaraComponent;
class UNiagaraSystem;
class UDGItemDefinition;

UCLASS()
class PROJECTDG_API ADGLootItemActor : public AActor
{
	GENERATED_BODY()
public:	
	// Sets default values for this actor's properties
	ADGLootItemActor();

	// 생성 시 컴포넌트에서 호출하여 아이템 데이터를 세팅
	void InitializeLoot(UDGItemDefinition* InItemDef, int32 InQuantity, EDGItemGrade InGrade = EDGItemGrade::Hero);
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Loot")
	USphereComponent* PickupSphere;

	// 등급별 시각 효과를 담당할 나이아가라 컴포넌트
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Loot")
	UNiagaraComponent* LootVFXComponent;

#pragma region VFX Assets
	// 에디터에서 할당할 등급별 나이아가라 에셋들
	UPROPERTY(EditDefaultsOnly, Category = "Loot|VFX")
	TObjectPtr<UNiagaraSystem> NormalFX;

	UPROPERTY(EditDefaultsOnly, Category = "Loot|VFX")
	TObjectPtr<UNiagaraSystem> HeroFX;

	UPROPERTY(EditDefaultsOnly, Category = "Loot|VFX")
	TObjectPtr<UNiagaraSystem> LegendaryFX;

	UPROPERTY(EditDefaultsOnly, Category = "Loot|VFX")
	TObjectPtr<UNiagaraSystem> AncientFX;
#pragma endregion

	// 네트워크 리플리케이트 시 클라이언트에서도 이펙트를 켜주기 위한 RepNotify
	UPROPERTY(ReplicatedUsing = OnRep_Grade)
	EDGItemGrade ReplicatedGrade;

	UFUNCTION()
	void OnRep_Grade();

	// 습득 처리 캐치
	UFUNCTION()
	void OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

private:
	// 습득 시 인벤토리로 넘길 아이템 메타데이터
	UPROPERTY(Transient)
	TObjectPtr<UDGItemDefinition> ItemDef;

	int32 Quantity;
};
