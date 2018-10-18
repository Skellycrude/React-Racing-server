// Copyright 2017-2018 Robin Zinser. All Rights Reserved.

#pragma once

#include "Runtime/Core/Public/GenericPlatform/GenericPlatformProcess.h"
#include "Runtime/Engine/Classes/GameFramework/PlayerController.h"
#include "Engine.h"
#include "MatchmakingBPLibrary.generated.h"


UCLASS()
class UMatchmakingBPLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_UCLASS_BODY()
		
	// Start a Dedicated Server on given Path with given Map
	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Start Dedicated Server", Keywords = "Matchmaking Start Dedicated Server"), Category = "Matchmaking")
	static void Matchmaking_Server_Start(FString ServerPath, FString Map);

	// Start a Dedicated Server on given Path with given Map
	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Travel to Dedicated Server", Keywords = "Matchmaking Travel to Dedicated Server"), Category = "Matchmaking")
	static void Matchmaking_Client_Travel(APlayerController* PlayerController, FString Adress);

	// Get (if this is the Server) the Port the Server is running on
	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Get used Server Port", Keywords = "Matchmaking Get used Server Port"), Category = "Matchmaking")
	static int Matchmaking_Server_GetPort(AActor* SourceActor);

	// return True if the Project is running in the Editor
	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Is running in Editor", Keywords = "Matchmaking running in Editor"), Category = "Matchmaking")
	static bool Matchmaking_RunningInEditor();

	// Shut down the server (used for dedicated servers)
	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Shut down Dedicated Server", Keywords = "Matchmaking Dedicated server shutdown"), Category = "Matchmaking")
	static void Matchmaking_DedicatedServer_ShutDown();
};
