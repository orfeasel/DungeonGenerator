// Copyright (c) 2022 Orfeas Eleftheriou

#pragma once
 
#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"
 

class FDungeonGeneratorEditorMode : public IModuleInterface
{
	public:
 
	/* This will get called when the editor loads the module */
	virtual void StartupModule() override;
 
	/* This will get called when the editor unloads the module */
	virtual void ShutdownModule() override;
};