// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "DG_GameState.generated.h"

class ADG_PlayerState;
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPlayerStateChangedSignature, ADG_PlayerState*, PlayerState);

/**
 * 
 */
UCLASS()
class PROJECTDG_API ADG_GameState : public AGameStateBase
{
	GENERATED_BODY()
	
public:
	// 누군가 레벨에 들어왔을 때 (PlayerState가 복제 완료됨)
	virtual void AddPlayerState(APlayerState* PlayerState) override;
	// 누군가 레벨에서 나갔을 때
	virtual void RemovePlayerState(APlayerState* PlayerState) override;

	UPROPERTY(BlueprintAssignable, Category = "DG|Party")
	FOnPlayerStateChangedSignature OnPlayerJoinedDelegate;

	UPROPERTY(BlueprintAssignable, Category = "DG|Party")
	FOnPlayerStateChangedSignature OnPlayerLeftDelegate;

};
