// Copyright (c) 2022 Orfeas Eleftheriou

#include "TileMatrix.h"

int32 FTileMatrix::ManhattanDistance(const Tile& A, const Tile& B)
{
	return FMath::Abs(A.Key - B.Key) + FMath::Abs(A.Value - B.Value);
}

void FTileMatrix::StoreGeneratedRoom(const FRoomTileCollection& InRoom)
{
	GeneratedRooms.Add(InRoom);
	if (GeneratedRooms.Num() > 1)
	{
		ConnectRooms(InRoom, GeneratedRooms.Last(1));
	}
}

void FTileMatrix::ConnectRooms(const FRoomTileCollection& A, const FRoomTileCollection& B)
{
	TArray<Tile> OccupiedTilesA = A.OccupiedTiles;
	TArray<Tile> OccupiedTilesB = B.OccupiedTiles;

	TArray<FTileConnection> TileConnections;

	//Connect all tiles from room with more tiles to all tiles of room with less tiles
	if (OccupiedTilesB.Num() > OccupiedTilesA.Num())
	{
		for (int32 i = 0; i < OccupiedTilesB.Num(); i++)
		{
			for (int32 j = 0; j < OccupiedTilesA.Num(); j++)
			{
				TileConnections.Add(FTileConnection(OccupiedTilesB[i], OccupiedTilesA[j]));
			}
		}
	}
	else
	{
		for (int32 i = 0; i < OccupiedTilesA.Num(); i++)
		{
			for (int32 j = 0; j < OccupiedTilesB.Num(); j++)
			{
				TileConnections.Add(FTileConnection(OccupiedTilesA[i], OccupiedTilesB[j]));
			}
		}
	}

	//Find shortest route
	//TODO: If PathLength == 1 rooms already connected
	int32 PathLength = MAX_int32;
	FTileConnection Path;

	for (int32 i = 0; i < TileConnections.Num(); i++)
	{
		if (TileConnections[i].Length() < PathLength)
		{
			Path = TileConnections[i];
			PathLength = TileConnections[i].Length();
		}
	}

	Tile PivotTile = Path.Start;

	while (PathLength > 1)
	{
		TArray<Tile> NearbyTiles = GetNearbyTiles(PivotTile);
		int32 TempPath = MAX_int32;
		Tile ClosestTile;
		//Get closest tile to target based on the nearby tiles
		for (int32 i = 0; i < NearbyTiles.Num(); i++)
		{
			if (ManhattanDistance(NearbyTiles[i], Path.End) < TempPath)
			{
				TempPath = ManhattanDistance(NearbyTiles[i], Path.End);
				ClosestTile = NearbyTiles[i];
			}
		}

		//GLog->Log("Added path on:"+TileToString(ClosestTile));
		OccupyTile(ClosestTile);
		PivotTile = ClosestTile;
		PathLength = ManhattanDistance(PivotTile, Path.End);
	}

}

FTileMatrix::FTileMatrix()
{
	//Init
	RowsNum = -1;
	ColumnsNum = -1;
	GeneratedRooms.Empty();
}

FTileMatrix::FTileMatrix(int32 RowCount, int32 ColumnCount)
{
	InitTileMap(RowCount, ColumnCount);
}

TArray<FTileMatrix::Tile> FTileMatrix::GetNearbyTiles(const Tile& InTile) const
{
	Tile UpTile = Tile(InTile.Key - 1, InTile.Value);
	Tile RightTile = Tile(InTile.Key, InTile.Value + 1);
	Tile LeftTile = Tile(InTile.Key, InTile.Value - 1);
	Tile DownTile = Tile(InTile.Key + 1, InTile.Value);

	TArray<Tile> NearbyTiles;
	if (IsTileInMap(UpTile))
	{
		NearbyTiles.Add(UpTile);
	}
	if (IsTileInMap(RightTile))
	{
		NearbyTiles.Add(RightTile);
	}
	if (IsTileInMap(LeftTile))
	{
		NearbyTiles.Add(LeftTile);
	}
	if (IsTileInMap(DownTile))
	{
		NearbyTiles.Add(DownTile);
	}
	return NearbyTiles;
}

void FTileMatrix::InitTileMap(int32 Rows, int32 Columns)
{
	RowsNum = Rows;
	ColumnsNum = Columns;

	TileMap.Empty();
	for (int32 i = 0; i < Rows; i++)
	{
		TArray<bool> SingleRow;
		SingleRow.Reserve(Columns);
		for (int32 j = 0; j < Columns; j++)
		{
			SingleRow.Add(false);
		}
		TileMap.Add(SingleRow);
	}
}

FString FTileMatrix::TileToString(const Tile& InTile) const
{
	return FString("[") + FString::FromInt(InTile.Key) + "," + FString::FromInt(InTile.Value) + FString("]");
}

void FTileMatrix::SetRoomSize(int32 NewMinRoomSize, int32 NewMaxRoomSize)
{
	MinRoomSize = NewMinRoomSize;
	MaxRoomSize = NewMaxRoomSize;
}

FTileMatrix::Tile FTileMatrix::GetRandomTile() const
{
	return Tile(FMath::RandRange(0, RowsNum - 1), FMath::RandRange(0, ColumnsNum - 1));
}

bool FTileMatrix::IsTileInMap(const Tile& InTile) const
{
	return TileMap.IsValidIndex(InTile.Key) && TileMap[InTile.Key].IsValidIndex(InTile.Value);
}

bool FTileMatrix::IsTileOccupied(Tile InTile) const
{
	if (IsTileInMap(InTile))
	{
		return (TileMap[InTile.Key])[InTile.Value];
	}
	return false;
}

bool FTileMatrix::AreTilesValid(const TArray<Tile>& InTiles) const
{
	bool bResult = true;
	for (int32 i = 0; i < InTiles.Num(); i++)
	{
		if (!IsTileInMap(InTiles[i]) || IsTileOccupied(InTiles[i]))
		{
			return false;
		}
	}
	return bResult;
}

bool FTileMatrix::GetLeftTile(const Tile& InTile, Tile& LeftTile) const
{
	if (IsTileInMap(InTile))
	{
		LeftTile = InTile;
		LeftTile.Value--;
		return IsTileInMap(LeftTile);
	}
	return false;
}

bool FTileMatrix::GetRightTile(const Tile& InTile, Tile& RightTile) const
{
	if (IsTileInMap(InTile))
	{
		RightTile = InTile;
		RightTile.Value++;
		return IsTileInMap(RightTile);
	}
	return false;
}

bool FTileMatrix::GetUpTile(const Tile& InTile, Tile& UpTile) const
{
	if (IsTileInMap(InTile))
	{
		UpTile = InTile;
		UpTile.Key--;
		return IsTileInMap(UpTile);
	}
	return false;
}

bool FTileMatrix::GetDownTile(const Tile& InTile, Tile& DownTile) const
{
	if (IsTileInMap(InTile))
	{
		DownTile = InTile;
		DownTile.Key++;
		return IsTileInMap(DownTile);
	}
	return false;
}

TArray<FTileMatrix::FWallSpawnPoint> FTileMatrix::GenerateWallSpawnPointsFromNearbyTiles(const Tile& CenterTile, float TileSize) const
{
	TArray<FWallSpawnPoint> WallSpawnPoints;

	FVector FloorCenter = FVector(CenterTile.Key * TileSize, CenterTile.Value * TileSize, 0);
	//up = -x
	//left = -y
	//down = +x
	//right = +y
	Tile NearbyTile;

	//Get nearby tile for each direction.
	//Store a wall location if:
	//The nearby tile isn't occupied OR
	//The nearby tile is out of bounds from the tilemap (means we're on the edge of the tiles)
	if ((GetUpTile(CenterTile, NearbyTile) && !IsTileOccupied(NearbyTile))
		|| !GetUpTile(CenterTile, NearbyTile))
	{
		FVector WallLocation = FloorCenter - FVector(TileSize / 2.f, 0.f, 0.f);
		WallSpawnPoints.Add(FWallSpawnPoint(WallLocation));
	}
	if ((GetRightTile(CenterTile, NearbyTile) && !IsTileOccupied(NearbyTile))
		|| !GetRightTile(CenterTile, NearbyTile))
	{
		FVector WallLocation = FloorCenter + FVector(0.f, TileSize / 2.f, 0.f);
		WallSpawnPoints.Add(FWallSpawnPoint(WallLocation, false));
	}
	if ((GetDownTile(CenterTile, NearbyTile) && !IsTileOccupied(NearbyTile))
		|| !GetDownTile(CenterTile, NearbyTile))
	{
		FVector WallLocation = FloorCenter + FVector(TileSize / 2.f, 0.f, 0.f);
		WallSpawnPoints.Add(FWallSpawnPoint(WallLocation));
	}
	if ((GetLeftTile(CenterTile, NearbyTile) && !IsTileOccupied(NearbyTile))
		|| !GetLeftTile(CenterTile, NearbyTile))
	{
		FVector WallLocation = FloorCenter - FVector(0.f, TileSize / 2.f, 0.f);
		WallSpawnPoints.Add(FWallSpawnPoint(WallLocation, false));
	}

	return WallSpawnPoints;
}

void FTileMatrix::OccupyTile(const Tile& InTile)
{
	TileMap[InTile.Key][InTile.Value] = true;
}

bool FTileMatrix::CreateUpperRightRoomExpansion(const Tile& StartTile, int32 ExpansionCount, TArray<Tile>& RoomTiles) const
{
	RoomTiles.Empty();
	Tile NewTile = Tile(StartTile.Key + 1, StartTile.Value);

	for (int32 i = 0; i < ExpansionCount; i++)
	{
		//We're moving up so decrement the key
		//NewTile.Key -= i;
		NewTile.Key--;
		NewTile.Value = StartTile.Value;

		for (int32 j = 0; j < ExpansionCount; j++)
		{
			//Moving right so increment the value
			NewTile.Value--;
			RoomTiles.Add(NewTile);
		}
	}
	return AreTilesValid(RoomTiles);
}

bool FTileMatrix::CreateLowerRightRoomExpansion(const Tile& StartTile, int32 ExpansionCount, TArray<Tile>& RoomTiles) const
{
	RoomTiles.Empty();
	Tile NewTile = Tile(StartTile.Key - 1, StartTile.Value);

	for (int32 i = 0; i < ExpansionCount; i++)
	{
		//We're moving down so increment the key
		NewTile.Key++;
		NewTile.Value = StartTile.Value;
		for (int32 j = 0; j < ExpansionCount; j++)
		{
			//Moving right so increment the value
			NewTile.Value++;
			RoomTiles.Add(NewTile);
		}
	}
	return AreTilesValid(RoomTiles);
}

bool FTileMatrix::CreateUpperLeftRoomExpansion(const Tile& StartTile, int32 ExpansionCount, TArray<Tile>& RoomTiles) const
{
	RoomTiles.Empty();
	Tile NewTile = Tile(StartTile.Key - 1, StartTile.Value);

	for (int32 i = 0; i < ExpansionCount; i++)
	{
		//We're moving up so increment the key
		NewTile.Key++;
		NewTile.Value = StartTile.Value;
		for (int32 j = 0; j < ExpansionCount; j++)
		{
			//Moving left so decrement the value
			NewTile.Value--;
			RoomTiles.Add(NewTile);
		}
	}
	return AreTilesValid(RoomTiles);
}

bool FTileMatrix::CreateLowerLeftRoomExpansion(const Tile& StartTile, int32 ExpansionCount, TArray<Tile>& RoomTiles) const
{
	RoomTiles.Empty();
	Tile NewTile = Tile(StartTile.Key + 1, StartTile.Value);

	for (int32 i = 0; i < ExpansionCount; i++)
	{
		//We're moving down so decrement the key
		NewTile.Key--;
		NewTile.Value = StartTile.Value;

		for (int32 j = 0; j < ExpansionCount; j++)
		{
			//Moving left so decrement the value
			NewTile.Value--;
			RoomTiles.Add(NewTile);
		}
	}

	return AreTilesValid(RoomTiles);
}

bool FTileMatrix::CanPlaceRoomInTileMap(Tile InTile, int32 RoomSize, TArray<Tile>& TilesToOccupy) const
{
	TilesToOccupy.Empty();
	if (!IsTileOccupied(InTile))
	{
		if (CreateUpperRightRoomExpansion(InTile, RoomSize, TilesToOccupy))
		{
			return true;
		}
		else if (CreateLowerRightRoomExpansion(InTile, RoomSize, TilesToOccupy))
		{
			return true;
		}
		else if (CreateUpperLeftRoomExpansion(InTile, RoomSize, TilesToOccupy))
		{
			return true;
		}
		else if (CreateLowerLeftRoomExpansion(InTile, RoomSize, TilesToOccupy))
		{
			return true;
		}
	}
	return false;
}

void FTileMatrix::CreateRooms(int32 RoomCount)
{
	GeneratedRooms.Empty();
	for (int32 i = 0; i < RoomCount; i++)
	{
		bool bGeneratedRandomRoom = false;

		for (int32 j = 0; j < MaxRandomAttemptsPerRoom && !bGeneratedRandomRoom; j++)
		{
			int32 RoomSize = FMath::RandRange(MinRoomSize, MaxRoomSize);
			Tile RandomTile = GetRandomTile();
			TArray<Tile> RoomTiles;

			if (CanPlaceRoomInTileMap(RandomTile, RoomSize, RoomTiles))
			{
				//Occupy tiles
				for (int32 k = 0; k < RoomTiles.Num(); k++)
				{
					OccupyTile(RoomTiles[k]);
				}
				StoreGeneratedRoom(FRoomTileCollection(RoomTiles));
				bGeneratedRandomRoom = true;
			}
		}

	}
	//PrintDebugTileMap();
}

void FTileMatrix::PrintDebugTileMap() const
{
	GLog->Log(" ---- Printing Debug Tile Map ---- ");
	for (int32 i = 0; i < TileMap.Num(); i++)
	{
		FString Row;
		for (int32 j = 0; j < TileMap[i].Num() - 1; j++)
		{
			FString ElementStr = ((TileMap[i])[j]) ? FString("1") : FString("0");
			ElementStr.Append(" - ");
			Row.Append(ElementStr);
		}
		FString ElementStr = ((TileMap[i])[TileMap[i].Num() - 1]) ? FString("1") : FString("0");
		Row.Append(ElementStr);

		GLog->Log(Row);
	}

	GLog->Log(" ---- End Of Printing Debug Tile Map ----");
}

void FTileMatrix::ProjectTileMapLocationsToWorld(float TileSize, TArray<FVector>& FloorLocations, TArray<FWallSpawnPoint>& WallLocations)
{
	FloorLocations.Empty();
	WallLocations.Empty();

	for (int32 i = 0; i < TileMap.Num(); i++)
	{
		for (int32 j = 0; j < TileMap[i].Num(); j++)
		{
			Tile CurrentTile = Tile(i, j);
			if (IsTileOccupied(CurrentTile))
			{
				FVector FloorCenter = FVector(i * TileSize, j * TileSize, 0);
				FloorLocations.Add(FloorCenter);

				TArray<FWallSpawnPoint> WallSpawnPoints = GenerateWallSpawnPointsFromNearbyTiles(CurrentTile, TileSize);
				for (int32 k = 0; k < WallSpawnPoints.Num(); k++)
				{
					WallLocations.Add(WallSpawnPoints[k]);
				}
			}
		}
	}
}

void FTileMatrix::ProjectTileMapLocationsToWorld(float TileSize, TArray<FRoom>& Rooms, TArray<FVector>& CorridorFloorTiles, TArray<FWallSpawnPoint>& CorridorWalls)
{
	//Stores the tiles that belong to rooms. If a tile isn't included in this set then it's a generic tile
	//And should be stored in the CorridorFloorTiles array (same goes with its respective walls)
	TSet<Tile> RecordedRoomTiles;

	Rooms.Empty();
	Rooms.Reserve(GeneratedRooms.Num() - 1);

	//Storing each room's tiles into a separate FRoom element
	for (int32 i = 0; i < GeneratedRooms.Num(); i++)
	{
		FRoom NewRoom;
		TArray<Tile> RoomTiles = GeneratedRooms[i].OccupiedTiles;

		for (int32 j = 0; j < RoomTiles.Num(); j++)
		{
			Tile CurrentTile = RoomTiles[j];
			RecordedRoomTiles.Add(CurrentTile); //Mark this tile as visited

			FVector FloorCenter = FVector(CurrentTile.Key * TileSize, CurrentTile.Value * TileSize, 0);
			NewRoom.FloorTileWorldLocations.Add(FloorCenter);
			TArray<FWallSpawnPoint> RoomWallSpawnPoints = GenerateWallSpawnPointsFromNearbyTiles(CurrentTile, TileSize);
			for (int32 k = 0; k < RoomWallSpawnPoints.Num(); k++)
			{
				NewRoom.WallSpawnPoints.Add(RoomWallSpawnPoints[k]);
			}

		}
		Rooms.Add(NewRoom);
	}

	for (int32 i = 0; i < TileMap.Num(); i++)
	{
		for (int32 j = 0; j < TileMap[i].Num(); j++)
		{
			Tile CurrentTile = Tile(i, j);
			if (IsTileOccupied(CurrentTile) && !RecordedRoomTiles.Contains(CurrentTile))
			{
				FVector FloorCenter = FVector(i * TileSize, j * TileSize, 0);
				CorridorFloorTiles.Add(FloorCenter);

				TArray<FWallSpawnPoint> WallSpawnPoints = GenerateWallSpawnPointsFromNearbyTiles(CurrentTile, TileSize);
				for (int32 k = 0; k < WallSpawnPoints.Num(); k++)
				{
					CorridorWalls.Add(WallSpawnPoints[k]);
				}
			}
		}
	}
}
