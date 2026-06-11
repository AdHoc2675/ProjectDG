#pragma once

#include "CoreMinimal.h"
#include "WeaponTrailTypes.generated.h"

class UNiagaraSystem;

USTRUCT(BlueprintType)
struct PROJECTDG_API FWeaponTrailAttachData
{
	GENERATED_BODY()

	/** 부착할 SkeletalMeshComponent의 Component Tag */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon Trail")
	FName MeshComponentTag = NAME_None;

	/** Niagara를 부착할 무기 소켓 */
	// UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon Trail")
	// FName SocketName = NAME_None;
	
	/** 손잡이 쪽 시작 소켓 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon Trail")
	FName StartSocketName = NAME_None;

	/** 칼끝 쪽 종료 소켓 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon Trail")
	FName EndSocketName = NAME_None;
	
	/** 두 소켓 사이 길이를 전달할 Niagara User Parameter */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon Trail")
	FName TrailSizeParameterName = TEXT("User.TrailSize");

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon Trail")
	TObjectPtr<UNiagaraSystem> TrailVFX = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon Trail")
	FVector LocationOffset = FVector::ZeroVector;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon Trail")
	FRotator RotationOffset = FRotator::ZeroRotator;
};
