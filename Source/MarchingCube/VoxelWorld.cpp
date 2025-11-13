// Fill out your copyright notice in the Description page of Project Settings.

#include "VoxelWorld.h"
#include "Kismet/GameplayStatics.h"
#include "VoxelSaveGame.h"

AVoxelWorld::AVoxelWorld()
{
    PrimaryActorTick.bCanEverTick = true;

    USceneComponent* RootComp = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));
    RootComponent = RootComp;

    // Créer l'indicateur de brush (sphère)
    BrushIndicator = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BrushIndicator"));
    BrushIndicator->SetupAttachment(RootComponent);
    
    static ConstructorHelpers::FObjectFinder<UStaticMesh> SphereMesh(TEXT("/Engine/BasicShapes/Sphere"));
    if (SphereMesh.Succeeded())
    {
        BrushIndicator->SetStaticMesh(SphereMesh.Object);
        BrushIndicator->SetCollisionEnabled(ECollisionEnabled::NoCollision);
        BrushIndicator->SetVisibility(false);
    }
}

void AVoxelWorld::BeginPlay()
{
    Super::BeginPlay();
    
    // Charger la sauvegarde si elle existe
    LoadWorld();
    
    // Générer les chunks initiaux
    UpdateChunksAroundPlayer();
}

void AVoxelWorld::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    APlayerController* PC = GetWorld()->GetFirstPlayerController();
    
    // Mettre à jour les chunks si le joueur se déplace
    if (PC && PC->GetPawn())
    {
        FVector PlayerLocation = PC->GetPawn()->GetActorLocation();
        FIntVector CurrentChunkCoord = WorldToChunkCoord(PlayerLocation);
        
        if (CurrentChunkCoord != LastPlayerChunkCoord)
        {
            LastPlayerChunkCoord = CurrentChunkCoord;
            UpdateChunksAroundPlayer();
        }
    }
    
    // Gestion du brush
    if (PC)
    {
        FHitResult Hit;
        bool bHit = PC->GetHitResultUnderCursor(ECC_Visibility, false, Hit);
        
        if (bHit)
        {
            BrushIndicator->SetVisibility(true);
            BrushIndicator->SetWorldLocation(Hit.Location);
            BrushIndicator->SetWorldScale3D(FVector(BrushRadius / 50.0f));
        }
        else
        {
            BrushIndicator->SetVisibility(false);
        }
        
        // Clic gauche = creuser
        if (PC->IsInputKeyDown(EKeys::LeftMouseButton) && bHit)
        {
            EditAtLocation(Hit.Location, false);
        }
        
        // Clic droit = ajouter
        if (PC->IsInputKeyDown(EKeys::RightMouseButton) && bHit)
        {
            EditAtLocation(Hit.Location, true);
        }
    }

    if (PC->IsInputKeyDown(EKeys::R))
    {
        ResetWorld();
    }
}

FIntVector AVoxelWorld::WorldToChunkCoord(FVector WorldPosition) const
{
    float ChunkWorldSize = ChunkSize * VoxelSize;
    return FIntVector(
        FMath::FloorToInt(WorldPosition.X / ChunkWorldSize),
        FMath::FloorToInt(WorldPosition.Y / ChunkWorldSize),
        0
    );
}

void AVoxelWorld::UpdateChunksAroundPlayer()
{
    APlayerController* PC = GetWorld()->GetFirstPlayerController();
    if (!PC || !PC->GetPawn()) return;
    
    FVector PlayerLocation = PC->GetPawn()->GetActorLocation();
    FIntVector PlayerChunkCoord = WorldToChunkCoord(PlayerLocation);
    
    // Générer les chunks autour du joueur
    for (int32 x = -RenderDistance; x <= RenderDistance; x++)
    {
        for (int32 y = -RenderDistance; y <= RenderDistance; y++)
        {
            FIntVector ChunkCoord(PlayerChunkCoord.X + x, PlayerChunkCoord.Y + y, 0);
            
            if (!ChunkMap.Contains(ChunkCoord))
            {
                GenerateChunkAtPosition(ChunkCoord);
            }
        }
    }
}

void AVoxelWorld::GenerateChunkAtPosition(FIntVector ChunkCoord)
{
    FString ChunkName = FString::Printf(TEXT("Chunk_%d_%d_%d"), ChunkCoord.X, ChunkCoord.Y, ChunkCoord.Z);
    UVoxelChunk* NewChunk = NewObject<UVoxelChunk>(this, *ChunkName);
    NewChunk->RegisterComponent();
    
    NewChunk->ChunkSize = ChunkSize;
    NewChunk->VoxelSize = VoxelSize;
    
    // Positionner le chunk
    float ChunkWorldSize = ChunkSize * VoxelSize;
    FVector ChunkWorldPosition(
        ChunkCoord.X * ChunkWorldSize,
        ChunkCoord.Y * ChunkWorldSize,
        0.0f
    );
    
    if (NewChunk->ProceduralMesh)
    {
        NewChunk->ProceduralMesh->SetWorldLocation(ChunkWorldPosition);
    }
    
    NewChunk->GenerateChunk();
    ChunkMap.Add(ChunkCoord, NewChunk);
}

UVoxelChunk* AVoxelWorld::GetChunkAtCoord(FIntVector ChunkCoord)
{
    if (ChunkMap.Contains(ChunkCoord))
    {
        return ChunkMap[ChunkCoord];
    }
    return nullptr;
}

void AVoxelWorld::EditAtLocation(FVector Location, bool bAddMaterial)
{
    // Éditer TOUS les chunks touchés par le brush
    FIntVector MinChunkCoord = WorldToChunkCoord(Location - FVector(BrushRadius));
    FIntVector MaxChunkCoord = WorldToChunkCoord(Location + FVector(BrushRadius));
    
    for (int32 x = MinChunkCoord.X; x <= MaxChunkCoord.X; x++)
    {
        for (int32 y = MinChunkCoord.Y; y <= MaxChunkCoord.Y; y++)
        {
            FIntVector ChunkCoord(x, y, 0);
            UVoxelChunk* Chunk = GetChunkAtCoord(ChunkCoord);
            
            if (Chunk)
            {
                Chunk->EditTerrain(Location, BrushRadius, BrushStrength, bAddMaterial);
                
                // Marquer ce chunk comme modifié
                ModifiedChunks.Add(ChunkCoord);
            }
        }
    }
}

void AVoxelWorld::SaveWorld()
{
    UVoxelSaveGame* SaveGameInstance = Cast<UVoxelSaveGame>(UGameplayStatics::CreateSaveGameObject(UVoxelSaveGame::StaticClass()));
    
    if (SaveGameInstance)
    {
        // Sauvegarder seulement les chunks modifiés
        for (const FIntVector& ChunkCoord : ModifiedChunks)
        {
            UVoxelChunk* Chunk = GetChunkAtCoord(ChunkCoord);
            if (Chunk)
            {
                FChunkSaveData ChunkData;
                ChunkData.ChunkCoordinate = ChunkCoord;
                ChunkData.VoxelDensities = Chunk->GetDensities();
                
                SaveGameInstance->SavedChunks.Add(ChunkData);
            }
        }
        
        // Sauvegarder dans un fichier
        if (UGameplayStatics::SaveGameToSlot(SaveGameInstance, SaveSlotName, 0))
        {
            UE_LOG(LogTemp, Warning, TEXT("World saved successfully! %d chunks saved."), SaveGameInstance->SavedChunks.Num());
        }
        else
        {
            UE_LOG(LogTemp, Error, TEXT("Failed to save world!"));
        }
    }
}

void AVoxelWorld::LoadWorld()
{
    if (UGameplayStatics::DoesSaveGameExist(SaveSlotName, 0))
    {
        UVoxelSaveGame* LoadedGame = Cast<UVoxelSaveGame>(UGameplayStatics::LoadGameFromSlot(SaveSlotName, 0));
        
        if (LoadedGame)
        {
            UE_LOG(LogTemp, Warning, TEXT("World loaded! %d chunks to restore."), LoadedGame->SavedChunks.Num());
            
            // Pour chaque chunk sauvegardé
            for (const FChunkSaveData& ChunkData : LoadedGame->SavedChunks)
            {
                // Générer le chunk s'il n'existe pas encore
                if (!ChunkMap.Contains(ChunkData.ChunkCoordinate))
                {
                    GenerateChunkAtPosition(ChunkData.ChunkCoordinate);
                }
                
                // Restaurer les densités
                UVoxelChunk* Chunk = GetChunkAtCoord(ChunkData.ChunkCoordinate);
                if (Chunk)
                {
                    Chunk->SetDensities(ChunkData.VoxelDensities);
                    Chunk->ApplyMarchingCubes();
                    
                    // Marquer comme modifié
                    ModifiedChunks.Add(ChunkData.ChunkCoordinate);
                }
            }
        }
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("No save file found."));
    }
}

void AVoxelWorld::ResetWorld()
{
    // Supprimer la sauvegarde
    if (UGameplayStatics::DeleteGameInSlot(SaveSlotName, 0))
    {
        UE_LOG(LogTemp, Warning, TEXT("Save file deleted!"));
    }
    
    // Vider la liste des chunks modifiés
    ModifiedChunks.Empty();
    
    // Détruire tous les chunks existants
    for (auto& Pair : ChunkMap)
    {
        if (Pair.Value)
        {
            Pair.Value->DestroyComponent();
        }
    }
    ChunkMap.Empty();
    
    // Régénérer le monde
    UpdateChunksAroundPlayer();
    
    UE_LOG(LogTemp, Warning, TEXT("World reset!"));
}

void AVoxelWorld::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    // Sauvegarder automatiquement à la fermeture
    SaveWorld();
    
    Super::EndPlay(EndPlayReason);
}
