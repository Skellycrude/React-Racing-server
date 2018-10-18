// Copyright 1998-2018 Epic Games, Inc. All Rights Reserved.
#pragma once
#include "GameFramework/HUD.h"
#include "ReactRacingHud.generated.h"


UCLASS(config = Game)
class AReactRacingHud : public AHUD
{
	GENERATED_BODY()

public:
	AReactRacingHud();

	/** Font used to render the vehicle info */
	UPROPERTY()
	UFont* HUDFont;

	// Begin AHUD interface
	virtual void DrawHUD() override;
	// End AHUD interface
};
