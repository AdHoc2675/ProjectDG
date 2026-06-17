// Fill out your copyright notice in the Description page of Project Settings.

#include "GameFramework/DG_PlayerController.h"

#include "LevelSequence.h"
#include "LevelSequenceActor.h"
#include "LevelSequencePlayer.h"
#include "MovieSceneSequencePlaybackSettings.h"

void ADG_PlayerController::Client_PlayBossPhaseTransitionCinematic_Implementation(
	FSoftObjectPath LevelSequencePath
)
{
	RestoreBossPhaseTransitionCinematic(true);

	if (!LevelSequencePath.IsValid())
	{
		UE_LOG(
			LogTemp,
			Warning,
			TEXT("[DG_PlayerController] Boss phase cinematic path invalid.")
		);
		return;
	}

	ULevelSequence* LevelSequence = Cast<ULevelSequence>(LevelSequencePath.TryLoad());
	if (!LevelSequence)
	{
		UE_LOG(
			LogTemp,
			Warning,
			TEXT("[DG_PlayerController] Boss phase cinematic load failed. Path=%s"),
			*LevelSequencePath.ToString()
		);
		return;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	BossPhasePreviousViewTarget = GetViewTarget();

	FMovieSceneSequencePlaybackSettings PlaybackSettings;
	PlaybackSettings.bAutoPlay = false;
	PlaybackSettings.bHideHud = false;
	PlaybackSettings.bDisableMovementInput = true;
	PlaybackSettings.bDisableLookAtInput = true;

	ALevelSequenceActor* CreatedSequenceActor = nullptr;

	ULevelSequencePlayer* CreatedSequencePlayer =
		ULevelSequencePlayer::CreateLevelSequencePlayer(
			World,
			LevelSequence,
			PlaybackSettings,
			CreatedSequenceActor
		);

	BossPhaseSequencePlayer = CreatedSequencePlayer;
	BossPhaseSequenceActor = CreatedSequenceActor;

	if (!BossPhaseSequencePlayer)
	{
		UE_LOG(
			LogTemp,
			Warning,
			TEXT("[DG_PlayerController] Failed to create boss phase LevelSequencePlayer. Sequence=%s"),
			*GetNameSafe(LevelSequence)
		);

		RestoreBossPhaseTransitionCinematic(false);
		return;
	}

	BossPhaseSequencePlayer->OnFinished.AddDynamic(
		this,
		&ADG_PlayerController::HandleBossPhaseCinematicFinished
	);

	SetCinematicMode(
		true,
		true,
		true,
		true,
		true
	);

	BossPhaseSequencePlayer->Play();

	UE_LOG(
		LogTemp,
		Warning,
		TEXT("[DG_PlayerController] Boss phase cinematic started. Sequence=%s Actor=%s PrevViewTarget=%s"),
		*GetNameSafe(LevelSequence),
		*GetNameSafe(BossPhaseSequenceActor.Get()),
		*GetNameSafe(BossPhasePreviousViewTarget.Get())
	);
}

void ADG_PlayerController::Client_StopBossPhaseTransitionCinematic_Implementation()
{
	RestoreBossPhaseTransitionCinematic(true);

	UE_LOG(
		LogTemp,
		Warning,
		TEXT("[DG_PlayerController] Boss phase cinematic stopped by server.")
	);
}

void ADG_PlayerController::HandleBossPhaseCinematicFinished()
{
	RestoreBossPhaseTransitionCinematic(false);

	UE_LOG(
		LogTemp,
		Warning,
		TEXT("[DG_PlayerController] Boss phase cinematic finished naturally.")
	);
}

void ADG_PlayerController::RestoreBossPhaseTransitionCinematic(bool bStopSequence)
{
	if (BossPhaseSequencePlayer)
	{
		BossPhaseSequencePlayer->OnFinished.RemoveAll(this);

		if (bStopSequence)
		{
			BossPhaseSequencePlayer->Stop();
		}

		BossPhaseSequencePlayer = nullptr;
	}

	if (BossPhaseSequenceActor)
	{
		BossPhaseSequenceActor->Destroy();
		BossPhaseSequenceActor = nullptr;
	}

	SetCinematicMode(
		false,
		true,
		true,
		true,
		true
	);

	AActor* RestoreViewTarget = BossPhasePreviousViewTarget.Get();

	if (!RestoreViewTarget)
	{
		RestoreViewTarget = GetPawn();
	}

	if (RestoreViewTarget)
	{
		SetViewTargetWithBlend(
			RestoreViewTarget,
			0.15f
		);
	}

	BossPhasePreviousViewTarget = nullptr;
}