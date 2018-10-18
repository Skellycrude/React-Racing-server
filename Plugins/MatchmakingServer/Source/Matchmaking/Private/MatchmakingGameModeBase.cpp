// Copyright 2017-2018 Robin Zinser. All Rights Reserved.

#include "MatchmakingGameModeBase.h"


FString AMatchmakingGameModeBase::InitNewPlayer(APlayerController* NewPlayerController, const FUniqueNetIdRepl& UniqueId, const FString& Options, const FString& Portal)
{
	Super::InitNewPlayer(NewPlayerController, UniqueId, Options, Portal);

	// Get the Options (Username and Pass)
	FString Client_Name = UGameplayStatics::ParseOption(Options, TEXT("name")).Left(20);
	FString Char_Name = UGameplayStatics::ParseOption(Options, TEXT("charname")).Left(20);
	FString Char_ID = UGameplayStatics::ParseOption(Options, TEXT("charid")).Left(20);
	FString Char_Team = UGameplayStatics::ParseOption(Options, TEXT("team")).Left(20);

	// And Call the Event
	this->Server_LoginClient(NewPlayerController, Client_Name, Char_Name, Char_ID, Char_Team, Options);

	return TEXT("");
}