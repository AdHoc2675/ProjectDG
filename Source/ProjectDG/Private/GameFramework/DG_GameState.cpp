#include "GameFramework/DG_GameState.h"
#include "GameFramework/DG_PlayerState.h"

void ADG_GameState::AddPlayerState(APlayerState* PlayerState)
{
	Super::AddPlayerState(PlayerState);

	if (ADG_PlayerState* DGPS = Cast<ADG_PlayerState>(PlayerState))
	{
		OnPlayerJoinedDelegate.Broadcast(DGPS);
	}
}

void ADG_GameState::RemovePlayerState(APlayerState* PlayerState)
{
	if (ADG_PlayerState* DGPS = Cast<ADG_PlayerState>(PlayerState))
	{
		OnPlayerLeftDelegate.Broadcast(DGPS);
	}

	Super::RemovePlayerState(PlayerState);
}