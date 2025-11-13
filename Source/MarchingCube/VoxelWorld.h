// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "VoxelChunk.h"
#include "VoxelWorld.generated.h"

UCLASS()
class MARCHINGCUBE_API AVoxelWorld : public AActor
{
    GENERATED_BODY()
    
public:    
    AVoxelWorld();

protected:
    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

public:    
    // Taille du chunk en voxels
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voxel|Generation")
    int32 ChunkSize = 32;

    // Taille d'un voxel en unités UE
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voxel|Generation")
    float VoxelSize = 100.0f;

    // Distance de rendu des chunks
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voxel|Generation")
    int32 RenderDistance = 3;

    // Rayon du brush d'édition
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voxel|Editing")
    float BrushRadius = 200.0f;
    
    // Force de l'édition
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voxel|Editing")
    float BrushStrength = 100.0f;
    
    // Fonction pour éditer au clic
    UFUNCTION(BlueprintCallable, Category = "Voxel|Editing")
    void EditAtLocation(FVector Location, bool bAddMaterial);

    // Sauvegarder le monde
    UFUNCTION(BlueprintCallable, Category = "Voxel|Save")
    void SaveWorld();
    
    // Charger le monde
    UFUNCTION(BlueprintCallable, Category = "Voxel|Save")
    void LoadWorld();
    
    // Réinitialiser le monde (supprimer la sauvegarde)
    UFUNCTION(BlueprintCallable, Category = "Voxel|Save")
    void ResetWorld();


private:
    // Map des chunks
    UPROPERTY()
    TMap<FIntVector, UVoxelChunk*> ChunkMap;

    // Sphère de visualisation du brush
    UPROPERTY()
    UStaticMeshComponent* BrushIndicator;

    // Position chunk du joueur
    FIntVector LastPlayerChunkCoord;

    // Fonctions helper
    void UpdateChunksAroundPlayer();
    void GenerateChunkAtPosition(FIntVector ChunkCoord);
    UVoxelChunk* GetChunkAtCoord(FIntVector ChunkCoord);
    FIntVector WorldToChunkCoord(FVector WorldPosition) const;

    // Nom du slot de sauvegarde
    FString SaveSlotName = TEXT("VoxelWorldSave");
    
    // Set des chunks modifiés (pour savoir quoi sauvegarder)
    UPROPERTY()
    TSet<FIntVector> ModifiedChunks;
    
public:
    virtual void Tick(float DeltaTime) override;
};