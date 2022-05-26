// Copyright (c) 2022 Orfeas Eleftheriou

#include "DungeonGeneratorDetailsPanel.h"
#include "IDetailsView.h"
#include "DetailLayoutBuilder.h"
#include "DetailWidgetRow.h"
#include "DetailCategoryBuilder.h"
#include "Widgets/SNullWidget.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/SBoxPanel.h"
#include "DungeonGenerator.h"
#include "UObject/Class.h"
#include "Engine/World.h"
#include "Components/StaticMeshComponent.h"
#include "Kismet/GameplayStatics.h"

TSharedRef<IDetailCustomization> FDungeonGeneratorDetailsPanel::MakeInstance()
{
	return MakeShareable(new FDungeonGeneratorDetailsPanel);
}

void FDungeonGeneratorDetailsPanel::CustomizeDetails(IDetailLayoutBuilder& DetailBuilder)
{
	//Edits a category. If it doesn't exist it creates a new one
	IDetailCategoryBuilder& CustomCategory = DetailBuilder.EditCategory("Dungeon Generation");

	//Store the currently selected objects from the viewport to the SelectedObjects array.
	DetailBuilder.GetObjectsBeingCustomized(SelectedObjects);

	//Adding a custom row
	CustomCategory.AddCustomRow(FText::FromString("Outline Color Changing Category"))
		.ValueContent()
		.VAlign(VAlign_Center) // set vertical alignment to center
		.HAlign(HAlign_Fill)
		//.MaxDesiredWidth(250)
		[ //With this operator we declare a new slate object inside our widget row
		  //In this case the slate object is a button
			SNew(SButton)
			.VAlign(VAlign_Center)
			.OnClicked(this, &FDungeonGeneratorDetailsPanel::ClickedOnButton)
			.Content()
		[ 
			SNew(STextBlock).Text(FText::FromString("Generate Dungeon"))
		]
		];
}

FReply FDungeonGeneratorDetailsPanel::ClickedOnButton()
{
	if (GEngine)
	{
		for (const TWeakObjectPtr<UObject>& Object : SelectedObjects)
		{
			ADungeonGenerator* Generator = Cast<ADungeonGenerator>(Object.Get());
			if (Generator)
			{
				Generator->GenerateDungeon();
				TArray<AActor*> SpawnedMeshes;
				UGameplayStatics::GetAllActorsWithTag(Generator,ADungeonGenerator::DUNGEON_MESH_TAG, SpawnedMeshes);
				for (int32 i = 0; i < SpawnedMeshes.Num(); i++)
				{
					if (SpawnedMeshes[i])
					{
						if (UActorComponent* ActorComp = SpawnedMeshes[i]->GetComponentByClass(UStaticMeshComponent::StaticClass()))
						{
							if (UStaticMeshComponent* SMComp = Cast<UStaticMeshComponent>(ActorComp))
							{
								SMComp->SetMobility(EComponentMobility::Static);
							}
						}
					}
				}
				return FReply::Handled();
			}
		}
	}
	return FReply::Handled();
}