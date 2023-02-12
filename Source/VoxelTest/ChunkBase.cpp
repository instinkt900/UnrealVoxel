
#include "ChunkBase.h"
#include "ProceduralMeshComponent.h"
#include "VoxelWorld.h"

AChunkBase::AChunkBase()
{
    PrimaryActorTick.bCanEverTick = false;

    Mesh = CreateDefaultSubobject<UProceduralMeshComponent>("Mesh");
    Mesh->SetCastShadow(false);
    SetRootComponent(Mesh);
}

void AChunkBase::BuildMesh()
{
    GenerateMesh();
    ApplyMesh();
}

void AChunkBase::OnConstruction(const FTransform& Transform)
{
    Super::OnConstruction(Transform);

    Setup();
    GenerateHeightMap(GetActorLocation() / 100);
}

void AChunkBase::BeginPlay()
{
    Super::BeginPlay();

    //Setup();
    //GenerateHeightMap(GetActorLocation() / 100);
    //GenerateMesh();
    //ApplyMesh();
}

void AChunkBase::ApplyMesh() const
{
    AVoxelWorld* World = CastChecked<AVoxelWorld>(GetOwner());
    Mesh->SetMaterial(0, World->Material);
    Mesh->CreateMeshSection(0, MeshData.Vertices, MeshData.Triangles, MeshData.Normals, MeshData.UV0, MeshData.Colors, TArray<FProcMeshTangent>(), false);
}

void AChunkBase::ClearMesh()
{
    VertexCount = 0;
    MeshData.Clear();
}
