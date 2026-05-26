#include "Character/Player/PlayerCharacterBase.h"
#include "Character/Player/Data/PlayerSkillData.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "Camera/CameraComponent.h"
#include "Core/DG_Debug.h"
#include "Core/DG_GameplayTags.h"
#include "Core/DG_Struct.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/DG_PlayerState.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/SpringArmComponent.h"
#include "GAS/Attributes/DG_AttributeSet.h"

#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputActionValue.h"
#include "Net/UnrealNetwork.h"
#include "DrawDebugHelpers.h"

#include "Components/Combat/CombatComponent.h"
#include "Kismet/GameplayStatics.h"

#include "Animation/AnimInstance.h"
#include "Character/Player/Data/PlayerCharacterMovementData.h"
#include "Character/Player/Data/PlayerCharacterClassData.h"
#include "Perception/AIPerceptionStimuliSourceComponent.h"
#include "Perception/AISense_Sight.h"

#include "UI/HUD/DG_HUD.h"

#include "Components/UI/DGMinimapCaptureComponent.h"
#include "Components/UI/DGMinimapMarkerComponent.h"
#include "Components/Targeting/LockOnComponent.h"


APlayerCharacterBase::APlayerCharacterBase()
{
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;

	// 컴포넌트 생성 및 할당
	// 외형 관련 컴포넌트
	HeadMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("HeadMesh"));
	Hair1Mesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("Hair1Mesh"));
	Hair2Mesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("Hair2Mesh"));
	Hair3Mesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("Hair3Mesh"));
	UpperBodyMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("UpperBodyMesh"));
	LowerBodyMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("LowerBodyMesh"));
	HelmetMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("HelmetMesh"));
	ShoesMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("ShoesMesh"));
	ShoulderMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("ShoulderMesh"));
	GlovesMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("GlovesMesh"));

	// 리스트를 만들어 일괄 설정
	TArray<USkeletalMeshComponent*> ModularMeshes = {
		HeadMesh, Hair1Mesh, Hair2Mesh, Hair3Mesh,
		UpperBodyMesh, LowerBodyMesh, HelmetMesh,
		ShoesMesh, ShoulderMesh, GlovesMesh
	};

	for (USkeletalMeshComponent* MeshComp : ModularMeshes)
	{
		if (MeshComp)
		{
			MeshComp->SetupAttachment(GetMesh());
		}
	}

	// AI관련 StimuliSourceComponent
	StimuliSourceComponent = CreateDefaultSubobject<UAIPerceptionStimuliSourceComponent>(TEXT("StimuliSourceComponent"));

	// 스프링암 생성
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(GetRootComponent());
	CameraBoom->bUsePawnControlRotation = true;
	CameraBoom->TargetArmLength = 450.f;

	// 카메라 생성
	FollowCam = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCam"));
	FollowCam->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	FollowCam->bUsePawnControlRotation = false;
	bUseControllerRotationYaw = false;

	// 컨트롤러(마우스) 회전이 캐릭터 몸통에 직접 영향을 주지 않게끔 설정
	bUseControllerRotationYaw = false;

	// 이동방향기준회전,속도
	GetCharacterMovement()->bOrientRotationToMovement = true;
	GetCharacterMovement()->RotationRate = FRotator(0.f, 720.0f, 0.f);

	AutoPossessPlayer = EAutoReceiveInput::Disabled;
	AutoPossessAI = EAutoPossessAI::Disabled;

	MinimapCaptureComponent = CreateDefaultSubobject<UDGMinimapCaptureComponent>(TEXT("MinimapCaptureComponent"));
	MinimapMarkerComponent = CreateDefaultSubobject<UDGMinimapMarkerComponent>(TEXT("MinimapMarkerComponent"));

	LockOnComponent = CreateDefaultSubobject<ULockOnComponent>(TEXT("LockOnComponent"));

	MinimapMarkerComponent->MarkerType = EMinimapMarkerType::Player;
}

void APlayerCharacterBase::BeginPlay()
{
	Super::BeginPlay();

	// 월드시작시 ASC초기화
	InitializePlayerAbilitySystem();
}

void APlayerCharacterBase::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
}

void APlayerCharacterBase::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
}

void APlayerCharacterBase::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	if (GetMesh())
	{
		TArray<USkeletalMeshComponent*> ModularMeshes = {
			HeadMesh, Hair1Mesh, Hair2Mesh, Hair3Mesh,
			UpperBodyMesh, LowerBodyMesh, HelmetMesh,
			ShoesMesh, ShoulderMesh, GlovesMesh
		};

		for (USkeletalMeshComponent* MeshComp : ModularMeshes)
		{
			if (MeshComp)
			{
				// 메인 메쉬(GetMesh())의 애니메이션 포즈를 따르도록 설정
				MeshComp->SetLeaderPoseComponent(GetMesh());

				// [최적화] 리더 포즈 사용 시 틱 옵션 조정
				MeshComp->VisibilityBasedAnimTickOption = EVisibilityBasedAnimTickOption::OnlyTickPoseWhenRendered;

				// 추가로, 굳이 애니메이션이 필요 없는 컴포넌트라면 애니메이션 모드를 아예 끕니다.
				MeshComp->SetAnimationMode(EAnimationMode::AnimationCustomMode);
			}
		}
	}
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
		return;
	}

	UAbilitySystemComponent* ASC = PS->GetAbilitySystemComponent();
	if (!ASC)
	{
		return;
	}

	// 이미 ActorInfo가 설정되어 있다면 불필요한 재설정 방지
	if (ASC->GetAvatarActor() == this)
	{
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
}

void APlayerCharacterBase::InitializePlayerUI()
{
	ADG_PlayerState* PS = GetPlayerState<ADG_PlayerState>();
	if (!PS)
	{
		return;
	}

	UAbilitySystemComponent* ASC = PS->GetAbilitySystemComponent();
	if (!ASC)
	{
		return;
	}

	// 로컬 플레이어 컨트롤러인지 확인 (화면에 UI를 띄워야 하는 유저만)
	if (APlayerController* PC = Cast<APlayerController>(GetController()))
	{
		if (PC->IsLocalPlayerController())
		{
			if (ADG_HUD* HUD = Cast<ADG_HUD>(PC->GetHUD()))
			{
				// HUD의 InitOverlay 함수 호출 (컨트롤러, State, ASC, 속성 데이터 전달)
				HUD->InitOverlay(PC, PS, ASC, PS->GetDGAttributeSet());
				UE_LOG(LogTemp, Log, TEXT("[PlayerCharacterBase] Player UI initialized on local player."));
			}
		}
	}
}

void APlayerCharacterBase::PawnClientRestart()
{
	Super::PawnClientRestart();

	// 클라이언트에서 Controller가 Pawn에 할당된 직후, 다시 한번 ActorInfo를 업데이트합니다.
	if (UAbilitySystemComponent* ASC = GetCharacterAbilitySystemComponent())
	{
		ADG_PlayerState* PS = GetPlayerState<ADG_PlayerState>();
		if (PS)
		{
			ASC->InitAbilityActorInfo(PS, this);
		}
	}

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
		EnhancedInputComponent->BindAction(IA_Move, ETriggerEvent::Triggered, this, &APlayerCharacterBase::MoveAction);
		EnhancedInputComponent->BindAction(IA_Move, ETriggerEvent::Completed, this, &APlayerCharacterBase::MoveAction);
	}

	if (IA_Look)
	{
		EnhancedInputComponent->BindAction(IA_Look, ETriggerEvent::Triggered, this, &APlayerCharacterBase::LookAction);
	}

	if (IA_Jump)
	{
		EnhancedInputComponent->BindAction(IA_Jump, ETriggerEvent::Started, this, &ACharacter::Jump);
		EnhancedInputComponent->BindAction(IA_Jump, ETriggerEvent::Completed, this, &ACharacter::StopJumping);
	}

	if (IA_Shift)
	{
		// Started: 버튼을 누르는 순간 즉시 Dodge 발동
		EnhancedInputComponent->BindAction(IA_Shift, ETriggerEvent::Started, this,
			&APlayerCharacterBase::ShiftActionStarted);
		// Completed: 버튼을 떼더라도 질주가 유지되게 하려면 여기서 질주를 끄지 않음
	}

	// Skill Mapping
	if (IA_Skill_1)
	{
		EnhancedInputComponent->BindAction(IA_Skill_1, ETriggerEvent::Started, this,
			&APlayerCharacterBase::OnSkillInputStarted,
			DGGameplayTags::Input_Slot_1.GetTag());
		EnhancedInputComponent->BindAction(IA_Skill_1, ETriggerEvent::Completed, this,
			&APlayerCharacterBase::OnSkillInputCompleted,
			DGGameplayTags::Input_Slot_1.GetTag());
	}

	if (IA_Skill_2)
	{
		EnhancedInputComponent->BindAction(IA_Skill_2, ETriggerEvent::Started, this,
			&APlayerCharacterBase::OnSkillInputStarted,
			DGGameplayTags::Input_Slot_2.GetTag());
		EnhancedInputComponent->BindAction(IA_Skill_2, ETriggerEvent::Completed, this,
			&APlayerCharacterBase::OnSkillInputCompleted,
			DGGameplayTags::Input_Slot_2.GetTag());
	}

	if (IA_Skill_3)
	{
		EnhancedInputComponent->BindAction(IA_Skill_3, ETriggerEvent::Started, this,
			&APlayerCharacterBase::OnSkillInputStarted,
			DGGameplayTags::Input_Slot_3.GetTag());
		EnhancedInputComponent->BindAction(IA_Skill_3, ETriggerEvent::Completed, this,
			&APlayerCharacterBase::OnSkillInputCompleted,
			DGGameplayTags::Input_Slot_3.GetTag());
	}

	if (IA_Skill_4)
	{
		EnhancedInputComponent->BindAction(IA_Skill_4, ETriggerEvent::Started, this,
			&APlayerCharacterBase::OnSkillInputStarted,
			DGGameplayTags::Input_Slot_4.GetTag());
		EnhancedInputComponent->BindAction(IA_Skill_4, ETriggerEvent::Completed, this,
			&APlayerCharacterBase::OnSkillInputCompleted,
			DGGameplayTags::Input_Slot_4.GetTag());
	}

	if (IA_Skill_Q)
	{
		EnhancedInputComponent->BindAction(IA_Skill_Q, ETriggerEvent::Started, this,
			&APlayerCharacterBase::OnSkillInputStarted,
			DGGameplayTags::Input_Slot_Q.GetTag());
		EnhancedInputComponent->BindAction(IA_Skill_Q, ETriggerEvent::Completed, this,
			&APlayerCharacterBase::OnSkillInputCompleted,
			DGGameplayTags::Input_Slot_Q.GetTag());
	}

	if (IA_Skill_E)
	{
		EnhancedInputComponent->BindAction(IA_Skill_E, ETriggerEvent::Started, this,
			&APlayerCharacterBase::OnSkillInputStarted,
			DGGameplayTags::Input_Slot_E.GetTag());
		EnhancedInputComponent->BindAction(IA_Skill_E, ETriggerEvent::Completed, this,
			&APlayerCharacterBase::OnSkillInputCompleted,
			DGGameplayTags::Input_Slot_E.GetTag());
	}

	// UI 토글 (맵, 인벤토리)
	if (IA_ToggleMap)
	{
		EnhancedInputComponent->BindAction(IA_ToggleMap, ETriggerEvent::Started, this,
			&APlayerCharacterBase::ToggleMapAction);
	}

	if (IA_ToggleInventory)
	{
		EnhancedInputComponent->BindAction(IA_ToggleInventory, ETriggerEvent::Started, this,
			&APlayerCharacterBase::ToggleInventoryAction);
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

	// 2. 서버에서 기본 GameplayEffect(회복 등) 부여
	// + 서버 단일 실행 보장
	if (HasAuthority())
	{
		InitializePlayerStateFromClassData();
		InitializeMovementStats();
		InitializeSkillSlotsFromClassData();

		// [수정] 중복 부여를 방지하기 위해 ASC에 이미 스킬이 있는지 확인
		UAbilitySystemComponent* ASC = GetCharacterAbilitySystemComponent();
		if (ASC && ASC->GetActivatableAbilities().Num() == 0)
		{
			GrantDefaultAbilities();
			GrantClassSkillAbilities();

			ApplyDefaultEffects();
		}
	}

	// 플레이어 UI 초기화 (로컬 플레이어만)
	InitializePlayerUI();
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
	InitializeMovementStats();
	InitializeSkillSlotsFromClassData();

	// 플레이어 UI 초기화 (로컬 플레이어만)
	InitializePlayerUI();
}

void APlayerCharacterBase::InitializePlayerStateFromClassData()
{
	if (!HasAuthority())
	{
		return;
	}

	if (!CharacterClassData)
	{
		return;
	}

	ADG_PlayerState* PS = GetPlayerState<ADG_PlayerState>();
	if (!PS)
	{
		return;
	}

	PS->InitializePlayerDataFromClassData(CharacterClassData);
}

void APlayerCharacterBase::InitializeSkillSlotsFromClassData()
{
	SkillSlotMapping.Empty();

	if (!CharacterClassData)
	{
		return;
	}

	for (const FSkillSlotDefinition& SkillSlot : CharacterClassData->SkillSlots)
	{
		if (!SkillSlot.SlotTag.IsValid())
		{
			continue;
		}

		if (!SkillSlot.SkillData)
		{
			continue;
		}

		if (!SkillSlot.SkillData->SkillTag.IsValid())
		{
			continue;
		}

		SkillSlotMapping.Add(SkillSlot.SlotTag, SkillSlot.SkillData->SkillTag);
	}
}

void APlayerCharacterBase::GrantClassSkillAbilities()
{
	if (!HasAuthority())
	{
		return;
	}

	UAbilitySystemComponent* ASC = GetCharacterAbilitySystemComponent();
	if (!ASC)
	{
		return;
	}

	if (!CharacterClassData)
	{
		return;
	}

	const ADG_PlayerState* PS = GetPlayerState<ADG_PlayerState>();
	const int32 CurrentLevel = PS ? PS->GetCharacterLevel() : 1;

	for (const FSkillSlotDefinition& SkillSlot : CharacterClassData->SkillSlots)
	{
		if (!SkillSlot.SkillData)
		{
			continue;
		}

		if (!SkillSlot.SkillData->AbilityClass)
		{
			continue;
		}

		if (SkillSlot.UnlockLevel > CurrentLevel)
		{
			continue;
		}

		FGameplayAbilitySpec AbilitySpec(
			SkillSlot.SkillData->AbilityClass,
			1,
			INDEX_NONE,
			SkillSlot.SkillData
		);

		ASC->GiveAbility(AbilitySpec);
	}
}

void APlayerCharacterBase::GrantDefaultAbilities()
{
	if (!HasAuthority())
	{
		return;
	}

	UAbilitySystemComponent* ASC = GetCharacterAbilitySystemComponent();
	if (!ASC || !CharacterClassData)
	{
		return;
	}

	for (const auto& AbilityClass : CharacterClassData->StartupAbilities)
	{
		if (AbilityClass)
		{
			ASC->GiveAbility(FGameplayAbilitySpec(AbilityClass, 1));
		}
	}
}

void APlayerCharacterBase::ApplyDefaultEffects()
{
	if (!HasAuthority())
	{
		return;
	}

	UAbilitySystemComponent* ASC = GetCharacterAbilitySystemComponent();
	if (!ASC || !CharacterClassData)
	{
		return;
	}

	FGameplayEffectContextHandle Context = ASC->MakeEffectContext();
	Context.AddSourceObject(this);

	for (const auto& EffectClass : CharacterClassData->StartupEffects)
	{
		if (EffectClass)
		{
			FGameplayEffectSpecHandle Spec = ASC->MakeOutgoingSpec(EffectClass, 1.f, Context);
			if (Spec.IsValid())
			{
				ASC->ApplyGameplayEffectSpecToSelf(*Spec.Data.Get());
			}
		}
	}
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
		return;
	}

	const FVector MoveDirection =
		GetCameraForwardOnPlane() * CurrentMoveInput.Y +
		GetCameraRightOnPlane() * CurrentMoveInput.X;

	AddMovementInput(MoveDirection.GetSafeNormal());
}

void APlayerCharacterBase::ShiftActionStarted()
{
	UAbilitySystemComponent* ASC = GetCharacterAbilitySystemComponent();
	if (!ASC || IsDead())
	{
		return;
	}

	const UDG_AttributeSet* PlayerAttributeSet = GetPlayerDGAttributeSet();
	if (!PlayerAttributeSet)
	{
		return;
	}

	// [핵심] 클라이언트에서 먼저 스태미나가 충분한지 확인합니다.
	// (GE_Player_Dodge_Cost의 소모량을 하드코딩하거나, 데이터 에셋에서 가져와 비교)
	const float DodgeCost = 10.f;
	if (PlayerAttributeSet->GetStamina() < DodgeCost)
	{
		// 스태미나가 부족하면 애니메이션도 안 틀고 서버에 요청도 안 보냅니다.
		return;
	}

	const FVector DesiredDir = GetDesiredMoveDirection();
	const bool bHasInput = !CurrentMoveInput.IsNearlyZero();

	// 1. 여기서 이벤트를 발생시키면, 클라이언트 GAS는 '스태미나가 깎일 것'이라고 믿고 애니메이션을 틉니다.
	SendDodgeEvent(DesiredDir, bHasInput);
}

void APlayerCharacterBase::SendDodgeEvent(FVector Direction, bool bHasInput)
{
	UAbilitySystemComponent* ASC = GetCharacterAbilitySystemComponent();
	if (!ASC)
	{
		return;
	}

	FGameplayEventData Payload;
	Payload.EventTag = FGameplayTag::RequestGameplayTag(TEXT("Skill.Common.Dodge"));

	if (bHasInput)
	{
		FGameplayAbilityTargetData_LocationInfo* LocData = new FGameplayAbilityTargetData_LocationInfo();
		LocData->TargetLocation.LiteralTransform.SetLocation(Direction);
		Payload.TargetData.Add(LocData);
	}

	ASC->HandleGameplayEvent(Payload.EventTag, &Payload);
}

FVector APlayerCharacterBase::GetCameraForwardOnPlane() const
{
	FVector Forward = FollowCam ? FollowCam->GetForwardVector() : FVector::ForwardVector;
	Forward.Z = 0.f;

	return Forward.GetSafeNormal();
}

FVector APlayerCharacterBase::GetCameraRightOnPlane() const
{
	FVector Right = FollowCam ? FollowCam->GetRightVector() : FVector::RightVector;
	Right.Z = 0.f;

	return Right.GetSafeNormal();
}

FVector APlayerCharacterBase::GetDesiredMoveDirection() const
{
	const FVector Direction =
		GetCameraForwardOnPlane() * CurrentMoveInput.Y +
		GetCameraRightOnPlane() * CurrentMoveInput.X;

	if (Direction.IsNearlyZero())
	{
		FVector Forward = GetActorForwardVector();
		Forward.Z = 0.f;
		return Forward.GetSafeNormal();
	}

	return Direction.GetSafeNormal();
}

// 맵 UI를 호출하는 토글 함수
void APlayerCharacterBase::ToggleMapAction()
{
	// 로컬 플레이어인지 확인 (자신의 UI만 컨트롤)
	if (APlayerController* PC = Cast<APlayerController>(GetController()))
	{
		if (ADG_HUD* HUD = Cast<ADG_HUD>(PC->GetHUD()))
		{
			HUD->ToggleMapWidget();
		}
	}
}

// 인벤토리 UI를 호출하는 토글 함수
void APlayerCharacterBase::ToggleInventoryAction()
{
	// 로컬 플레이어인지 확인 (자신의 UI만 컨트롤)
	if (APlayerController* PC = Cast<APlayerController>(GetController()))
	{
		if (ADG_HUD* HUD = Cast<ADG_HUD>(PC->GetHUD()))
		{
			HUD->ToggleInventoryWidget();
		}
	}
}

void APlayerCharacterBase::OnSkillInputStarted(FGameplayTag SlotTag)
{
	HeldSkillSlots.FindOrAdd(SlotTag) = true;

	if (!HasAuthority())
	{
		ServerSetSkillInputHeld(SlotTag, true);
	}

	UAbilitySystemComponent* ASC = GetCharacterAbilitySystemComponent();
	if (!ASC)
	{
		return;
	}

	const FGameplayTag SkillTag = GetSkillTagForSlot(SlotTag);

	if (SkillTag.IsValid())
	{
		ASC->TryActivateAbilitiesByTag(FGameplayTagContainer(SkillTag));

		const FGameplayTag SkillInputEventTag = GetSkillInputEventTag(SkillTag);

		if (SkillInputEventTag.IsValid())
		{
			SendSkillInputStartedEvent(SkillInputEventTag);

			if (!HasAuthority())
			{
				ServerSendSkillInputStartedEvent(SkillInputEventTag);
			}
		}
	}
}

void APlayerCharacterBase::OnSkillInputCompleted(FGameplayTag SlotTag)
{
	HeldSkillSlots.FindOrAdd(SlotTag) = false;

	if (!HasAuthority())
	{
		ServerSetSkillInputHeld(SlotTag, false);
	}

	const FGameplayTag SkillTag = GetSkillTagForSlot(SlotTag);
	if (SkillTag.IsValid())
	{
		const FString Msg = FString::Printf(
			TEXT("Skill Input Completed: %s -> Ability: %s"),
			*SlotTag.ToString(),
			*SkillTag.ToString()
		);
	}
}

void APlayerCharacterBase::ServerSendSkillInputStartedEvent_Implementation(FGameplayTag SkillInputEventTag)
{
	SendSkillInputStartedEvent(SkillInputEventTag);
}

void APlayerCharacterBase::SendSkillInputStartedEvent(FGameplayTag SkillInputEventTag)
{
	if (!SkillInputEventTag.IsValid())
	{
		return;
	}

	FGameplayEventData Payload;
	Payload.EventTag = SkillInputEventTag;
	Payload.Instigator = this;

	UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(this, SkillInputEventTag, Payload);
}

void APlayerCharacterBase::ServerSetSkillInputHeld_Implementation(FGameplayTag SlotTag, bool bHeld)
{
	HeldSkillSlots.FindOrAdd(SlotTag) = bHeld;
}

bool APlayerCharacterBase::IsSkillSlotHeld(FGameplayTag SlotTag) const
{
	const bool* bHeld = HeldSkillSlots.Find(SlotTag);
	return bHeld && *bHeld;
}

bool APlayerCharacterBase::IsSkillTagHeld(FGameplayTag SkillTag) const
{
	if (!SkillTag.IsValid())
	{
		return false;
	}

	for (const TPair<FGameplayTag, FGameplayTag>& SkillSlotPair : SkillSlotMapping)
	{
		if (SkillSlotPair.Value == SkillTag)
		{
			return IsSkillSlotHeld(SkillSlotPair.Key);
		}
	}

	return false;
}

FGameplayTag APlayerCharacterBase::GetSkillTagForSlot(FGameplayTag SlotTag) const
{
	if (SkillSlotMapping.Contains(SlotTag))
	{
		return SkillSlotMapping[SlotTag];
	}

	return FGameplayTag::EmptyTag;
}

FGameplayTag APlayerCharacterBase::GetSkillInputEventTag(FGameplayTag SkillTag) const
{
	if (!SkillTag.IsValid() || !CharacterClassData)
	{
		return FGameplayTag::EmptyTag;
	}

	for (const FSkillSlotDefinition& SkillSlot : CharacterClassData->SkillSlots)
	{
		if (!SkillSlot.SkillData)
		{
			continue;
		}

		if (SkillSlot.SkillData->SkillTag == SkillTag)
		{
			return SkillSlot.SkillData->InputEventTag;
		}
	}

	return FGameplayTag::EmptyTag;
}


void APlayerCharacterBase::ClientDrawAttackTraceDebug_Implementation(
	FVector_NetQuantize Start,
	FVector_NetQuantize End,
	float Radius,
	FColor Color,
	float Duration
)
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	DrawDebugSphere(World, Start, Radius, 12, Color, false, Duration);
	DrawDebugSphere(World, End, Radius, 12, Color, false, Duration);
	DrawDebugLine(World, Start, End, Color, false, Duration, 0, 1.5f);
}

void APlayerCharacterBase::ClientDrawAttackBoxDebug_Implementation(
	FVector_NetQuantize Center,
	FVector_NetQuantize BoxHalfExtent,
	FRotator BoxRotation,
	FColor Color,
	float Duration
)
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	DrawDebugBox(
		World,
		Center,
		BoxHalfExtent,
		BoxRotation.Quaternion(),
		Color,
		false,
		Duration
	);
}

void APlayerCharacterBase::InitializeMovementStats()
{
	if (!CharacterClassData || !CharacterClassData->MovementData)
	{
		return;
	}

	UCharacterMovementComponent* MoveComp = GetCharacterMovement();
	if (!MoveComp)
	{
		return;
	}

	// 데이터 에셋의 수치를 CMC에 적용
	MoveComp->MaxWalkSpeed = CharacterClassData->MovementData->WalkSpeed;
	MoveComp->RotationRate = CharacterClassData->MovementData->RotationRate;
	MoveComp->JumpZVelocity = CharacterClassData->MovementData->JumpZVelocity;
	MoveComp->AirControl = CharacterClassData->MovementData->AirControl;
}

const FPlayerMovementAnimationSet& APlayerCharacterBase::GetCurrentMovementAnims() const
{
	UAbilitySystemComponent* ASC = GetCharacterAbilitySystemComponent();
	// 전투 상태 태그 확인 (태그 이름은 기획서에 따라 수정)
	// if (ASC && ASC->HasMatchingGameplayTag(FGameplayTag::RequestGameplayTag(TEXT("State.Movement.Combat"))))
	// {
	// 	return CharacterClassData->CombatAnims;
	// }

	return CharacterClassData->StandardAnims;
}

void APlayerCharacterBase::ServerHandleShiftAction_Implementation(
	FVector_NetQuantizeNormal DodgeDirection,
	bool bHasInput
)
{
	UAbilitySystemComponent* ASC = GetCharacterAbilitySystemComponent();
	if (!ASC)
	{
		return;
	}

	FGameplayEventData Payload;
	Payload.EventTag = FGameplayTag::RequestGameplayTag(TEXT("Skill.Common.Dodge"));

	if (bHasInput)
	{
		FGameplayAbilityTargetData_LocationInfo* LocData = new FGameplayAbilityTargetData_LocationInfo();
		LocData->TargetLocation.LiteralTransform.SetLocation(DodgeDirection);
		Payload.TargetData.Add(LocData);
	}

	// 이 호출이 성공하면 서버 로그에 어빌리티 시작 메시지가 떠야 합니다.
	ASC->HandleGameplayEvent(Payload.EventTag, &Payload);
}

void APlayerCharacterBase::Server_TestApplyDamage_Implementation()
{
	if (!HasAuthority())
	{
		return;
	}

	UCombatComponent* SourceCombatComponent = GetCombatComponent();
	if (!SourceCombatComponent)
	{
		return;
	}

	ABaseCharacter* BestTarget = nullptr;
	float BestDistanceSq = TNumericLimits<float>::Max();

	TArray<AActor*> FoundActors;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), ABaseCharacter::StaticClass(), FoundActors);

	for (AActor* Actor : FoundActors)
	{
		ABaseCharacter* Candidate = Cast<ABaseCharacter>(Actor);
		if (!Candidate)
		{
			continue;
		}

		if (Candidate == this)
		{
			continue;
		}

		if (Candidate->IsDead())
		{
			continue;
		}

		if (IsFriendlyTo(Candidate))
		{
			continue;
		}

		const float DistanceSq = FVector::DistSquared(GetActorLocation(), Candidate->GetActorLocation());
		constexpr float MaxTestDamageRange = 3000.f;

		if (DistanceSq > FMath::Square(MaxTestDamageRange))
		{
			continue;
		}

		if (DistanceSq < BestDistanceSq)
		{
			BestDistanceSq = DistanceSq;
			BestTarget = Candidate;
		}
	}

	if (!BestTarget)
	{
		return;
	}

	FDGDamageRequest DamageRequest;
	DamageRequest.SourceActor = this;
	DamageRequest.TargetActor = BestTarget;
	DamageRequest.BaseDamage = 100.f;
	DamageRequest.SourceTag = DGGameplayTags::Input_Slot_1;
	DamageRequest.HitLocation = BestTarget->GetActorLocation();
	DamageRequest.bHasHitLocation = true;

	const FDGDamageResult DamageResult = SourceCombatComponent->ApplyDamageRequest(DamageRequest);
}