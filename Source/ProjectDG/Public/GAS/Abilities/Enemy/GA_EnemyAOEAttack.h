// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GAS/Abilities/Base/GameplayAbilityBase.h"
#include "GA_EnemyAOEAttack.generated.h"

class UAnimMontage;
class UNiagaraSystem;
class UAbilityTask_PlayMontageAndWait;
class UAbilityTask_WaitGameplayEvent;

/**
 * UGA_EnemyAOEAttack
 *
 * Telegraph(범위 예고)가 있는 적 AOE 공격 어빌리티.
 *
 * 몽타주 흐름:
 *   [ANS_EnemyAOETelegraphWindow: Begin]
 *     → AI 타겟 위치에 Telegraph VFX 스폰 (모든 클라이언트)
 *   [ANS_EnemyAOETelegraphWindow: End]
 *     → Telegraph VFX 제거
 *   [ANS_AOEOverlapWindow: Begin]
 *     → 타겟 위치 기준 구형 오버랩으로 범위 내 적에게 데미지 적용 (서버)
 *
 * 블루프린트에서 이 클래스를 상속받아:
 *   1. AttackMontage 할당
 *   2. AOERadius / AOEHalfHeight / AOEDamage 설정
 *   3. TelegraphVFX 할당 및 TelegraphScaleMultiplier 조정
 *   4. TargetActorKeyName을 Blackboard 키와 일치시킴
 */
UCLASS()
class PROJECTDG_API UGA_EnemyAOEAttack : public UGameplayAbilityBase
{
	GENERATED_BODY()

public:
	UGA_EnemyAOEAttack();

protected:
	virtual void ActivateAbility(
		const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo,
		const FGameplayEventData* TriggerEventData
	) override;

	virtual void EndAbility(
		const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo,
		bool bReplicateEndAbility,
		bool bWasCancelled
	) override;

protected:
	/** 재생할 공격 몽타주 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Attack")
	TObjectPtr<UAnimMontage> AttackMontage;

	/** AOE 판정 반경 (cm) */
	UPROPERTY(EditDefaultsOnly, Category = "AOE")
	float AOERadius = 300.f;

	/** AOE 판정 캡슐 반높이 (cm) */
	UPROPERTY(EditDefaultsOnly, Category = "AOE")
	float AOEHalfHeight = 150.f;

	/** AOE 충돌 채널 */
	UPROPERTY(EditDefaultsOnly, Category = "AOE")
	TEnumAsByte<ECollisionChannel> AOETraceChannel = ECC_Pawn;

	/** AOE 기본 데미지 */
	UPROPERTY(EditDefaultsOnly, Category = "AOE")
	float AOEDamage = 30.f;

	/** Telegraph로 표시할 Niagara 시스템 */
	UPROPERTY(EditDefaultsOnly, Category = "Telegraph")
	TObjectPtr<UNiagaraSystem> TelegraphVFX;

	/**
	 * Niagara User Parameter 이름 (크기 제어용).
	 * Niagara 시스템의 User 변수 이름과 일치해야 합니다.
	 */
	UPROPERTY(EditDefaultsOnly, Category = "Telegraph")
	FName TelegraphScaleParamName = TEXT("Scale");

	/**
	 * AOERadius에 곱해 Niagara Scale 값을 결정합니다.
	 * Scale 1.0에서 VFX의 실제 크기를 기준으로 조정하세요.
	 */
	UPROPERTY(EditDefaultsOnly, Category = "Telegraph")
	float TelegraphScaleMultiplier = 1.f;

	/** AI Blackboard에서 타겟 액터를 읽을 키 이름 */
	UPROPERTY(EditDefaultsOnly, Category = "AI")
	FName TargetActorKeyName = TEXT("TargetActor");

private:
	UPROPERTY()
	TObjectPtr<UAbilityTask_PlayMontageAndWait> MontageTask;

	UPROPERTY()
	TObjectPtr<UAbilityTask_WaitGameplayEvent> TelegraphBeginTask;

	UPROPERTY()
	TObjectPtr<UAbilityTask_WaitGameplayEvent> TelegraphEndTask;

	UPROPERTY()
	TObjectPtr<UAbilityTask_WaitGameplayEvent> OverlapBeginTask;

	bool bEnding = false;
	FVector CachedTargetLocation = FVector::ZeroVector;

	void StartEventTasks();
	void PerformAOEDamage(const FVector& Center);
	void EndAOEAbility();
	FVector GetTargetLocation() const;

	UFUNCTION() void OnTelegraphBegin(FGameplayEventData Payload);
	UFUNCTION() void OnTelegraphEnd(FGameplayEventData Payload);
	UFUNCTION() void OnOverlapBegin(FGameplayEventData Payload);
	UFUNCTION() void OnMontageCompleted();
	UFUNCTION() void OnMontageInterrupted();
};
