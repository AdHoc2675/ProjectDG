

#include "Character/Player/PlayerCharacterBase.h"

#include "AbilitySystemComponent.h"
#include "Camera/CameraComponent.h"
#include "Core/DG_Debug.h"
#include "Core/DG_GameplayTags.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/DG_PlayerState.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/SpringArmComponent.h"
#include "GAS/Attributes/DG_AttributeSet.h"

#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputActionValue.h"
#include "Net/UnrealNetwork.h"

#include "Animation/AnimInstance.h"
#include "Character/Player/Data/PlayerCharacterMovementData.h"

APlayerCharacterBase::APlayerCharacterBase()
{
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;
	
	//스프링암 생성
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(GetRootComponent());
	//컨트롤러 회전 -> 스프링암 따라가도록 
	CameraBoom->bUsePawnControlRotation = true;
	CameraBoom->TargetArmLength = 450.f;

	//카메라 생성
	FollowCam = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCam"));
	FollowCam->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	FollowCam->bUsePawnControlRotation = false;
	bUseControllerRotationYaw = false;

	//이동방향기준회전,속도
	GetCharacterMovement()->bOrientRotationToMovement = true;
	GetCharacterMovement()->RotationRate = FRotator(0.f, 720.0f, 0.f);
	
}

void APlayerCharacterBase::BeginPlay()
{
    Super::BeginPlay();
    
	//월드시작시 ASC초기화
    InitializePlayerAbilitySystem();
	ApplyMovementData();
	ApplyCurrentMovementSpeed();
}

void APlayerCharacterBase::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	// sprint/dodge 테스트용 임시함수 - 이후 GA 및 GE로 관리
	UpdateSprintStamina(DeltaSeconds);
}

void APlayerCharacterBase::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	
	DOREPLIFETIME(APlayerCharacterBase, bIsSprinting);
	DOREPLIFETIME(APlayerCharacterBase, bIsDodging);
	DOREPLIFETIME(APlayerCharacterBase, bIsParkouring);
}

void APlayerCharacterBase::InitializePlayerAbilitySystem()
{
	/**
	 * 현재 Character에 연결된 PlayerState를
	 * DG_PlayerState 타입으로 가져온다.
	 */
	ADG_PlayerState* PS = GetPlayerState<ADG_PlayerState>();
	if (!PS)
	{
		Debug::Print(TEXT("[PlayerCharacterBase] DG_PlayerState is null."));
		return;
	}

	/**
	 * PlayerState가 소유한 ASC를 가져온다.
	 */
	UAbilitySystemComponent* ASC = PS->GetAbilitySystemComponent();
	if (!ASC)
	{
		Debug::Print(TEXT("[PlayerCharacterBase] ASC is null on DG_PlayerState."));
		return;
	}

	/**
	 * GAS ActorInfo 초기화
	 *
	 * Player 구조에서:
	 * - OwnerActor  = PlayerState
	 * - AvatarActor = Character(this)
	 *
	 * 이유:
	 * - ASC의 실제 소유자는 PlayerState
	 * - 실제 월드에서 움직이고 스킬을 사용하는 존재는 Character
	 */
	ASC->InitAbilityActorInfo(PS, this);

	Debug::Print(TEXT("[PlayerCharacterBase] ASC initialized from DG_PlayerState."));
}

void APlayerCharacterBase::PawnClientRestart()
{
	Super::PawnClientRestart();

	/**
	 * 로컬 플레이어 컨트롤러 가져오기
	 *
	 * PawnClientRestart는 로컬 입력과 관련된 재초기화 시점에서 중요하다.
	 */
	APlayerController* OwningPlayerController = GetController<APlayerController>();
	if (!OwningPlayerController)
	{
		return;
	}

	/**
	 * LocalPlayer 체크
	 *
	 * GetLocalPlayer()가 nullptr일 수 있으므로
	 * 바로 접근하지 않고 한 단계 검사를 둔다.
	 */
	ULocalPlayer* LocalPlayer = OwningPlayerController->GetLocalPlayer();
	if (!LocalPlayer)
	{
		return;
	}

	/**
	 * Enhanced Input LocalPlayer Subsystem 가져오기
	 */
	UEnhancedInputLocalPlayerSubsystem* InputSubSystem =
		LocalPlayer->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>();

	if (!InputSubSystem)
	{
		return;
	}

	/**
	 * Mapping Context가 설정되어 있다면
	 * 기존 것을 제거 후 다시 등록한다.
	 *
	 * 왜 Remove 후 Add 하냐:
	 * - 재접속 / 재시작 / 재소유 상황에서 중복 등록 방지
	 * - 현재 입력 상태를 다시 깔끔하게 맞추기 위함
	 */
	if (BasicInputMappingContext)
	{
		InputSubSystem->RemoveMappingContext(BasicInputMappingContext);
		InputSubSystem->AddMappingContext(BasicInputMappingContext, 0);
	}
}

void APlayerCharacterBase::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	/**
	 * EnhancedInputComponent로 캐스팅
	 *
	 * 현재 프로젝트 입력 구조가 Enhanced Input 기준이므로
	 * 캐스팅에 성공해야 BindAction을 사용할 수 있다.
	 */
	UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent);
	if (!EnhancedInputComponent)
	{
		Debug::Print(TEXT("[PlayerCharacterBase] EnhancedInputComponent cast failed."));
		return;
	}

	/**
	 * 각 InputAction이 유효할 때만 바인딩
	 *
	 * nullptr 상태에서 바인딩하려 하면 문제 생길 수 있으므로
	 * 하나씩 방어적으로 체크한다.
	 */
	 if (IA_Move)
        {
                EnhancedInputComponent->BindAction(IA_Move, ETriggerEvent::Triggered,this,&APlayerCharacterBase::MoveAction);
                EnhancedInputComponent->BindAction(IA_Move,ETriggerEvent::Completed,this,&APlayerCharacterBase::MoveAction);
        }

        if (IA_Look)
        {
                EnhancedInputComponent->BindAction(IA_Look,ETriggerEvent::Triggered,this,&APlayerCharacterBase::LookAction);
        }

        if (IA_Jump)
        {
                EnhancedInputComponent->BindAction(IA_Jump,ETriggerEvent::Started,this,&ACharacter::Jump);
                EnhancedInputComponent->BindAction(IA_Jump,ETriggerEvent::Completed,this,&ACharacter::StopJumping);
        }

        if (IA_Dodge)
        {
                EnhancedInputComponent->BindAction(IA_Dodge,ETriggerEvent::Triggered,this,&APlayerCharacterBase::DodgeAction);
        }

        if (IA_Sprint)
        {
                EnhancedInputComponent->BindAction(IA_Sprint,ETriggerEvent::Started,this,&APlayerCharacterBase::SprintStarted);
                EnhancedInputComponent->BindAction(IA_Sprint,ETriggerEvent::Completed,this,&APlayerCharacterBase::SprintCompleted);
        }
}

UAbilitySystemComponent* APlayerCharacterBase::GetCharacterAbilitySystemComponent() const
{
	/**
	 * BaseCharacter 공용 ASC getter override
	 *
	 * Player는 Character가 ASC를 직접 들고 있지 않기 때문에
	 * PlayerState에서 찾아 반환해야 한다.
	 */
	ADG_PlayerState* PS = GetPlayerState<ADG_PlayerState>();
	if (!PS)
	{
		return nullptr;
	}

	return PS->GetAbilitySystemComponent();
}

const UAttributeSet* APlayerCharacterBase::GetCharacterAttributeSet() const
{
	/**
	 * BaseCharacter 공용 AttributeSet getter override
	 *
	 * PlayerState가 가진 DG_AttributeSet을
	 * 상위 타입(UAttributeSet)으로 반환한다.
	 */
	ADG_PlayerState* PS = GetPlayerState<ADG_PlayerState>();
	if (!PS)
	{
		return nullptr;
	}

	return PS->GetDGAttributeSet();
}

UDG_AttributeSet* APlayerCharacterBase::GetPlayerDGAttributeSet() const
{
	/**
	 * Player 전용 AttributeSet getter
	 *
	 * 추후 DG_AttributeSet 전용 속성 접근 시
	 * 매번 캐스팅하지 않도록 편의 함수로 제공
	 */
	ADG_PlayerState* PS = GetPlayerState<ADG_PlayerState>();
	if (!PS)
	{
		return nullptr;
	}

	return PS->GetDGAttributeSet();
}

void APlayerCharacterBase::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);

	/**
	 * Controller가 Pawn을 점유한 시점은
	 * PlayerState 기반 ASC 초기화 재시도 타이밍으로 중요하다.
	 */
	InitializePlayerAbilitySystem();
}

void APlayerCharacterBase::OnRep_PlayerState()
{
	Super::OnRep_PlayerState();

	/**
	 * PlayerState가 복제 완료된 뒤 ASC 초기화 재시도
	 *
	 * 네트워크 환경에서 매우 중요
	 */
	InitializePlayerAbilitySystem();
}

void APlayerCharacterBase::LookAction(const FInputActionValue& InputActionValue)
{
	const FVector2D InputValue = InputActionValue.Get<FVector2D>();
	
	AddControllerPitchInput(-InputValue.Y);
	AddControllerYawInput(InputValue.X);
}

void APlayerCharacterBase::MoveAction(const FInputActionValue& InputActionValue)
{
	CurrentMoveInput = InputActionValue.Get<FVector2D>();
	
	if (CurrentMoveInput.IsNearlyZero())
	{
		if (bIsSprinting)
		{
			SprintCompleted();
		}
		
		return;
	}
	
	const FVector MoveDirection = GetCameraForwardOnPlane() * CurrentMoveInput.Y + GetCameraRightOnPlane() * CurrentMoveInput.X;
	AddMovementInput(MoveDirection.GetSafeNormal());
}

void APlayerCharacterBase::OnRep_IsSprinting()
{
	ApplyCurrentMovementSpeed();
}

void APlayerCharacterBase::DodgeAction()
{
	if (!CanDodge())
	{
		return;
	}
	ServerPerformDodge(GetDesiredMoveDirection());
}

void APlayerCharacterBase::ServerSetSprinting_Implementation(bool bNewSprinting,
	FVector_NetQuantizeNormal DesiredDirection)
{
	if (bNewSprinting)
	{
		if (IsDead() || bIsDodging)
		{
			return;
		}
		if (!MovementData || FVector(DesiredDirection).IsNearlyZero())
		{
			return;
		}
		if (!HasEnoughStamina(1.f))
		{
			return;
		}
	}
	
	SetSprintingState(bNewSprinting);
}

void APlayerCharacterBase::ServerPerformDodge_Implementation(FVector_NetQuantizeNormal DodgeDirection)
{
	if (!MovementData)
	{
		return;
	}
	if (IsDead() || bIsDodging)
	{
		return;
	}
	
	const FVector Direction = FVector(DodgeDirection).GetSafeNormal();
	if (Direction.IsNearlyZero())
	{
		return;
	}
	
	if (!TryConsumeStamina(MovementData->DodgeStaminaCost))
	{
		return;
	}
	
	SetSprintingState(false);
	SetDodgingState(true);
	
	LaunchCharacter(Direction * MovementData->DodgeStrength, true, false);
	
	if (MovementData->DodgeMontage)
	{
		MulticastPlayMontage(MovementData->DodgeMontage);
	}
	
	GetWorldTimerManager().SetTimer(
			DodgeTimerHandle,
			this,
			&APlayerCharacterBase::FinishDodge,
			MovementData->DodgeDuration,
			false
	);
}

void APlayerCharacterBase::MulticastPlayMontage_Implementation(UAnimMontage* Montage)
{
	if (!Montage)
	{
		return;
	}

	USkeletalMeshComponent* MeshComp = GetMesh();
	if (!MeshComp)
	{
		return;
	}

	UAnimInstance* AnimInstance = MeshComp->GetAnimInstance();
	if (!AnimInstance)
	{
		return;
	}

	AnimInstance->Montage_Play(Montage);
}

void APlayerCharacterBase::SprintStarted()
{
	if (!CanSprint())
	{
		return;
	}
	
	ServerSetSprinting(true, GetDesiredMoveDirection());
}

void APlayerCharacterBase::SprintCompleted()
{
	ServerSetSprinting(false, FVector::ZeroVector);
}

FVector APlayerCharacterBase::GetCameraForwardOnPlane() const
{
	FVector Forward = (FollowCam ? FollowCam->GetForwardVector() : FVector::ForwardVector);
	Forward.Z = 0.f;
	
	return Forward.GetSafeNormal();
	
}

void APlayerCharacterBase::ApplyMovementData()
{
	if (!MovementData)
	{
		Debug::Print(TEXT("[PlayerCharacterBase] MovementData is null."));
		return;
	}
	
	UCharacterMovementComponent* MoveComp = GetCharacterMovement();
	if (!MoveComp)
	{
		return;
	}
	
	MoveComp->RotationRate = MovementData->RotationRate;
	MoveComp->JumpZVelocity = MovementData->JumpZVelocity;
	MoveComp->AirControl = MovementData->AirControl;
}

void APlayerCharacterBase::UpdateSprintStamina(float DeltaSeconds)
{
	if (!HasAuthority() || !bIsSprinting || !MovementData)
	{
		return;
	}
	
	const float Cost = MovementData->SprintStaminaCostPerSecond * DeltaSeconds;
	if (!TryConsumeStamina(Cost))
	{
		SetSprintingState(false);
	}
}

bool APlayerCharacterBase::HasEnoughStamina(float Amount) const
{
	const UDG_AttributeSet* Attr = GetPlayerDGAttributeSet();
	return Attr && Attr->GetStamina() >= Amount;
}

bool APlayerCharacterBase::TryConsumeStamina(float Amount)
{
	UDG_AttributeSet* Attr = GetPlayerDGAttributeSet();
	if (!Attr || Attr->GetStamina() < Amount)
	{
		return false;
	}
	
	Attr->SetStamina(FMath::Max(0.f, Attr->GetStamina() - Amount));
	return true;
}

bool APlayerCharacterBase::CanSprint() const
{
	if (!MovementData)
	{
		return false;
	}
	
	if (IsDead() || bIsDodging)
	{
		return false;
	}
	
	if (CurrentMoveInput.IsNearlyZero())
	{
		return false;
	}
	
	return HasEnoughStamina(1.f);
}

bool APlayerCharacterBase::CanDodge() const
{
	if (!MovementData)
	{
		return false;
	}
	
	if (IsDead() || bIsDodging)
	{
		return false;
	}
	
	if (CurrentMoveInput.IsNearlyZero())
	{
		return false;
	}
	
	return HasEnoughStamina(MovementData->DodgeStaminaCost);
}

void APlayerCharacterBase::ApplyCurrentMovementSpeed()
{
	if (!MovementData)
	{
		return;
	}
	
	UCharacterMovementComponent* MoveComp = GetCharacterMovement();
	if (!MoveComp)
	{
		return;
	}
	
	MoveComp->MaxWalkSpeed = bIsSprinting ? MovementData->SprintSpeed : MovementData->WalkSpeed;
}

void APlayerCharacterBase::SetSprintingState(bool bNewSprinting)
{
	bIsSprinting = bNewSprinting;
	ApplyCurrentMovementSpeed();
}

void APlayerCharacterBase::SetDodgingState(bool bNewDodging)
{
	bIsDodging = bNewDodging;
}

void APlayerCharacterBase::FinishDodge()
{
	SetDodgingState(false);
}

FVector APlayerCharacterBase::GetCameraRightOnPlane() const
{
	FVector Right = FollowCam ? FollowCam->GetRightVector() : FVector::RightVector;
	Right.Z = 0.f;
	
	return Right.GetSafeNormal();
}

FVector APlayerCharacterBase::GetDesiredMoveDirection() const
{
	const FVector Direction = GetCameraForwardOnPlane() * CurrentMoveInput.Y + GetCameraRightOnPlane() * CurrentMoveInput.X;
	
	if (Direction.IsNearlyZero())
	{
		FVector Forward = GetActorForwardVector();
		Forward.Z = 0.f;
		return Forward.GetSafeNormal();
	}
	return Direction.GetSafeNormal();
}
