
#include "Character/Enemy/EnemyCharacterBase.h"

#include "AI/Controller/AIControllerBase.h"
#include "AbilitySystemComponent.h"
#include "Animation/AnimInstance.h"
#include "Animation/AnimMontage.h"
#include "AttributeSet.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Core/DG_Debug.h"
#include "Core/DG_GameplayTags.h"
#include "GAS/Attributes/DG_AttributeSet.h"
#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "Components/UI/DGMinimapMarkerComponent.h"

AEnemyCharacterBase::AEnemyCharacterBase() {
  AbilitySystemComponent = CreateDefaultSubobject<UAbilitySystemComponent>(
      TEXT("AbilitySystemComponent"));

  AttributeSet = CreateDefaultSubobject<UDG_AttributeSet>(TEXT("AttributeSet"));

  // 드롭 컴포넌트 생성
  LootDropComponent = CreateDefaultSubobject<UDGLootDropComponent>(TEXT("LootDropComponent"));

  // 미니맵 마커 생성 및 기본 타입 설정
  MinimapMarkerComponent = CreateDefaultSubobject<UDGMinimapMarkerComponent>(TEXT("MinimapMarkerComponent"));
  MinimapMarkerComponent->MarkerType = EMinimapMarkerType::Enemy;

  // 서버에서도 소켓 기반 트레이스가 정상 작동하도록 애니메이션 본을 항상 갱신하게 설정
  if (USkeletalMeshComponent* MeshComp = GetMesh())
  {
      MeshComp->VisibilityBasedAnimTickOption = EVisibilityBasedAnimTickOption::AlwaysTickPoseAndRefreshBones;
  }
}

void AEnemyCharacterBase::BeginPlay() {
  ABaseCharacter::BeginPlay();

  InitializeEnemyAbilitySystem();
}

void AEnemyCharacterBase::PossessedBy(AController *NewController) {
  ABaseCharacter::PossessedBy(NewController);

  InitializeEnemyAbilitySystem();

  // 서버에서 기본 어빌리티와 초기 스탯(GE) 부여
  if (HasAuthority()) {
    GrantDefaultAbilities();
    ApplyDefaultEffects();

    if (AbilitySystemComponent) {
      AbilitySystemComponent
          ->GetGameplayAttributeValueChangeDelegate(
              UDG_AttributeSet::GetHealthAttribute())
          .AddUObject(this, &AEnemyCharacterBase::OnHealthChanged_DeathCheck);
    }
  }
}

void AEnemyCharacterBase::InitializeEnemyAbilitySystem() {
  /**
   * Enemy 구조:
   * - OwnerActor  = this
   * - AvatarActor = this
   *
   * 즉 캐릭터 본체가 ASC를 직접 들고 있으므로
   * 둘 다 자기 자신으로 초기화한다.
   */
  AbilitySystemComponent->InitAbilityActorInfo(this, this);
}

UAbilitySystemComponent *
AEnemyCharacterBase::GetCharacterAbilitySystemComponent() const {
  return AbilitySystemComponent;
}

const UAttributeSet *AEnemyCharacterBase::GetCharacterAttributeSet() const {
  return AttributeSet;
}

void AEnemyCharacterBase::GrantDefaultAbilities() {
  if (!HasAuthority() || !AbilitySystemComponent)
    return;

  for (const auto &AbilityClass : DefaultAbilities) {
    if (AbilityClass) {
      AbilitySystemComponent->GiveAbility(
          FGameplayAbilitySpec(AbilityClass, 1));
    }
  }
}

void AEnemyCharacterBase::ApplyDefaultEffects() {
  if (!HasAuthority() || !AbilitySystemComponent)
    return;

  FGameplayEffectContextHandle Context =
      AbilitySystemComponent->MakeEffectContext();
  Context.AddSourceObject(this);

  for (const auto &EffectClass : DefaultEffects) {
    if (EffectClass) {
      FGameplayEffectSpecHandle Spec =
          AbilitySystemComponent->MakeOutgoingSpec(EffectClass, 1.f, Context);
      if (Spec.IsValid()) {
        AbilitySystemComponent->ApplyGameplayEffectSpecToSelf(*Spec.Data.Get());
      }
    }
  }
}

void AEnemyCharacterBase::HandleDeath() {
  if (HasAuthority()) {
    if (AbilitySystemComponent) {
      AbilitySystemComponent->AddLooseGameplayTag(
          DGGameplayTags::State_Enemy_Dead, 1,
          EGameplayTagReplicationState::TagOnly);
    }

    if (AAIControllerBase *AIController =
            Cast<AAIControllerBase>(GetController())) {
      AIController->StopAIOnDeath();
    }
  }

  if (HasAuthority()) {
    MulticastPlayDeathMontage();
  }

  // (서버에서만) 아이템 드롭 로직 호출
  if (HasAuthority() && LootDropComponent) {
      // 몬스터의 현재 위치를 드롭 좌표로 사용
      LootDropComponent->ProcessDrop(GetActorLocation());
  }

  Super::HandleDeath();
}

void AEnemyCharacterBase::MulticastPlayDeathMontage_Implementation() {
  // 사망 시 플레이어가 시체를 통과할 수 있도록 Pawn과의 충돌을 무시(Ignore)로 변경합니다.
  if (UCapsuleComponent* Capsule = GetCapsuleComponent()) {
    Capsule->SetCollisionResponseToChannel(ECC_Pawn, ECR_Ignore);
  }
  if (USkeletalMeshComponent* MeshComp = GetMesh()) {
    MeshComp->SetCollisionResponseToChannel(ECC_Pawn, ECR_Ignore);
  }

  // 사망과 동시에 미니맵에서 마커 지우기
  if (MinimapMarkerComponent) {
      MinimapMarkerComponent->UnregisterFromSubsystem();
  }

  if (!DeathMontage) {
    return;
  }

  if (USkeletalMeshComponent* MeshComp = GetMesh()) {
    if (UAnimInstance *AnimInstance = MeshComp->GetAnimInstance()) {
      AnimInstance->Montage_Play(DeathMontage);
    }
  }
}

void AEnemyCharacterBase::Multicast_SpawnAOETelegraph_Implementation(UNiagaraSystem* VFX, FVector Location, FName ScaleParamName, float Scale)
{
	if (!VFX) return;

	if (AOETelegraphComponent)
	{
		AOETelegraphComponent->DeactivateImmediate();
		AOETelegraphComponent = nullptr;
	}

	AOETelegraphComponent = UNiagaraFunctionLibrary::SpawnSystemAtLocation(this, VFX, Location);

	if (AOETelegraphComponent && ScaleParamName != NAME_None)
	{
		AOETelegraphComponent->SetVariableFloat(ScaleParamName, Scale);
	}
}

void AEnemyCharacterBase::Multicast_DestroyAOETelegraph_Implementation()
{
	if (AOETelegraphComponent)
	{
		AOETelegraphComponent->DeactivateImmediate();
		AOETelegraphComponent = nullptr;
	}
}

void AEnemyCharacterBase::OnHealthChanged_DeathCheck(
    const FOnAttributeChangeData &Data) {
  if (!HasAuthority()) {
    return;
  }

  if (bIsDead) {
    return;
  }

  if (Data.NewValue <= 0.f) {
    Die();
  }
}
