
#include "VoxelWorld.h"
#include "Kismet/GameplayStatics.h"

AVoxelWorld::AVoxelWorld()
{
    PrimaryActorTick.bCanEverTick = false;
    Seed = FMath::Rand();
    auto SceneComponent = CreateDefaultSubobject<USceneComponent>("Root");
    SetRootComponent(SceneComponent);
}

FVoxel AVoxelWorld::GetVoxel(const FVector& WorldCoord) const
{
    const double NoiseScale = 0.03;

    const float PosX = static_cast<float>(WorldCoord.X);
    const float PosY = static_cast<float>(WorldCoord.Y);
    const float PosZ = static_cast<float>(Seed);

    float Height = 0;
    float Mul = 1.0f;
    for (int o = 0; o < Octaves; ++o)
    {
        Height += Mul * FMath::PerlinNoise3D(FVector{ PosX * Frequency / Mul, PosY * Frequency / Mul, PosZ });
        Mul /= 2;
    }
    Height = (Height + 1.0f) / 2.0f;
    Height *= ChunkSize;

    FVoxel V;
    V.Solid = static_cast<float>(WorldCoord.Z) <= Height;
    return V;
}

FVoxel* AVoxelWorld::GetWorldBlock(const FVector& WorldCoord) const
{
    const FVector ChunkCoord = ToChunkCoord(WorldCoord);
    if (ChunkCoord.X < 0 || ChunkCoord.X >= DrawDistance * 2
        || ChunkCoord.Y < 0 || ChunkCoord.Y >= DrawDistance * 2
        || ChunkCoord.Z < 0 || ChunkCoord.Z >= 1)
    {
        return nullptr;
    }
    const int ChunkIndex = GetChunkIndex(ChunkCoord);
    const FVector ChunkLocal = ToChunkLocal(WorldCoord);
    return Chunks[GetChunkIndex(ChunkCoord)]->GetChunkBlock(ChunkLocal);
}

void AVoxelWorld::OnConstruction(const FTransform& Transform)
{
    Super::OnConstruction(Transform);
    GenerateWorld();
}

void AVoxelWorld::Destroyed()
{
    Super::Destroyed();
    ClearExistingChunks();
}

void AVoxelWorld::BeginPlay()
{
    Super::BeginPlay();
}

void AVoxelWorld::GenerateWorld()
{
    ClearExistingChunks();

    const int FullWidth = DrawDistance * 2;
    Chunks.SetNumZeroed(FullWidth * FullWidth);
    for (int x = 0; x < FullWidth; ++x)
    {
        for (int y = 0; y < FullWidth; ++y)
        {
            const auto Offset = FVector{ static_cast<float>(-DrawDistance), static_cast<float>(-DrawDistance), 0 };
            const auto ChunkScale = FVector::OneVector * static_cast<float>(ChunkSize);
            const auto BlockScale = FVector::OneVector * static_cast<float>(BlockSize);
            const auto WorldLocation = (Offset + FVector{ static_cast<float>(x), static_cast<float>(y), 0 }) * ChunkScale * BlockScale;
            const auto Transform = FTransform(FRotator::ZeroRotator, WorldLocation, FVector::OneVector);
            const auto Chunk = GetWorld()->SpawnActorDeferred<AChunkBase>(ChunkType, Transform, this);
            UGameplayStatics::FinishSpawningActor(Chunk, Transform);
            if (Chunk)
            {
                Chunk->AttachToActor(this, FAttachmentTransformRules::KeepWorldTransform);
            }
            Chunks[GetChunkIndex(FVector{ static_cast<float>(x),static_cast<float>(y),0.0f })] = Chunk;
        }
    }

    for (auto* Chunk : Chunks)
    {
        if (Chunk)
        {
            Chunk->BuildMesh();
        }
    }
}

void AVoxelWorld::ClearExistingChunks()
{
    for (auto* Chunk : Chunks)
    {
        if (Chunk)
        {
            Chunk->Destroy();
        }
    }
    Chunks.Empty();
}

FVector AVoxelWorld::ToBlockCoord(const FVector& WorldCoord) const
{
    return FVector{
        FMath::Floor(WorldCoord.X / BlockSize),
        FMath::Floor(WorldCoord.Y / BlockSize),
        FMath::Floor(WorldCoord.Z / BlockSize) }
      + FVector{ static_cast<float>(DrawDistance * ChunkSize), static_cast<float>(DrawDistance * ChunkSize), 0 };
}

FVector AVoxelWorld::ToChunkCoord(const FVector& WorldCoord) const
{
    return FVector{
        FMath::Floor(WorldCoord.X / (ChunkSize * BlockSize)),
        FMath::Floor(WorldCoord.Y / (ChunkSize * BlockSize)),
        FMath::Floor(WorldCoord.Z / (ChunkSize * BlockSize)) }
      + FVector{static_cast<float>(DrawDistance), static_cast<float>(DrawDistance), 0};
}

FVector AVoxelWorld::ToChunkLocal(const FVector& WorldCoord) const
{
    const FVector Chunk = ToChunkCoord(WorldCoord);
    const FVector Blocks = ToBlockCoord(WorldCoord);
    const FVector Result = Blocks - Chunk * ChunkSize;
    return Result;
    //const FVector Temp = FVector{ static_cast<float>(DrawDistance) * ChunkSize, static_cast<float>(DrawDistance) * ChunkSize, 0.0f};
    //return FVector{
    //    FMath::Floor(FMath::Fmod(Temp.X, ChunkSize)),
    //    FMath::Floor(FMath::Fmod(Temp.Y, ChunkSize)),
    //    FMath::Floor(FMath::Fmod(Temp.Z, ChunkSize)) };
}

int AVoxelWorld::GetChunkIndex(const FVector& ChunkCoord) const
{
    const float FullWidth = DrawDistance * 2.0f;
    return ChunkCoord.Z * FullWidth * FullWidth + ChunkCoord.Y * FullWidth + ChunkCoord.X;
}
