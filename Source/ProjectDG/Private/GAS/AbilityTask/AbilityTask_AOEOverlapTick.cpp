// Fill out your copyright notice in the Description page of Project Settings.


#include "GAS/AbilityTask/AbilityTask_AOEOverlapTick.h"

#include "Engine/OverlapResult.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"

UAbilityTask_AOEOverlapTick* UAbilityTask_AOEOverlapTick::AOEOverlapTick(
      UGameplayAbility* OwningAbility,
      AActor* InAvatarActor,
      float InRadius,
      float InHalfHeight,
      float InTickInterval,
      TEnumAsByte<ECollisionChannel> InTraceChannel
)
{
      UAbilityTask_AOEOverlapTick* Task = NewAbilityTask<UAbilityTask_AOEOverlapTick>(OwningAbility);
      Task->AvatarActor = InAvatarActor;
      Task->Radius = InRadius;
      Task->HalfHeight = InHalfHeight;
      Task->TickInterval = FMath::Max(InTickInterval, 0.02f);
      Task->TraceChannel = InTraceChannel;
      Task->bTickingTask = true;
      return Task;
}

void UAbilityTask_AOEOverlapTick::Activate()
{
      Super::Activate();

      PerformOverlap();
}

void UAbilityTask_AOEOverlapTick::TickTask(float DeltaTime)
{
      Super::TickTask(DeltaTime);

      ElapsedTime += DeltaTime;
      if (ElapsedTime < TickInterval)
      {
              return;
      }

      ElapsedTime = 0.f;
      PerformOverlap();
}

void UAbilityTask_AOEOverlapTick::PerformOverlap()
{
      if (!AvatarActor)
      {
              EndTask();
              return;
      }

      UWorld* World = AvatarActor->GetWorld();
      if (!World)
      {
              EndTask();
              return;
      }

      FCollisionQueryParams QueryParams(SCENE_QUERY_STAT(AOEOverlapTick), false, AvatarActor);
      QueryParams.AddIgnoredActor(AvatarActor);

      TArray<FOverlapResult> OverlapResults;
      World->OverlapMultiByChannel(
              OverlapResults,
              AvatarActor->GetActorLocation(),
              FQuat::Identity,
              TraceChannel,
              FCollisionShape::MakeCapsule(Radius, HalfHeight),
              QueryParams
      );

      for (const FOverlapResult& OverlapResult : OverlapResults)
      {
              AActor* TargetActor = OverlapResult.GetActor();
              if (!IsValid(TargetActor))
              {
                      continue;
              }

              TWeakObjectPtr<AActor> TargetActorPtr(TargetActor);
              if (HitActors.Contains(TargetActorPtr))
              {
                      continue;
              }

              HitActors.Add(TargetActorPtr);
      		  if (ShouldBroadcastAbilityTaskDelegates())
      		  {
      			  OnTargetFound.Broadcast(TargetActor);
      		  }
      }
}



