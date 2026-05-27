#include "GAS/Abilities/Enemy/GA_SpawnHitboxSkill.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"
#include "Actor/AttackHitboxActor.h"
#include "AbilitySystemComponent.h"
#include "GameFramework/Character.h"

UGA_SpawnHitboxSkill::UGA_SpawnHitboxSkill()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
}

void UGA_SpawnHitboxSkill::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	if (AttackMontage)
	{
		// 몽타주 재생 태스크
		UAbilityTask_PlayMontageAndWait* PlayMontageTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(
			this, NAME_None, AttackMontage, 1.f, NAME_None, false, 1.f, 0.f);

		PlayMontageTask->OnBlendOut.AddDynamic(this, &UGA_SpawnHitboxSkill::OnMontageCompleted);
		PlayMontageTask->OnCompleted.AddDynamic(this, &UGA_SpawnHitboxSkill::OnMontageCompleted);
		PlayMontageTask->OnInterrupted.AddDynamic(this, &UGA_SpawnHitboxSkill::OnMontageInterrupted);
		PlayMontageTask->OnCancelled.AddDynamic(this, &UGA_SpawnHitboxSkill::OnMontageInterrupted);
		
		PlayMontageTask->ReadyForActivation();

		// 특정 시점(예: ANS에서 이벤트를 보낼 때)을 기다리는 태스크
		if (SpawnEventTag.IsValid())
		{
			UAbilityTask_WaitGameplayEvent* WaitEventTask = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(
				this, SpawnEventTag, nullptr, false, false);
			
			WaitEventTask->EventReceived.AddDynamic(this, &UGA_SpawnHitboxSkill::OnSpawnEventReceived);
			WaitEventTask->ReadyForActivation();
		}
	}
	else
	{
		// 몽타주가 없으면 바로 종료
		EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
	}
}

void UGA_SpawnHitboxSkill::OnSpawnEventReceived(FGameplayEventData Payload)
{
	// 이벤트 수신 시 Hitbox 액터 소환
	AActor* Avatar = GetAvatarActorFromAbility();
	if (Avatar && HasAuthorityAvatar() && HitboxActorClass)
	{
		UWorld* World = Avatar->GetWorld();
		if (World)
		{
			// 소환 위치 및 회전 계산 (기본적으로 캐릭터 앞쪽으로 설정, 필요시 소켓 등 활용 가능)
			FVector SpawnLocation = Avatar->GetActorLocation() + Avatar->GetActorForwardVector() * 100.f;
			FRotator SpawnRotation = Avatar->GetActorRotation();

			FActorSpawnParameters SpawnParams;
			SpawnParams.Instigator = Cast<APawn>(Avatar);
			SpawnParams.Owner = Avatar;
			SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

			AAttackHitboxActor* HitboxActor = World->SpawnActor<AAttackHitboxActor>(HitboxActorClass, SpawnLocation, SpawnRotation, SpawnParams);
			
			if (HitboxActor)
			{
				UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo();
				HitboxActor->InitializeHitbox(ASC, DamageEffectClass, DamageLevel);
			}
		}
	}
}

void UGA_SpawnHitboxSkill::OnMontageCompleted()
{
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}

void UGA_SpawnHitboxSkill::OnMontageInterrupted()
{
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
}

void UGA_SpawnHitboxSkill::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}
