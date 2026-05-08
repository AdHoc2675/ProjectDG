// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/DG_GameMode.h"
#include "DGServerGameMode.generated.h"

class APlayerController;
/**
 * 
 */
UCLASS()
class PROJECTDG_API ADGServerGameMode : public ADG_GameMode
{
	GENERATED_BODY()
	
public:
	virtual void PreLogin(
		const FString& Options,
		const FString& Address,
		const FUniqueNetIdRepl& UniqueId,
		FString& ErrorMessage
	) override;

	virtual FString InitNewPlayer(
		APlayerController* NewPlayerController,
		const FUniqueNetIdRepl& UniqueId,
		const FString& Options,
		const FString& Portal
	) override;

	virtual void PostLogin(APlayerController* NewPlayer) override;

private:
	UPROPERTY(EditDefaultsOnly, Category = "DG|Server")
	FString BackendBaseUrl = TEXT("http://localhost:8080");

	void ValidateJoinTokenAsync(
		APlayerController* PlayerController,
		const FString& SessionId,
		const FString& JoinToken
	);

	void KickPlayerWithReason(
		APlayerController* PlayerController,
		const FString& Reason
	);

	static FString BuildValidateJoinJson(
		const FString& SessionId,
		const FString& JoinToken
	);

	static bool ParseValidateJoinResponse(
		const FString& ResponseBody,
		FString& OutMessage
	);
	
};
