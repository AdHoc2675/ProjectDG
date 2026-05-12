// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/Player/Animation/Notifies/ANS_AttackHitWindow.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "Character/Player/Warrior/WarriorCharacter.h"
#include "Components/SkeletalMeshComponent.h"
#include "Core/DG_GameplayTags.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"

#include "AbilitySystemComponent.h"
 #include "Core/DG_Debug.h"
 #include "DrawDebugHelpers.h"

UANS_AttackHitWindow::UANS_AttackHitWindow()
  {
        TraceSocketNames =
        {
                TEXT("WP_Hit"),
                TEXT("WP_Hit_01"),
                TEXT("WP_Hit_02"),
                TEXT("WP_Hit_03")
        };

        HitEventTag = DGGameplayTags::Event_Attack_Hit;

  #if WITH_EDITORONLY_DATA
        NotifyColor = FColor(255, 80, 80);
  #endif
  }

  FString UANS_AttackHitWindow::GetNotifyName_Implementation() const
  {
        return TEXT("AttackHitWindow");
  }

  void UANS_AttackHitWindow::NotifyBegin(
        USkeletalMeshComponent* MeshComp,
        UAnimSequenceBase* Animation,
        float TotalDuration,
        const FAnimNotifyEventReference& EventReference
  )
  {
        Super::NotifyBegin(MeshComp, Animation, TotalDuration, EventReference);

        USkeletalMeshComponent* WeaponMesh = ResolveWeaponMesh(MeshComp);
        if (!MeshComp || !WeaponMesh)
        {
                return;
        }

        InitializeRuntimeData(MeshComp, WeaponMesh);
        
        if (bLogWindowLifecycle)
        {
                Debug::Print(TEXT("[ANS_AttackHitWindow] Begin"), FColor::Red);
        }
  }

  void UANS_AttackHitWindow::NotifyTick(
        USkeletalMeshComponent* MeshComp,
        UAnimSequenceBase* Animation,
        float FrameDeltaTime,
        const FAnimNotifyEventReference& EventReference
  )
  {
        Super::NotifyTick(MeshComp, Animation, FrameDeltaTime, EventReference);

        USkeletalMeshComponent* WeaponMesh = ResolveWeaponMesh(MeshComp);
        if (!MeshComp || !WeaponMesh)
        {
                return;
        }

        TraceWeaponSockets(MeshComp, WeaponMesh);
  }

  void UANS_AttackHitWindow::NotifyEnd(
        USkeletalMeshComponent* MeshComp,
        UAnimSequenceBase* Animation,
        const FAnimNotifyEventReference& EventReference
  )
  {
        Super::NotifyEnd(MeshComp, Animation, EventReference);

        RuntimeDataMap.Remove(MeshComp);
        
        if (bLogWindowLifecycle)
        {
                Debug::Print(TEXT("[ANS_AttackHitWindow] End"), FColor::Red);
        }
  }

  USkeletalMeshComponent* UANS_AttackHitWindow::ResolveWeaponMesh(USkeletalMeshComponent*
  CharacterMesh) const
  {
        if (!CharacterMesh)
        {
                return nullptr;
        }

        AWarriorCharacter* WarriorCharacter = Cast<AWarriorCharacter>(CharacterMesh->GetOwner());
        if (!WarriorCharacter)
        {
                return nullptr;
        }

        return WarriorCharacter->GetMainWeaponMesh();
  }

  void UANS_AttackHitWindow::InitializeRuntimeData(USkeletalMeshComponent* CharacterMesh,
  USkeletalMeshComponent* WeaponMesh)
  {
        if (!CharacterMesh || !WeaponMesh)
        {
                return;
        }

        FAttackHitWindowRuntimeData& RuntimeData = RuntimeDataMap.FindOrAdd(CharacterMesh);
        RuntimeData.PreviousSocketLocations.Reset();
        RuntimeData.HitActors.Reset();

        for (const FName& SocketName : TraceSocketNames)
        {
                if (WeaponMesh->DoesSocketExist(SocketName))
                {
                        RuntimeData.PreviousSocketLocations.Add(SocketName, WeaponMesh->GetSocketLocation(SocketName));
                }
        }
  }

  void UANS_AttackHitWindow::TraceWeaponSockets(USkeletalMeshComponent* CharacterMesh,USkeletalMeshComponent* WeaponMesh)
  {
        if (!CharacterMesh || !WeaponMesh)
        {
                return;
        }

        FAttackHitWindowRuntimeData* RuntimeDataPtr = RuntimeDataMap.Find(CharacterMesh);
        if (!RuntimeDataPtr)
        {
                InitializeRuntimeData(CharacterMesh, WeaponMesh);
                RuntimeDataPtr = RuntimeDataMap.Find(CharacterMesh);
                if (!RuntimeDataPtr)
                {
                        return;
                }
        }

        FAttackHitWindowRuntimeData& RuntimeData = *RuntimeDataPtr;

        AActor* OwnerActor = CharacterMesh->GetOwner();
        UWorld* World = OwnerActor ? OwnerActor->GetWorld() : nullptr;
        if (!OwnerActor || !World)
        {
                return;
        }

        FCollisionQueryParams QueryParams(SCENE_QUERY_STAT(AttackHitWindowTrace), false, OwnerActor);
        QueryParams.AddIgnoredActor(OwnerActor);

        for (const FName& SocketName : TraceSocketNames)
        {
                if (!WeaponMesh->DoesSocketExist(SocketName))
                {
                        continue;
                }

                const FVector CurrentLocation = WeaponMesh->GetSocketLocation(SocketName);
                FVector* PreviousLocationPtr = RuntimeData.PreviousSocketLocations.Find(SocketName);

                if (!PreviousLocationPtr)
                {
                        RuntimeData.PreviousSocketLocations.Add(SocketName, CurrentLocation);
                        continue;
                }

                const FVector Start = *PreviousLocationPtr;
                const FVector End = CurrentLocation;

                TArray<FHitResult> HitResults;
                World->SweepMultiByChannel(
                        HitResults,
                        Start,
                        End,
                        FQuat::Identity,
                        TraceChannel,
                        FCollisionShape::MakeSphere(TraceRadius),
                        QueryParams
                );

                // for (const FHitResult& HitResult : HitResults)
                // {
                //         AActor* HitActor = HitResult.GetActor();
                //         if (!IsValid(HitActor) || HitActor == OwnerActor)
                //         {
                //                 continue;
                //         }
                //
                //         if (RuntimeData.HitActors.Contains(HitActor))
                //         {
                //                 continue;
                //         }
                //
                //         RuntimeData.HitActors.Add(HitActor);
                //         SendHitEvent(OwnerActor, HitActor);
                // }
                
                for (const FHitResult& HitResult : HitResults)
                {
                        AActor* HitActor = HitResult.GetActor();
                        if (!IsValid(HitActor) || HitActor == OwnerActor)
                        {
                                continue;
                        }

                        if (ShouldIgnoreHitActor(OwnerActor, HitActor))
                        {
                                if (bEnableDebugDraw)
                                {
                                        DrawTraceDebug(World, Start, End, FColor::Yellow);
                                }

                                if (bLogAcceptedHits)
                                {
                                        const FString Msg = FString::Printf(
                                                TEXT("[ANS_AttackHitWindow] Ignored hit actor: %s"),
                                                *GetNameSafe(HitActor)
                                        );
                                        Debug::Print(Msg, FColor::Yellow);
                                }

                                continue;
                        }

                        if (RuntimeData.HitActors.Contains(HitActor))
                        {
                                continue;
                        }

                        RuntimeData.HitActors.Add(HitActor);

                        if (bEnableDebugDraw)
                        {
                                DrawTraceDebug(World, Start, End, FColor::Green);
                        }

                        if (bLogAcceptedHits)
                        {
                                const FString Msg = FString::Printf(
                                        TEXT("[ANS_AttackHitWindow] Accepted hit actor: %s"),
                                        *GetNameSafe(HitActor)
                                );
                                Debug::Print(Msg, FColor::Green);
                        }

                        SendHitEvent(OwnerActor, HitActor);
                }

                *PreviousLocationPtr = CurrentLocation;
        }
  }

  void UANS_AttackHitWindow::SendHitEvent(AActor* OwnerActor, AActor* HitActor) const
  {
        if (!OwnerActor || !HitActor || !HitEventTag.IsValid())
        {
                return;
        }

        FGameplayEventData Payload;
        Payload.EventTag = HitEventTag;
        Payload.Instigator = OwnerActor;
        Payload.Target = HitActor;

        UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(OwnerActor, HitEventTag, Payload);
  }

bool UANS_AttackHitWindow::ShouldIgnoreHitActor(AActor* OwnerActor, AActor* HitActor) const
{
        if (!OwnerActor || !HitActor)
        {
                return true;
        }

        if (OwnerActor == HitActor)
        {
                return true;
        }

        if (bIgnoreSameTeam && AreActorsOnSameTeam(OwnerActor, HitActor))
        {
                return true;
        }

        return false;
}

bool UANS_AttackHitWindow::AreActorsOnSameTeam(AActor* FirstActor, AActor* SecondActor) const
{
        if (!FirstActor || !SecondActor)
        {
                return false;
        }

        UAbilitySystemComponent* FirstASC =
  UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(FirstActor);
        UAbilitySystemComponent* SecondASC =
  UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(SecondActor);

        if (!FirstASC || !SecondASC)
        {
                return false;
        }

        const bool bBothPlayer =
                FirstASC->HasMatchingGameplayTag(DGGameplayTags::Team_Player.GetTag()) &&
                SecondASC->HasMatchingGameplayTag(DGGameplayTags::Team_Player.GetTag());

        const bool bBothEnemy =
                FirstASC->HasMatchingGameplayTag(DGGameplayTags::Team_Enemy.GetTag()) &&
                SecondASC->HasMatchingGameplayTag(DGGameplayTags::Team_Enemy.GetTag());

        const bool bBothObject =
                FirstASC->HasMatchingGameplayTag(DGGameplayTags::Team_Object.GetTag()) &&
                SecondASC->HasMatchingGameplayTag(DGGameplayTags::Team_Object.GetTag());

        return bBothPlayer || bBothEnemy || bBothObject;
}

void UANS_AttackHitWindow::DrawTraceDebug(UWorld* World, const FVector& Start, const FVector& End,
        const FColor& Color) const
{
        if (!World)
        {
                return;
        }

        DrawDebugSphere(World, Start, TraceRadius, 12, Color, false, DebugDrawDuration);
        DrawDebugSphere(World, End, TraceRadius, 12, Color, false, DebugDrawDuration);
        DrawDebugLine(World, Start, End, Color, false, DebugDrawDuration, 0, 1.5f);
}
