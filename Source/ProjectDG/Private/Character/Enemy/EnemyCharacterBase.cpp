
#include "Character/Enemy/EnemyCharacterBase.h"

#include "AbilitySystemComponent.h"
#include "AttributeSet.h"
#include "Core/DG_Debug.h"
#include "Core/DG_GameplayTags.h"
#include "Data/DT_Attribute.h"
#include "Engine/DataTable.h"
#include "GAS/Attributes/DG_AttributeSet.h"


AEnemyCharacterBase::AEnemyCharacterBase()
{
    // Enemy는 Character에서 직접 ASC를 소유한다.
    AbilitySystemComponent = CreateDefaultSubobject<UAbilitySystemComponent>(TEXT("AbilitySystemComponent"));
    AbilitySystemComponent->SetIsReplicated(true);
    AbilitySystemComponent->SetReplicationMode(EGameplayEffectReplicationMode::Minimal);

    // Enemy도 공통 전투 AttributeSet을 가진다.
    // Health / Defense / Damage 메타 Attribute는 여기서 처리된다.
    AttributeSet = CreateDefaultSubobject<UDG_AttributeSet>(TEXT("AttributeSet"));

    // Enemy 기본 팀 태그.
    TeamTag = DGGameplayTags::Team_Enemy;
}

void AEnemyCharacterBase::BeginPlay()
{
    ABaseCharacter::BeginPlay();
    
    InitializeEnemyAbilitySystem();
    InitializeEnemyAttributesFromDataTable();
}

void AEnemyCharacterBase::PossessedBy(AController* NewController)
{
    ABaseCharacter::PossessedBy(NewController);
    
    InitializeEnemyAbilitySystem();
    InitializeEnemyAttributesFromDataTable();
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

void AEnemyCharacterBase::InitializeEnemyAttributesFromDataTable()
{
    if (!HasAuthority())
    {
        return;
    }

    if (!AttributeInitDataTable)
    {
        Debug::Print(TEXT("[EnemyCharacterBase] AttributeInitDataTable is null."));
        return;
    }

    if (!AttributeSet)
    {
        Debug::Print(TEXT("[EnemyCharacterBase] AttributeSet is null."));
        return;
    }

    if (AttributeInitRowName.IsNone())
    {
        Debug::Print(TEXT("[EnemyCharacterBase] AttributeInitRowName is none."));
        return;
    }

    const FDT_Attribute* InitRow =
        AttributeInitDataTable->FindRow<FDT_Attribute>(
            AttributeInitRowName,
            TEXT("EnemyCharacterBase::InitializeEnemyAttributesFromDataTable")
        );

    if (!InitRow)
    {
        Debug::Print(FString::Printf(
            TEXT("[EnemyCharacterBase] Failed to find Attribute row. RowName=%s"),
            *AttributeInitRowName.ToString()
        ));
        return;
    }

    AttributeSet->InitHealth(InitRow->MaxHealth);
    AttributeSet->InitMaxHealth(InitRow->MaxHealth);

    AttributeSet->InitMental(InitRow->MaxMental);
    AttributeSet->InitMaxMental(InitRow->MaxMental);

    AttributeSet->InitStamina(InitRow->MaxStamina);
    AttributeSet->InitMaxStamina(InitRow->MaxStamina);

    AttributeSet->InitMainStat(InitRow->MainStat);
    AttributeSet->InitAttackPower(InitRow->AttackPower);
    AttributeSet->InitDefense(InitRow->Defense);
    AttributeSet->InitHealthCoefficient(InitRow->HealthCoefficient);
    AttributeSet->InitDefenseCoefficient(InitRow->DefenseCoefficient);
    AttributeSet->InitCriticalRate(InitRow->CriticalRate);
    AttributeSet->InitCriticalDamage(InitRow->CriticalDamage);
    AttributeSet->InitMoveSpeed(InitRow->MoveSpeed);
    AttributeSet->InitAttackSpeed(InitRow->AttackSpeed);
    AttributeSet->InitGroggyDamage(InitRow->GroggyDamage);
    AttributeSet->InitFinalDamageIncrease(InitRow->FinalDamageIncrease);
    AttributeSet->InitDamageReduction(InitRow->DamageReduction);
    AttributeSet->InitCooldownReduction(InitRow->CooldownReduction);
    AttributeSet->InitMentalRecoveryIncrease(InitRow->MentalRecoveryIncrease);
    AttributeSet->InitLifeSteal(InitRow->LifeSteal);
    AttributeSet->InitGroggyDamageIncreaseRate(InitRow->GroggyDamageIncreaseRate);

    Debug::Print(FString::Printf(
        TEXT("[EnemyCharacterBase] Attributes initialized. Row=%s Health=%.1f Defense=%.1f DefenseCoeff=%.2f"),
        *AttributeInitRowName.ToString(),
        AttributeSet->GetHealth(),
        AttributeSet->GetDefense(),
        AttributeSet->GetDefenseCoefficient()
    ));
}

UAbilitySystemComponent* AEnemyCharacterBase::GetCharacterAbilitySystemComponent() const
{
    return  AbilitySystemComponent;
}

const UAttributeSet* AEnemyCharacterBase::GetCharacterAttributeSet() const
{
    return AttributeSet;
}

