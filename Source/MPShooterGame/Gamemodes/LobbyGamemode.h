// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameMode.h"
#include "LobbyGamemode.generated.h"

/**
 * 
 */
UCLASS()
class MPSHOOTERGAME_API ALobbyGamemode : public AGameMode
{
	GENERATED_BODY()
public:
	virtual void PostLogin(APlayerController* newPlayer) override;
	
};
