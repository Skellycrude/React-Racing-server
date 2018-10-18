// Copyright 2017-2018 Robin Zinser. All Rights Reserved.

#pragma once

#include "ModuleManager.h"

class FMatchmakingModule : public IModuleInterface
{
public:

	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
};