// Fill out your copyright notice in the Description page of Project Settings.


#include "LobbyGamemode.h"
#include "GameFramework/GameStateBase.h"

void ALobbyGamemode::PostLogin(APlayerController* newPlayer)
{
	Super::PostLogin(newPlayer);

	int32 numOfPlayers = GameState.Get()->PlayerArray.Num();
	if (numOfPlayers == 2)
	{
		UWorld* world = GetWorld();
		if (world)
		{
			bUseSeamlessTravel = true;
			world->ServerTravel(FString("/Game/Maps/DefaultGameMap?listen"));
		}
	}
}
