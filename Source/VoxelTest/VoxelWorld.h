#pragma once

#include "CoreMinimal.h"
#include "ChunkBase.h"
#include "Voxel.h"
#include "VoxelWorld.generated.h"

UCLASS()
class AVoxelWorld : public AActor
{
    GENERATED_BODY()

public:
    AVoxelWorld();

    FVoxel GetVoxel(const FVector& WorldCoord) const;
    FVoxel* GetWorldBlock(const FVector& WorldCoord) const;

    FVector ToBlockCoord(const FVector& WorldCoord) const;
    FVector ToChunkCoord(const FVector& WorldCoord) const;
    FVector ToChunkLocal(const FVector& WorldCoord) const;

    int GetChunkIndex(const FVector& ChunkCoord) const;

    UPROPERTY(EditInstanceOnly, Category = "World")
    TSubclassOf<AChunkBase> ChunkType;

    UPROPERTY(EditInstanceOnly, Category = "World")
    int DrawDistance = 5;

    UPROPERTY(EditInstanceOnly, Category = "World")
    TObjectPtr<UMaterialInterface> Material;

    UPROPERTY(EditInstanceOnly, Category = "World")
    int BlockSize = 100;

    UPROPERTY(EditInstanceOnly, Category = "World")
    int ChunkSize = 32;

    UPROPERTY(EditInstanceOnly, Category = "World")
    uint32 Seed = 0;

    UPROPERTY(EditInstanceOnly, Category = "World")
    int Octaves = 3;

    UPROPERTY(EditInstanceOnly, Category = "World")
    float Frequency = 0.03;

protected:
    virtual void BeginPlay() override;
    virtual void OnConstruction(const FTransform& Transform) override;
    virtual void Destroyed() override;

private:
    void GenerateWorld();
    void ClearExistingChunks();

    TArray<AChunkBase*> Chunks;
};
