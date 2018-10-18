// Copyright 2017-2018 Robin Zinser. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine.h"
#include "GameFramework/GameMode.h"
#include "MatchmakingGameModeBase.generated.h"

/**
*
*/
UCLASS()
class MATCHMAKING_API AMatchmakingGameModeBase : public AGameMode
{
	GENERATED_BODY()

		virtual FString InitNewPlayer(APlayerController* NewPlayerController, const FUniqueNetIdRepl& UniqueId, const FString& Options, const FString& Portal = TEXT("")) override;

public:
	UFUNCTION(BlueprintImplementableEvent, Category = "Matchmaking")
		void Server_LoginClient(APlayerController* NewPlayerController, const FString& Client_Name, const FString& Char_Name, const FString& Char_ID, const FString& Team, const FString& FullParameters);
};
