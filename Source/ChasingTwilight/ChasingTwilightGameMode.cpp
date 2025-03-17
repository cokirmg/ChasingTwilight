// Copyright Epic Games, Inc. All Rights Reserved.

#include "ChasingTwilightGameMode.h"
#include "ChasingTwilightCharacter.h"
#include "UObject/ConstructorHelpers.h"

AChasingTwilightGameMode::AChasingTwilightGameMode()
	: Super()
{
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnClassFinder(TEXT("/Game/FirstPerson/Blueprints/BP_FirstPersonCharacter"));
	DefaultPawnClass = PlayerPawnClassFinder.Class;

}
