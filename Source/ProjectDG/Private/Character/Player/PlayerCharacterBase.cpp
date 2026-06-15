#include "Character/Player/PlayerCharacterBase.h"
#include "Character/Player/Data/PlayerSkillData.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "Abilities/GameplayAbility.h"
#include "Camera/CameraComponent.h"
#include "Camera/PlayerCameraManager.h"
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
#include "Components/CapsuleComponent.h"
#include "Perception/AIPerceptionStimuliSourceComponent.h"
#include "Perception/AISense_Sight.h"

#include "UI/HUD/DG_HUD.h"
#include "UI/WidgetController/DGOverlayWidgetController.h"

#include "Components/UI/DGMinimapCaptureComponent.h"
#include "Components/UI/DGMinimapMarkerComponent.h"
#include "Components/Targeting/LockOnComponent.h"
#include "Components/Inventory/DGInventoryComponent.h"
#include "GameFramework/GameModeBase.h"
#include "Item/DGItemDefinition.h"
#include "Item/DGItemInstance.h"

namespace
{
	bool HasGrantedAbilityClass(UAbilitySystemComponent* ASC, TSubclassOf<UGameplayAbility> AbilityClass)
	{
		if (!ASC || !AbilityClass)
		{
			return false;
		}

		for (const FGameplayAbilitySpec& Spec : ASC->GetActivatableAbilities())
		{
			if (Spec.Ability && Spec.Ability->GetClass() == AbilityClass)
			{
				return true;
			}
		}

		return false;
	}

	int32 ClearGrantedClassSkillAbilities(UAbilitySystemComponent* ASC)
	{
		if (!ASC)
		{
			return 0;
		}

		TArray<FGameplayAbilitySpecHandle> HandlesToClear;

		for (const FGameplayAbilitySpec& Spec : ASC->GetActivatableAbilities())
		{
			if (Cast<UPlayerSkillData>(Spec.SourceObject.Get()))
			{
				HandlesToClear.Add(Spec.Handle);
			}
		}

		for (const FGameplayAbilitySpecHandle& Handle : HandlesToClear)
		{
			ASC->ClearAbility(Handle);
		}

		return HandlesToClear.Num();
	}
}


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

	InventoryComponent = CreateDefaultSubobject<UDGInventoryComponent>(TEXT("InventoryComponent"));

	MinimapMarkerComponent->MarkerType = EMinimapMarkerType::Player;
	MinimapMarkerComponent->bTrackRotation = true;

	// 네트워크 컬 거리(기본값은 225,000,000 = 약 150미터)를 엄청나게 크게 설정
	// 맵이 너무 넓거나 동접자가 높으면 연산량과 트래픽 패킷 낭비가 심해진다는 단점 있음
	// 근데 우린 동접자 4명이니까 아마 괜찮을듯
	SetNetCullDistanceSquared(1000000000000.0f); // 10km 반경
}

void APlayerCharacterBase::BeginPlay()
{
	Super::BeginPlay();

	if (UCapsuleComponent* Capsule = GetCapsuleComponent())
	{
		InitialCapsuleCollisionEnabled =
				Capsule->GetCollisionEnabled();
	}

	if (USkeletalMeshComponent* MeshComp = GetMesh())
	{
		InitialMeshCollisionEnabled =
				MeshComp->GetCollisionEnabled();
	}
	
	// 월드시작시 ASC초기화
	InitializePlayerAbilitySystem();

	// 인벤토리 이벤트 바인딩
	if (InventoryComponent)
	{
		InventoryComponent->OnEquipmentChanged.AddDynamic(this, &APlayerCharacterBase::OnEquipmentChanged);
	}
}

void APlayerCharacterBase::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
}

void APlayerCharacterBase::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	
	DOREPLIFETIME(APlayerCharacterBase,bPlayerDead);
}

void APlayerCharacterBase::Landed(const FHitResult& Hit)
{
	Super::Landed(Hit);
	
	FGameplayEventData Payload;
	Payload.EventTag = DGGameplayTags::Event_Movement_Jump_Landed;
	Payload.Instigator = this;

	UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(
			this,
			DGGameplayTags::Event_Movement_Jump_Landed,
			Payload
	);
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
		BindHealthChangeCameraShakeDelegate();
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

	BindHealthChangeCameraShakeDelegate();
}

void APlayerCharacterBase::BindHealthChangeCameraShakeDelegate()
{
	if (!HasAuthority() || bHealthChangeCameraShakeDelegateBound)
	{
		return;
	}

	UAbilitySystemComponent* ASC = GetCharacterAbilitySystemComponent();
	if (!ASC)
	{
		return;
	}

	ASC->GetGameplayAttributeValueChangeDelegate(UDG_AttributeSet::GetHealthAttribute())
		.AddUObject(this, &APlayerCharacterBase::OnHealthChanged);

	bHealthChangeCameraShakeDelegateBound = true;
}

void APlayerCharacterBase::OnHealthChanged(const FOnAttributeChangeData& Data)
{
	if (!HasAuthority())
	{
		return;
	}

	if (Data.NewValue >= Data.OldValue || bPlayerDead || IsDead())
	{
		return;
	}

	const UDG_AttributeSet* AttributeSet = GetPlayerDGAttributeSet();
	const float MaxHealth = AttributeSet ? AttributeSet->GetMaxHealth() : 0.0f;
	const float DamageRatio = MaxHealth > 0.0f ? (Data.OldValue - Data.NewValue) / MaxHealth : 0.0f;
	const float ShakeScale = FMath::Clamp(DamageRatio * DamageCameraShakeScale, 0.2f, 1.0f);

	ClientPlayDamageCameraShake(ShakeScale);
}

void APlayerCharacterBase::ClientPlayDamageCameraShake_Implementation(float ShakeScale)
{
	if (!DamageCameraShakeClass)
	{
		return;
	}

	APlayerController* PlayerController = Cast<APlayerController>(GetController());
	if (!PlayerController || !PlayerController->PlayerCameraManager)
	{
		return;
	}

	PlayerController->PlayerCameraManager->StartCameraShake(DamageCameraShakeClass, ShakeScale);
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

				// 락온 기능과 타겟 상태창 연동
				if (LockOnComponent)
				{
					LockOnComponent->OnLockOnTargetChanged.AddLambda([HUD](const FLockOnTargetResult& Result) {
						if (UDGOverlayWidgetController* Controller = HUD->GetOverlayWidgetController(FWidgetControllerParams()))
						{
							Controller->NotifyTargetChanged(Result.TargetActor);
						}
						});

					LockOnComponent->OnLockOnReleased.AddLambda([HUD](const FLockOnTargetResult& Result) {
						if (UDGOverlayWidgetController* Controller = HUD->GetOverlayWidgetController(FWidgetControllerParams()))
						{
							Controller->NotifyTargetChanged(nullptr);
						}
						});
				}
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
		EnhancedInputComponent->BindAction(IA_Jump, ETriggerEvent::Started, this, &APlayerCharacterBase::JumpActionStarted);
		EnhancedInputComponent->BindAction(IA_Jump, ETriggerEvent::Completed, this, &APlayerCharacterBase::JumpActionCompleted);
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
			DGGameplayTags::Input_SkillSlot_LeftMouse.GetTag());
		EnhancedInputComponent->BindAction(IA_Skill_1, ETriggerEvent::Completed, this,
			&APlayerCharacterBase::OnSkillInputCompleted,
			DGGameplayTags::Input_SkillSlot_LeftMouse.GetTag());
	}

	if (IA_Skill_2)
	{
		EnhancedInputComponent->BindAction(IA_Skill_2, ETriggerEvent::Started, this,
			&APlayerCharacterBase::OnSkillInputStarted,
			DGGameplayTags::Input_SkillSlot_RightMouse.GetTag());
		EnhancedInputComponent->BindAction(IA_Skill_2, ETriggerEvent::Completed, this,
			&APlayerCharacterBase::OnSkillInputCompleted,
			DGGameplayTags::Input_SkillSlot_RightMouse.GetTag());
	}

	if (IA_Skill_3)
	{
		EnhancedInputComponent->BindAction(IA_Skill_3, ETriggerEvent::Started, this,
			&APlayerCharacterBase::OnSkillInputStarted,
			DGGameplayTags::Input_SkillSlot_Key1.GetTag());
		EnhancedInputComponent->BindAction(IA_Skill_3, ETriggerEvent::Completed, this,
			&APlayerCharacterBase::OnSkillInputCompleted,
			DGGameplayTags::Input_SkillSlot_Key1.GetTag());
	}

	if (IA_Skill_4)
	{
		EnhancedInputComponent->BindAction(IA_Skill_4, ETriggerEvent::Started, this,
			&APlayerCharacterBase::OnSkillInputStarted,
			DGGameplayTags::Input_SkillSlot_Key2.GetTag());
		EnhancedInputComponent->BindAction(IA_Skill_4, ETriggerEvent::Completed, this,
			&APlayerCharacterBase::OnSkillInputCompleted,
			DGGameplayTags::Input_SkillSlot_Key2.GetTag());
	}

	if (IA_Skill_Q)
	{
		EnhancedInputComponent->BindAction(IA_Skill_Q, ETriggerEvent::Started, this,
			&APlayerCharacterBase::OnSkillInputStarted,
			DGGameplayTags::Input_SkillSlot_Key3.GetTag());
		EnhancedInputComponent->BindAction(IA_Skill_Q, ETriggerEvent::Completed, this,
			&APlayerCharacterBase::OnSkillInputCompleted,
			DGGameplayTags::Input_SkillSlot_Key3.GetTag());
	}

	if (IA_Skill_E)
	{
		EnhancedInputComponent->BindAction(IA_Skill_E, ETriggerEvent::Started, this,
			&APlayerCharacterBase::OnSkillInputStarted,
			DGGameplayTags::Input_SkillSlot_Key4.GetTag());
		EnhancedInputComponent->BindAction(IA_Skill_E, ETriggerEvent::Completed, this,
			&APlayerCharacterBase::OnSkillInputCompleted,
			DGGameplayTags::Input_SkillSlot_Key4.GetTag());
	}

	// 아이템 맵핑
	if (IA_HPPotion)
	{
		EnhancedInputComponent->BindAction(IA_HPPotion, ETriggerEvent::Started, this,
			&APlayerCharacterBase::UseHealthItem);
	}

	if (IA_MPPotion)
	{
		EnhancedInputComponent->BindAction(IA_MPPotion, ETriggerEvent::Started, this,
			&APlayerCharacterBase::UseMentalItem);
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

void APlayerCharacterBase::UseHealthItem()
{
	float CurrentTime = GetWorld()->GetTimeSeconds();
	if (CurrentTime - LastHealthItemUseTime < 10.0f && LastHealthItemUseTime > 0.0f)
	{
		return;
	}

	LastHealthItemUseTime = CurrentTime;

	if (APlayerController* PC = Cast<APlayerController>(GetController()))
	{
		if (ADG_HUD* HUD = Cast<ADG_HUD>(PC->GetHUD()))
		{
			if (UDGOverlayWidgetController* WC = HUD->GetOverlayWidgetController(FWidgetControllerParams()))
			{
				WC->OnHealthItemCooldown.Broadcast(10.0f);
			}
		}
	}

	Server_UseHealthItem();
}

void APlayerCharacterBase::UseMentalItem()
{
	float CurrentTime = GetWorld()->GetTimeSeconds();
	if (CurrentTime - LastMentalItemUseTime < 10.0f && LastMentalItemUseTime > 0.0f)
	{
		return;
	}

	LastMentalItemUseTime = CurrentTime;

	if (APlayerController* PC = Cast<APlayerController>(GetController()))
	{
		if (ADG_HUD* HUD = Cast<ADG_HUD>(PC->GetHUD()))
		{
			if (UDGOverlayWidgetController* WC = HUD->GetOverlayWidgetController(FWidgetControllerParams()))
			{
				WC->OnMentalItemCooldown.Broadcast(10.0f);
			}
		}
	}

	Server_UseMentalItem();
}

void APlayerCharacterBase::Server_UseHealthItem_Implementation()
{
	if (UAbilitySystemComponent* ASC = GetCharacterAbilitySystemComponent())
	{
		float MaxHealth = ASC->GetNumericAttribute(UDG_AttributeSet::GetMaxHealthAttribute());
		float CurrentHealth = ASC->GetNumericAttribute(UDG_AttributeSet::GetHealthAttribute());
		float HealAmount = MaxHealth * 0.3f; // 30% 회복
		float NewHealth = FMath::Min(MaxHealth, CurrentHealth + HealAmount);
		
		ASC->SetNumericAttributeBase(UDG_AttributeSet::GetHealthAttribute(), NewHealth);
	}
}

void APlayerCharacterBase::Server_UseMentalItem_Implementation()
{
	if (UAbilitySystemComponent* ASC = GetCharacterAbilitySystemComponent())
	{
		float MaxMental = ASC->GetNumericAttribute(UDG_AttributeSet::GetMaxMentalAttribute());
		float CurrentMental = ASC->GetNumericAttribute(UDG_AttributeSet::GetMentalAttribute());
		float HealAmount = MaxMental * 0.3f; // 30% 회복
		float NewMental = FMath::Min(MaxMental, CurrentMental + HealAmount);
		
		ASC->SetNumericAttributeBase(UDG_AttributeSet::GetMentalAttribute(), NewMental);
	}
}

void APlayerCharacterBase::HandleDeath()
{
	if (!HasAuthority())
	{
		return;
	}

	bPlayerDead = true;

	if (UAbilitySystemComponent* ASC =
			GetCharacterAbilitySystemComponent())
	{
		ASC->CancelAllAbilities();

		ASC->AddLooseGameplayTag(
				DGGameplayTags::State_Player_Dead,
				1,
				EGameplayTagReplicationState::TagOnly
		);

		ASC->AddLooseGameplayTag(
				DGGameplayTags::State_Movement_Locked,
				1,
				EGameplayTagReplicationState::TagOnly
		);
	}
	
	Super::HandleDeath();
	
	SendDeathEvent();

	GetWorldTimerManager().SetTimer(
			RespawnTimerHandle,
			this,
			&APlayerCharacterBase::RespawnPlayer,
			RespawnDelay,
			false
	);

	ForceNetUpdate();
}

void APlayerCharacterBase::SendDeathEvent()
{
	FGameplayEventData Payload;
	Payload.EventTag = DGGameplayTags::Event_Player_Death;
	Payload.Instigator = this;
	Payload.Target = this;

	UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(
			this,
			DGGameplayTags::Event_Player_Death,
			Payload
	);
}

void APlayerCharacterBase::RespawnPlayer()
{
	if (!HasAuthority() || !bPlayerDead)
      {
              return;
      }

      UAbilitySystemComponent* ASC =
              GetCharacterAbilitySystemComponent();

      if (ASC)
      {
              // Death GA 또는 남아 있는 Ability를 종료한다.
              ASC->CancelAllAbilities();

              if (const UDG_AttributeSet* Attributes =
                      GetPlayerDGAttributeSet())
              {
                      ASC->SetNumericAttributeBase(
                              UDG_AttributeSet::GetHealthAttribute(),
                              Attributes->GetMaxHealth()
                      );

                      ASC->SetNumericAttributeBase(
                              UDG_AttributeSet::GetMentalAttribute(),
                              Attributes->GetMaxMental()
                      );

                      ASC->SetNumericAttributeBase(
                              UDG_AttributeSet::GetStaminaAttribute(),
                              Attributes->GetMaxStamina()
                      );
              }
      }

      AGameModeBase* GameMode =
              GetWorld() ? GetWorld()->GetAuthGameMode() : nullptr;

      AActor* RespawnPoint = GameMode
              ? GameMode->FindPlayerStart(
                      GetController(),
                      RespawnPlayerStartTag.ToString()
              )
              : nullptr;

      if (RespawnPoint)
      {
              SetActorLocationAndRotation(
                      RespawnPoint->GetActorLocation(),
                      RespawnPoint->GetActorRotation(),
                      false,
                      nullptr,
                      ETeleportType::TeleportPhysics
              );
      }

      if (ASC)
      {
              ASC->RemoveLooseGameplayTag(
                      DGGameplayTags::State_Player_Dead,
                      1
              );

              ASC->RemoveLooseGameplayTag(
                      DGGameplayTags::State_Movement_Locked,
                      1
              );
      }

      // BaseCharacter::Die()에서 설정된 서버 사망 상태
      bIsDead = false;

      // 클라이언트에 복제되는 플레이어 사망 상태
      bPlayerDead = false;

      RestorePlayerAfterRespawn();
      ForceNetUpdate();
}

void APlayerCharacterBase::RestorePlayerAfterRespawn()
{
	if (UCapsuleComponent* Capsule = GetCapsuleComponent())
	{
		Capsule->SetCollisionEnabled(
				InitialCapsuleCollisionEnabled
		);
	}

	if (USkeletalMeshComponent* MeshComp = GetMesh())
	{
		MeshComp->SetCollisionEnabled(
				InitialMeshCollisionEnabled
		);
	}

	if (UCharacterMovementComponent* Movement =
			GetCharacterMovement())
	{
		Movement->StopMovementImmediately();
		Movement->SetMovementMode(MOVE_Walking);
	}
}

void APlayerCharacterBase::OnRep_PlayerDead()
{
	if (bPlayerDead)
	{
		DisableCharacterAfterDeath();
	}
	else
	{
		RestorePlayerAfterRespawn();
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

		UAbilitySystemComponent* ASC = GetCharacterAbilitySystemComponent();

		const int32 AbilityCountBeforeClear = ASC ? ASC->GetActivatableAbilities().Num() : -1;
		const bool bWasAbilityListEmpty = ASC && ASC->GetActivatableAbilities().Num() == 0;


		if (ASC)
		{
			const int32 ClearedClassSkillCount = ClearGrantedClassSkillAbilities(ASC);

			GrantDefaultAbilities();
			GrantClassSkillAbilities();

			if (bWasAbilityListEmpty)
			{
				ApplyDefaultEffects();
			}

		}
	}

	// 플레이어 UI 초기화 (로컬 플레이어만) - HUD 및 위젯 생성이 완료되도록 약간의 딜레이 대기
	FTimerHandle UIInitTimerHandle;
	GetWorldTimerManager().SetTimer(UIInitTimerHandle, this, &APlayerCharacterBase::InitializePlayerUI, 0.2f, false);
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

	// 플레이어 UI 초기화 (로컬 플레이어만) - HUD 및 위젯 생성이 완료되도록 약간의 딜레이 대기
	FTimerHandle UIInitTimerHandle;
	GetWorldTimerManager().SetTimer(UIInitTimerHandle, this, &APlayerCharacterBase::InitializePlayerUI, 0.2f, false);
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
		if (!AbilityClass)
		{
			continue;
		}

		if (HasGrantedAbilityClass(ASC, AbilityClass))
		{	
			continue;
		}

		ASC->GiveAbility(FGameplayAbilitySpec(AbilityClass, 1));
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
	if (bPlayerDead || IsDead())
	{
		return;
	}
	
	CurrentMoveInput = InputActionValue.Get<FVector2D>();

	if (IsMovementInputLocked())
	{
		return;
	}

	if (CurrentMoveInput.IsNearlyZero())
	{
		return;
	}

	const FVector MoveDirection =
		GetCameraForwardOnPlane() * CurrentMoveInput.Y +
		GetCameraRightOnPlane() * CurrentMoveInput.X;

	AddMovementInput(MoveDirection.GetSafeNormal());
	
	FGameplayEventData Payload;
	Payload.EventTag = DGGameplayTags::Event_Movement_Skill_CancelByMove;
	Payload.Instigator = this;
	Payload.Target = this;

	UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(
		  this,
		  DGGameplayTags::Event_Movement_Skill_CancelByMove,
		  Payload
	);
}

void APlayerCharacterBase::JumpActionStarted()
{
	SendJumpEvent();
}

void APlayerCharacterBase::JumpActionCompleted()
{
	StopJumping();
}

void APlayerCharacterBase::SendJumpEvent()
{
	UAbilitySystemComponent* ASC = GetCharacterAbilitySystemComponent();
	if (!ASC)
	{
		return;
	}

	FGameplayEventData Payload;
	Payload.EventTag = DGGameplayTags::Skill_Common_Jump;
	Payload.Instigator = this;

	ASC->HandleGameplayEvent(Payload.EventTag, &Payload);
}

bool APlayerCharacterBase::IsMovementInputLocked() const
{
	UAbilitySystemComponent* ASC = GetCharacterAbilitySystemComponent();
	if (!ASC)
	{
		return false;
	}

	return ASC->HasMatchingGameplayTag(DGGameplayTags::State_Movement_Locked);
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

	FVector DesiredDir = FVector::ZeroVector;
	bool bHasInput = false;

	if (!CurrentMoveInput.IsNearlyZero())
	{
		DesiredDir = GetDesiredMoveDirection();
		bHasInput = true;
	}
	else
	{
		DesiredDir = -GetActorForwardVector();
		DesiredDir.Z = 0.f;
		DesiredDir = DesiredDir.GetSafeNormal();

		bHasInput = true;
	}

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
			HUD->ToggleCharacterProfileWidget();
		}
	}
}

void APlayerCharacterBase::OnSkillInputStarted(FGameplayTag SlotTag)
{
	if (bPlayerDead || IsDead())
	{
		return;
	}
	
	HeldSkillSlots.FindOrAdd(SlotTag) = true;

	// if (!HasAuthority())
	// {
	// 	ServerSetSkillInputHeld(SlotTag, true);
	// }

	UAbilitySystemComponent* ASC = GetCharacterAbilitySystemComponent();
	if (!ASC)
	{
		return;
	}

	const FGameplayTag SkillTag = GetSkillTagForSlot(SlotTag);
	

	if (SkillTag.IsValid())
	{
		const bool bActivated = ASC->TryActivateAbilitiesByTag(FGameplayTagContainer(SkillTag));
		

		const FGameplayTag SkillInputEventTag = GetSkillInputEventTag(SkillTag);

		if (SkillInputEventTag.IsValid())
		{

			SendSkillInputStartedEvent(SkillInputEventTag);

			// if (!HasAuthority())
			// {
			// 	ServerSendSkillInputStartedEvent(SkillInputEventTag);
			// }
		}
		else
		{
			
		}
	}
	else
	{
		
	}
}

void APlayerCharacterBase::OnSkillInputCompleted(FGameplayTag SlotTag)
{
	HeldSkillSlots.FindOrAdd(SlotTag) = false;

	// if (!HasAuthority())
	// {
	// 	ServerSetSkillInputHeld(SlotTag, false);
	// }

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

void APlayerCharacterBase::ServerRequestMeleeComboInput_Implementation(FGameplayTag SkillTag, int32 ComboIndex, float ClientInputServerTime)
{
	if (!SkillTag.IsValid() || ComboIndex < 1)
	{
		return;
	}

	FGameplayEventData Payload;
	Payload.EventTag = DGGameplayTags::Event_Combo_InputRequest.GetTag();
	Payload.Instigator = this;
	Payload.Target = this;
	Payload.EventMagnitude = static_cast<float>(ComboIndex);
	Payload.InstigatorTags.AddTag(SkillTag);
	Payload.TargetData.UniqueId = FMath::RoundToInt(ClientInputServerTime * 1000.f);

	UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(
		this,
		DGGameplayTags::Event_Combo_InputRequest.GetTag(),
		Payload
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

void APlayerCharacterBase::SendDamageEvent(FVector DamageSourceLocation, bool bHasDamageSourceLocation)
{
	UAbilitySystemComponent* ASC = GetCharacterAbilitySystemComponent();
	if (!ASC)
	{
		return;
	}

	FGameplayEventData Payload;
	Payload.EventTag = DGGameplayTags::Event_Player_Damage;
	Payload.Instigator = this;
	Payload.Target = this;

	if (bHasDamageSourceLocation)
	{
		FHitResult HitResult;
		HitResult.ImpactPoint = DamageSourceLocation;
		HitResult.Location = DamageSourceLocation;

		FGameplayEffectContextHandle ContextHandle = ASC->MakeEffectContext();
		ContextHandle.AddHitResult(HitResult);

		Payload.ContextHandle = ContextHandle;
	}

	UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(
			this,
			DGGameplayTags::Event_Player_Damage,
			Payload
	);
}

void APlayerCharacterBase::ServerTeleportToLocation_Implementation(FVector TargetLocation)
{
	SetActorLocation(TargetLocation);
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

void APlayerCharacterBase::OnEquipmentChanged(EDGEquipmentType SlotType, UDGItemDefinition* EquippedItemDef)
{
	// 방어구(Armor) 슬롯이 변경되었을 때 상의 메쉬(UpperBodyMesh) 교체
	if (SlotType == EDGEquipmentType::Armor)
	{
		USkeletalMesh* NewMesh = DefaultUpperBodyMesh; // 기본값은 맨몸 메쉬
		
		if (EquippedItemDef && !EquippedItemDef->EquipmentMesh.IsNull())
		{
			// 설정된 메쉬 에셋 동기 로드
			NewMesh = EquippedItemDef->EquipmentMesh.LoadSynchronous();
		}
		
		if (UpperBodyMesh)
		{
			UpperBodyMesh->SetSkeletalMesh(NewMesh);
		}
	}
}
