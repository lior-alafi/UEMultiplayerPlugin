// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "LobbyGM.generated.h"

/**
 * 
 */
UCLASS()
class MULTIPLAYERPLUGIN_API ALobbyGM : public AGameModeBase
{
	GENERATED_BODY()
public:
	
	virtual void PostLogin(APlayerController* player) override;
	virtual void Logout(AController* Exiting) override;

};
