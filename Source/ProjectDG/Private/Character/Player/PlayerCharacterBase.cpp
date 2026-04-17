

#include "Character/Player/PlayerCharacterBase.h"

#include "AbilitySystemComponent.h"
#include "AttributeSet.h"
#include "Core/DG_Debug.h"
#include "Core/DG_GameplayTags.h"
#include "GameFramework/DG_PlayerState.h"

APlayerCharacterBase::APlayerCharacterBase()
{
}

void APlayerCharacterBase::BeginPlay()
{
    ABaseCharacter::BeginPlay();
    
    InitializePlayerAbilitySystem();
}

void APlayerCharacterBase::InitializePlayerAbilitySystem()
{
}

UAbilitySystemComponent* APlayerCharacterBase::GetCharacterAbilitySystemComponent() const
{
    return ABaseCharacter::GetCharacterAbilitySystemComponent();
}

const UAttributeSet* APlayerCharacterBase::GetCharacterAttributeSet() const
{
    return ABaseCharacter::GetCharacterAttributeSet();
}

void APlayerCharacterBase::PossessedBy(AController* NewController)
{
    ABaseCharacter::PossessedBy(NewController);
}

void APlayerCharacterBase::OnRep_PlayerState()
{
    ABaseCharacter::OnRep_PlayerState();
}
