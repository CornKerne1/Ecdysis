// Copyright Epic Games, Inc. All Rights Reserved.

#include "EcdysisGameMode.h"
#include "EcdysisHUD.h"
#include "EcdysisCharacter.h"
#include "UObject/ConstructorHelpers.h"

AEcdysisGameMode::AEcdysisGameMode()
	: Super()
{
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnClassFinder(TEXT("/Game/FirstPersonCPP/Blueprints/FirstPersonCharacter"));
	DefaultPawnClass = PlayerPawnClassFinder.Class;

	// use our custom HUD class
	HUDClass = AEcdysisHUD::StaticClass();
}
