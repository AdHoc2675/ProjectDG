
#include "Character/Enemy/EnemyCharacterBase.h"

#include "AbilitySystemComponent.h"
#include "AttributeSet.h"
#include "GAS/Attributes/DG_AttributeSet.h"
#include "Core/DG_Debug.h"
#include "Core/DG_GameplayTags.h"


AEnemyCharacterBase::AEnemyCharacterBase()
{
    AbilitySystemComponent = CreateDefaultSubobject<UAbilitySystemComponent>(TEXT("AbilitySystemComponent"));
    
    AttributeSet = CreateDefaultSubobject<UDG_AttributeSet>(TEXT("AttributeSet"));
}

void AEnemyCharacterBase::BeginPlay()
{
    ABaseCharacter::BeginPlay();
    
    InitializeEnemyAbilitySystem();
}

void AEnemyCharacterBase::PossessedBy(AController* NewController)
{
    ABaseCharacter::PossessedBy(NewController);
    
    InitializeEnemyAbilitySystem();

    // 서버에서 기본 어빌리티와 초기 스탯(GE) 부여
    if (HasAuthority())
    {
        GrantDefaultAbilities();
        ApplyDefaultEffects();
    }
}

void AEnemyCharacterBase::InitializeEnemyAbilitySystem()
{
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

UAbilitySystemComponent* AEnemyCharacterBase::GetCharacterAbilitySystemComponent() const
{
    return  AbilitySystemComponent;
}

const UAttributeSet* AEnemyCharacterBase::GetCharacterAttributeSet() const
{
    return AttributeSet;
}

void AEnemyCharacterBase::GrantDefaultAbilities()
{
    if (!HasAuthority() || !AbilitySystemComponent) return;

    for (const auto& AbilityClass : DefaultAbilities)
    {
        if (AbilityClass)
        {
            AbilitySystemComponent->GiveAbility(FGameplayAbilitySpec(AbilityClass, 1));
        }
    }
}

void AEnemyCharacterBase::ApplyDefaultEffects()
{
    if (!HasAuthority() || !AbilitySystemComponent) return;

    FGameplayEffectContextHandle Context = AbilitySystemComponent->MakeEffectContext();
    Context.AddSourceObject(this);

    for (const auto& EffectClass : DefaultEffects)
    {
        if (EffectClass)
        {
            FGameplayEffectSpecHandle Spec = AbilitySystemComponent->MakeOutgoingSpec(EffectClass, 1.f, Context);
            if (Spec.IsValid())
            {
                AbilitySystemComponent->ApplyGameplayEffectSpecToSelf(*Spec.Data.Get());
            }
        }
    }
}

