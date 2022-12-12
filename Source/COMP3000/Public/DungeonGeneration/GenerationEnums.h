// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GenerationEnums.generated.h"


UENUM(BlueprintType)
enum ETileType
{
	GenericTile,
	StartTile,
	EndTile,
	AdditionalTile,
	ReplacementTile,
	OffTile
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


class COMP3000_API GenerationEnums
{
public:
	GenerationEnums();
	~GenerationEnums();
};
