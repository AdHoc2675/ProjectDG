

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
#include "Character/Player/Data/PlayerCharacterClassData.h"
#include "Perception/AIPerceptionStimuliSourceComponent.h"
#include "Perception/AISense_Sight.h"

#include "UI/HUD/DG_HUD.h"

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
			MeshComp->SetupAttachment(GetMesh()); // 애니메이션 동기화를 위해 보통 메인 메쉬에 부착합니다.
		}
	}
	
	//AI관련 StimuliSourceComponent
	StimuliSourceComponent = CreateDefaultSubobject<UAIPerceptionStimuliSourceComponent>(TEXT("StimuliSourceComponent"));
	
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

	// 컨트롤러(마우스) 회전이 캐릭터 몸통에 직접 영향을 주지 않게끔 설정
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
	// ApplyMovementData();
	// ApplyCurrentMovementSpeed();
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

	// 이미 ActorInfo가 설정되어 있다면 불필요한 재설정 방지
	if (ASC->GetAvatarActor() == this) return;
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

void APlayerCharacterBase::InitializePlayerUI()
{
	ADG_PlayerState* PS = GetPlayerState<ADG_PlayerState>();
	if (!PS) return;

	UAbilitySystemComponent* ASC = PS->GetAbilitySystemComponent();
	if (!ASC) return;

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
	
	if (IA_Shift)
	{
		// Started: 버튼을 누르는 순간 즉시 Dodge 발동
		EnhancedInputComponent->BindAction(IA_Shift, ETriggerEvent::Started, this, &APlayerCharacterBase::ShiftActionStarted);
		// Completed: 버튼을 떼더라도 질주가 유지되게 하려면 여기서 질주를 끄지 않음
	}
	
	// Skill Mapping
	if (IA_Skill_1) EnhancedInputComponent->BindAction(IA_Skill_1, ETriggerEvent::Started, this, &APlayerCharacterBase::OnSkillInput_1);
	if (IA_Skill_2) EnhancedInputComponent->BindAction(IA_Skill_2, ETriggerEvent::Started, this, &APlayerCharacterBase::OnSkillInput_2);
	if (IA_Skill_3) EnhancedInputComponent->BindAction(IA_Skill_3, ETriggerEvent::Started, this, &APlayerCharacterBase::OnSkillInput_3);
	if (IA_Skill_4) EnhancedInputComponent->BindAction(IA_Skill_4, ETriggerEvent::Started, this, &APlayerCharacterBase::OnSkillInput_4);
	if (IA_Skill_Q) EnhancedInputComponent->BindAction(IA_Skill_Q, ETriggerEvent::Started, this, &APlayerCharacterBase::OnSkillInput_Q);
	if (IA_Skill_E) EnhancedInputComponent->BindAction(IA_Skill_E, ETriggerEvent::Started, this, &APlayerCharacterBase::OnSkillInput_E);
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
	
	// 로그에 "누가" 빙의를 요청하는지 출력 (Warrior에서 GA 및 GE의 이중 적용 문제)
	Debug::Print(FString::Printf(TEXT("PossessedBy: Controller: %s, Pawn: %s"), 
		*NewController->GetName(), *GetName()), FColor::Red);

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
		Debug::Print(TEXT("[PlayerCharacterBase] CharacterClassData is null."));
		return;
	}

	ADG_PlayerState* PS = GetPlayerState<ADG_PlayerState>();
	if (!PS)
	{
		Debug::Print(TEXT("[PlayerCharacterBase] DG_PlayerState is null."));
		return;
	}

	PS->InitializePlayerDataFromClassData(CharacterClassData);
}

void APlayerCharacterBase::InitializeSkillSlotsFromClassData()
{
	SkillSlotMapping.Empty();

	if (!CharacterClassData)
	{
		Debug::Print(TEXT("[PlayerCharacterBase] CharacterClassData is null. Skill slots skipped."));
		return;
	}

	for (const FSkillSlotDefinition& SkillSlot : CharacterClassData->SkillSlots)
	{
		if (!SkillSlot.SlotTag.IsValid() || !SkillSlot.SkillTag.IsValid())
		{
			continue;
		}

		SkillSlotMapping.Add(SkillSlot.SlotTag, SkillSlot.SkillTag);
	}
}

void APlayerCharacterBase::GrantClassSkillAbilities()
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

	const ADG_PlayerState* PS = GetPlayerState<ADG_PlayerState>();
	const int32 CurrentLevel = PS ? PS->GetCharacterLevel() : 1;

	for (const FSkillSlotDefinition& SkillSlot : CharacterClassData->SkillSlots)
	{
		if (!SkillSlot.AbilityClass)
		{
			continue;
		}

		if (SkillSlot.UnlockLevel > CurrentLevel)
		{
			continue;
		}

		ASC->GiveAbility(FGameplayAbilitySpec(SkillSlot.AbilityClass, 1));
	}
}

void APlayerCharacterBase::GrantDefaultAbilities()
{
	if (!HasAuthority()) return;

	UAbilitySystemComponent* ASC = GetCharacterAbilitySystemComponent();
	if (!ASC || !CharacterClassData) return;

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
	if (!HasAuthority()) return;

	UAbilitySystemComponent* ASC = GetCharacterAbilitySystemComponent();
	if (!ASC || !CharacterClassData) return;

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
		// if (bIsSprinting)
		// {
		// 	// 서버에 질주 종료 알림
		// 	ServerSetSprinting(false, FVector::ZeroVector);
		// 	
		// 	//SprintCompleted();
		// }
		
		return;
	}
	
	const FVector MoveDirection = GetCameraForwardOnPlane() * CurrentMoveInput.Y + GetCameraRightOnPlane() * CurrentMoveInput.X;
	AddMovementInput(MoveDirection.GetSafeNormal());
}

void APlayerCharacterBase::ShiftActionStarted()
{
	UAbilitySystemComponent* ASC = GetCharacterAbilitySystemComponent();
	if (!ASC || IsDead()) return;

	// [핵심] 클라이언트에서 먼저 스태미나가 충분한지 확인합니다.
	// (GE_Player_Dodge_Cost의 소모량을 하드코딩하거나, 데이터 에셋에서 가져와 비교)
	float DodgeCost = 10.f; // 예시 수치
	if (GetPlayerDGAttributeSet()->GetStamina() < DodgeCost)
	{
		// 스태미나가 부족하면 애니메이션도 안 틀고 서버에 요청도 안 보냅니다.
		return;
	}

	FVector DesiredDir = GetDesiredMoveDirection();
	bool bHasInput = !CurrentMoveInput.IsNearlyZero();

	// 1. 여기서 이벤트를 발생시키면, 클라이언트 GAS는 '스태미나가 깎일 것'이라고 믿고 애니메이션을 틉니다.
	SendDodgeEvent(DesiredDir, bHasInput);
}

void APlayerCharacterBase::SendDodgeEvent(FVector Direction, bool bHasInput)
{
	UAbilitySystemComponent* ASC = GetCharacterAbilitySystemComponent();
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
	FVector Forward = (FollowCam ? FollowCam->GetForwardVector() : FVector::ForwardVector);
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
	const FVector Direction = GetCameraForwardOnPlane() * CurrentMoveInput.Y + GetCameraRightOnPlane() * CurrentMoveInput.X;
	
	if (Direction.IsNearlyZero())
	{
		FVector Forward = GetActorForwardVector();
		Forward.Z = 0.f;
		return Forward.GetSafeNormal();
	}
	return Direction.GetSafeNormal();
}

void APlayerCharacterBase::OnSkillInput(FGameplayTag SlotTag)
{
	UAbilitySystemComponent* ASC = GetCharacterAbilitySystemComponent();
	if (!ASC) return;

	FGameplayTag SkillTag = GetSkillTagForSlot(SlotTag);
	if (SkillTag.IsValid())
	{
		// 태그 기반으로 스킬 활성화
		ASC->TryActivateAbilitiesByTag(FGameplayTagContainer(SkillTag));
		
		// [수정된 부분] FString::Printf 결과를 FString 변수에 먼저 담아서 넘깁니다.
		FString Msg = FString::Printf(TEXT("Slot Input: %s -> Ability: %s"), *SlotTag.ToString(), *SkillTag.ToString());
		Debug::Print(Msg, FColor::Green);
	}
}

FGameplayTag APlayerCharacterBase::GetSkillTagForSlot(FGameplayTag SlotTag) const
{
	if (SkillSlotMapping.Contains(SlotTag))
	{
		return SkillSlotMapping[SlotTag];
	}
	return FGameplayTag::EmptyTag;
}

void APlayerCharacterBase::OnSkillInput_1()
{
	OnSkillInput(DGGameplayTags::Input_Slot_1);
}

void APlayerCharacterBase::OnSkillInput_2()
{
	OnSkillInput(DGGameplayTags::Input_Slot_2);
}

void APlayerCharacterBase::OnSkillInput_3()
{
	OnSkillInput(DGGameplayTags::Input_Slot_3);
}

void APlayerCharacterBase::OnSkillInput_4()
{
	OnSkillInput(DGGameplayTags::Input_Slot_4);
}

void APlayerCharacterBase::OnSkillInput_Q()
{
	OnSkillInput(DGGameplayTags::Input_Slot_Q);
}

void APlayerCharacterBase::OnSkillInput_E()
{
	OnSkillInput(DGGameplayTags::Input_Slot_E);
}

void APlayerCharacterBase::InitializeMovementStats()
{
	if (!CharacterClassData || !CharacterClassData->MovementData) return;

	UCharacterMovementComponent* MoveComp = GetCharacterMovement();
	if (!MoveComp) return;

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

void APlayerCharacterBase::ServerHandleShiftAction_Implementation(FVector_NetQuantizeNormal DodgeDirection, bool bHasInput)
{
	// 서버 로그 추가
	UE_LOG(LogTemp, Warning, TEXT("Server: Received Shift Action RPC. Direction: %s"), *DodgeDirection.ToString());

	UAbilitySystemComponent* ASC = GetCharacterAbilitySystemComponent();
	if (!ASC) return;

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
