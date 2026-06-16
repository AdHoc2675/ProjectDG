// Fill out your copyright notice in the Description page of Project Settings.

#include "GAS/Abilities/Enemy/Boss/Kashapa/GA_Boss_Kashapa_PhaseTransition.h"

#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"
#include "AIController.h"
#include "Animation/AnimInstance.h"
#include "Animation/AnimMontage.h"
#include "Character/Enemy/Boss/BossCharacterBase.h"
#include "Character/Enemy/Data/EnemySkillData.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "LevelSequence.h"
#include "LevelSequenceActor.h"
#include "LevelSequencePlayer.h"
#include "MovieSceneSequencePlaybackSettings.h"
#include "TimerManager.h"
#include "UObject/ConstructorHelpers.h"

UGA_Boss_Kashapa_PhaseTransition::UGA_Boss_Kashapa_PhaseTransition()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::ServerOnly;

	PhaseApplyEventTag = FGameplayTag::RequestGameplayTag(
		FName(TEXT("Event.Boss.PhaseApply")),
		false
	);

	static ConstructorHelpers::FObjectFinder<ULevelSequence> PhaseTransitionSequenceFinder(
		TEXT("/Script/LevelSequence.LevelSequence'/Game/__ProjectDG/__BP/CutScene/LS_Kashapa_Phase1To2.LS_Kashapa_Phase1To2'")
	);

	if (PhaseTransitionSequenceFinder.Succeeded())
	{
		PhaseTransitionLevelSequence = PhaseTransitionSequenceFinder.Object;
	}
}

void UGA_Boss_Kashapa_PhaseTransition::ActivateAbility(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData
)
{
	Super::ActivateAbility(
		Handle,
		ActorInfo,
		ActivationInfo,
		TriggerEventData
	);

	AActor* AvatarActor = GetAvatarActorFromActorInfo();
	if (!AvatarActor || !AvatarActor->HasAuthority())
	{
		EndAbility(
			Handle,
			ActorInfo,
			ActivationInfo,
			true,
			true
		);
		return;
	}

	if (!CommitAbility(
		Handle,
		ActorInfo,
		ActivationInfo
	))
	{
		EndAbility(
			Handle,
			ActorInfo,
			ActivationInfo,
			true,
			true
		);
		return;
	}

	bPhaseAppliedByNotify = false;
	bPhaseApplyQueued = false;
	QueuedPhaseApplyIndex = INDEX_NONE;
	bWaitingForPostPhaseApplyMontageEnd = false;

	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(PhaseApplyDelayTimerHandle);
	}

	// 중요:
	// 여기서 ApplyPendingPhaseChangeFromNotify 호출 금지.
	// 여기서 2페 Mesh / Material / AnimClass 적용 금지.
	// AM_Phase1To2 시작 시점까지는 반드시 1페 외형 유지.

	StopBossPhaseTransitionMovement();

	// 컷신 카메라는 보스 최초 배치 위치 기준으로 제작되어 있으므로,
	// PhaseTransition 시작 시 보스를 기준 위치로 되돌린다.
	// 이 시점에서도 아직 1페 Mesh / Material / AnimClass 유지.
	if (ABossCharacterBase* BossCharacter = Cast<ABossCharacterBase>(AvatarActor))
	{
		BossCharacter->MoveToInitialBossTransformForCutscene();
	}

	RegisterPhaseApplyEvent();

	PlayPhaseTransitionCinematic();

	// PlayMontageAndWait 사용 금지.
	// PhaseApply 시점에 Mesh / AnimClass가 바뀌면 MontageTask가 Interrupt되어 GA가 조기 종료될 수 있다.
	if (!PlayPhaseTransitionMontageManually(NAME_None))
	{
		UE_LOG(
			LogTemp,
			Warning,
			TEXT("[GA_Boss_Kashapa_PhaseTransition] Failed to play phase transition montage manually.")
		);

		EndAbility(
			Handle,
			ActorInfo,
			ActivationInfo,
			true,
			true
		);
		return;
	}

	UE_LOG(
		LogTemp,
		Warning,
		TEXT("[GA_Boss_Kashapa_PhaseTransition] Activated. Phase1 visual is kept until PhaseApply Notify. Owner=%s"),
		*AvatarActor->GetName()
	);
}

void UGA_Boss_Kashapa_PhaseTransition::EndAbility(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	bool bReplicateEndAbility,
	bool bWasCancelled
)
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(PhaseApplyDelayTimerHandle);
	}

	PhaseTransitionMontageEndedDelegate.Unbind();

	if (PhaseApplyEventTask)
	{
		PhaseApplyEventTask->EndTask();
		PhaseApplyEventTask = nullptr;
	}

	StopPhaseTransitionCinematic();

	Super::EndAbility(
		Handle,
		ActorInfo,
		ActivationInfo,
		bReplicateEndAbility,
		bWasCancelled
	);
}

void UGA_Boss_Kashapa_PhaseTransition::OnEnemySkillFinished(bool bWasCancelled)
{
	UE_LOG(
		LogTemp,
		Warning,
		TEXT("[GA_Boss_Kashapa_PhaseTransition] Finished. Cancelled=%s PhaseApplied=%s"),
		bWasCancelled ? TEXT("true") : TEXT("false"),
		bPhaseAppliedByNotify ? TEXT("true") : TEXT("false")
	);
}

void UGA_Boss_Kashapa_PhaseTransition::RegisterPhaseApplyEvent()
{
	if (!PhaseApplyEventTag.IsValid())
	{
		PhaseApplyEventTag = FGameplayTag::RequestGameplayTag(
			FName(TEXT("Event.Boss.PhaseApply")),
			false
		);
	}

	if (!PhaseApplyEventTag.IsValid())
	{
		UE_LOG(
			LogTemp,
			Warning,
			TEXT("[GA_Boss_Kashapa_PhaseTransition] PhaseApplyEventTag Invalid")
		);
		return;
	}

	if (PhaseApplyEventTask)
	{
		return;
	}

	PhaseApplyEventTask = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(
		this,
		PhaseApplyEventTag,
		nullptr,
		false,
		true
	);

	if (!PhaseApplyEventTask)
	{
		UE_LOG(
			LogTemp,
			Warning,
			TEXT("[GA_Boss_Kashapa_PhaseTransition] Failed to create PhaseApplyEventTask")
		);
		return;
	}

	PhaseApplyEventTask->EventReceived.AddDynamic(
		this,
		&UGA_Boss_Kashapa_PhaseTransition::OnPhaseApplyEventReceived
	);

	PhaseApplyEventTask->ReadyForActivation();

	UE_LOG(
		LogTemp,
		Warning,
		TEXT("[GA_Boss_Kashapa_PhaseTransition] Wait PhaseApply Event Registered. Tag=%s"),
		*PhaseApplyEventTag.ToString()
	);
}

void UGA_Boss_Kashapa_PhaseTransition::OnPhaseApplyEventReceived(FGameplayEventData Payload)
{
	if (bPhaseAppliedByNotify || bPhaseApplyQueued)
	{
		UE_LOG(
			LogTemp,
			Warning,
			TEXT("[GA_Boss_Kashapa_PhaseTransition] PhaseApply ignored. Already applied or queued. Applied=%s Queued=%s"),
			bPhaseAppliedByNotify ? TEXT("true") : TEXT("false"),
			bPhaseApplyQueued ? TEXT("true") : TEXT("false")
		);
		return;
	}

	QueuePendingPhaseApplyFromEvent(Payload);
}

void UGA_Boss_Kashapa_PhaseTransition::QueuePendingPhaseApplyFromEvent(
	const FGameplayEventData& Payload
)
{
	int32 TargetPhaseIndex = FMath::RoundToInt(Payload.EventMagnitude);
	if (TargetPhaseIndex <= 0)
	{
		TargetPhaseIndex = 2;
	}

	bPhaseApplyQueued = true;
	QueuedPhaseApplyIndex = TargetPhaseIndex;

	UWorld* World = GetWorld();
	if (!World)
	{
		ApplyQueuedPendingPhase();
		return;
	}

	// 중요:
	// AnimNotify 처리 중에는 SkeletalMesh / AnimClass를 바꾸면 PostAnimEvaluation 재귀 크래시가 난다.
	// 따라서 약간의 지연을 둬서 Anim Evaluation 이후에 적용한다.
	World->GetTimerManager().SetTimer(
		PhaseApplyDelayTimerHandle,
		this,
		&UGA_Boss_Kashapa_PhaseTransition::ApplyQueuedPendingPhase,
		0.03f,
		false
	);

	UE_LOG(
		LogTemp,
		Warning,
		TEXT("[GA_Boss_Kashapa_PhaseTransition] PhaseApply queued. TargetPhase=%d"),
		QueuedPhaseApplyIndex
	);
}

void UGA_Boss_Kashapa_PhaseTransition::ApplyQueuedPendingPhase()
{
	if (bPhaseAppliedByNotify)
	{
		return;
	}

	const int32 TargetPhaseIndex = QueuedPhaseApplyIndex > 0
		? QueuedPhaseApplyIndex
		: 2;

	FGameplayEventData DeferredPayload;
	DeferredPayload.EventTag = PhaseApplyEventTag;
	DeferredPayload.Instigator = GetAvatarActorFromActorInfo();
	DeferredPayload.Target = GetAvatarActorFromActorInfo();
	DeferredPayload.EventMagnitude = static_cast<float>(TargetPhaseIndex);

	const bool bApplied = ApplyPendingPhaseFromEvent(DeferredPayload);

	bPhaseApplyQueued = false;
	QueuedPhaseApplyIndex = INDEX_NONE;

	if (bApplied)
	{
		// PhaseApply로 Mesh / AnimClass가 교체되면 기존 Montage는 끊길 수 있다.
		// 따라서 P2 AnimInstance에서 Reveal 섹션부터 다시 재생하고,
		// 이 Montage가 끝났을 때 Ability를 종료한다.
		PlayPostPhaseApplyMontage();
	}

	UE_LOG(
		LogTemp,
		Warning,
		TEXT("[GA_Boss_Kashapa_PhaseTransition] Queued PhaseApply executed. TargetPhase=%d Applied=%s"),
		TargetPhaseIndex,
		bApplied ? TEXT("true") : TEXT("false")
	);
}

bool UGA_Boss_Kashapa_PhaseTransition::ApplyPendingPhaseFromEvent(const FGameplayEventData& Payload)
{
	ABossCharacterBase* BossCharacter = Cast<ABossCharacterBase>(GetAvatarActorFromActorInfo());
	if (!BossCharacter)
	{
		UE_LOG(
			LogTemp,
			Warning,
			TEXT("[GA_Boss_Kashapa_PhaseTransition] BossCharacter invalid.")
		);
		return false;
	}

	int32 TargetPhaseIndex = FMath::RoundToInt(Payload.EventMagnitude);
	if (TargetPhaseIndex <= 0)
	{
		TargetPhaseIndex = 2;
	}

	const bool bApplied = BossCharacter->ApplyPendingPhaseChangeFromNotify(TargetPhaseIndex);
	bPhaseAppliedByNotify = bApplied;

	UE_LOG(
		LogTemp,
		Warning,
		TEXT("[GA_Boss_Kashapa_PhaseTransition] PhaseApply Event Received. TargetPhase=%d Applied=%s Owner=%s"),
		TargetPhaseIndex,
		bApplied ? TEXT("true") : TEXT("false"),
		*GetNameSafe(BossCharacter)
	);

	return bApplied;
}

bool UGA_Boss_Kashapa_PhaseTransition::PlayPhaseTransitionMontageManually(FName StartSectionName)
{
	UEnemySkillData* CurrentSkillData = GetEnemySkillData();
	if (!CurrentSkillData || !CurrentSkillData->Montage)
	{
		UE_LOG(
			LogTemp,
			Warning,
			TEXT("[GA_Boss_Kashapa_PhaseTransition] Montage invalid. SkillData=%s"),
			*GetNameSafe(CurrentSkillData)
		);
		return false;
	}

	ACharacter* Character = Cast<ACharacter>(GetAvatarActorFromActorInfo());
	if (!Character || !Character->GetMesh())
	{
		return false;
	}

	UAnimInstance* AnimInstance = Character->GetMesh()->GetAnimInstance();
	if (!AnimInstance)
	{
		return false;
	}

	const float PlayRate = FMath::Max(CurrentSkillData->PlayRate, 0.01f);

	const float PlayedDuration = AnimInstance->Montage_Play(
		CurrentSkillData->Montage,
		PlayRate
	);

	if (PlayedDuration <= 0.0f)
	{
		UE_LOG(
			LogTemp,
			Warning,
			TEXT("[GA_Boss_Kashapa_PhaseTransition] Montage_Play failed. Montage=%s AnimInstance=%s"),
			*GetNameSafe(CurrentSkillData->Montage),
			*GetNameSafe(AnimInstance)
		);
		return false;
	}

	if (StartSectionName != NAME_None)
	{
		AnimInstance->Montage_JumpToSection(
			StartSectionName,
			CurrentSkillData->Montage
		);
	}

	PhaseTransitionMontageEndedDelegate.Unbind();
	PhaseTransitionMontageEndedDelegate.BindUObject(
		this,
		&UGA_Boss_Kashapa_PhaseTransition::OnPhaseTransitionMontageEnded
	);

	AnimInstance->Montage_SetEndDelegate(
		PhaseTransitionMontageEndedDelegate,
		CurrentSkillData->Montage
	);

	UE_LOG(
		LogTemp,
		Warning,
		TEXT("[GA_Boss_Kashapa_PhaseTransition] Montage played manually. Montage=%s Section=%s PlayRate=%.2f AnimInstance=%s"),
		*GetNameSafe(CurrentSkillData->Montage),
		StartSectionName != NAME_None ? *StartSectionName.ToString() : TEXT("None"),
		PlayRate,
		*GetNameSafe(AnimInstance)
	);

	return true;
}

bool UGA_Boss_Kashapa_PhaseTransition::PlayPostPhaseApplyMontage()
{
	bWaitingForPostPhaseApplyMontageEnd = true;

	const bool bPlayed = PlayPhaseTransitionMontageManually(PostPhaseApplySectionName);

	UE_LOG(
		LogTemp,
		Warning,
		TEXT("[GA_Boss_Kashapa_PhaseTransition] PostPhaseApply Montage Play Result=%s Section=%s"),
		bPlayed ? TEXT("true") : TEXT("false"),
		*PostPhaseApplySectionName.ToString()
	);

	if (!bPlayed)
	{
		// P2 Montage 재생 실패 시 컷신을 무한 대기하지 않게 종료.
		FinishPhaseTransitionByMontageEnd();
		return false;
	}

	return true;
}

void UGA_Boss_Kashapa_PhaseTransition::OnPhaseTransitionMontageEnded(
	UAnimMontage* Montage,
	bool bInterrupted
)
{
	UE_LOG(
		LogTemp,
		Warning,
		TEXT("[GA_Boss_Kashapa_PhaseTransition] Montage Ended. Montage=%s Interrupted=%s PhaseApplied=%s PhaseQueued=%s WaitingPostApplyEnd=%s"),
		*GetNameSafe(Montage),
		bInterrupted ? TEXT("true") : TEXT("false"),
		bPhaseAppliedByNotify ? TEXT("true") : TEXT("false"),
		bPhaseApplyQueued ? TEXT("true") : TEXT("false"),
		bWaitingForPostPhaseApplyMontageEnd ? TEXT("true") : TEXT("false")
	);

	// PhaseApply 전, Mesh/AnimClass 교체로 인해 기존 Montage가 끊기는 경우가 있다.
	// 이 Interrupt는 Ability 종료 조건이 아니다.
	if (!bWaitingForPostPhaseApplyMontageEnd)
	{
		// 단, Notify가 누락되어 첫 Montage가 정상 종료된 경우는 FailSafe로 종료 처리한다.
		if (!bInterrupted && !bPhaseAppliedByNotify && !bPhaseApplyQueued)
		{
			FinishPhaseTransitionByMontageEnd();
		}

		return;
	}

	// PhaseApply 후 P2 AnimInstance에서 재생한 Montage가 끝났을 때만 종료.
	if (bInterrupted)
	{
		UE_LOG(
			LogTemp,
			Warning,
			TEXT("[GA_Boss_Kashapa_PhaseTransition] PostPhaseApply Montage interrupted. Keep ability alive.")
		);
		return;
	}

	FinishPhaseTransitionByMontageEnd();
}

void UGA_Boss_Kashapa_PhaseTransition::FinishPhaseTransitionByMontageEnd()
{
	if (!bPhaseAppliedByNotify &&
		!bPhaseApplyQueued &&
		bFailSafeApplyPhaseOnMontageEndIfNotifyMissed)
	{
		UE_LOG(
			LogTemp,
			Warning,
			TEXT("[GA_Boss_Kashapa_PhaseTransition] PhaseApply Notify missed. FailSafe apply before finish.")
		);

		FGameplayEventData DummyPayload;
		DummyPayload.EventTag = PhaseApplyEventTag;
		DummyPayload.Instigator = GetAvatarActorFromActorInfo();
		DummyPayload.Target = GetAvatarActorFromActorInfo();
		DummyPayload.EventMagnitude = 2.0f;

		ApplyPendingPhaseFromEvent(DummyPayload);
	}

	UE_LOG(
		LogTemp,
		Warning,
		TEXT("[GA_Boss_Kashapa_PhaseTransition] Finish by AM end.")
	);

	FinishEnemySkill(false);
}

void UGA_Boss_Kashapa_PhaseTransition::PlayPhaseTransitionCinematic()
{
	if (!PhaseTransitionLevelSequence)
	{
		UE_LOG(
			LogTemp,
			Warning,
			TEXT("[GA_Boss_Kashapa_PhaseTransition] PhaseTransitionLevelSequence null. Skip camera sequence.")
		);
		return;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	FMovieSceneSequencePlaybackSettings PlaybackSettings;
	PlaybackSettings.bAutoPlay = false;
	PlaybackSettings.bHideHud = false;
	PlaybackSettings.bDisableMovementInput = false;
	PlaybackSettings.bDisableLookAtInput = false;

	ALevelSequenceActor* CreatedSequenceActor = nullptr;

	ULevelSequencePlayer* CreatedSequencePlayer =
		ULevelSequencePlayer::CreateLevelSequencePlayer(
			World,
			PhaseTransitionLevelSequence,
			PlaybackSettings,
			CreatedSequenceActor
		);

	PhaseTransitionSequencePlayer = CreatedSequencePlayer;
	PhaseTransitionSequenceActor = CreatedSequenceActor;

	if (!PhaseTransitionSequencePlayer)
	{
		UE_LOG(
			LogTemp,
			Warning,
			TEXT("[GA_Boss_Kashapa_PhaseTransition] Failed to create LevelSequencePlayer.")
		);
		return;
	}

	PhaseTransitionSequencePlayer->Play();

	UE_LOG(
		LogTemp,
		Warning,
		TEXT("[GA_Boss_Kashapa_PhaseTransition] LevelSequence started. SequenceActor=%s"),
		*GetNameSafe(PhaseTransitionSequenceActor.Get())
	);
}

void UGA_Boss_Kashapa_PhaseTransition::StopPhaseTransitionCinematic()
{
	if (PhaseTransitionSequencePlayer)
	{
		PhaseTransitionSequencePlayer->Stop();
		PhaseTransitionSequencePlayer = nullptr;
	}

	if (PhaseTransitionSequenceActor)
	{
		PhaseTransitionSequenceActor->Destroy();
		PhaseTransitionSequenceActor = nullptr;
	}
}

void UGA_Boss_Kashapa_PhaseTransition::StopBossPhaseTransitionMovement() const
{
	AActor* AvatarActor = GetAvatarActorFromActorInfo();
	APawn* AvatarPawn = Cast<APawn>(AvatarActor);

	if (AvatarPawn)
	{
		if (AAIController* AIController = Cast<AAIController>(AvatarPawn->GetController()))
		{
			AIController->StopMovement();
			AIController->ClearFocus(EAIFocusPriority::Gameplay);
		}
	}

	ACharacter* Character = Cast<ACharacter>(AvatarActor);
	if (Character && Character->GetCharacterMovement())
	{
		Character->GetCharacterMovement()->StopMovementImmediately();
	}
}