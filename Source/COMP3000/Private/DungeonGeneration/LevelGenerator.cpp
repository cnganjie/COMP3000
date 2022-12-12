// Fill out your copyright notice in the Description page of Project Settings.


#include "DungeonGeneration/LevelGenerator.h"

#include "Kismet/GameplayStatics.h"

// Sets default values
ALevelGenerator::ALevelGenerator()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	TilesSize = FVector(3000, 3000, 3000);

	//Points to seek in generation
	SeekingPoints.Add(ECardinalPoints::North, FVector2D(1, 0));
	SeekingPoints.Add(ECardinalPoints::East, FVector2D(0, 1));
	SeekingPoints.Add(ECardinalPoints::South, FVector2D(-1, 0));
	SeekingPoints.Add(ECardinalPoints::West, FVector2D(0, -1));

}

// Called when the game starts or when spawned
void ALevelGenerator::BeginPlay()
{
	Super::BeginPlay();
	GenerateLevel();

	PlayerController = UGameplayStatics::GetPlayerController(GetWorld(), 0);
}

// Called every frame
void ALevelGenerator::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	//DEBUG 
	if (PlayerController != nullptr)
	{
		if (PlayerController->WasInputKeyJustPressed(EKeys::E))
		{
			ResetGeneratedLevel();
			GenerateLevel();
		}
	}

}

void ALevelGenerator::GenerateLevel()
{
	SetOffTiles(); //Disable vectors inside level generation.
	TileLayoutGeneration();
	GenerateNewLinks();
	SpawnTileLayout();
	this->SetActorRotation(FRotator(0, 45, 0));
}

void ALevelGenerator::ResetGeneratedLevel()
{
	//UE_LOG(LogTemp, Warning, TEXT("GripState: %d"), SpawnedTiles.Num());
	for (auto SpawnedTile : SpawnedTiles)
	{
		SpawnedTile->Destroy();
	}
	SpawnedTiles.Empty();
	TileLayout.Empty();
}

//Generate dungeon layout
void ALevelGenerator::TileLayoutGeneration()
{
	//Start Tile
	FTileInfo BuildInfo;
	BuildInfo.Type = ETileType::StartTile;
	BuildInfo.Directions = StartTile->GetDefaultObject<ATileBase>()->TileConnections;
	TileLayout.Add(FVector2D(0,0), BuildInfo);

	PreviousLocation = FVector2D(0,0);
	CurrentLocation = FVector2D(0,0);
	TempDirect = StartDirection;

	FTileInfo MakerInfo;
	//Loop as many times as ValueAmount (amount of tiles to generate)
	for (int i = 0; i < ValueAmount; i++)
	{
		
		if (TileLayout.Num() - OffTiles.Num() == 1) CurrentLocation = FindNextRandomLocation(PreviousLocation, StartDirection);
		else CurrentLocation = FindNextRandomLocation(PreviousLocation, Random);
		if (CurrentLocation == PreviousLocation) CurrentLocation = Backtracker(PreviousLocation, Random);
		TileLayout.Add(CurrentLocation, MakerInfo);
		TileConnector(PreviousLocation, CurrentLocation, TempDirect);
		PreviousLocation = CurrentLocation;
	}
	
	MakerInfo.Type = ETileType::EndTile;
	CurrentLocation = FindNextRandomLocation(PreviousLocation, Random);
	if (CurrentLocation == PreviousLocation) CurrentLocation = Backtracker(PreviousLocation, Random);
	TileLayout.Add(CurrentLocation, MakerInfo);
	TileConnector(PreviousLocation, CurrentLocation, TempDirect);
}

//Iterate through TileLayout and spawn each item.
void ALevelGenerator::SpawnTileLayout()
{
	for (auto Tile : TileLayout)
	{
		std::vector<UClass*> ValidTiles;
		switch (Tile.Value.Type)
		{
			case ETileType::StartTile:
				SpawnTile(StartTile, GetTileWorldspace(Tile.Key));
				break;
			case ETileType::GenericTile:
				if (LinkAssetTiles(Tile, GenericTiles) != nullptr) ValidTiles.push_back(LinkAssetTiles(Tile, GenericTiles));
				if (ValidTiles.size() > 0) SpawnTile(ValidTiles[FMath::RandRange(0, ValidTiles.size()-1)], GetTileWorldspace(Tile.Key));
				break;
			case ETileType::EndTile:
				if (LinkAssetTiles(Tile, EndTiles) != nullptr) ValidTiles.push_back(LinkAssetTiles(Tile, EndTiles));
				if (ValidTiles.size() > 0) SpawnTile(ValidTiles[FMath::RandRange(0, ValidTiles.size()-1)], GetTileWorldspace(Tile.Key));
				break;
		}
	}
}

//Spawn a Tile
void ALevelGenerator::SpawnTile(UClass* TileClass, const FVector& position)
{
	//SpawnedActor Rules
	FActorSpawnParameters SpawnInfo;
	SpawnInfo.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	FAttachmentTransformRules TransformInfo (EAttachmentRule::KeepRelative,
		EAttachmentRule::SnapToTarget, EAttachmentRule::KeepRelative,true);
	
	ATileBase* TempTile = GetWorld()->SpawnActor<ATileBase>(TileClass, position, FRotator::ZeroRotator, SpawnInfo);
	TempTile->AttachToActor(this, TransformInfo);
	SpawnedTiles.Add(TempTile);
}

//Get a tiles vector2d, convert into worldspace location.
FVector ALevelGenerator::GetTileWorldspace(FVector2D InVec)
{
	return FVector(InVec.X * TilesSize.X, InVec.Y * TilesSize.Y, 0);
}

FVector2D ALevelGenerator::FindNextRandomLocation(FVector2D InVec, TEnumAsByte<ECardinalPoints> WantedDirection)
{
	//Locations to seek
	if (WantedDirection == ECardinalPoints::Random)
	{
		std::vector<FVector2D> ValidLocations;
		std::vector<TEnumAsByte<ECardinalPoints>> ValidDirects;
		for (auto CurrentSeek : SeekingPoints) {
			if (TileLayout.Find(InVec + CurrentSeek.Value) == nullptr) {
				ValidLocations.push_back(InVec + CurrentSeek.Value);
				ValidDirects.push_back(CurrentSeek.Key);
			}
		}
		int rand = FMath::RandRange(0, ValidLocations.size() - 1);
		if (ValidLocations.size() > 1) {
			if (IsThisCorridor(InVec, ValidDirects[rand]))
			{
				CorridorsSpawned++;
				if (CorridorsSpawned >= MaxInRowAmount)
				{
					UE_LOG(LogTemp, Warning, TEXT("Corridors Spawned: %d"), CorridorsSpawned);
					ValidLocations.erase(ValidLocations.begin() + rand);
					ValidDirects.erase(ValidDirects.begin() + rand);
					CorridorsSpawned = 0;
					if (ValidLocations.size() > 1)
					{
						rand = FMath::RandRange(0, ValidLocations.size() - 1);
						TempDirect = ValidDirects[rand];
						return ValidLocations[rand];
					}
				}
			}
			else
			{
				CorridorsSpawned = 0;
				TempDirect = ValidDirects[rand];
				return ValidLocations[rand];
			}
			
		}
		if (ValidLocations.size() > 0) {
			TempDirect = ValidDirects[0];
			return ValidLocations[0];
		}
		return InVec;
	}
	if (TileLayout.Find(InVec + SeekingPoints.FindRef(WantedDirection)) == nullptr)
	{
		TempDirect = WantedDirection;
		return InVec + SeekingPoints.FindRef(WantedDirection);
	}
	return InVec;
}

FVector2D ALevelGenerator::Backtracker(FVector2D InVec, TEnumAsByte<ECardinalPoints> WantedDirection)
{
	UE_LOG(LogTemp, Warning, TEXT("Deadend: %d"), TileLayout.Num());
	TArray<FVector2D> Tilelocations;
	TileLayout.GenerateKeyArray(Tilelocations);
	for (int i = 0; i < OffTiles.Num(); i++) Tilelocations.RemoveAt(0);
	int counter = Tilelocations.Num() - 1;
	FVector2D returnVec = InVec;
	for (counter; counter > 1; counter--)
	{
		returnVec = Tilelocations[counter];
		UE_LOG(LogTemp, Warning, TEXT("ReturnVec: %s"), *returnVec.ToString());
		if (returnVec != FindNextRandomLocation(Tilelocations[counter], WantedDirection))
		{
			UE_LOG(LogTemp, Warning, TEXT("ReturnVecDone: %s"), *returnVec.ToString());
			UE_LOG(LogTemp, Warning, TEXT("Tilelocations: %s"), *Tilelocations[counter].ToString());
			PreviousLocation = Tilelocations[counter];
			return FindNextRandomLocation(Tilelocations[counter], WantedDirection);
		}
	}
	return InVec;
}

void ALevelGenerator::TileConnector(FVector2D FirstTileVec, FVector2D SecondTileVec, TEnumAsByte<ECardinalPoints> FirstCardinal)
{
	//UE_LOG(LogTemp, Warning, TEXT("GripState: %i"), FirstCardinal);
	SetConnection(FirstTileVec, FirstCardinal, true);
	SetConnection(SecondTileVec, DirectionInverter(FirstCardinal), true);
}

void ALevelGenerator::SetConnection(FVector2D InTileVec, TEnumAsByte<ECardinalPoints> ToCardinal, bool enable)
{
	if (TileLayout.Find(InTileVec) != nullptr) TileLayout.Find(InTileVec)->Directions.Add(ToCardinal, enable);
}

UClass* ALevelGenerator::LinkAssetTiles(TTuple<UE::Math::TVector2<double>, FTileInfo> InTile, TArray<UClass*> InList)
{
	for (auto CurrentGenericTile : InList)
	{
		//Loop through current tile, compare against list of generics. If both directions are equal, add to list of valid spawns for a tile.
		TArray<bool> CurrentTileMapDirections;
		TArray<bool> CurrentTileListDirections;
		InTile.Value.Directions.GenerateValueArray(CurrentTileMapDirections);
		CurrentGenericTile->GetDefaultObject<ATileBase>()->TileConnections.GenerateValueArray(CurrentTileListDirections);
		for (int i = 0; i < 4; i++)
		{
			if (CurrentTileMapDirections[i] != CurrentTileListDirections[i]) break;
			if (i == 3) return CurrentGenericTile;
		}
	}
	return nullptr;
}

void ALevelGenerator::GenerateNewLinks()
{
	int counter = 0;
	for (auto Tile : TileLayout)
	{
		if (counter >= ExtraLinksAmount) break;
		
		std::vector<TTuple<UE::Math::TVector2<double>, FTileInfo>> ValidLocations;
		for (auto CurrentSeek : SeekingPoints) {
			if (TileLayout.Find(Tile.Key + CurrentSeek.Value) != nullptr
				&& TileLayout.Find(Tile.Key + CurrentSeek.Value)->Type == ETileType::GenericTile
				&& Tile.Value.Type == ETileType::GenericTile
				&& counter < ExtraLinksAmount) {
				if (!AreTilesConnected(Tile.Key + CurrentSeek.Value, CurrentSeek.Key))
				{
					TileConnector(Tile.Key, Tile.Key + CurrentSeek.Value, CurrentSeek.Key);
					counter++;
				}
			}
		}
	}
}

bool ALevelGenerator::AreTilesConnected(FVector2D InTile, TEnumAsByte<ECardinalPoints> Direction)
{
	return TileLayout.Find(InTile)->Directions.FindRef(DirectionInverter(Direction));
}

TEnumAsByte<ECardinalPoints> ALevelGenerator::DirectionInverter(TEnumAsByte<ECardinalPoints> Direction)
{
	switch (Direction)
	{
	case North: return South;
	case South: return North; 
	case East: return West; 
	case West: return  East; 
	}
	return North;
}

void ALevelGenerator::SetOffTiles()
{
	if (!OffTiles.IsEmpty())
	{
		for (auto OffTile : OffTiles)
		{
			FTileInfo OffTileInfo;
			OffTileInfo.Type = ETileType::OffTile;
			TileLayout.Add(OffTile, OffTileInfo);
		}
	}
}

bool ALevelGenerator::IsThisCorridor(FVector2D InVec, TEnumAsByte<ECardinalPoints> Direction)
{
	if (TileLayout.Find(InVec)->Directions.FindRef(DirectionInverter(Direction))) return true;
	return false;
}
