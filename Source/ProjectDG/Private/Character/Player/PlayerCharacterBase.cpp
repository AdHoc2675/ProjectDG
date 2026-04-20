

#include "Character/Player/PlayerCharacterBase.h"

#include "Gameframework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"

#include "AbilitySystemComponent.h"
#include "AttributeSet.h"
#include "Core/DG_Debug.h"
#include "Core/DG_GameplayTags.h"
#include "Gameframework/PlayerController.h"
#include "Gameframework/CharacterMovementComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "GameFramework/DG_PlayerState.h"

APlayerCharacterBase::APlayerCharacterBase()
{
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(GetRootComponent());
	CameraBoom->bUsePawnControlRotation = true;

	FollowCam = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCam"));
	FollowCam->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);

	bUseControllerRotationYaw = false;

	GetCharacterMovement()->bOrientRotationToMovement = true;
	GetCharacterMovement()->RotationRate = FRotator(0.f, 720.0f, 0.f);
}

void APlayerCharacterBase::BeginPlay()
{
    ABaseCharacter::BeginPlay();
    
    InitializePlayerAbilitySystem();
}

void APlayerCharacterBase::InitializePlayerAbilitySystem()
{
}

void APlayerCharacterBase::PawnClientRestart()
{
	Super::PawnClientRestart();
	APlayerController* OwningPlayerController = GetController<APlayerController>();
	if (OwningPlayerController)
	{
		UEnhancedInputLocalPlayerSubsystem* InputSubSystem = OwningPlayerController->GetLocalPlayer()->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>();
		if (InputSubSystem)
		{
			//재접속시 기존에 IMC를 지우고 새로 등록한다.
			InputSubSystem->RemoveMappingContext(BasicInputMappingContext);
			InputSubSystem->AddMappingContext(BasicInputMappingContext, 0);
		}
	}
}

void APlayerCharacterBase::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
	UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent);
	if (EnhancedInputComponent)
	{
		EnhancedInputComponent->BindAction(IA_Jump, ETriggerEvent::Triggered, this, &APlayerCharacterBase::Jump);
		EnhancedInputComponent->BindAction(IA_Look, ETriggerEvent::Triggered, this, &APlayerCharacterBase::LookAction);
		EnhancedInputComponent->BindAction(IA_Move, ETriggerEvent::Triggered, this, &APlayerCharacterBase::MoveAction);
	}
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

void APlayerCharacterBase::LookAction(const FInputActionValue& InputActionValue)
{
	FVector2D InputValue = InputActionValue.Get<FVector2D>();

	AddControllerPitchInput(-InputValue.Y);
	AddControllerYawInput(InputValue.X);
}

void APlayerCharacterBase::MoveAction(const FInputActionValue& InputActionValue)
{
	FVector2D InputValue = InputActionValue.Get<FVector2D>();

	AddMovementInput(GetLookForwardDirection() * InputValue.Y + GetLookRightDirection() * InputValue.X);
}

FVector APlayerCharacterBase::GetLookRightDirection() const
{
	return FollowCam->GetRightVector();
}

FVector APlayerCharacterBase::GetLookForwardDirection() const
{
	return FollowCam->GetForwardVector();
}

FVector APlayerCharacterBase::GetMoveForwardDirection() const
{
	//외적으로("오른쪽 방향과 위쪽 방향에 수직인 방향 → 즉, 전방 방향을 구한다")
	return FVector::CrossProduct(GetLookRightDirection(), FVector::UpVector);
}
