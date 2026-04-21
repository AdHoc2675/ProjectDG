

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

APlayerCharacterBase::APlayerCharacterBase()
{
	//스프링암 생성
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(GetRootComponent());
	
	//컨트롤러 회전 -> 스프링암 따라가도록 
	CameraBoom->bUsePawnControlRotation = true;

	//카메라 생성
	FollowCam = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCam"));
	FollowCam->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);

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
	if (IA_Jump)
	{
		EnhancedInputComponent->BindAction(IA_Jump, ETriggerEvent::Triggered, this, &APlayerCharacterBase::Jump);
	}

	if (IA_Look)
	{
		EnhancedInputComponent->BindAction(IA_Look, ETriggerEvent::Triggered, this, &APlayerCharacterBase::LookAction);
	}

	if (IA_Move)
	{
		EnhancedInputComponent->BindAction(IA_Move, ETriggerEvent::Triggered, this, &APlayerCharacterBase::MoveAction);
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
	/**
	 * 상위 클래스 공통 초기화 먼저 수행
	 */
	Super::PossessedBy(NewController);

	/**
	 * Controller가 Pawn을 점유한 시점은
	 * PlayerState 기반 ASC 초기화 재시도 타이밍으로 중요하다.
	 */
	InitializePlayerAbilitySystem();
}

void APlayerCharacterBase::OnRep_PlayerState()
{
	/**
	 * 상위 클래스 공통 처리 먼저 수행
	 */
	Super::OnRep_PlayerState();

	/**
	 * PlayerState가 복제 완료된 뒤
	 * ASC 초기화 재시도
	 *
	 * 네트워크 환경에서 매우 중요
	 */
	InitializePlayerAbilitySystem();
}

void APlayerCharacterBase::LookAction(const FInputActionValue& InputActionValue)
{
	/**
	 * Look 입력은 FVector2D로 들어온다고 가정
	 * X = 좌우(Yaw), Y = 상하(Pitch)
	 */
	const FVector2D InputValue = InputActionValue.Get<FVector2D>();

	/**
	 * 일반적으로 마우스 Y를 올리면 화면은 위로 가지만
	 * 입력값 체계에 따라 반전되는 경우가 있으므로
	 * 현재는 -Y를 사용
	 */
	AddControllerPitchInput(-InputValue.Y);
	AddControllerYawInput(InputValue.X);
}

void APlayerCharacterBase::MoveAction(const FInputActionValue& InputActionValue)
{
	/**
	 * Move 입력 역시 FVector2D로 받는다.
	 * X = 좌우 이동
	 * Y = 전후 이동
	 */
	const FVector2D InputValue = InputActionValue.Get<FVector2D>();

	/**
	 * 현재 구현은 카메라 기준 Forward / Right 벡터를 그대로 사용한다.
	 *
	 * 주의:
	 * - 카메라 Pitch가 크게 들어가면 이동 벡터에 Z 성분이 섞일 수 있다.
	 * - 보통 3인칭은 Yaw 평면 기준 이동으로 한 번 더 정리하기도 한다.
	 *
	 * 하지만 현재 단계에서는 기존 네 구조를 최대한 유지한다.
	 */
	AddMovementInput(GetLookForwardDirection() * InputValue.Y + GetLookRightDirection() * InputValue.X);
}

FVector APlayerCharacterBase::GetLookRightDirection() const
{
	/**
	 * 카메라 기준 오른쪽 방향 반환
	 */
	return FollowCam ? FollowCam->GetRightVector() : FVector::RightVector;
}

FVector APlayerCharacterBase::GetLookForwardDirection() const
{
	/**
	 * 카메라 기준 전방 방향 반환
	 */
	return FollowCam ? FollowCam->GetForwardVector() : FVector::ForwardVector;
}

FVector APlayerCharacterBase::GetMoveForwardDirection() const
{
	/**
	 * 오른쪽 벡터와 월드 UpVector의 외적을 사용해
	 * 평면상 전방 벡터를 구하는 보조 함수
	 *
	 * 현재 MoveAction에서는 직접 사용하지 않지만,
	 * 추후 Yaw-only 이동 방식으로 바꿀 때 재사용 가능
	 */
	return FVector::CrossProduct(GetLookRightDirection(), FVector::UpVector).GetSafeNormal();
}
