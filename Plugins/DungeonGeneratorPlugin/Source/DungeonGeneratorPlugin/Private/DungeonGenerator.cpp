// Copyright (c) 2022 Orfeas Eleftheriou

#include "DungeonGenerator.h"
#include "DrawDebugHelpers.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Engine/StaticMeshActor.h"
#include "Materials/MaterialInterface.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"


DEFINE_LOG_CATEGORY(DungeonGenerator);

const FName ADungeonGenerator::DUNGEON_MESH_TAG = FName("Orfeas_Dungeon_Generator");

float ADungeonGenerator::CalculateFloorTileSize(const UStaticMesh& Mesh) const
{
	return FMath::Abs(Mesh.GetBoundingBox().Min.Y) + FMath::Abs(Mesh.GetBoundingBox().Max.Y);
}

FRotator ADungeonGenerator::CalculateWallRotation(bool bWallFacingXProperty, const FTileMatrix::FWallSpawnPoint& WallSpawnPoint, const FVector& WallPivotOffsetOverride, FVector& LocationOffset) const
{
	FRotator WallRotation = FRotator::ZeroRotator;
	LocationOffset = FVector();

	//If the point is generated in a way that is looking at the X axis and the wall is rotated to look at Y make sure to 
	//rotate the wall and apply an offset
	//Note: Points looking at X axis are spread along Y
	//WallSpawnPoint.bFacingX = true when the wall is located in an "up/down" tile
	if (!bWallFacingXProperty && WallSpawnPoint.bFacingX)
	{
		WallRotation = FRotator(0.f, -90.f, 0.f);
		//LocationOffset.Y += FMath::Abs(WallSMPivotOffset.X);
		LocationOffset.Y += FMath::Abs(WallPivotOffsetOverride.X);
	}
	else if (!WallSpawnPoint.bFacingX && bWallFacingXProperty)
	{
		WallRotation = FRotator(0.f, -90.f, 0.f);
	}
	else //No rotation adjustments needed; just apply the original offset
	{
		//LocationOffset+=WallSMPivotOffset;
		LocationOffset += WallPivotOffsetOverride;
	}


	return WallRotation;
}

void ADungeonGenerator::SpawnDungeonFromDataTable()
{
	TArray<FRoomTemplate*> RoomTemplates;
	FString ContextStr;
	RoomTemplatesDataTable->GetAllRows<FRoomTemplate>(ContextStr, RoomTemplates);

	ensure(RoomTemplates.Num() > 0);

	float DataTableFloorTileSize = CalculateFloorTileSize(*(*RoomTemplates[0]).RoomTileMesh);

	TArray<FTileMatrix::FRoom> Rooms;
	TArray<FVector> CorridorFloorTiles;
	TArray<FTileMatrix::FWallSpawnPoint> CorridorWalls;
	TileMatrix.ProjectTileMapLocationsToWorld(DataTableFloorTileSize, Rooms, CorridorFloorTiles, CorridorWalls);

	//Spawn rooms & walls using a random template from the provided table
	for (int32 i = 0; i < Rooms.Num(); i++)
	{
		FRoomTemplate RoomTemplate = *RoomTemplates[FMath::RandRange(0, RoomTemplates.Num() - 1)];

		for (int32 j = 0; j < Rooms[i].FloorTileWorldLocations.Num(); j++)
		{
			FVector WorldSpawnLocation = Rooms[i].FloorTileWorldLocations[j];
			SpawnDungeonMesh(FTransform(FRotator::ZeroRotator, WorldSpawnLocation + RoomTemplate.RoomTilePivotOffset), RoomTemplate.RoomTileMesh, RoomTemplate.RoomTileMeshMaterialOverride);
		}

		for (int32 j = 0; j < Rooms[i].WallSpawnPoints.Num(); j++)
		{
			//FVector WorldSpawnLocation = Rooms[i].WallSpawnPoints[j].WorldLocation;
			FVector WallModifiedOffset = FVector();
			FRotator WallRotation = CalculateWallRotation(RoomTemplate.bIsWallFacingX, Rooms[i].WallSpawnPoints[j], RoomTemplate.WallMeshPivotOffset, WallModifiedOffset);
			FVector WallSpawnLocation = Rooms[i].WallSpawnPoints[j].WorldLocation + WallModifiedOffset;
			SpawnDungeonMesh(FTransform(WallRotation, WallSpawnLocation), RoomTemplate.WallMesh, RoomTemplate.WallMeshMaterialOverride);
		}
	}

	
	//Get the 1st element of the data table to retrieve any pivot offsets
	//The 1st row of the data table will be used to create corridors connecting various spawned rooms
	FVector FloorTileOffset = RoomTemplates[0]->RoomTilePivotOffset;
	UStaticMesh* CorridorFloorTile = RoomTemplates[0]->RoomTileMesh;
	UStaticMesh* CorridorWall = RoomTemplates[0]->WallMesh;

	//Spawn floor tiles for corridors
	for (int32 i = 0; i < CorridorFloorTiles.Num(); i++)
	{
		//CorridorFloorTiles[i]+=FloorTileOffset;
		SpawnDungeonMesh(FTransform(FRotator::ZeroRotator,CorridorFloorTiles[i] + FloorTileOffset), CorridorFloorTile);
	}

	bool bCorridorWallFacingX = RoomTemplates[0]->bIsWallFacingX;
	FVector RoomTemplateWallOffset = RoomTemplates[0]->WallMeshPivotOffset;

	//Spawn walls for corridors
	for (int32 i = 0; i < CorridorWalls.Num(); i++)
	{
		FVector WallModifiedOffset = FVector();
		FRotator WallRotation = CalculateWallRotation(bCorridorWallFacingX, CorridorWalls[i], RoomTemplateWallOffset, WallModifiedOffset);
		FVector WallSpawnPoint = CorridorWalls[i].WorldLocation + WallModifiedOffset;

		SpawnDungeonMesh(FTransform(WallRotation,WallSpawnPoint), CorridorWall);
	}
}

void ADungeonGenerator::SpawnGenericDungeon(const TArray<FVector>& FloorTileLocations, const TArray<FTileMatrix::FWallSpawnPoint>& WallSpawnPoints)
{
	for (int32 i = 0; i < FloorTileLocations.Num(); i++)
	{
		//Draw debug boxes if needed
#if WITH_EDITOR
		if (bDebugActive)
		{
			DrawDebugBox(GetWorld(), FloorTileLocations[i], DebugVertexBoxExtents, DefaultFloorSpawnLocationColor.ToFColor(true), true, 1555.f, 15);
			DrawDebugBox(GetWorld(), FloorTileLocations[i] + FloorPivotOffset, DebugVertexBoxExtents, OffsetedFloorSpawnLocationColor.ToFColor(true), true, 1555.f, 15);
		}
#endif

		SpawnDungeonMesh(FTransform(FRotator::ZeroRotator, FloorTileLocations[i] + FloorPivotOffset), FloorSM);
	}
	for (int32 i = 0; i < WallSpawnPoints.Num(); i++)
	{
		
		FVector WallModifiedOffset = FVector();
		FRotator WallRotation = CalculateWallRotation(bWallFacingX, WallSpawnPoints[i], WallSMPivotOffset, WallModifiedOffset);
		FVector WallSpawnPoint = WallSpawnPoints[i].WorldLocation + WallModifiedOffset;

		//Draw debug boxes if needed
#if WITH_EDITOR
		if (bDebugActive)
		{
			DrawDebugBox(GetWorld(), WallSpawnPoints[i].WorldLocation, DebugVertexBoxExtents, DefaultWallSpawnLocationColor.ToFColor(true), true, 1555.f, 15);
			DrawDebugBox(GetWorld(), WallSpawnPoint, DebugVertexBoxExtents, OffsetedWallSpawnLocationColor.ToFColor(true), true, 1555.f, 15);
		}
#endif

		SpawnDungeonMesh(FTransform(WallRotation,WallSpawnPoint),WallSM);
	}

	if (OnDungeonSpawned.IsBound())
	{
		OnDungeonSpawned.Broadcast();
	}
}

void ADungeonGenerator::DestroyDungeonMeshes()
{
	//Erase previously spawned stuff
	TArray<AActor*> SpawnedActors;
	UGameplayStatics::GetAllActorsOfClassWithTag(GetWorld(), AActor::StaticClass(), DUNGEON_MESH_TAG, SpawnedActors);

	for (int32 i = SpawnedActors.Num() - 1; i >= 0; i--)
	{
		if (SpawnedActors[i])
		{
			SpawnedActors[i]->Destroy();
		}
	}
}

AStaticMeshActor* ADungeonGenerator::SpawnDungeonMesh(const FTransform& InTransform, UStaticMesh* SMToSpawn, UMaterialInterface* OverrideMaterial)
{
	FActorSpawnParameters ActorSpawnParams;
	ActorSpawnParams.Owner = this;

	AStaticMeshActor* SMActor = GetWorld()->SpawnActor<AStaticMeshActor>(AStaticMeshActor::StaticClass(), InTransform, ActorSpawnParams);
	if (SMActor)
	{
		//Only needed for runtime because the editor generates warnings and doesn't assign meshes
		//Meshes will switch static if used from within the editor
		SMActor->SetMobility(EComponentMobility::Movable);

		SMActor->GetStaticMeshComponent()->SetStaticMesh(SMToSpawn);

		if (OverrideMaterial)
		{
			SMActor->GetStaticMeshComponent()->SetMaterial(0,OverrideMaterial);
		}

		SMActor->Tags.Add(DUNGEON_MESH_TAG);
	}
	return SMActor;
}

// Sets default values
ADungeonGenerator::ADungeonGenerator()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

}

// Called when the game starts or when spawned
void ADungeonGenerator::BeginPlay()
{
	Super::BeginPlay();
}

#if WITH_EDITOR
void ADungeonGenerator::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	if (FloorSM && bAutoFloorTileSizeGeneration)
	{
		FloorTileSize = CalculateFloorTileSize(*FloorSM);
	}

}
#endif

void ADungeonGenerator::GenerateDungeon()
{
	
	TileMatrix = FTileMatrix(TileMapRows, TileMapColumns);
	TileMatrix.MaxRandomAttemptsPerRoom = MaxRandomAttemptsPerRoom;
	TileMatrix.SetRoomSize(MinRoomSize, MaxRoomSize);

	TileMatrix.CreateRooms(RoomsToGenerate);
	DestroyDungeonMeshes();

	if (RoomTemplatesDataTable)
	{
		SpawnDungeonFromDataTable();
	}
	else
	{
		if (!FloorSM)
		{
			UE_LOG(DungeonGenerator, Warning, TEXT("Cannot generate dungeon"));
			UE_LOG(DungeonGenerator, Error, TEXT("Invalid FloorSM. Verify you have assigned a valid floor mesh"));
			return;
		}

		if (!WallSM)
		{
			UE_LOG(DungeonGenerator, Warning, TEXT("Cannot generate dungeon"));
			UE_LOG(DungeonGenerator, Error, TEXT("Invalid WallSM. Verify you have assigned a valid wall mesh"));
			return;
		}

		TArray<FVector> FloorTiles;
		TArray<FTileMatrix::FWallSpawnPoint> WallSpawnPoints;
		TileMatrix.ProjectTileMapLocationsToWorld(FloorTileSize, FloorTiles, WallSpawnPoints);
		SpawnGenericDungeon(FloorTiles, WallSpawnPoints);
	}
}

void ADungeonGenerator::SetNewRoomSize(int32 NewMinRoomSize, int32 NewMaxRoomSize)
{
	MinRoomSize = NewMinRoomSize;
	MaxRoomSize = NewMaxRoomSize;
}

void ADungeonGenerator::SetNewFloorMesh(UStaticMesh* NewFloorMesh, FVector NewFloorPivotOffset, bool bAutoFloorSizeGeneration, float OverrideFloorTileSize)
{
	if (NewFloorMesh)
	{
		FloorSM = NewFloorMesh;

		FloorTileSize = (bAutoFloorTileSizeGeneration) ? CalculateFloorTileSize(*FloorSM) : OverrideFloorTileSize;
		FloorPivotOffset = NewFloorPivotOffset;
	}
}

void ADungeonGenerator::SetNewWallMesh(UStaticMesh* NewWallMesh, FVector NewWallSMPivotOffset, bool bIsWallFacingX /*= true*/)
{
	if (NewWallMesh)
	{
		WallSM = NewWallMesh;
		WallSMPivotOffset = NewWallSMPivotOffset;
		bWallFacingX = bIsWallFacingX;
	}
}
