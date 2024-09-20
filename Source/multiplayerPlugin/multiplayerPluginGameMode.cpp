// Copyright Epic Games, Inc. All Rights Reserved.

#include "multiplayerPluginGameMode.h"
#include "multiplayerPluginCharacter.h"
#include "UObject/ConstructorHelpers.h"

AmultiplayerPluginGameMode::AmultiplayerPluginGameMode()
{
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(TEXT("/Game/ThirdPerson/Blueprints/BP_ThirdPersonCharacter"));
	if (PlayerPawnBPClass.Class != NULL)
	{
		DefaultPawnClass = PlayerPawnBPClass.Class;
	}
}
