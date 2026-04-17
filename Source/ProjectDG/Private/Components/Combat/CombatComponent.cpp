#include "Components/Combat/CombatComponent.h"

#include "Character/BaseCharacter.h"
#include  "AbilitySystemComponent.h"


UCombatComponent::UCombatComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
}


void UCombatComponent::BeginPlay()
{
   Super::BeginPlay();
    
    InitializeCombatComponent();
}

void UCombatComponent::InitializeCombatComponent()
{
    //GetOwner는 이 컴포넌트를 들고 있는 Actor를 반환한다.
    //만약 CombatComponent 가 BaseCharacter 에 붙어있다면, Owner 는 Basecharacter로 캐스팅되어 캐싱된다.
    OwnerBaseCharacter=Cast<ABaseCharacter>(GetOwner());
}

ABaseCharacter* UCombatComponent::GetOwnerBaseCharacter() const
{
    return OwnerBaseCharacter;
}

bool UCombatComponent::HasValidOwnerCharacter() const
{
    return OwnerBaseCharacter != nullptr;
}

UAbilitySystemComponent* UCombatComponent::GetOwnerAbilitySystemComponent() const
{
    //Never CombatComponent 는 ASC 를 직접 소유하지 않는다. 
    //대신 Owner에게 ASC에 대해 요청한다.
    //Player 는 PlayerState 에 ASC를
    //Enemy는 캐릭터에 직접 ASC를 넣는다.
    //필요할떄 ASC에 접근한다.
    
    if (!OwnerBaseCharacter)
    {
        return nullptr;
    }
    
    return OwnerBaseCharacter->GetCharacterAbilitySystemComponent();
}


