// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotifyState.h"
#include "Data/VFX/WeaponTrailTypes.h"
#include "ANS_WeaponTrail.generated.h"

class UNiagaraComponent;
class USkeletalMeshComponent;

struct FWeaponTrailNotifyRuntimeInstance
{
      TArray<TWeakObjectPtr<UNiagaraComponent>> TrailComponents;
};

/**
 * 몽타주 구간 동안 무기 소켓 사이에 Niagara Trail을 출력한다.
 *
 * 몽타주가 재생되는 각 클라이언트에서 로컬로 실행되며,
 * 전용 서버에서는 Niagara를 생성하지 않는다.
 */
UCLASS()
class PROJECTDG_API UANS_WeaponTrail : public UAnimNotifyState
{
      GENERATED_BODY()

public:
      UANS_WeaponTrail();

      virtual FString GetNotifyName_Implementation() const override;

      virtual void NotifyBegin(
              USkeletalMeshComponent* MeshComp,
              UAnimSequenceBase* Animation,
              float TotalDuration,
              const FAnimNotifyEventReference& EventReference
      ) override;

      virtual void NotifyEnd(
              USkeletalMeshComponent* MeshComp,
              UAnimSequenceBase* Animation,
              const FAnimNotifyEventReference& EventReference
      ) override;

protected:
      /**
       * 하나의 ANS에서 여러 무기를 지원한다.
       * 예: Weapon.Left, Weapon.Right
       */
      UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon Trail")
      TArray<FWeaponTrailAttachData> WeaponTrails;

private:
      /**
       * AnimNotifyState 객체는 캐릭터 간 공유되므로 단일 컴포넌트 포인터를
       * 보관하지 않고 MeshComp별 실행 인스턴스를 관리한다.
       *
       * 동일 몽타주가 빠르게 재실행될 경우 이전 Begin/End 순서를 보존하기
       * 위해 MeshComp별 배열을 FIFO 큐로 사용한다.
       */
      TMap<
              TWeakObjectPtr<USkeletalMeshComponent>,
              TArray<FWeaponTrailNotifyRuntimeInstance>
      > RuntimeInstancesByMesh;

      USkeletalMeshComponent* ResolveMeshComponent(
              USkeletalMeshComponent* CharacterMesh,
              FName ComponentTag
      ) const;

      UNiagaraComponent* SpawnTrailComponent(
              USkeletalMeshComponent* TargetMesh,
              const FWeaponTrailAttachData& TrailData
      ) const;

      void StopTrailInstance(
              FWeaponTrailNotifyRuntimeInstance& RuntimeInstance
      ) const;

      void CleanupInvalidRuntimeEntries();
};