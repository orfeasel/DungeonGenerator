// Copyright (c) 2022 Orfeas Eleftheriou

#pragma once

#include "CoreMinimal.h"

/**
 * 
 */
class DUNGEONGENERATORPLUGIN_API FTileMatrix
{
public:

	/**
	 * Handy structure which stores a wall spawn point in the world
	 * Contains a boolean property to determine if we want to rotate the wall later on depending on the assigned wall settings in the Dungeon Generator
	 */
	struct FWallSpawnPoint
	{
		/* World location of this point */
		FVector WorldLocation;

		/************************************************************************/
		/* Used to determine which walls to rotate during spawning.				*/
		/* By default this property is true to sync settings between Dungeon	*/
		/* Generator. If the wall settings on the Dungeon Generator are flipped	*/
		/* the correct rotation is determined from the Dungeon Generator		*/
		/************************************************************************/

		bool bFacingX;

		FWallSpawnPoint() : WorldLocation(FVector()), bFacingX(true) {}

		FWallSpawnPoint(FVector NewWorldLocation) : WorldLocation(NewWorldLocation), bFacingX(true) {}

		FWallSpawnPoint(FVector NewWorldLocation, bool IsFacingX) : WorldLocation(NewWorldLocation), bFacingX(IsFacingX) {}
	};

	FTileMatrix();

	FTileMatrix(int32 RowCount, int32 ColumnCount);

	/**
	 * Returns true if the tile matrix has generated a tilemap
	 */
	inline bool IsValid() const { return RowsNum > 0 && ColumnsNum > 0; }

	/**
	 * Initializes tile map to Rows * Columns.
	 * @param Rows - the total number of rows
	 * @param Columns - the total number of columns
	 */
	void InitTileMap(int32 Rows, int32 Columns);

	/**
	 * Sets the new min and max room sizes.
	 * Keep in mind that size is uniform ie a value of 2 means that a room will try to occupy 4 tiles (2 in horizontal axis and 2 in vertical)
	 * @param NewMinRoomSize - the minimum size of the room in terms of tiles
	 * @param NewMaxRoomSize - the maximum size of the room in terms of tiles
	 */
	void SetRoomSize(int32 NewMinRoomSize, int32 NewMaxRoomSize);

	/**
	 * Max Random Attempts for each room. To avoid an infinite loop try to find a fitting room for a location only a certain amount of times.
	 * If the process fails just proceed to the next room
	 */
	int32 MaxRandomAttemptsPerRoom = 1500;

	/**
	 * Will try to create <=RoomCount rooms in the tilemap.
	 * If you're unlucky with the pseudorandoms you'll get fewer rooms.
	 * If you're getting a lower room count than what you expect try to fine tune Rows and Columns of TileMap as well as the MaxRandomAttemptsPerRoom
	 * @param RoomCount - max rooms to generate
	 */
	void CreateRooms(int32 RoomCount);

	/**
	 * Prints the generated Tile Map in the console
	 */
	void PrintDebugTileMap() const;

	/**
	 * Project the generated Tile Map in the world
	 * @param TileSize - the size of each tile (ie floor size)
	 * @param FloorLocations - the world location for every floor tile
	 * @param WallLocations - the world locations for every wall mesh packed in a FWallSpawnPoint structure to handle any rotations that need to take place
	 */
	void ProjectTileMapLocationsToWorld(float TileSize, TArray<FVector>& FloorLocations, TArray<FWallSpawnPoint>& WallLocations);

	struct FRoom
	{
		TArray<FVector> FloorTileWorldLocations;
		TArray<FWallSpawnPoint> WallSpawnPoints;
	};

	void ProjectTileMapLocationsToWorld(float TileSize, TArray<FRoom>& Rooms, TArray<FVector>& CorridorFloorTiles, TArray<FWallSpawnPoint>& CorridorWalls);

protected:

	/**
	 * Key: row of tile in TileMap
	 * Value: column of tile in TileMap
	 */
	typedef TTuple<int32, int32> Tile;

	/**
	 * Manhattan distance / Taxicab metric between two tiles
	 * @param A the first tile
	 * @param B the second tile
	 * @return the distance between A and B
	 */
	static int32 ManhattanDistance(const Tile& A, const Tile& B);

	/**
	 * Converts a tile to a readable string
	 */
	FString TileToString(const Tile& InTile) const;

private:

	/*TileMap Rows*/
	int32 RowsNum;
	/*TileMap Columns*/
	int32 ColumnsNum;

	/**
	 * Contents of this matrix.
	 * True values mean that a location / tile is occupied.
	 * False for tiles that are available
	 */
	TArray<TArray<bool>> TileMap;


	/**
	 * Room Sizes = tile count in length & width
	 */
	int32 MinRoomSize = 2;
	int32 MaxRoomSize = 4;

	/**
	 * Gets a random tile from the TileMap
	 * @return a random tile
	 */
	Tile GetRandomTile() const;

	/**
	 * Gets nearby tiles of a tile
	 * @param InTile - a tile in the tile map
	 * @return an array of tiles, containing tiles with a distance of 1 in directions up, right, left and down
	 */
	TArray<Tile> GetNearbyTiles(const Tile& InTile) const;

	/**
	 * Checking indices of tile to verify they are inside tile map's indices
	 */
	bool IsTileInMap(const Tile& InTile) const;

	/**
	 * Checks if a tile is occupied (ie its location in the TileMap is marked as true)
	 * @param InTile - the tile to check
	 * @return true if the corresponding location of the TileMap is marked as true, false otherwise
	 */
	bool IsTileOccupied(Tile InTile) const;

	/**
	 * Checks if all provided tiles are:
	 * Inside the tile map and
	 * NOT occupied
	 * @return true if all tiles are inside the tile map and available
	 */
	bool AreTilesValid(const TArray<Tile>& InTiles) const;

	/**
	 * Gets the tile which is located on the left side of a given tile
	 * @param InTile - the "pivot" tile
	 * @param LeftTile - the tile which is sitting on the left side of the pivot tile
	 * @return true, if the LeftTile is within the tilemap, false if we're on an edge case (meaning the InTile is the left-most tile in the tilemap)
	 */
	bool GetLeftTile(const Tile& InTile, Tile& LeftTile) const;

	/**
	 * Gets the tile which is located on the right side of a given tile
	 * @param InTile - the "pivot" tile
	 * @param RightTile - the tile which is sitting on the right side of the pivot tile
	 * @return true, if the RightTile is within the tilemap, false if we're on an edge case (meaning the InTile is the right-most tile in the tilemap)
	 */
	bool GetRightTile(const Tile& InTile, Tile& RightTile) const;

	/**
	 * Gets the tile which is located above a given tile
	 * @param InTile - the "pivot" tile
	 * @param UpTile - the tile which is sitting above the pivot tile
	 * @return true, if the UpTile is within the tilemap, false if we're on an edge case (meaning the InTile doesn't have a tile above)
	 */
	bool GetUpTile(const Tile& InTile, Tile& UpTile) const;

	/**
	 * Gets the tile which is located bellow a given tile
	 * @param InTile - the "pivot" tile
	 * @param DownTile - the tile which is sitting bellow the pivot tile
	 * @return true, if the DownTile is within the tilemap, false if we're on an edge case (meaning the InTile doesn't have a tile bellow)
	 */
	bool GetDownTile(const Tile& InTile, Tile& DownTile) const;

	TArray<FWallSpawnPoint> GenerateWallSpawnPointsFromNearbyTiles(const Tile& CenterTile, float TileSize) const;

	/**
	 * Marks the corresponding tilemap tile as true
	 */
	void OccupyTile(const Tile& InTile);

	/**
	 * Starts from a location and expands tiles to occupy the same space along up & right directions
	 * @param StartTile - the starting tile of the expansion
	 * @param ExpansionCount - the size of tiles to occupy above and right (ie a value of 3 will try to take up 9 tiles in total)
	 * @param RoomTiles - tiles that correspond to the specific expansion
	 * @return true, if all RoomTiles are valid, false otherwise
	 */
	bool CreateUpperRightRoomExpansion(const Tile& StartTile, int32 ExpansionCount, TArray<Tile>& RoomTiles) const;

	/* See CreateUpperRightRoomExpansion */
	bool CreateLowerRightRoomExpansion(const Tile& StartTile, int32 ExpansionCount, TArray<Tile>& RoomTiles) const;

	/* See CreateUpperRightRoomExpansion */
	bool CreateUpperLeftRoomExpansion(const Tile& StartTile, int32 ExpansionCount, TArray<Tile>& RoomTiles) const;

	/* See CreateUpperRightRoomExpansion */
	bool CreateLowerLeftRoomExpansion(const Tile& StartTile, int32 ExpansionCount, TArray<Tile>& RoomTiles) const;

	/**
	 * Handy way to store a connection between tiles
	 */
	struct FTileConnection
	{
		Tile Start;
		Tile End;

		FTileConnection()
		{
			Start = Tile(0, 0);
			End = Tile(0, 0);
		}
		FTileConnection(const Tile& TileA, const Tile& TileB) : Start(TileA), End(TileB) {}

		inline int32 Length() const { return ManhattanDistance(Start, End); }
	};

	/**
	 * Handy way of storing separate room tiles in a collection
	 */
	struct FRoomTileCollection
	{
		TArray<Tile> OccupiedTiles;

		FRoomTileCollection()
		{
			OccupiedTiles.Empty();
		}
		FRoomTileCollection(TArray<Tile> RoomTiles)
		{
			OccupiedTiles = RoomTiles;
		}
	};

	/**
	 * A collection of GeneratedRooms
	 */
	TArray<FRoomTileCollection> GeneratedRooms;

	/**
	 * Stores a new generated room.
	 * Once we have more than a single room will also call the ConnectRooms to connect newly spawned rooms
	 * @param InRoom - the new room we want to store
	 */
	void StoreGeneratedRoom(const FRoomTileCollection& InRoom);

	/**
	 * Finds the closest tiles between two rooms and connects them in the shortest possible distance
	 * @param A - the first room of the connection
	 * @param B - the second room to connect to first
	 */
	void ConnectRooms(const FRoomTileCollection& A, const FRoomTileCollection& B);

	/**
	 * Goes through all possible room expansions in a location to see if a room of a given size can be placed
	 * in the tilemap
	 * @param InTile - a tile to use as a starting location for the expansion
	 * @param RoomSize - uniform size of the room
	 * @param TilesToOccupy - expanded tiles of the room
	 * @return true, if the room can be placed in the tilemap, false otherwise
	 */
	bool CanPlaceRoomInTileMap(Tile InTile, int32 RoomSize, TArray<Tile>& TilesToOccupy) const;
};
