// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Animation/AnimNotifies/AnimNotifyState.h"
#include "ANS_AttackHitWindow.generated.h"

class USkeletalMeshComponent;

struct FAttackHitWindowRuntimeData
{
	TMap<FName, FVector> PreviousSocketLocations;
	TSet<TWeakObjectPtr<AActor>> HitActors;
};

/**
 * 
 */
UCLASS()
class PROJECTDG_API UANS_AttackHitWindow : public UAnimNotifyState
{
	GENERATED_BODY()
	
	
public:
	UANS_AttackHitWindow();

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
	float TraceRadius = 8.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Attack")
	TEnumAsByte<ECollisionChannel> TraceChannel = ECC_Pawn;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Attack")
	FGameplayTag HitEventTag;
	
	//주로 디버그를 위한 변수
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Attack|Debug")
	bool bEnableDebugDraw = true;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Attack|Debug")
	bool bLogWindowLifecycle = true;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Attack|Debug")
	bool bLogTraceSweeps = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Attack|Debug")
	bool bLogAcceptedHits = true;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Attack|Debug")
	float DebugDrawDuration = 5.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Attack|Filter")
	bool bIgnoreSameTeam = false;

private:
	TMap<TWeakObjectPtr<USkeletalMeshComponent>, FAttackHitWindowRuntimeData> RuntimeDataMap;

	USkeletalMeshComponent* ResolveWeaponMesh(USkeletalMeshComponent* CharacterMesh) const;
	void InitializeRuntimeData(USkeletalMeshComponent* CharacterMesh, USkeletalMeshComponent*WeaponMesh);
	void TraceWeaponSockets(USkeletalMeshComponent* CharacterMesh, USkeletalMeshComponent* WeaponMesh);
	void SendHitEvent(AActor* OwnerActor, AActor* HitActor) const;
	
	//팀 확인 및 디버깅
	bool ShouldIgnoreHitActor(AActor* OwnerActor, AActor* HitActor) const;
	bool AreActorsOnSameTeam(AActor* FirstActor, AActor* SecondActor) const;
	void DrawTraceDebug(UWorld* World, const FVector& Start, const FVector& End, const FColor& Color) const;
};
