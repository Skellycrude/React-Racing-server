// Copyright 2017-2018 Robin Zinser. All Rights Reserved.

#include "Matchmaking.h"

#define LOCTEXT_NAMESPACE "FMatchmakingModule"

void FMatchmakingModule::StartupModule()
{
	// This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module
	
}

void FMatchmakingModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.
	
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FMatchmakingModule, Matchmaking)

DECLARE_LOG_CATEGORY_EXTERN(LogUDPCommand, Verbose, All);
DEFINE_LOG_CATEGORY(LogUDPCommand);
