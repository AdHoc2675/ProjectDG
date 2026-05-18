// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/Player/Animation/Notifies/ANS_AttackHitWindow.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "Character/Player/PlayerCharacterBase.h"
#include "Character/Player/Warrior/WarriorCharacter.h"
#include "Components/SkeletalMeshComponent.h"
#include "Core/DG_GameplayTags.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/Character.h"

#include "AbilitySystemComponent.h"
#include "Core/DG_Debug.h"
#include "DrawDebugHelpers.h"
#include "Engine/OverlapResult.h"

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

void UANS_AttackHitWindow::NotifyBegin(USkeletalMeshComponent* MeshComp,UAnimSequenceBase* Animation,float TotalDuration,const FAnimNotifyEventReference& EventReference)
{
        Super::NotifyBegin(MeshComp, Animation, TotalDuration, EventReference);

        AActor* OwnerActor = MeshComp ? MeshComp->GetOwner() : nullptr;
        if (!OwnerActor || !OwnerActor->HasAuthority())
        {
                return;
        }
        
        if (!MeshComp)
        {
                return;
        }

        if (TraceMode == EAttackHitTraceMode::ForwardBoxOverlap)
        {
                InitializeRuntimeData(MeshComp, nullptr);
                
                if (bLogWindowLifecycle)
                {
                        // AActor* OwnerActor = MeshComp ? MeshComp->GetOwner() : nullptr;
                        UAnimInstance* AnimInstance = MeshComp ? MeshComp->GetAnimInstance() : nullptr;
                        UAnimMontage* Montage = Cast<UAnimMontage>(Animation);
        
                        const float MontagePosition = (AnimInstance && Montage)
                                ? AnimInstance->Montage_GetPosition(Montage)
                                : -1.f;
        
                        const FString NetRoleText = OwnerActor
                                ? FString::Printf(TEXT("Authority=%s LocalRole=%d RemoteRole=%d"),
                                        OwnerActor->HasAuthority() ? TEXT("true") : TEXT("false"),
                                        static_cast<int32>(OwnerActor->GetLocalRole()),
                                        static_cast<int32>(OwnerActor->GetRemoteRole()))
                                : TEXT("Owner=null");
        
                        Debug::Print(FString::Printf(
                                TEXT("[ANS_AttackHitWindow] Begin Owner=%s Mesh=%s Anim=%s Duration=%.3f Pos=%.3f %s"),
                                *GetNameSafe(OwnerActor),
                                *GetNameSafe(MeshComp),
                                *GetNameSafe(Animation),
                                TotalDuration,
                                MontagePosition,
                                *NetRoleText
                        ), FColor::Blue);
                }
                
                return;
        }

        USkeletalMeshComponent* WeaponMesh = ResolveWeaponMesh(MeshComp);
        if (!WeaponMesh)
        {
                return;
        }

        InitializeRuntimeData(MeshComp, WeaponMesh);

        
}

void UANS_AttackHitWindow::NotifyTick(USkeletalMeshComponent* MeshComp,UAnimSequenceBase* Animation,float FrameDeltaTime,const FAnimNotifyEventReference& EventReference)
{
        Super::NotifyTick(MeshComp, Animation, FrameDeltaTime, EventReference);

        // 서버에서만 Sweep 로직 -> Server가 아니면 return
        AActor* OwnerActor = MeshComp ? MeshComp->GetOwner() : nullptr;
        if (!OwnerActor || !OwnerActor->HasAuthority())
        {
                return;
        }

        if (!MeshComp)
        {
                return;
        }

        if (TraceMode == EAttackHitTraceMode::ForwardBoxOverlap)
        {
                TraceForwardBox(MeshComp);
                return;
        }

        USkeletalMeshComponent* WeaponMesh = ResolveWeaponMesh(MeshComp);
        if (!WeaponMesh)
        {
                return;
        }

        TraceWeaponSockets(MeshComp, WeaponMesh);
}

void UANS_AttackHitWindow::NotifyEnd(USkeletalMeshComponent* MeshComp,UAnimSequenceBase* Animation,const FAnimNotifyEventReference& EventReference)
{
        Super::NotifyEnd(MeshComp, Animation, EventReference);

        AActor* OwnerActor = MeshComp ? MeshComp->GetOwner() : nullptr;
        if (!OwnerActor || !OwnerActor->HasAuthority())
        {
                return;
        }
        
        RuntimeDataMap.Remove(MeshComp);
        
        if (bLogWindowLifecycle)
        {
                // AActor* OwnerActor = MeshComp ? MeshComp->GetOwner() : nullptr;
                UAnimInstance* AnimInstance = MeshComp ? MeshComp->GetAnimInstance() : nullptr;
                UAnimMontage* Montage = Cast<UAnimMontage>(Animation);
        
                const float MontagePosition = (AnimInstance && Montage)
                        ? AnimInstance->Montage_GetPosition(Montage)
                        : -1.f;
        
                const FString NetRoleText = OwnerActor
                        ? FString::Printf(TEXT("Authority=%s LocalRole=%d RemoteRole=%d"),
                                OwnerActor->HasAuthority() ? TEXT("true") : TEXT("false"),
                                static_cast<int32>(OwnerActor->GetLocalRole()),
                                static_cast<int32>(OwnerActor->GetRemoteRole()))
                        : TEXT("Owner=null");
        
                Debug::Print(FString::Printf(
                        TEXT("[ANS_AttackHitWindow] End Owner=%s Mesh=%s Anim=%s Pos=%.3f %s"),
                        *GetNameSafe(OwnerActor),
                        *GetNameSafe(MeshComp),
                        *GetNameSafe(Animation),
                        MontagePosition,
                        *NetRoleText
                ), FColor::Blue);
        }
}

// WeaponMesh 획득
USkeletalMeshComponent* UANS_AttackHitWindow::ResolveWeaponMesh(USkeletalMeshComponent* CharacterMesh) const
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

void UANS_AttackHitWindow::InitializeRuntimeData(USkeletalMeshComponent* CharacterMesh, USkeletalMeshComponent* WeaponMesh)
{
        if (!CharacterMesh)
        {
                return;
        }

        FAttackHitWindowRuntimeData& RuntimeData = RuntimeDataMap.FindOrAdd(CharacterMesh);
        RuntimeData.PreviousSocketLocations.Reset();
        // RuntimeData.HitActors.Reset();
        
        if (!WeaponMesh)
        {
                return;
        }

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
        QueryParams.AddIgnoredActor(OwnerActor);  // 자기 자신은 제외

        for (const FName& SocketName : TraceSocketNames)
        {
                if (!WeaponMesh->DoesSocketExist(SocketName))
                {
                        if (bLogTraceSweeps)
                        {
                                // const FString Msg = FString::Printf(
                                //         TEXT("[ANS_AttackHitWindow] Missing socket on weapon mesh: %s"),
                                //         *SocketName.ToString()
                                // );
                                // Debug::Print(Msg, FColor::Red);
                        }
                        continue;
                }

                const FVector CurrentLocation = WeaponMesh->GetSocketLocation(SocketName);
                FVector* PreviousLocationPtr = RuntimeData.PreviousSocketLocations.Find(SocketName);

                if (!PreviousLocationPtr)
                {
                        RuntimeData.PreviousSocketLocations.Add(SocketName, CurrentLocation);

                        if (bEnableDebugDraw)
                        {
                                DrawDebugSphere(World, CurrentLocation, TraceRadius, 12, FColor::Blue, false, DebugDrawDuration);
                        }

                        if (bLogTraceSweeps)
                        {
                                // const FString Msg = FString::Printf(
                                //         TEXT("[ANS_AttackHitWindow] First frame socket init: %s"),
                                //         *SocketName.ToString()
                                // );
                                // Debug::Print(Msg, FColor::Blue);
                        }

                        continue;
                }

                const FVector Start = *PreviousLocationPtr;
                const FVector End = CurrentLocation;
                
                TArray<FHitResult> HitResults;
                World->SweepMultiByChannel(HitResults,Start,End,FQuat::Identity,TraceChannel,FCollisionShape::MakeSphere(TraceRadius),QueryParams);

                bool bAcceptedAnyHit = false;
                bool bIgnoredAnyHit = false;

                for (const FHitResult& HitResult : HitResults)
                {
                        AActor* HitActor = HitResult.GetActor();
                        if (!IsValid(HitActor) || HitActor == OwnerActor)
                        {
                                continue;
                        }

                        if (ShouldIgnoreHitActor(OwnerActor, HitActor))
                        {
                                bIgnoredAnyHit = true;

                                if (bLogAcceptedHits)
                                {
                                        // const FString Msg = FString::Printf(
                                        //         TEXT("[ANS_AttackHitWindow] Ignored hit actor: %s"),
                                        //         *GetNameSafe(HitActor)
                                        // );
                                        // Debug::Print(Msg, FColor::Yellow);
                                }

                                continue;
                        }

                        // if (RuntimeData.HitActors.Contains(HitActor))
                        // {
                        //         bIgnoredAnyHit = true;
                        //
                        //         if (bLogAcceptedHits)
                        //         {
                        //                 // const FString Msg = FString::Printf(
                        //                 //         TEXT("[ANS_AttackHitWindow] Ignored duplicate hit actor: %s"),
                        //                 //         *GetNameSafe(HitActor)
                        //                 // );
                        //                 // Debug::Print(Msg, FColor::Yellow);
                        //         }
                        //
                        //         continue;
                        // }
                        //
                        // RuntimeData.HitActors.Add(HitActor);
                        bAcceptedAnyHit = true;

                        if (bLogAcceptedHits)
                        {
                                // const FString Msg = FString::Printf(
                                //         TEXT("[ANS_AttackHitWindow] Accepted hit actor: %s"),
                                //         *GetNameSafe(HitActor)
                                // );
                                // Debug::Print(Msg, FColor::Green);
                        }

                        SendHitEvent(OwnerActor, HitActor);
                }

                if (bLogTraceSweeps)
                {
                        // const FString Msg = FString::Printf(
                        //         TEXT("[ANS_AttackHitWindow] PostSweep %s : rawHitCount=%d accepted=%d ignored=%d"),
                        //         *SocketName.ToString(),
                        //         HitResults.Num(),
                        //         bAcceptedAnyHit ? 1 : 0,
                        //         bIgnoredAnyHit ? 1 : 0
                        // );
                        // Debug::Print(Msg, FColor::Silver);
                }

                if (bEnableDebugDraw)
                {
                        if (bAcceptedAnyHit)
                        {
                                // DrawTraceDebug(OwnerActor, World, Start, End, FColor::Green);
                        }
                        else if (bIgnoredAnyHit)
                        {
                                // DrawTraceDebug(OwnerActor, World, Start, End, FColor::Yellow);
                        }
                        else
                        {
                                // DrawTraceDebug(OwnerActor, World, Start, End, FColor::Red);
                        }
                }
                

                *PreviousLocationPtr = CurrentLocation;
        }
}

void UANS_AttackHitWindow::TraceForwardBox(USkeletalMeshComponent* CharacterMesh)
{
      if (!CharacterMesh)
      {
              return;
      }
        
      AActor* OwnerActor = CharacterMesh->GetOwner();
      UWorld* World = OwnerActor ? OwnerActor->GetWorld() : nullptr;
      if (!OwnerActor || !World)
      {
              return;
      }

      // FAttackHitWindowRuntimeData* RuntimeDataPtr = RuntimeDataMap.Find(CharacterMesh);
      // if (!RuntimeDataPtr)
      // {
      //     InitializeRuntimeData(CharacterMesh, nullptr);
      //     RuntimeDataPtr = RuntimeDataMap.Find(CharacterMesh);
      //     if (!RuntimeDataPtr)
      //     {
      //             return;
      //     }
      // }

      // FAttackHitWindowRuntimeData& RuntimeData = *RuntimeDataPtr;

      FCollisionQueryParams QueryParams(SCENE_QUERY_STAT(AttackHitWindowForwardBox), false, OwnerActor); 
        
      QueryParams.AddIgnoredActor(OwnerActor);

      const FVector Forward = CharacterMesh->GetForwardVector();
      const FVector Right = CharacterMesh->GetRightVector();
      const FVector Center =
          CharacterMesh->GetComponentLocation() +
          Forward * ForwardBoxForwardOffset +
          Right * ForwardBoxRightOffset +
          FVector(0.f, 0.f, ForwardBoxHeightOffset);
        
      const FQuat BoxRotation = CharacterMesh->GetComponentQuat();
        
      const FVector BoxHalfExtent(
          ForwardBoxRange * 0.5f,
          ForwardBoxWidth * 0.5f,
          ForwardBoxHeight * 0.5f
      );

      TArray<FOverlapResult> OverlapResults;
      World->OverlapMultiByChannel(
          OverlapResults,
          Center,
          BoxRotation,
          TraceChannel,
          FCollisionShape::MakeBox(BoxHalfExtent),
          QueryParams
      );

      bool bAcceptedAnyHit = false;

      for (const FOverlapResult& OverlapResult : OverlapResults)
      {
          AActor* HitActor = OverlapResult.GetActor();
          if (!IsValid(HitActor) || HitActor == OwnerActor)
          {
                  continue;
          }

          if (ShouldIgnoreHitActor(OwnerActor, HitActor))
          {
                  continue;
          }

          // if (RuntimeData.HitActors.Contains(HitActor))
          // {
          //         continue;
          // }

          // RuntimeData.HitActors.Add(HitActor);
          bAcceptedAnyHit = true;

          SendHitEvent(OwnerActor, HitActor);
      }

      if (bEnableDebugDraw)
      {
              if (APlayerCharacterBase* PlayerCharacter = Cast<APlayerCharacterBase>(OwnerActor))
              {
                      PlayerCharacter->ClientDrawAttackBoxDebug(
                              Center,
                              BoxHalfExtent,
                              BoxRotation.Rotator(),
                              bAcceptedAnyHit ? FColor::Green : FColor::Red,
                              DebugDrawDuration
                      );
              }
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

        UAbilitySystemComponent* HitASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(HitActor);
        if (!HitASC)
        {
                return true;
        }

        // 현재는 같은 팀이어도 PvP 허용
        // if (bIgnoreSameTeam && AreActorsOnSameTeam(OwnerActor, HitActor))
        // {
        //         return true;
        // }

        // 현재 단계에서는 적 팀만 유효 타겟으로 간주
        // if (!HitASC->HasMatchingGameplayTag(DGGameplayTags::Team_Enemy.GetTag()))
        // {
        //         return true;
        // }

        return false;
}

bool UANS_AttackHitWindow::AreActorsOnSameTeam(AActor* FirstActor, AActor* SecondActor) const
{
        if (!FirstActor || !SecondActor)
        {
                return false;
        }

        UAbilitySystemComponent* FirstASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(FirstActor);
        UAbilitySystemComponent* SecondASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(SecondActor);

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

// void UANS_AttackHitWindow::DrawTraceDebug(AActor* OwnerActor, UWorld* World, const FVector& Start, const FVector& End, const FColor& Color) const
// {
//         if (!World)
//         {
//                 return;
//         }
//
//         DrawDebugSphere(World, Start, TraceRadius, 12, Color, false, DebugDrawDuration);
//         DrawDebugSphere(World, End, TraceRadius, 12, Color, false, DebugDrawDuration);
//         DrawDebugLine(World, Start, End, Color, false, DebugDrawDuration, 0, 1.5f);
//
//         APlayerCharacterBase* PlayerCharacter = Cast<APlayerCharacterBase>(OwnerActor);
//         if (PlayerCharacter && PlayerCharacter->HasAuthority())
//         {
//                 PlayerCharacter->ClientDrawAttackTraceDebug(Start, End, TraceRadius, Color, DebugDrawDuration);
//         }
// }
