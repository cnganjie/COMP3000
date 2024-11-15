// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GenerationEnums.generated.h"

UENUM(BlueprintType)
enum EConnectionType
{
	Corner,
	Threeway,
	Fourway,
	Line,
	Single,
	NONE
};

UENUM(BlueprintType)
enum ETileType
{
	GenericTile,
	StartTile,
	EndTile,
	AdditionalTile,
	SpecialTile,
	OffTile,
	KeyTile
};

UENUM(BlueprintType)
enum ECardinalPoints
{
	North,
	East,
	South,
	West,
	Random
};

UENUM(BlueprintType)
enum ETileDirection
{
	N,
	E,
	S,
	W,
	NS,
	EW,
	NE,
	NW,
	SE,
	SW,
	NSE,
	NSW,
	SEW,
	NEW,
	NSEW
};

UENUM(BlueprintType)
enum class EGeneratorMode : uint8
{
	FullyRandom UMETA(DisplayName = "Fully Random"),
	Hybrid UMETA(DisplayName = "Hybrid"),
	Fixed UMETA(DisplayName = "Fixed"),
};

USTRUCT(BlueprintType)
struct FTileInfo
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Struct")
	TEnumAsByte<ETileType> Type;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Struct")
	TMap<TEnumAsByte<ECardinalPoints>, bool> Directions;

	FTileInfo()
	{
		Type = ETileType::GenericTile;
		Directions.Add(ECardinalPoints::North, false);
		Directions.Add(ECardinalPoints::East, false);
		Directions.Add(ECardinalPoints::South, false);
		Directions.Add(ECardinalPoints::West, false);
	}
};

USTRUCT(BlueprintType)
struct FDungeonPathNode {
	GENERATED_BODY()

	FVector2D Location;
	float FScore;
	float GScore;
	TSharedPtr<FDungeonPathNode> Parent;

	// Default constructor
	FDungeonPathNode()
		: Location(FVector2D::ZeroVector), FScore(0.f), GScore(0.f), Parent(nullptr) {}

	FDungeonPathNode(FVector2D InLocation, float InFScore, float InGScore, TSharedPtr<FDungeonPathNode> InParent)
		: Location(InLocation), FScore(InFScore), GScore(InGScore), Parent(InParent) {}
};




class COMP3000_API GenerationEnums
{
public:
	GenerationEnums();
	~GenerationEnums();
};
