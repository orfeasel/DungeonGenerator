// Copyright (c) 2022 Orfeas Eleftheriou

#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"

class FDungeonGeneratorPluginModule : public IModuleInterface
{

public:

	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
};
