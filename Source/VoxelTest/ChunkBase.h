#pragma once

#include "CoreMinimal.h"
#include "ChunkMeshData.h"
#include "Voxel.h"
#include "ChunkBase.generated.h"

class UProceduralMeshComponent;

UCLASS(Abstract, Transient)
class VOXELTEST_API AChunkBase : public AActor
{
    GENERATED_BODY()

public:
    AChunkBase();

    void BuildMesh();

    virtual FVoxel* GetChunkBlock(const FVector& Coord) PURE_VIRTUAL(AChunkBase::GetChunkBlock, return nullptr;);

protected:
    TObjectPtr<UProceduralMeshComponent> Mesh;
    FChunkMeshData MeshData;
    int VertexCount = 0;

    virtual void OnConstruction(const FTransform& Transform) override;
    virtual void BeginPlay() override;
    virtual void Setup() PURE_VIRTUAL(AChunkBase::Setup);
    virtual void GenerateHeightMap(const FVector& Position) PURE_VIRTUAL(AChunkBase::GenerateHeightMap);
    virtual void GenerateMesh() PURE_VIRTUAL(AChunkBase::GenerateMesh);

private:
    void ApplyMesh() const;
    void ClearMesh();
};
