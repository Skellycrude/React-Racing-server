// Copyright 2017-2018 Robin Zinser. All Rights Reserved.

#include "MatchmakingBPLibrary.h"
#include "Matchmaking.h"

UMatchmakingBPLibrary::UMatchmakingBPLibrary(const FObjectInitializer& ObjectInitializer)
: Super(ObjectInitializer)
{

}


// Matchmaking_Server_Start()
//
// Task: Start a Dedicated Server on given Path with given Map
//
void UMatchmakingBPLibrary::Matchmaking_Server_Start(FString ServerPath, FString Map) {
	
	FPlatformProcess::CreateProc(*ServerPath, *Map, true, false, false, nullptr, 0, 0, nullptr);
};
// Matchmaking_Server_Start()



// Matchmaking_Server_Start()
//
// Task: Start a Dedicated Server on given Path with given Map
//
void UMatchmakingBPLibrary::Matchmaking_Client_Travel(APlayerController* PlayerController, FString Adress) {

	FGuid TempGUID;
	PlayerController->ClientTravel(Adress, TRAVEL_Relative, false, TempGUID);
};
// Matchmaking_Server_Start()



// Matchmaking_Server_GetPort()
//
// Task: Get (if this is the Server) the Port the Server is running on
//
int UMatchmakingBPLibrary::Matchmaking_Server_GetPort(AActor* SourceActor) {

	if (SourceActor->GetWorld()->IsServer()) {
		return SourceActor->GetWorld()->URL.Port;
	}

	return -1;
};
// Matchmaking_Server_GetPort()



// Matchmaking_RunningInEditor()
//
// Task: return True if the Project is running in the Editor
//
bool UMatchmakingBPLibrary::Matchmaking_RunningInEditor() {
	#if WITH_EDITOR
		return true;
	#else
		return false;
	#endif
};
// Matchmaking_RunningInEditor()



// Matchmaking_DedicatedServer_ShutDown()
//
// Task: Shut down the server (used for dedicated servers)
//
void UMatchmakingBPLibrary::Matchmaking_DedicatedServer_ShutDown() {
	FGenericPlatformMisc::RequestExit(false);
};
// Matchmaking_DedicatedServer_ShutDown()