// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "DG_PlayerController.generated.h"

class ULevelSequencePlayer;
class ALevelSequenceActor;

/**
 * 구조:
 * - Character는 PlayerState에서 ASC / AttributeSet을 찾아서 사용
 * - PlayerController는 PlayerState의 ASC / AttributeSet을 사용하여 입력에 따른 능력 실행
 *
 * 목적:
 * - 클라이언트 로컬 카메라/입력/화면 연출 처리
 */
UCLASS()
class PROJECTDG_API ADG_PlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	UFUNCTION(Client, Reliable)
	void Client_PlayBossPhaseTransitionCinematic(FSoftObjectPath LevelSequencePath);

	UFUNCTION(Client, Reliable)
	void Client_StopBossPhaseTransitionCinematic();

private:
	UPROPERTY(Transient)
	TObjectPtr<ULevelSequencePlayer> BossPhaseSequencePlayer = nullptr;

	UPROPERTY(Transient)
	TObjectPtr<ALevelSequenceActor> BossPhaseSequenceActor = nullptr;

	UPROPERTY(Transient)
	TObjectPtr<AActor> BossPhasePreviousViewTarget = nullptr;

private:
	UFUNCTION()
	void HandleBossPhaseCinematicFinished();

	void RestoreBossPhaseTransitionCinematic(bool bStopSequence);
};