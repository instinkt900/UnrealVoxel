#pragma once

#include "CoreMinimal.h"
#include "ChunkBase.h"
#include "Voxel.h"
#include "ChunkNormal.generated.h"

UCLASS()
class AChunkNormal : public AChunkBase
{
    GENERATED_BODY()

protected:
    virtual FVoxel* GetChunkBlock(const FVector& Coord) override;

    virtual void Setup() override;
    virtual void GenerateHeightMap(const FVector& Position) override;
    virtual void GenerateMesh() override;

private:
    int Size = 0;
    TArray<FVoxel> Blocks;

    int GetBlockIndex(int X, int Y, int Z) const;
};
