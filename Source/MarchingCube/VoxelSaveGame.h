// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "VoxelSaveGame.generated.h"

// Structure pour sauvegarder un chunk
USTRUCT(BlueprintType)
struct FChunkSaveData
{
	GENERATED_BODY()

	UPROPERTY()
	FIntVector ChunkCoordinate;

	UPROPERTY()
	TArray<float> VoxelDensities;
};

UCLASS()
class MARCHINGCUBE_API UVoxelSaveGame : public USaveGame
{
	GENERATED_BODY()
    
public:
	UPROPERTY()
	TArray<FChunkSaveData> SavedChunks;
};