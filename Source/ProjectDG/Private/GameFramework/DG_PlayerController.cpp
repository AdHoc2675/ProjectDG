// Fill out your copyright notice in the Description page of Project Settings.


#include "GameFramework/DG_PlayerController.h"
#include "GameFramework/PlayerState.h"
#include "Engine/World.h"

void ADG_PlayerController::ServerSendChatMessage_Implementation(const FString& Message)
{
	FString SenderName = PlayerState ? PlayerState->GetPlayerName() : TEXT("Unknown");

	if (UWorld* World = GetWorld())
	{
		for (FConstPlayerControllerIterator It = World->GetPlayerControllerIterator(); It; ++It)
		{
			if (ADG_PlayerController* PC = Cast<ADG_PlayerController>(It->Get()))
			{
				PC->ClientReceiveChatMessage(SenderName, Message);
			}
		}
	}
}

void ADG_PlayerController::ClientReceiveChatMessage_Implementation(const FString& SenderName, const FString& Message)
{
	OnChatMessageReceivedDelegate.Broadcast(SenderName, Message);
}
