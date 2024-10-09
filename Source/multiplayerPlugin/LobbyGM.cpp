// Fill out your copyright notice in the Description page of Project Settings.


#include "LobbyGM.h"
#include "GameFramework/GameStateBase.h"
#include "GameFramework/PlayerState.h"

void ALobbyGM::PostLogin(APlayerController* player)
{
	Super::PostLogin(player);
	if (GameState) {
		int32 numOfPlayers = GameState.Get()->PlayerArray.Num();

		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(1, //same tag so each message will replace the previous one
				60.f, FColor::Emerald, FString::Printf(TEXT("Players in game: %d"), numOfPlayers));

			//print who joined
			APlayerState *playerState = player->GetPlayerState<APlayerState>();
			if (playerState)
			{
				FString playerName = playerState->GetPlayerName();
				GEngine->AddOnScreenDebugMessage(-1, //same tag so each message will replace the previous one
					15.f, FColor::Green, FString::Printf(TEXT("%s joined the fray"), *playerName));
			}
		
		}
		 
	}
}

void ALobbyGM::Logout(AController* Exiting)
{
	Super::Logout(Exiting);


	int32 numOfPlayers = GameState.Get()->PlayerArray.Num();

	GEngine->AddOnScreenDebugMessage(1, //same tag so each message will replace the previous one
		60.f, FColor::Emerald, FString::Printf(TEXT("Players in game: %d"), numOfPlayers -1));

	APlayerState* playerState = Exiting->GetPlayerState<APlayerState>();
	if (playerState)
	{
		FString playerName = playerState->GetPlayerName();
		GEngine->AddOnScreenDebugMessage(-1, //same tag so each message will replace the previous one
			15.f, FColor::Orange, FString::Printf(TEXT("%s left the game"), *playerName));
	}
}
