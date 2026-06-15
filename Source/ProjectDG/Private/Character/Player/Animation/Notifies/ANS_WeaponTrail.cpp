// Fill out your copyright notice in the Description page of Project Settings.

#include "Character/Player/Animation/Notifies/ANS_WeaponTrail.h"

#include "Components/SkeletalMeshComponent.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"

UANS_WeaponTrail::UANS_WeaponTrail()
{
#if WITH_EDITORONLY_DATA
      NotifyColor = FColor(120, 220, 255);
#endif
}

FString UANS_WeaponTrail::GetNotifyName_Implementation() const
{
      return TEXT("WeaponTrail");
}

void UANS_WeaponTrail::NotifyBegin(
      USkeletalMeshComponent* MeshComp,
      UAnimSequenceBase* Animation,
      float TotalDuration,
      const FAnimNotifyEventReference& EventReference
)
{
      Super::NotifyBegin(
              MeshComp,
              Animation,
              TotalDuration,
              EventReference
      );

      if (!MeshComp || !MeshComp->GetOwner())
      {
              return;
      }

      UWorld* World = MeshComp->GetWorld();
      if (!World || World->GetNetMode() == NM_DedicatedServer)
      {
              return;
      }

      CleanupInvalidRuntimeEntries();

      FWeaponTrailNotifyRuntimeInstance RuntimeInstance;

      for (const FWeaponTrailAttachData& TrailData : WeaponTrails)
      {
              USkeletalMeshComponent* TargetMesh =
                      ResolveMeshComponent(
                              MeshComp,
                              TrailData.MeshComponentTag
                      );

              if (!TargetMesh)
              {
                      continue;
              }

              UNiagaraComponent* TrailComponent =
                      SpawnTrailComponent(TargetMesh, TrailData);

              if (TrailComponent)
              {
                      RuntimeInstance.TrailComponents.Add(
                              TrailComponent
                      );
              }
      }

      RuntimeInstancesByMesh
              .FindOrAdd(MeshComp)
              .Add(MoveTemp(RuntimeInstance));
}

void UANS_WeaponTrail::NotifyEnd(
      USkeletalMeshComponent* MeshComp,
      UAnimSequenceBase* Animation,
      const FAnimNotifyEventReference& EventReference
)
{
      Super::NotifyEnd(MeshComp, Animation, EventReference);

      if (!MeshComp)
      {
              return;
      }

      TArray<FWeaponTrailNotifyRuntimeInstance>* RuntimeInstances =
              RuntimeInstancesByMesh.Find(MeshComp);

      if (!RuntimeInstances || RuntimeInstances->IsEmpty())
      {
              return;
      }

      // 가장 먼저 시작된 실행 인스턴스를 먼저 종료한다.
      FWeaponTrailNotifyRuntimeInstance& RuntimeInstance =
              (*RuntimeInstances)[0];

      StopTrailInstance(RuntimeInstance);
      RuntimeInstances->RemoveAt(0);

      if (RuntimeInstances->IsEmpty())
      {
              RuntimeInstancesByMesh.Remove(MeshComp);
      }
}

USkeletalMeshComponent*
UANS_WeaponTrail::ResolveMeshComponent(
      USkeletalMeshComponent* CharacterMesh,
      FName ComponentTag
) const
{
      if (!CharacterMesh)
      {
              return nullptr;
      }

      if (ComponentTag.IsNone())
      {
              return CharacterMesh;
      }

      AActor* OwnerActor = CharacterMesh->GetOwner();
      if (!OwnerActor)
      {
              return nullptr;
      }

      TArray<USkeletalMeshComponent*> MeshComponents;
      OwnerActor->GetComponents(MeshComponents);

      for (USkeletalMeshComponent* MeshComponent : MeshComponents)
      {
              if (MeshComponent &&
                      MeshComponent->ComponentHasTag(ComponentTag))
              {
                      return MeshComponent;
              }
      }

      return nullptr;
}

UNiagaraComponent* UANS_WeaponTrail::SpawnTrailComponent(
      USkeletalMeshComponent* TargetMesh,
      const FWeaponTrailAttachData& TrailData
) const
{
      if (!TargetMesh ||
              !TrailData.TrailVFX ||
              TrailData.StartSocketName.IsNone() ||
              TrailData.EndSocketName.IsNone())
      {
              return nullptr;
      }

      if (!TargetMesh->DoesSocketExist(
                      TrailData.StartSocketName
              ) ||
              !TargetMesh->DoesSocketExist(
                      TrailData.EndSocketName
              ))
      {
              return nullptr;
      }

      const FVector StartLocation =
              TargetMesh->GetSocketLocation(
                      TrailData.StartSocketName
              );

      const FVector EndLocation =
              TargetMesh->GetSocketLocation(
                      TrailData.EndSocketName
              );

      const FVector TrailDirection =
              StartLocation - EndLocation;

      const float TrailSize = TrailDirection.Length();
      if (TrailSize <= KINDA_SMALL_NUMBER)
      {
              return nullptr;
      }

      FRotator TrailRotation =
              FRotationMatrix::MakeFromZ(
                      TrailDirection
              ).Rotator();

      TrailRotation += TrailData.RotationOffset;

      const FVector TrailCenter =
              (StartLocation + EndLocation) * 0.5f +
              TrailRotation.RotateVector(
                      TrailData.LocationOffset
              );

      UNiagaraComponent* TrailComponent =
              UNiagaraFunctionLibrary::SpawnSystemAttached(
                      TrailData.TrailVFX,
                      TargetMesh,
                      NAME_None,
                      TrailCenter,
                      TrailRotation,
                      EAttachLocation::KeepWorldPosition,
                      true,
                      false,
                      ENCPoolMethod::None,
                      true
              );

      if (!TrailComponent)
      {
              return nullptr;
      }

      if (!TrailData.TrailSizeParameterName.IsNone())
      {
              TrailComponent->SetVariableFloat(
                      TrailData.TrailSizeParameterName,
                      TrailSize
              );
      }

      TrailComponent->Activate(true);
      return TrailComponent;
}

void UANS_WeaponTrail::StopTrailInstance(
      FWeaponTrailNotifyRuntimeInstance& RuntimeInstance
) const
{
      for (const TWeakObjectPtr<UNiagaraComponent>& WeakComponent :
              RuntimeInstance.TrailComponents)
      {
              if (WeakComponent.IsValid())
              {
                      WeakComponent->Deactivate();
              }
      }

      RuntimeInstance.TrailComponents.Reset();
}

void UANS_WeaponTrail::CleanupInvalidRuntimeEntries()
{
      for (auto Iterator =
                      RuntimeInstancesByMesh.CreateIterator();
              Iterator;
              ++Iterator)
      {
              if (!Iterator.Key().IsValid())
              {
                      Iterator.RemoveCurrent();
              }
      }
}




