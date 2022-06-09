// Copyright (c) 2021 Orfeas Eleftheriou

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "TileMatrix.h"
#include "Engine/DataTable.h"
#include "DungeonGenerator.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(DungeonGenerator, Log, All);

class AStaticMeshActor;
class UStaticMesh;
class UMaterialInterface;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnDungeonSpawned);

USTRUCT(BlueprintType)
struct FRoomTemplate : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RoomTemplate")
	UStaticMesh* RoomTileMesh;

	/**
	 * If assigned, we're going to replace the default material of the RoomTileMesh with the given mat
	 * Leave empty in case you want the default material
	 * Useful in cases where you want the same static mesh but with different material variations as room templates
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category= "RoomTemplate")
	UMaterialInterface* RoomTileMeshMaterialOverride;

	/**
	 * Same functionality as FloorPivotOffset - check comments in Source Code or In-Editor Details Panel!
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RoomTemplate")
	FVector RoomTilePivotOffset;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RoomTemplate")
	UStaticMesh* WallMesh;

	/**
	 * Used to replace default material of WallMesh
	 * Check RoomTileMeshMaterialOverride docs for more info
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RoomTemplate")
	UMaterialInterface* WallMeshMaterialOverride;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RoomTemplate")
	FVector WallMeshPivotOffset;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RoomTemplate")
	bool bIsWallFacingX = true;
};


UCLASS()
class DUNGEONGENERATORPLUGIN_API ADungeonGenerator : public AActor
{
	GENERATED_BODY()
	
private:

	/**
	 * The tile matrix we're going to use to generate the floor tile locations and the wall locations / rotations
	 */
	FTileMatrix TileMatrix;

	/*void SpawnFloorTiles(const TArray<FVector>& SpawnLocations, UMaterialInterface* MaterialOverride = nullptr);

	void SpawnWallTiles(const TArray<FVector>& SpawnLocations, UMaterialInterface* MaterialOverride = nullptr);*/

	/**
	 * Destroys all previously generated meshes from this dungeon generator
	 */
	void DestroyDungeonMeshes();

	/**
	 * Spawns the assigned floorsm at the given transform
	 * @param InTransform - the transform to spawn the mesh at
	 * @param SMToSpawn - the mesh to spawn
	 * @param OverrideMaterial - if assigned, we're going to replace the 1st default material of SMToSpawn
	 * @return the spawned mesh. Should check for nullptr
	 */
	AStaticMeshActor* SpawnDungeonMesh(const FTransform& InTransform, UStaticMesh* SMToSpawn, UMaterialInterface* OverrideMaterial = nullptr);

	/**
	 * Checks the bounding box of the mesh and returns its extend along Y axis
	 * @return the extend along Y axis
	 */
	float CalculateFloorTileSize(const UStaticMesh& Mesh) const;

	/**
	 * Checks if a wall mesh needs to be rotated by 90 degrees
	 * @param bWallFacingXProperty - true if the wall mesh we're using is facing the X axis
	 * @param WallSpawnPoint - the wall spawn point we're using in order to spawn the wall
	 * @param LocationOffset - in case the wall needs rotating, we're first applying any rotation and then generate a correct location offset based on the original one
	 * @return the correct rotation of the assigned wall mesh
	 */
	FRotator CalculateWallRotation(bool bWallFacingXProperty, const FTileMatrix::FWallSpawnPoint& WallSpawnPoint, const FVector& WallPivotOffsetOverride, FVector& LocationOffset) const;

	/**
	 * Spawns a dungeon using random room templates from a provided data table
	 * Assumes the data table contains correct values in terms of mesh sizes etc.
	 */
	void SpawnDungeonFromDataTable();

	/**
	 * Spawns a generic dungeon using the same floor mesh and wall mesh for all the rooms/corridors
	 * @param FloorTileLocation - the locations for each floor tile
	 * @param WallSpawnPoints - the spawn points for each wall
	 */
	void SpawnGenericDungeon(const TArray<FVector>& FloorTileLocations, const TArray<FTileMatrix::FWallSpawnPoint>& WallSpawnPoints);

public:
	// Sets default values for this actor's properties
	ADungeonGenerator();

#if WITH_EDITOR

	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;

#endif

	/* True if you want the editor to auto-retrieve the extends of the mesh */
	UPROPERTY(EditAnywhere, Category = "Generator Properties")
	bool bAutoFloorTileSizeGeneration = true;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	/**
	 * Number of rows in tile map
	 * Total size of tile map will be Rows * Columns
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Generator Properties")
	int32 TileMapRows = 50;

	/**
	 * Number of columns in tile map
	 * Total size of tile map will be Rows * Columns
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Generator Properties")
	int32 TileMapColumns = 50;

	/**
	 * MinRoomSize is the min number of tiles we're going to use for each room in each direction.
	 * For instance, a number of 2 means that the min room size can be 2x2
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Generator Properties")
	int32 MinRoomSize = 5;

	/**
	 * MaxRoomSize is the max number of tiles we're going to use for each room in each direction.
	 * For instance, a number of 3 means that the max room size can be 3x3.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Generator Properties")
	int32 MaxRoomSize = 7;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Generator Properties")
	int32 RoomsToGenerate = 15;

	/**
	 * Max Random Attempts for each room. To avoid an infinite loop try to find a fitting room for a location only a certain amount of times.
	 * If the process fails just proceed to the next room
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category= "Generator Properties")
	int32 MaxRandomAttemptsPerRoom = 1500;

	/**
	 * The static mesh for each floor
	 */
	UPROPERTY(EditAnywhere, Category = "Generator Properties - Floor Settings")
	UStaticMesh* FloorSM;

	/**
	 * Tile size of floor SM. If you manually want to override this setting use the //TODO: Create function
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Generator Properties - Floor Settings")
	float FloorTileSize;

	/**
	 * By default, each tile location is pointing at the center of the tile. If your FloorSM isn't
	 * centered, adjust this value to match your settings
	 * For instance, if the floor's pivot point is located on the bottom left corner the FloorPivotOffset
	 * needs to be (-TileSize_X,-TileSize_Y,0)
	 */
	UPROPERTY(EditAnywhere, Category = "Generator Properties - Floor Settings")
	FVector FloorPivotOffset;

	/**
	 * Static mesh of walls
	 */
	UPROPERTY(EditAnywhere, Category = "Generator Properties - Wall Settings")
	UStaticMesh* WallSM;

	/**
	 * By default, each wall location is centered at the top,right,left or bottom middle of each tile. If your
	 * WallSM isn't centered, adjust this value to match your settings
	 * See FloorPivotOffset for more info regarding this setting.
	 */
	UPROPERTY(EditAnywhere, Category = "Generator Properties - Wall Settings")
	FVector WallSMPivotOffset;

	/**
	 * If the wall is facing the X axis (ie the wall is extending by default along the Y axis) mark this setting as true and false otherwise
	 * During wall spawning the generator will make sure to rotate the walls accordingly
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Generator Properties - Wall Settings")
	bool bWallFacingX = true;

	/**
	 * A data table describing some room templates in order to spawn various floor tiles & wall meshes
	 * Assumes that the assigned floor and wall meshes have the same dimensions as the generic floor and wall meshes
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Generator Properties")
	UDataTable* RoomTemplatesDataTable;

public:

	/**
	 * Will assign this tag to each mesh that is spawned by the generator.
	 * Used to destroy any previously generated dungeons if needed
	 */
	static const FName DUNGEON_MESH_TAG;

	/**
	 * Generates a dungeon
	 */
	UFUNCTION(BlueprintCallable, Category = "Dungeon Generation")
	void GenerateDungeon();

	/**
	 * Sets new properties regarding the room size
	 * @param NewMinRoomSize - the minimum room size (uniform)
	 * @param NewMaxRoomSize - the maximum room size (uniform)
	 */
	UFUNCTION(BlueprintCallable, Category = "Dungeon Generation")
	void SetNewRoomSize(int32 NewMinRoomSize, int32 NewMaxRoomSize);

	/**
	 * Assigns a new floor mesh for the dungeon generator
	 * @param NewFloorMesh - the mesh to use for each floor tile
	 * @param NewFloorPivotOffset - any offset to apply if the floor tile mesh hasn't a centered pivot point
	 * @param bAutoFloorSizeGeneration - true if you want the generator to generate the tile size for you
	 * @param OverrideFloorTileSize - the tile size. Only takes effect when bAutoFloorSizeGeneration is set to false
	 */
	UFUNCTION(BlueprintCallable, Category = "Dungeon Generation")
	void SetNewFloorMesh(UStaticMesh* NewFloorMesh, FVector NewFloorPivotOffset, bool bAutoFloorSizeGeneration = true, float OverrideFloorTileSize = 0.f);

	/**
	 * Assigns a new wall mesh for the dungeon generator
	 * @param NewWallMesh - the mesh to use for each wall
	 * @param NewWallSMPivotOffset - any offset to apply if the wall isn't centered
	 * @param bIsWallFacingX - true if the wall is facing the X axis (ie it extends along the Y axis), false otherwise
	 */
	UFUNCTION(BlueprintCallable, Category = "Dungeon Generation")
	void SetNewWallMesh(UStaticMesh* NewWallMesh, FVector NewWallSMPivotOffset, bool bIsWallFacingX = true);

	/**
	 * Called when dungeon generator has finished spawning all the meshes
	 */
	UPROPERTY(BlueprintAssignable, Category = "Dungeon Generation")
	FOnDungeonSpawned OnDungeonSpawned;

#if WITH_EDITORONLY_DATA

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dungeon Generation - Debug")
	bool bDebugActive=false;

	/**
	 * If debug is active the generator will spawn boxes in each spawn location.
	 * Adjust the size of the box here and the individual colors below
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dungeon Generation - Debug")
	FVector DebugVertexBoxExtents;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dungeon Generation - Debug")
	FLinearColor DefaultFloorSpawnLocationColor;

	/**
	 * If no offset is used this will overlap with DefaultFloorSpawnLocationColor box
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dungeon Generation - Debug")
	FLinearColor OffsetedFloorSpawnLocationColor;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dungeon Generation - Debug")
	FLinearColor DefaultWallSpawnLocationColor;

	/**
	 * If no offset is used this will overlap with DefaultWallSpawnLocationColor box
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dungeon Generation - Debug")
	FLinearColor OffsetedWallSpawnLocationColor;

#endif

};
