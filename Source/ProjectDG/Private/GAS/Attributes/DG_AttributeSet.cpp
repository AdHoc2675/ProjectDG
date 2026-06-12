// Fill out your copyright notice in the Description page of Project Settings.


#include "GAS/Attributes/DG_AttributeSet.h"
#include "Net/UnrealNetwork.h"
#include "Core/DG_Debug.h"
#include "GameplayEffectExtension.h"
#include "Character/Enemy/EnemyCharacterBase.h"
#include "Character/Player/PlayerCharacterBase.h"
#include "Components/Item/DGLootDropComponent.h"
#include "GameFramework/DG_PlayerState.h"

UDG_AttributeSet::UDG_AttributeSet()
{
}

// 1. 네트워크 복제를 위한 변수 등록
void UDG_AttributeSet::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	
	DOREPLIFETIME_CONDITION_NOTIFY(UDG_AttributeSet, Health, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UDG_AttributeSet, MaxHealth, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UDG_AttributeSet, Mental, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UDG_AttributeSet, MaxMental, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UDG_AttributeSet, Stamina, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UDG_AttributeSet, MaxStamina, COND_None, REPNOTIFY_Always);

	DOREPLIFETIME_CONDITION_NOTIFY(UDG_AttributeSet, MainStat, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UDG_AttributeSet, AttackPower, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UDG_AttributeSet, Defense, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UDG_AttributeSet, HealthCoefficient, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UDG_AttributeSet, DefenseCoefficient, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UDG_AttributeSet, CriticalRate, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UDG_AttributeSet, CriticalDamage, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UDG_AttributeSet, MoveSpeed, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UDG_AttributeSet, AttackSpeed, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UDG_AttributeSet, GroggyDamage, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UDG_AttributeSet, FinalDamageIncrease, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UDG_AttributeSet, DamageReduction, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UDG_AttributeSet, CooldownReduction, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UDG_AttributeSet, MentalRecoveryIncrease, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UDG_AttributeSet, LifeSteal, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UDG_AttributeSet, GroggyDamageIncreaseRate, COND_None, REPNOTIFY_Always);

	// 레거시 스탯 (주스탯으로 통합 예정)
	DOREPLIFETIME_CONDITION_NOTIFY(UDG_AttributeSet, Strength, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UDG_AttributeSet, Dexterity, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UDG_AttributeSet, Intelligence, COND_None, REPNOTIFY_Always);
}

// 2. 값 변경 전 Clamp 처리 (최대치 초과 및 음수 방지)
void UDG_AttributeSet::PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue)
{
	Super::PreAttributeChange(Attribute, NewValue);

	if (Attribute == GetHealthAttribute())
	{
		NewValue = FMath::Clamp(NewValue, 0.0f, GetMaxHealth());
	}
	else if (Attribute == GetMaxHealthAttribute())
	{
		// 최대 체력은 최소 1 이상이어야 합니다.
		// 0 이하가 되면 Health Clamp 범위가 무너지고, 생존/사망 판정도 애매해집니다.
		NewValue = FMath::Max(NewValue, 1.0f);
	}
	
	else if (Attribute == GetMentalAttribute())
	{
		NewValue = FMath::Clamp(NewValue, 0.0f, GetMaxMental());
	}
	else if (Attribute == GetMaxMentalAttribute())
	{
		// 최대 정신력은 음수가 될 수 없습니다. 최소값만 0으로 제한합니다.
		NewValue = FMath::Max(NewValue, 0.0f);
	}
	
	else if (Attribute == GetStaminaAttribute())
	{
		NewValue = FMath::Clamp(NewValue, 0.0f, GetMaxStamina());
	}
	else if (Attribute == GetMaxStaminaAttribute())
	{
		// 최대 스태미나는 음수가 될 수 없습니다. 현재 이동 스킬 자원으로 쓰이므로 0 이상을 보장합니다.
		NewValue = FMath::Max(NewValue, 0.0f);
	}
	
	
	else if (Attribute == GetMainStatAttribute())
        {
                // 주스탯은 캐릭터 성장/장비/버프의 기본 능력치입니다. 음수 주스탯은 공격력 등 파생 계산을 왜곡할 수 있으므로 0 이상으로 제한합니다.
                NewValue = FMath::Max(NewValue, 0.0f);
        }
        else if (Attribute == GetAttackPowerAttribute())
        {
                // 공격력은 모든 피해 계산의 기본값입니다. 음수가 되면 피해 공식에서 회복처럼 작동하거나 계산이 뒤집힐 수 있으므로 0 이상으로 제한합니다.
                NewValue = FMath::Max(NewValue, 0.0f);
        }
        else if (Attribute == GetDefenseAttribute())
        {
                // 방어력은 피해감소 공식에 들어갑니다. 음수 방어력은 피해감소율 공식을 비정상적으로 만들 수 있으므로 0 이상으로 제한합니다.
                NewValue = FMath::Max(NewValue, 0.0f);
        }
        else if (Attribute == GetHealthCoefficientAttribute())
        {
                // 체력 계수는 최종 체력 계산에 곱해지는 값입니다. 음수가 되면 최종 체력이 음수가 될 수 있으므로 0 이상으로 제한합니다.
                NewValue = FMath::Max(NewValue, 0.0f);
        }
        else if (Attribute == GetDefenseCoefficientAttribute())
        {
                // 방어 계수는 최종 방어력 계산에 곱해지는 값입니다. 음수가 되면 유효 방어력이 음수가 되어 피해감소 공식이 깨질 수 있으므로 0 이상으로 제한합니다.
                NewValue = FMath::Max(NewValue, 0.0f);
        }
        else if (Attribute == GetCriticalRateAttribute())
        {
                // 치명타 확률은 0% ~ 100% 범위의 확률값입니다. 프로젝트에서는 0.05f = 5%, 1.0f = 100% 기준으로 관리하는 것이 안전합니다. 
                NewValue = FMath::Clamp(NewValue, 0.0f, 1.0f);
        }
        else if (Attribute == GetCriticalDamageAttribute())
        {
                // 치명타 피해는 치명타 발생 시 곱해지는 배율입니다. 1.0f = 100%를 최소값으로 두면 치명타가 일반 공격보다 약해지는 비정상 상황을 막을 수 있습니다.
                NewValue = FMath::Max(NewValue, 1.0f);
        }
        else if (Attribute == GetMoveSpeedAttribute())
        {
                // 이동속도는 CharacterMovementComponent의 이동 수치에 반영될 값입니다. 음수 이동속도는 이동 처리와 애니메이션 보정에 문제를 만들 수 있으므로 0 이상으로 제한합니다.
                NewValue = FMath::Max(NewValue, 0.0f);
        }
        else if (Attribute == GetAttackSpeedAttribute())
        {
                // 공격속도는 기본 공격/스킬 시전 속도 또는 몽타주 재생속도에 영향을 줄 수 있습니다. 음수 속도는 재생/시전 계산을 깨뜨리므로 0 이상으로 제한합니다.
                NewValue = FMath::Max(NewValue, 0.0f);
        }
        else if (Attribute == GetGroggyDamageAttribute())
        {
                // 그로기 피해는 보스 그로기 게이지에 누적되는 피해량입니다. 음수 그로기 피해는 게이지를 회복시키는 식으로 작동할 수 있으므로 0 이상으로 제한합니다.
                NewValue = FMath::Max(NewValue, 0.0f);
        }
        else if (Attribute == GetFinalDamageIncreaseAttribute())
        {
                // 최종 피해 증가는 공격자가 주는 피해를 마지막 단계에서 증가시키는 값입니다. 현재 기획에는 최종 피해 감소 디버프가 명시되어 있지 않으므로 우선 0 이상으로 제한합니다.
                NewValue = FMath::Max(NewValue, 0.0f);
        }
        else if (Attribute == GetDamageReductionAttribute())
        {
                // 받는 피해 감소는 최종 피해량을 줄이는 비율값입니다.
                // 0.8f = 80% 임시 상한을 두어 피해가 0이 되거나 무적에 가까워지는 상황을 막습니다.
                // 실제 상한은 밸런싱 정책에 따라 조정할 수 있습니다.
                NewValue = FMath::Clamp(NewValue, 0.0f, 0.8f);
        }
        else if (Attribute == GetCooldownReductionAttribute())
        {
                // 스킬 재사용 시간 감소는 쿨타임에 곱해지는 비율값입니다.
                // 0.8f = 80% 임시 상한을 두어 쿨타임이 0 이하가 되는 무한 사용 상황을 막습니다.
                // 실제 상한은 장비/버프 밸런싱 단계에서 조정 가능합니다.
                NewValue = FMath::Clamp(NewValue, 0.0f, 0.8f);
        }
        else if (Attribute == GetMentalRecoveryIncreaseAttribute())
        {
                // 정신력 회복량 증가는 기본 공격 등으로 얻는 정신력 회복량의 보정값입니다.
                // 현재 기획에는 회복량 감소 스탯이 없으므로 0 이상으로 제한합니다.
                NewValue = FMath::Max(NewValue, 0.0f);
        }
        else if (Attribute == GetLifeStealAttribute())
        {
                // 생명력 흡수는 준 피해량의 일부를 회복하는 비율값입니다.
                // 음수가 되면 공격할수록 체력을 잃는 비정상 동작이 되므로 0 이상으로 제한합니다.
                // 상한은 추후 밸런싱 단계에서 필요하면 추가합니다.
                NewValue = FMath::Max(NewValue, 0.0f);
        }
        else if (Attribute == GetGroggyDamageIncreaseRateAttribute())
        {
                // 그로기 피해 증가율은 스킬별 그로기 피해량에 곱해지는 비율 보정값입니다.
                // 현재 기획에는 그로기 피해 감소 스탯이 없으므로 0 이상으로 제한합니다.
                NewValue = FMath::Max(NewValue, 0.0f);
        }
}

// 3. Gameplay Effect(GE)가 적용된 직후 호출 (핵심 데미지 처리)
void UDG_AttributeSet::PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data)
{
	Super::PostGameplayEffectExecute(Data);

	// Meta 어트리뷰트인 Damage가 들어온 경우 결산
	if (Data.EvaluatedData.Attribute == GetDamageAttribute())
	{
		// 받아온 데미지 수치
		const float LocalDamageDone = GetDamage();
		// 메타 어트리뷰트는 사용 후 무조건 0으로 초기화
		SetDamage(0.f);

		// 데미지 적용 전 체력 기록
		const float OldHealth = GetHealth();

		if (LocalDamageDone > 0.0f)
		{
			// 현재 체력에서 데미지 차감
			const float NewHealth = OldHealth - LocalDamageDone;
			SetHealth(FMath::Clamp(NewHealth, 0.0f, GetMaxHealth()));

			// AttributeSet의 OwningActor는 PlayerState이므로
			// Target ASC의 AvatarActor에서 실제 PlayerCharacter를 가져온다.
			APlayerCharacterBase* TargetPlayer = nullptr;

			if (Data.Target.AbilityActorInfo.IsValid())
			{
				TargetPlayer = Cast<APlayerCharacterBase>(
						Data.Target.AbilityActorInfo->AvatarActor.Get()
				);
			}

			if (TargetPlayer)
			{
				if (GetHealth() <= 0.f)
				{
					TargetPlayer->Die();
				}
				else
				{
					AActor* DamageSourceActor =
							Data.EffectSpec.GetContext().GetEffectCauser();

					if (!DamageSourceActor)
					{
						DamageSourceActor =
								Data.EffectSpec.GetContext().GetInstigator();
					}

					const bool bHasDamageSourceLocation =
							IsValid(DamageSourceActor);

					const FVector DamageSourceLocation =
							bHasDamageSourceLocation
									? DamageSourceActor->GetActorLocation()
									: FVector::ZeroVector;

					TargetPlayer->SendDamageEvent(
							DamageSourceLocation,
							bHasDamageSourceLocation
					);
				}
			}
			
		}

		if (AEnemyCharacterBase* TargetEnemy = Cast<AEnemyCharacterBase>(GetOwningActor()))
		{
			bool bIsCritical = false;

			// [추가] GAS EffectContext에서 데미지를 가한 오리지널 주체(보통 PlayerCharacter)를 가져옵니다.
			AActor* InstigatorActor = Data.EffectSpec.GetContext().GetInstigator();

			// 데미지를 띄우라고 전송하면서 공격자 정보도 넘겨줍니다.
			TargetEnemy->Multicast_ShowDamageNumber(LocalDamageDone, bIsCritical, InstigatorActor);

			// 체력이 0이 되어 사망하는 순간 보상(경험치, 골드) 즉시 지급 처리
			// SetHealth 이후 델리게이트를 통해 bIsDead가 true로 이미 변경되었을 수 있으므로 OldHealth를 이용해 판별
			if (OldHealth > 0.f && GetHealth() <= 0.f)
			{
				ADG_PlayerState* PS = nullptr;
				if (APawn* InstigatorPawn = Cast<APawn>(InstigatorActor))
				{
					PS = InstigatorPawn->GetPlayerState<ADG_PlayerState>();
				}
				else if (AController* InstigatorController = Cast<AController>(InstigatorActor))
				{
					PS = InstigatorController->GetPlayerState<ADG_PlayerState>();
				}
				else if (ADG_PlayerState* InstigatorPS = Cast<ADG_PlayerState>(InstigatorActor))
				{
					PS = InstigatorPS;
				}

				if (PS)
				{
					if (UDGLootDropComponent* LootComp = TargetEnemy->FindComponentByClass<UDGLootDropComponent>())
					{
						int32 EarnedGold = FMath::RandRange(LootComp->GetMinRewardGold(), LootComp->GetMaxRewardGold());
						PS->AddExpAndGold(LootComp->GetRewardExp(), EarnedGold);

						UE_LOG(LogTemp, Log, TEXT("[DG_AttributeSet] Player earned %d EXP and %d Gold"), LootComp->GetRewardExp(), EarnedGold);
					}
				}
			}
		}
	}
	else if (Data.EvaluatedData.Attribute == GetHealthAttribute())
	{
		SetHealth(FMath::Clamp(GetHealth(), 0.0f, GetMaxHealth()));
	}
	else if (Data.EvaluatedData.Attribute == GetMaxHealthAttribute())
	{
		SetMaxHealth(FMath::Max(GetMaxHealth(), 1.0f));
		SetHealth(FMath::Clamp(GetHealth(), 0.0f, GetMaxHealth()));
	}
	else if (Data.EvaluatedData.Attribute == GetMentalAttribute())
	{
		SetMental(FMath::Clamp(GetMental(), 0.0f, GetMaxMental()));
	}
	else if (Data.EvaluatedData.Attribute == GetMaxMentalAttribute())
	{
		SetMaxMental(FMath::Max(GetMaxMental(), 0.0f));
		SetMental(FMath::Clamp(GetMental(), 0.0f, GetMaxMental()));
	}
	else if (Data.EvaluatedData.Attribute == GetStaminaAttribute())
	{
		SetStamina(FMath::Clamp(GetStamina(), 0.0f, GetMaxStamina()));
	}
	else if (Data.EvaluatedData.Attribute == GetMaxStaminaAttribute())
	{
		SetMaxStamina(FMath::Max(GetMaxStamina(), 0.0f));
		SetStamina(FMath::Clamp(GetStamina(), 0.0f, GetMaxStamina()));
	}
}

// 4. OnRep 함수 구현 (클라이언트에게 값 변경을 통지하는 GAS 매크로 사용)
void UDG_AttributeSet::OnRep_Health(const FGameplayAttributeData& OldHealth)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UDG_AttributeSet, Health, OldHealth);
}

void UDG_AttributeSet::OnRep_MaxHealth(const FGameplayAttributeData& OldMaxHealth)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UDG_AttributeSet, MaxHealth, OldMaxHealth);
}

void UDG_AttributeSet::OnRep_Mental(const FGameplayAttributeData& OldMental)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UDG_AttributeSet, Mental, OldMental);
}

void UDG_AttributeSet::OnRep_MaxMental(const FGameplayAttributeData& OldMaxMental)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UDG_AttributeSet, MaxMental, OldMaxMental);
}

void UDG_AttributeSet::OnRep_Stamina(const FGameplayAttributeData& OldStamina)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UDG_AttributeSet, Stamina, OldStamina);
}

void UDG_AttributeSet::OnRep_MaxStamina(const FGameplayAttributeData& OldMaxStamina)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UDG_AttributeSet, MaxStamina, OldMaxStamina);
}


void UDG_AttributeSet::OnRep_MainStat(const FGameplayAttributeData& OldMainStat)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UDG_AttributeSet, MainStat, OldMainStat);
}

void UDG_AttributeSet::OnRep_AttackPower(const FGameplayAttributeData& OldAttackPower)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UDG_AttributeSet, AttackPower, OldAttackPower);
}

void UDG_AttributeSet::OnRep_Defense(const FGameplayAttributeData& OldDefense)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UDG_AttributeSet, Defense, OldDefense);
}

void UDG_AttributeSet::OnRep_HealthCoefficient(const FGameplayAttributeData& OldHealthCoefficient)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UDG_AttributeSet, HealthCoefficient, OldHealthCoefficient);
}

void UDG_AttributeSet::OnRep_DefenseCoefficient(const FGameplayAttributeData& OldDefenseCoefficient)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UDG_AttributeSet, DefenseCoefficient, OldDefenseCoefficient);
}

void UDG_AttributeSet::OnRep_CriticalRate(const FGameplayAttributeData& OldCriticalRate)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UDG_AttributeSet, CriticalRate, OldCriticalRate);
}

void UDG_AttributeSet::OnRep_CriticalDamage(const FGameplayAttributeData& OldCriticalDamage)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UDG_AttributeSet, CriticalDamage, OldCriticalDamage);
}

void UDG_AttributeSet::OnRep_MoveSpeed(const FGameplayAttributeData& OldMoveSpeed)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UDG_AttributeSet, MoveSpeed, OldMoveSpeed);
}

void UDG_AttributeSet::OnRep_AttackSpeed(const FGameplayAttributeData& OldAttackSpeed)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UDG_AttributeSet, AttackSpeed, OldAttackSpeed);
}

void UDG_AttributeSet::OnRep_GroggyDamage(const FGameplayAttributeData& OldGroggyDamage)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UDG_AttributeSet, GroggyDamage, OldGroggyDamage);
}

void UDG_AttributeSet::OnRep_FinalDamageIncrease(const FGameplayAttributeData&
OldFinalDamageIncrease)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UDG_AttributeSet, FinalDamageIncrease, OldFinalDamageIncrease);
}

void UDG_AttributeSet::OnRep_DamageReduction(const FGameplayAttributeData& OldDamageReduction)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UDG_AttributeSet, DamageReduction, OldDamageReduction);
}

void UDG_AttributeSet::OnRep_CooldownReduction(const FGameplayAttributeData& OldCooldownReduction)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UDG_AttributeSet, CooldownReduction, OldCooldownReduction);
}

void UDG_AttributeSet::OnRep_MentalRecoveryIncrease(const FGameplayAttributeData&
OldMentalRecoveryIncrease)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UDG_AttributeSet, MentalRecoveryIncrease, OldMentalRecoveryIncrease);
}

void UDG_AttributeSet::OnRep_LifeSteal(const FGameplayAttributeData& OldLifeSteal)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UDG_AttributeSet, LifeSteal, OldLifeSteal);
}

void UDG_AttributeSet::OnRep_GroggyDamageIncreaseRate(const FGameplayAttributeData&
OldGroggyDamageIncreaseRate)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UDG_AttributeSet, GroggyDamageIncreaseRate,
OldGroggyDamageIncreaseRate);
}



void UDG_AttributeSet::OnRep_Strength(const FGameplayAttributeData& OldStrength)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UDG_AttributeSet, Strength, OldStrength);
}

void UDG_AttributeSet::OnRep_Dexterity(const FGameplayAttributeData& OldDexterity)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UDG_AttributeSet, Dexterity, OldDexterity);
}

void UDG_AttributeSet::OnRep_Intelligence(const FGameplayAttributeData& OldIntelligence)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UDG_AttributeSet, Intelligence, OldIntelligence);
}