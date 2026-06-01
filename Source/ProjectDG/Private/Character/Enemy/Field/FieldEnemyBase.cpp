// Fill out your copyright notice in the Description page of Project Settings.

#include "Character/Enemy/Field/FieldEnemyBase.h"

#include "AIController.h"
#include "AbilitySystemComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Character/Enemy/Field/Data/FieldCharacterClassData.h"
#include "Core/DG_GameplayTags.h"
#include "GAS/Attributes/DG_EnemyAttributeSet.h"

AFieldEnemyBase::AFieldEnemyBase() {
  // 기본값 초기화
  LeashDistance = 1500.f;
  PatrolRadius = 800.f;
  EnemyLevel = 1;
  RewardExp = 50;
  MinRewardGold = 10;
  MaxRewardGold = 30;
  bIsReturning = false;

  EnemyAttributeSet =
      CreateDefaultSubobject<UDG_EnemyAttributeSet>(TEXT("EnemyAttributeSet"));
}

// Field Class Data에서 태그 초기화
void AFieldEnemyBase::InitializeFieldTagFromClassData() {
  if (!HasAuthority()) {
    return;
  }

  if (!FieldClassData) {
    return;
  }

  if (!FieldClassData->FieldTag.IsValid()) {
    return;
  }

  FieldTag = FieldClassData->FieldTag;

  if (AbilitySystemComponent) {
    AbilitySystemComponent->AddLooseGameplayTag(
        FieldTag, 1, EGameplayTagReplicationState::TagOnly);
  }
}

void AFieldEnemyBase::ApplyDefaultEffects() {
  if (!HasAuthority() || !AbilitySystemComponent) {
    return;
  }

  const bool bHasStartupEffects =
      FieldClassData && FieldClassData->StartupEffects.Num() > 0;
  const bool bHasEnemyStartupEffects =
      FieldClassData && FieldClassData->EnemyStartupEffects.Num() > 0;

  if (!FieldClassData || (!bHasStartupEffects && !bHasEnemyStartupEffects)) {
    Super::ApplyDefaultEffects();
    return;
  }

  FGameplayEffectContextHandle Context =
      AbilitySystemComponent->MakeEffectContext();
  Context.AddSourceObject(this);

  if (bHasStartupEffects) {
    for (const auto &EffectClass : FieldClassData->StartupEffects) {
      if (EffectClass) {
        FGameplayEffectSpecHandle Spec =
            AbilitySystemComponent->MakeOutgoingSpec(EffectClass, 1.f, Context);
        if (Spec.IsValid()) {
          AbilitySystemComponent->ApplyGameplayEffectSpecToSelf(*Spec.Data.Get());
        }
      }
    }
  }

  if (bHasEnemyStartupEffects) {
    for (const auto &EffectClass : FieldClassData->EnemyStartupEffects) {
      if (EffectClass) {
        FGameplayEffectSpecHandle Spec =
            AbilitySystemComponent->MakeOutgoingSpec(EffectClass, 1.f, Context);
        if (Spec.IsValid()) {
          AbilitySystemComponent->ApplyGameplayEffectSpecToSelf(*Spec.Data.Get());
        }
      }
    }
  }
}

void AFieldEnemyBase::BeginPlay() {
  Super::BeginPlay();

  // 스폰되었을 때의 최초 위치를 원점으로 기억
  SpawnOriginLocation = GetActorLocation();

  // BeginPlay 시점에 이미 Possess된 경우를 대비해 Blackboard 값 초기화 시도
  if (HasAuthority()) {
    UpdateBlackboardValues();
  }
}

void AFieldEnemyBase::PossessedBy(AController *NewController) {
  Super::PossessedBy(NewController);

  // Possession 시점에 태그 및 Blackboard 값 세팅 시도
  if (HasAuthority()) {
    InitializeFieldTagFromClassData();
    UpdateBlackboardValues();

    if (AbilitySystemComponent) {
      AbilitySystemComponent
          ->GetGameplayAttributeValueChangeDelegate(
              UDG_EnemyAttributeSet::GetGroggyGaugeAttribute())
          .AddUObject(this, &AFieldEnemyBase::OnGroggyGaugeChanged);
    }
  }
}

void AFieldEnemyBase::OnGroggyGaugeChanged(const FOnAttributeChangeData &Data) {
  if (!HasAuthority() || !AbilitySystemComponent || !EnemyAttributeSet) {
    return;
  }

  if (AbilitySystemComponent->HasMatchingGameplayTag(
          DGGameplayTags::State_Enemy_Groggy) ||
      IsDead()) {
    return;
  }

  const float MaxGroggyGauge = EnemyAttributeSet->GetMaxGroggyGauge();
  if (MaxGroggyGauge <= 0.f) {
    return;
  }

  if (Data.NewValue < MaxGroggyGauge) {
    return;
  }

  FGameplayEventData Payload;
  AbilitySystemComponent->HandleGameplayEvent(DGGameplayTags::Event_Enemy_Groggy,
                                              &Payload);
}

void AFieldEnemyBase::StartReturnToOrigin() {
  if (bIsReturning) {
    return;
  }

  bIsReturning = true;

  // 1. 귀환중 GameplayTag 부여
  if (AbilitySystemComponent) {
    AbilitySystemComponent->AddLooseGameplayTag(
        DGGameplayTags::State_Enemy_Returning, 1,
        EGameplayTagReplicationState::TagOnly);
  }

  // 2. 귀환 중 플레이어 공격 무력화(무적) 및 빠른 체력 회복 GameplayEffect 적용
  if (HasAuthority() && AbilitySystemComponent && ReturningEffectClass) {
    FGameplayEffectContextHandle Context =
        AbilitySystemComponent->MakeEffectContext();
    Context.AddSourceObject(this);
    FGameplayEffectSpecHandle Spec = AbilitySystemComponent->MakeOutgoingSpec(
        ReturningEffectClass, 1.f, Context);
    if (Spec.IsValid()) {
      ReturningEffectHandle =
          AbilitySystemComponent->ApplyGameplayEffectSpecToSelf(
              *Spec.Data.Get());
    }
  }

  // 3. AI Blackboard 갱신
  if (AAIController *AIController = Cast<AAIController>(GetController())) {
    if (UBlackboardComponent *BBComp = AIController->GetBlackboardComponent()) {
      BBComp->SetValueAsBool(IsReturningKeyName, true);
    }
  }
}

void AFieldEnemyBase::CompleteReturnToOrigin() {
  if (!bIsReturning) {
    return;
  }

  bIsReturning = false;

  // 1. 귀환중 GameplayTag 제거
  if (AbilitySystemComponent) {
    AbilitySystemComponent->RemoveLooseGameplayTag(
        DGGameplayTags::State_Enemy_Returning, 1);
  }

  // 2. 무적/체력 회복 버프(GameplayEffect) 제거
  if (HasAuthority() && AbilitySystemComponent &&
      ReturningEffectHandle.IsValid()) {
    AbilitySystemComponent->RemoveActiveGameplayEffect(ReturningEffectHandle);
    ReturningEffectHandle.Invalidate();
  }

  // 3. AI Blackboard 갱신
  if (AAIController *AIController = Cast<AAIController>(GetController())) {
    if (UBlackboardComponent *BBComp = AIController->GetBlackboardComponent()) {
      BBComp->SetValueAsBool(IsReturningKeyName, false);
    }
  }
}

void AFieldEnemyBase::UpdateBlackboardValues() {
  if (AAIController *AIController = Cast<AAIController>(GetController())) {
    if (UBlackboardComponent *BBComp = AIController->GetBlackboardComponent()) {
      BBComp->SetValueAsVector(SpawnOriginKeyName, SpawnOriginLocation);
      BBComp->SetValueAsFloat(PatrolRadiusKeyName, PatrolRadius);
      BBComp->SetValueAsFloat(LeashDistanceKeyName, LeashDistance);
      BBComp->SetValueAsBool(IsReturningKeyName, bIsReturning);
    }
  }
}
