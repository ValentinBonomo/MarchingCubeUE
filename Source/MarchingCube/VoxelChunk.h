// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "ProceduralMeshComponent.h"
#include "VoxelChunk.generated.h"

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class MARCHINGCUBE_API UVoxelChunk : public UActorComponent
{
    GENERATED_BODY()

public:
    UVoxelChunk();

    // ========================================================================
    // PROPRIETES PUBLIQUES
    // ========================================================================
    
    // Taille du chunk en voxels
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voxel|Generation")
    int32 ChunkSize = 32;

    // Taille d'un voxel en unités Unreal
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voxel|Generation")
    float VoxelSize = 100.0f;

    // Référence au ProceduralMeshComponent
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Voxel|Rendering")
    UProceduralMeshComponent* ProceduralMesh;

    // ========================================================================
    // FONCTIONS PUBLIQUES
    // ========================================================================
    
    // Générer le chunk
    UFUNCTION(BlueprintCallable, Category = "Voxel|Generation")
    void GenerateChunk();

    // Obtenir la densité d'un voxel (publique pour l'édition)
    UFUNCTION(BlueprintCallable, Category = "Voxel|Data")
    float GetDensity(int32 X, int32 Y, int32 Z) const;

    // Éditer le terrain à une position donnée
    UFUNCTION(BlueprintCallable, Category = "Voxel|Editing")
    void EditTerrain(FVector WorldPosition, float Radius, float Strength, bool bAddMaterial);
    
    // Définir manuellement une densité
    UFUNCTION(BlueprintCallable, Category = "Voxel|Data")
    void SetDensity(int32 X, int32 Y, int32 Z, float Density);

    // Obtenir les densités (pour partager entre chunks)
    UFUNCTION(BlueprintCallable, Category = "Voxel|Data")
    TArray<float>& GetDensities() { return VoxelDensities; }
    
    // Définir les densités depuis l'extérieur
    UFUNCTION(BlueprintCallable, Category = "Voxel|Data")
    void SetDensities(const TArray<float>& NewDensities) { VoxelDensities = NewDensities; }
    
    // Obtenir les coordonnées du chunk
    UPROPERTY(BlueprintReadWrite, Category = "Voxel|Data")
    FIntVector ChunkCoordinate;
    // Appliquer le Marching Cubes pour créer le mesh
    void ApplyMarchingCubes();
    
protected:
    virtual void BeginPlay() override;

private:
    // ========================================================================
    // DONNEES PRIVEES
    // ========================================================================
    
    // Stockage des densités des voxels
    TArray<float> VoxelDensities;

    // ========================================================================
    // FONCTIONS PRIVEES - GENERATION
    // ========================================================================
    
    // Générer les densités avec FastNoiseLite
    void GenerateDensities();
    
    
    // ========================================================================
    // FONCTIONS PRIVEES - MARCHING CUBES
    // ========================================================================
    
    // Calculer le vertex interpolé sur une arête
    FVector InterpolateVertex(float isoLevel, const FVector& p1, const FVector& p2, float valP1, float valP2) const;
    
    // ========================================================================
    // TABLES MARCHING CUBES (static)
    // ========================================================================
    
    // Table des arêtes (indique quelles arêtes sont coupées pour chaque configuration)
    static const int32 EdgeTable[256];
    
    // Table de triangulation (indique comment connecter les vertices)
    static const int32 TriTable[256][16];
    
    // Table des positions des coins du cube (pour les calculs)
    static const FVector CornerTable[8];
    
    // Table de connexion des arêtes (quel coin à quel coin)
    static const int32 EdgeConnection[12][2];

    
};