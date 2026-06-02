// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Animation/AnimNotifies/AnimNotifyState.h"
#include "ANS_FieldEnemyAttackHitWindow.generated.h"

class USkeletalMeshComponent;

struct FFieldEnemyAttackHitWindowRuntimeData
{
	TMap<FName, FVector> PreviousSocketLocations;
	TSet<TWeakObjectPtr<AActor>> HitActors;
};

/**
 * Field Enemy 전용 공격 히트 윈도우 노티파이.
 * 필드 몬스터는 무기 메시가 별도 존재하지 않으므로 캐릭터 스켈레탈 메시의 소켓을 직접 트레이스한다.
 */
UCLASS()
class PROJECTDG_API UANS_FieldEnemyAttackHitWindow : public UAnimNotifyState
{
	GENERATED_BODY()

public:
	UANS_FieldEnemyAttackHitWindow();

	virtual FString GetNotifyName_Implementation() const override;

	virtual void NotifyBegin(
		USkeletalMeshComponent* MeshComp,
		UAnimSequenceBase* Animation,
		float TotalDuration,
		const FAnimNotifyEventReference& EventReference
	) override;

	virtual void NotifyTick(
		USkeletalMeshComponent* MeshComp,
		UAnimSequenceBase* Animation,
		float FrameDeltaTime,
		const FAnimNotifyEventReference& EventReference
	) override;

	virtual void NotifyEnd(
		USkeletalMeshComponent* MeshComp,
		UAnimSequenceBase* Animation,
		const FAnimNotifyEventReference& EventReference
	) override;

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Attack")
	TArray<FName> TraceSocketNames;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Attack")
	float TraceRadius = 20.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Attack")
	TEnumAsByte<ECollisionChannel> TraceChannel = ECC_Pawn;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Attack")
	float AttackDamage = 30.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Attack|Debug")
	bool bEnableDebugDraw = true;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Attack|Debug")
	bool bLogWindowLifecycle = true;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Attack|Debug")
	bool bLogAcceptedHits = true;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Attack|Debug")
	float DebugDrawDuration = 5.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Attack|Filter")
	bool bIgnoreSameTeam = true;

private:
	TMap<TWeakObjectPtr<USkeletalMeshComponent>, FFieldEnemyAttackHitWindowRuntimeData> RuntimeDataMap;

	void InitializeRuntimeData(USkeletalMeshComponent* MeshComp);
	void TraceBodySockets(USkeletalMeshComponent* MeshComp);
	void SendHitEvent(AActor* OwnerActor, AActor* HitActor) const;

	bool ShouldIgnoreHitActor(AActor* OwnerActor, AActor* HitActor) const;
	bool AreActorsOnSameTeam(AActor* FirstActor, AActor* SecondActor) const;
	void DrawTraceDebug(UWorld* World, const FVector& Start, const FVector& End, const FColor& Color) const;
};
