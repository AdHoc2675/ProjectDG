#include "GAS/GameplayCues/GCN_WeaponTrail.h"

#include "Character/Enemy/Data/EnemySkillData.h"
#include "Character/Player/Data/PlayerSkillData.h"
#include "Components/SkeletalMeshComponent.h"
#include "Data/VFX/WeaponTrailTypes.h"
#include "GameFramework/Character.h"
#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"

bool AGCN_WeaponTrail::OnActive_Implementation(
      AActor* MyTarget,
      const FGameplayCueParameters& Parameters
)
{
      if (!MyTarget)
      {
              return false;
      }

      const TArray<FWeaponTrailAttachData>* TrailData =
              ResolveTrailData(Parameters);

      if (!TrailData)
      {
          UE_LOG(LogTemp, Error, TEXT("[WeaponTrail][GCN] TrailData resolve failed"));
          return false;
      }

      StopActiveTrails();

      for (const FWeaponTrailAttachData& Trail : *TrailData)
      {
              if (!Trail.TrailVFX ||
                      Trail.StartSocketName.IsNone() ||
                      Trail.EndSocketName.IsNone())
              {
                      continue;
              }

              USkeletalMeshComponent* MeshComponent =
                      ResolveMeshComponent(MyTarget, Trail.MeshComponentTag);

              if (!MeshComponent ||
                      !MeshComponent->DoesSocketExist(Trail.StartSocketName) ||
                      !MeshComponent->DoesSocketExist(Trail.EndSocketName))
              {
                      continue;
              }

              const FVector StartLocation =
                      MeshComponent->GetSocketLocation(Trail.StartSocketName);

              const FVector EndLocation =
                      MeshComponent->GetSocketLocation(Trail.EndSocketName);

              const FVector TrailDirection =
                      StartLocation - EndLocation;

              const float TrailSize = TrailDirection.Length();

              if (TrailSize <= KINDA_SMALL_NUMBER)
              {
                      continue;
              }

              const FVector TrailCenter =
                      (StartLocation + EndLocation) * 0.5f;

              const FRotator TrailRotation =
                      FRotationMatrix::MakeFromZ(TrailDirection).Rotator();

              UNiagaraComponent* TrailComponent =
                      UNiagaraFunctionLibrary::SpawnSystemAttached(
                              Trail.TrailVFX,
                              MeshComponent,
                              NAME_None,
                              TrailCenter,
                              TrailRotation,
                              EAttachLocation::KeepWorldPosition,
                              false,
                              false,
                              ENCPoolMethod::None,
                              true
                      );

              if (!TrailComponent)
              {
                      continue;
              }

              if (!Trail.TrailSizeParameterName.IsNone())
              {
                      TrailComponent->SetVariableFloat(
                              Trail.TrailSizeParameterName,
                              TrailSize
                      );
              }

              TrailComponent->Activate(true);
              ActiveTrailComponents.Add(TrailComponent);
      }

      return !ActiveTrailComponents.IsEmpty();
}

bool AGCN_WeaponTrail::OnRemove_Implementation(
      AActor* MyTarget,
      const FGameplayCueParameters& Parameters
)
{
      StopActiveTrails();
      return true;
}

const TArray<FWeaponTrailAttachData>* AGCN_WeaponTrail::ResolveTrailData(
      const FGameplayCueParameters& Parameters
) const
{
        
      const UObject* SourceObject = Parameters.SourceObject.Get();

      if (const UPlayerSkillData* PlayerData =
              Cast<UPlayerSkillData>(SourceObject))
      {
              return &PlayerData->WeaponTrails;
      }

      // EnemySkillData에 WeaponTrails를 추가한 후 활성화합니다.
      /*
      if (const UEnemySkillData* EnemyData =
              Cast<UEnemySkillData>(SourceObject))
      {
              return &EnemyData->WeaponTrails;
      }
      */

      return nullptr;
}

USkeletalMeshComponent* AGCN_WeaponTrail::ResolveMeshComponent(
      AActor* Target,
      FName ComponentTag
) const
{
      if (!Target)
      {
              return nullptr;
      }

      if (ComponentTag.IsNone())
      {
              if (const ACharacter* Character = Cast<ACharacter>(Target))
              {
                      return Character->GetMesh();
              }

              return nullptr;
      }

      TArray<USkeletalMeshComponent*> MeshComponents;
      Target->GetComponents(MeshComponents);

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

void AGCN_WeaponTrail::StopActiveTrails()
{
      for (const TWeakObjectPtr<UNiagaraComponent>& TrailComponent :
              ActiveTrailComponents)
      {
              if (TrailComponent.IsValid())
              {
                      TrailComponent->Deactivate();
              }
      }

      ActiveTrailComponents.Reset();
}