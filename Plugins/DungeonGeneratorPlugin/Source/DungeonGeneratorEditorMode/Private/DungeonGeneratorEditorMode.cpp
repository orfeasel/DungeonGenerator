// Copyright (c) 2022 Orfeas Eleftheriou

#include "DungeonGeneratorEditorMode.h"
#include "DungeonGenerator.h"
#include "DungeonGeneratorDetailsPanel.h"
#include "PropertyEditorModule.h"

#define LOCTEXT_NAMESPACE "FDungeonGeneratorEditorMode"

void FDungeonGeneratorEditorMode::StartupModule()
{
	// This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module

	//UE_LOG(FDungeonGeneratorEditorMode, Warning, TEXT("Loading Dungeon Generator Editor Plugin!"));
	FPropertyEditorModule& PropertyModule = FModuleManager::LoadModuleChecked<FPropertyEditorModule>("PropertyEditor");
	PropertyModule.RegisterCustomClassLayout(ADungeonGenerator::StaticClass()->GetFName(), FOnGetDetailCustomizationInstance::CreateStatic(&FDungeonGeneratorDetailsPanel::MakeInstance));
}

void FDungeonGeneratorEditorMode::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.
	//UE_LOG(FDungeonGeneratorPluginModule, Warning, TEXT("Dungeon Generator Plugin has shut down!"));
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FDungeonGeneratorEditorMode, DungeonGeneratorPlugin)