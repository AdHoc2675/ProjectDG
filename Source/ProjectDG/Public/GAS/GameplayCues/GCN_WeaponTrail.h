#pragma once

#include "CoreMinimal.h"
#include "GameplayCueNotify_Actor.h"
#include "GCN_WeaponTrail.generated.h"

class UNiagaraComponent;
class USkeletalMeshComponent;
struct FWeaponTrailAttachData;

UCLASS()
class PROJECTDG_API AGCN_WeaponTrail : public AGameplayCueNotify_Actor
{
	GENERATED_BODY()

public:
	virtual bool OnActive_Implementation(
			AActor* MyTarget,
			const FGameplayCueParameters& Parameters
	) override;

	virtual bool OnRemove_Implementation(
			AActor* MyTarget,
			const FGameplayCueParameters& Parameters
	) override;

private:
	const TArray<FWeaponTrailAttachData>* ResolveTrailData(
			const FGameplayCueParameters& Parameters
	) const;

	USkeletalMeshComponent* ResolveMeshComponent(
			AActor* Target,
			FName ComponentTag
	) const;

	void StopActiveTrails();

	TArray<TWeakObjectPtr<UNiagaraComponent>> ActiveTrailComponents;
};