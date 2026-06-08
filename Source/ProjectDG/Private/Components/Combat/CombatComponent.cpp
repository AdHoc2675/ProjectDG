#include "Components/Combat/CombatComponent.h"

#include "AbilitySystemComponent.h"
#include "Character/BaseCharacter.h"
#include "Core/DG_Debug.h"
#include "Core/DG_GameplayTags.h"
#include "GAS/Effects/Damage/GE_Damage.h"
#include "GameplayEffect.h"
#include "GameplayEffectTypes.h"
#include "UI/HUD/DG_HUD.h"
#include "UI/WidgetController/DGOverlayWidgetController.h"

UCombatComponent::UCombatComponent()
{
	PrimaryComponentTick.bCanEverTick = false;

	// ActorComponent 자체도 네트워크에서 존재해야 RPC/서버 흐름 확장 시 안전하다.
	SetIsReplicatedByDefault(true);

	DefaultDamageEffectClass = UGE_Damage::StaticClass();
}

void UCombatComponent::BeginPlay()
{
	Super::BeginPlay();

	InitializeCombatComponent();
}

void UCombatComponent::InitializeCombatComponent()
{
	OwnerBaseCharacter = Cast<ABaseCharacter>(GetOwner());
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
	if (!OwnerBaseCharacter)
	{
		return nullptr;
	}

	return OwnerBaseCharacter->GetCharacterAbilitySystemComponent();
}

FDGDamageResult UCombatComponent::ApplyDamageRequest(const FDGDamageRequest& DamageRequest)
{
	FDGDamageResult Result;

	if (!GetOwner())
	{
		Result.Message = TEXT("CombatComponent owner is null.");
		return Result;
	}

	/**
	 * 데미지 적용은 서버 권한에서만 처리한다.
	 *
	 * Client는 입력/요청만 보내고,
	 * 실제 판정과 Health 변경은 Dedicated Server가 담당해야 한다.
	 */
	if (!GetOwner()->HasAuthority())
	{
		Result.Message = TEXT("ApplyDamageRequest rejected. Only server can apply damage.");
		return Result;
	}

	if (!OwnerBaseCharacter)
	{
		InitializeCombatComponent();
	}

	FString FailReason;
	if (!ValidateDamageRequest(DamageRequest, FailReason))
	{
		Result.Message = FailReason;
		
		return Result;
	}

	AActor* SourceActor = DamageRequest.SourceActor
		                      ? DamageRequest.SourceActor.Get()
		                      : GetOwner();

	AActor* TargetActor = DamageRequest.TargetActor.Get();

	UAbilitySystemComponent* SourceASC = ResolveASCFromActor(SourceActor);
	UAbilitySystemComponent* TargetASC = ResolveASCFromActor(TargetActor);

	if (!SourceASC)
	{
		Result.Message = TEXT("Source ASC is null.");
		return Result;
	}

	if (!TargetASC)
	{
		Result.Message = TEXT("Target ASC is null.");
		return Result;
	}

	if (!DefaultDamageEffectClass)
	{
		Result.Message = TEXT("DefaultDamageEffectClass is null.");
		return Result;
	}

	FGameplayEffectContextHandle EffectContext = SourceASC->MakeEffectContext();
	EffectContext.AddSourceObject(SourceActor);

	if (DamageRequest.bHasHitLocation)
	{
		FHitResult HitResult;
		HitResult.ImpactPoint = DamageRequest.HitLocation;
		HitResult.Location = DamageRequest.HitLocation;
		HitResult.HitObjectHandle = FActorInstanceHandle(TargetActor);
		EffectContext.AddHitResult(HitResult);
	}

	FGameplayEffectSpecHandle SpecHandle = SourceASC->MakeOutgoingSpec(
		DefaultDamageEffectClass,
		1.0f,
		EffectContext
	);

	if (!SpecHandle.IsValid() || !SpecHandle.Data.IsValid())
	{
		Result.Message = TEXT("Failed to make damage GameplayEffect spec.");
		return Result;
	}

	/**
 * DGExecCalc_Damage에서 읽을 데미지 입력값.
 *
 * 약속:
 * - Data.BaseDamage = 고정 피해
 * - Data.DamageMultiplier = Source AttackPower에 곱할 스킬 계수
 *
 * 기존 Data.Damage는 구버전 호환용으로 유지한다.
 */
	SpecHandle.Data->SetSetByCallerMagnitude(
		DGGameplayTags::Data_BaseDamage,
		DamageRequest.BaseDamage
	);

	SpecHandle.Data->SetSetByCallerMagnitude(
		DGGameplayTags::Data_DamageMultiplier,
		DamageRequest.DamageMultiplier
	);

	SpecHandle.Data->SetSetByCallerMagnitude(
		DGGameplayTags::Data_GroggyDamage,
		DamageRequest.GroggyDamage
	);

	SpecHandle.Data->SetSetByCallerMagnitude(
		DGGameplayTags::Data_Damage,
		DamageRequest.BaseDamage
	);


	SourceASC->ApplyGameplayEffectSpecToTarget(
		*SpecHandle.Data.Get(),
		TargetASC
	);

	Result.bSuccess = true;
	Result.FinalDamage = 0.f;
	Result.Message = TEXT("Damage GameplayEffect applied.");

	

	if (SourceActor)
	{
		if (APawn* SourcePawn = Cast<APawn>(SourceActor))
		{
			if (SourcePawn->IsLocallyControlled())
			{
				if (APlayerController* PC = Cast<APlayerController>(SourcePawn->GetController()))
				{
					// HUD 캐스팅
					if (ADG_HUD* HUD = Cast<ADG_HUD>(PC->GetHUD()))
					{
						// GetOverlayWidgetController 호출 (파라미터 빈값이어도 됨, 이미 생성되어있으므로)
						if (UDGOverlayWidgetController* Controller = HUD->GetOverlayWidgetController(FWidgetControllerParams()))
						{
							// 방금 때린 적을 타겟으로 넘겨줌
							Controller->NotifyEnemyDamaged(TargetActor);
						}
					}
				}
			}
		}
	}

	return Result;
}

bool UCombatComponent::ValidateDamageRequest(
	const FDGDamageRequest& DamageRequest,
	FString& OutFailReason
) const
{
	AActor* SourceActor = DamageRequest.SourceActor
		                      ? DamageRequest.SourceActor.Get()
		                      : GetOwner();

	AActor* TargetActor = DamageRequest.TargetActor.Get();

	if (!SourceActor)
	{
		OutFailReason = TEXT("SourceActor is null.");
		return false;
	}

	if (!TargetActor)
	{
		OutFailReason = TEXT("TargetActor is null.");
		return false;
	}

	if (SourceActor == TargetActor)
	{
		OutFailReason = TEXT("SourceActor and TargetActor are same.");
		return false;
	}

	if (DamageRequest.BaseDamage <= 0.0f && DamageRequest.DamageMultiplier <= 0.0f)
	{
		OutFailReason = TEXT("BaseDamage or DamageMultiplier must be greater than 0.");
		return false;
	}

	const ABaseCharacter* SourceCharacter = Cast<ABaseCharacter>(SourceActor);
	const ABaseCharacter* TargetCharacter = Cast<ABaseCharacter>(TargetActor);

	if (!SourceCharacter)
	{
		OutFailReason = TEXT("SourceActor is not BaseCharacter.");
		return false;
	}

	if (!TargetCharacter)
	{
		OutFailReason = TEXT("TargetActor is not BaseCharacter.");
		return false;
	}

	if (SourceCharacter->IsDead())
	{
		OutFailReason = TEXT("SourceCharacter is dead.");
		return false;
	}

	if (TargetCharacter->IsDead())
	{
		OutFailReason = TEXT("TargetCharacter is dead.");
		return false;
	}

	// if (SourceCharacter->IsFriendlyTo(TargetCharacter))
	// {
	// 	OutFailReason = TEXT("Target is friendly.");
	// 	return false;
	// }

	return true;
}

UAbilitySystemComponent* UCombatComponent::ResolveASCFromActor(AActor* Actor) const
{
	const ABaseCharacter* BaseCharacter = Cast<ABaseCharacter>(Actor);
	if (!BaseCharacter)
	{
		return nullptr;
	}

	return BaseCharacter->GetCharacterAbilitySystemComponent();
}
