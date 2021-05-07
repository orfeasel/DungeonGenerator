// Copyright Epic Games, Inc. All Rights Reserved.

#include "DungeonGeneratorPlugin.h"
#include "DungeonGenerator.h"
#include "Editor/DungeonGeneratorDetailsPanel.h"
#include "PropertyEditorModule.h"

#define LOCTEXT_NAMESPACE "FDungeonGeneratorPluginModule"

void FDungeonGeneratorPluginModule::StartupModule()
{
	// This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module

	//UE_LOG(FDungeonGeneratorPluginModule, Warning, TEXT("Loading Dungeon Generator Plugin!"));
	FPropertyEditorModule& PropertyModule = FModuleManager::LoadModuleChecked<FPropertyEditorModule>("PropertyEditor");
	PropertyModule.RegisterCustomClassLayout(ADungeonGenerator::StaticClass()->GetFName(), FOnGetDetailCustomizationInstance::CreateStatic(&FDungeonGeneratorDetailsPanel::MakeInstance));
}

void FDungeonGeneratorPluginModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.
	//UE_LOG(FDungeonGeneratorPluginModule, Warning, TEXT("Dungeon Generator Plugin has shut down!"));
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FDungeonGeneratorPluginModule, DungeonGeneratorPlugin)