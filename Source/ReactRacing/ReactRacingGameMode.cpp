// Copyright 1998-2018 Epic Games, Inc. All Rights Reserved.

#include "ReactRacingGameMode.h"
#include "ReactRacingPawn.h"
#include "ReactRacingHud.h"

AReactRacingGameMode::AReactRacingGameMode()
{
	DefaultPawnClass = AReactRacingPawn::StaticClass();
	HUDClass = AReactRacingHud::StaticClass();
}
