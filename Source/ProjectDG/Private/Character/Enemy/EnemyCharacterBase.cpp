
#include "Character/Enemy/EnemyCharacterBase.h"

#include "AbilitySystemComponent.h"
#include "AttributeSet.h"
#include "Core/DG_Debug.h"
#include "Core/DG_GameplayTags.h"


AEnemyCharacterBase::AEnemyCharacterBase()
{
    //Enemy는 Character 에서 직접 ASC 를 소유한다.    
    AbilitySystemComponent = CreateDefaultSubobject<UAbilitySystemComponent>(TEXT("AbilitySystemComponent"));
    
    //추후 Attribute , TeamTag등이 생기면 여기에 붙이면 됨
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

