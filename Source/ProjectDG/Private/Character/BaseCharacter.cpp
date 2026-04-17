#include "Character/BaseCharacter.h"

#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/CharacterMovementComponent.h"

ABaseCharacter::ABaseCharacter()
{
	PrimaryActorTick.bCanEverTick = true;
	bIsDead = false;
}

void ABaseCharacter::BeginPlay()
{
	Super::BeginPlay();
	InitializeCharacterIfReady();
}

void ABaseCharacter::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
}

void ABaseCharacter::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);
	InitializeCharacterIfReady();
}

void ABaseCharacter::OnRep_PlayerState()
{
	Super::OnRep_PlayerState();
	InitializeCharacterIfReady();
}

void ABaseCharacter::InitializeCharacterIfReady()
{
	RefreshAbilitySystemReferences();
	InitializeAttributesIfReady();
}

void ABaseCharacter::InitializeAttributesIfReady()
{
	// 추후 DT / GE 초기 스탯 적용
}

void ABaseCharacter::RefreshAbilitySystemReferences()
{
	// 추후 PlayerCharacterBase / EnemyCharacterBase에서 ASC 연결
}

UAbilitySystemComponent* ABaseCharacter::GetAbilitySystemComponent() const
{
	return GetCharacterAbilitySystemComponent();
}

UAbilitySystemComponent* ABaseCharacter::GetCharacterAbilitySystemComponent() const
{
	return nullptr;
}

const UAttributeSet* ABaseCharacter::GetCharacterAttributeSet() const
{
	return nullptr;
}

FGameplayTag ABaseCharacter::GetTeamTag() const
{
	return TeamTag;
}

void ABaseCharacter::SetTeamTag(FGameplayTag InTeamTag)
{
	TeamTag = InTeamTag;
}

bool ABaseCharacter::HasTeamTag(FGameplayTag InTeamTag) const
{
	return TeamTag.IsValid() && TeamTag == InTeamTag;
}

bool ABaseCharacter::IsFriendlyTo(const ABaseCharacter* OtherCharacter) const
{
	if (!OtherCharacter)
	{
		return false;
	}

	if (!TeamTag.IsValid() || !OtherCharacter->TeamTag.IsValid())
	{
		return false;
	}

	return TeamTag == OtherCharacter->TeamTag;
}

bool ABaseCharacter::IsDead() const
{
	return bIsDead;
}

void ABaseCharacter::Die()
{
	if (bIsDead)
	{
		return;
	}

	bIsDead = true;
	HandleDeath();
}

void ABaseCharacter::HandleDeath()
{
	DisableCharacterAfterDeath();
}

void ABaseCharacter::DisableCharacterAfterDeath()
{
	if (UCharacterMovementComponent* MoveComp = GetCharacterMovement())
	{
		MoveComp->StopMovementImmediately();
		MoveComp->DisableMovement();
	}

	if (UCapsuleComponent* CapsuleComp = GetCapsuleComponent())
	{
		CapsuleComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}

	if (USkeletalMeshComponent* MeshComp = GetMesh())
	{
		MeshComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}
}