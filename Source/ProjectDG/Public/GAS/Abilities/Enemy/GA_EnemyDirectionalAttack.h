// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GAS/Abilities/Base/GameplayAbilityBase.h"
#include "GA_EnemyDirectionalAttack.generated.h"

class UAnimMontage;
class UNiagaraSystem;
class UAbilityTask_PlayMontageAndWait;
class UAbilityTask_WaitGameplayEvent;

/**
 * UGA_EnemyDirectionalAttack
 *
 * Telegraph(범위 예고)가 있는 방향성(직사각형) 적 공격 어빌리티.
 * 보스(또는 적) 몸에 데칼이 부착되어 플레이어 방향을 향해 생성됩니다.
 *
 * 몽타주 흐름:
 *   [ANS_EnemyAOETelegraphWindow: Begin]
 *     → 보스 바닥에 플레이어를 향하는 방향으로 Telegraph VFX 스폰(Attach)
 *   [ANS_EnemyAOETelegraphWindow: End]
 *     → Telegraph VFX 제거
 *   [ANS_AOEOverlapWindow: Begin]
 *     → 보스 위치 기준, 앞쪽으로 길게 뻗은 Box Overlap을 통해 데미지 적용
 *
 * 블루프린트에서 이 클래스를 상속받아:
 *   1. AttackMontage 할당
 *   2. AOELength / AOEWidth / AOEHalfHeight / AOEDamage 설정
 *   3. TelegraphVFX 할당 및 파라미터 이름(Lenght, Width) 일치 확인
 *   4. TargetActorKeyName을 Blackboard 키와 일치시킴
 */
UCLASS()
class PROJECTDG_API UGA_EnemyDirectionalAttack : public UGameplayAbilityBase
{
	GENERATED_BODY()

public:
	UGA_EnemyDirectionalAttack();

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

	/** 공격 판정의 전방 길이 (cm) */
	UPROPERTY(EditDefaultsOnly, Category = "AOE|Box")
	float AOELength = 800.f;

	/** 공격 판정의 좌우 폭 (cm) */
	UPROPERTY(EditDefaultsOnly, Category = "AOE|Box")
	float AOEWidth = 200.f;

	/** 상하 판정 반높이 (cm) */
	UPROPERTY(EditDefaultsOnly, Category = "AOE|Box")
	float AOEHalfHeight = 150.f;

	/** AOE 충돌 채널 */
	UPROPERTY(EditDefaultsOnly, Category = "AOE|Collision")
	TEnumAsByte<ECollisionChannel> AOETraceChannel = ECC_Pawn;

	/** AOE 기본 데미지 */
	UPROPERTY(EditDefaultsOnly, Category = "AOE|Damage")
	float AOEDamage = 30.f;

	/** Telegraph로 표시할 Niagara 시스템 (보스에 Attach 됨) */
	UPROPERTY(EditDefaultsOnly, Category = "Telegraph")
	TObjectPtr<UNiagaraSystem> TelegraphVFX;

	/**
	 * Niagara User Parameter 이름 (길이 제어용).
	 * 오타가 잦은 파라미터(Lenght 등)를 블루프린트에서 수정 가능하도록 지원합니다.
	 */
	UPROPERTY(EditDefaultsOnly, Category = "Telegraph")
	FName TelegraphLengthParamName = TEXT("Lenght");

	/** Niagara User Parameter 이름 (폭 제어용) */
	UPROPERTY(EditDefaultsOnly, Category = "Telegraph")
	FName TelegraphWidthParamName = TEXT("Width");

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
	
	// 타겟을 바라보도록 계산된 월드 회전값
	FRotator CachedDirectionRotation = FRotator::ZeroRotator;

	void StartEventTasks();
	void PerformAOEDamage(const FRotator& BoxRotation);
	void EndAOEAbility();
	FVector GetTargetLocation() const;

	UFUNCTION() void OnTelegraphBegin(FGameplayEventData Payload);
	UFUNCTION() void OnTelegraphEnd(FGameplayEventData Payload);
	UFUNCTION() void OnOverlapBegin(FGameplayEventData Payload);
	UFUNCTION() void OnMontageCompleted();
	UFUNCTION() void OnMontageInterrupted();
};
