
#include "ChunkNormal.h"
#include "VoxelWorld.h"

FVoxel* AChunkNormal::GetChunkBlock(const FVector& Coord)
{
    if (Coord.X < 0 || Coord.X >= Size
        || Coord.Y < 0 || Coord.Y >= Size
        || Coord.Z < 0 || Coord.Z >= Size)
    {
        return nullptr;
    }
    return &Blocks[GetBlockIndex(Coord.X, Coord.Y, Coord.Z)];
}

void AChunkNormal::Setup()
{
    AVoxelWorld* World = CastChecked<AVoxelWorld>(GetOwner());
    Size = World->ChunkSize;
    Blocks.SetNum(Size * Size * Size);
}

void AChunkNormal::GenerateHeightMap(const FVector& Position)
{
    AVoxelWorld* World = CastChecked<AVoxelWorld>(GetOwner());

    for (int x = 0; x < Size; ++x)
    {
        for (int y = 0; y < Size; ++y)
        {
            for (int z = 0; z < Size; ++z)
            {
                Blocks[GetBlockIndex(x, y, z)] = World->GetVoxel(FVector{x + Position.X, y + Position.Y, z + Position.Z});
            }
        }
    }
}

void AChunkNormal::GenerateMesh()
{
    AVoxelWorld* World = CastChecked<AVoxelWorld>(GetOwner());

    const auto f = [this, World](float x, float y, float z) {
        if (x < 0 || y < 0 || z < 0 || x >= Size || y >= Size || z >= Size)
        {
            const FVector Position = GetActorLocation();
            const FVector SamplePosition = Position + FVector{x,y,z} * World->BlockSize;
            const FVoxel* V = World->GetWorldBlock(SamplePosition);
            return V ? V->Solid : false;
        }
        return Blocks[GetBlockIndex(x, y, z)].Solid;
    };

    int index = 0;
    const double Scale = 100.0;

    const uint32 iSize = static_cast<uint32>(Size);
    const FUintVector Dims{ iSize, iSize, iSize };
    for (uint32 d = 0; d < 3; ++d)
    {
        const uint32 u = (d + 1) % 3;
        const uint32 v = (d + 2) % 3;
        FVector TestPos{ 0, 0, 0 };
        FVector TestDir{ 0, 0, 0 };
        TArray<int32> Mask;

        Mask.SetNum(Dims[u] * Dims[v]);
        TestDir[d] = 1;

        for (TestPos[d] = -1; TestPos[d] < Dims[d]; )
        {
            uint32 n = 0;
            for (TestPos[v] = 0; TestPos[v] < Dims[v]; ++TestPos[v])
            {
                for (TestPos[u] = 0; TestPos[u] < Dims[u]; ++TestPos[u])
                {
                    const bool PosFilled = f(TestPos[0], TestPos[1], TestPos[2]);
                    const bool TestFilled = f(TestPos[0] + TestDir[0], TestPos[1] + TestDir[1], TestPos[2] + TestDir[2]);
                    Mask[n++] = (PosFilled && !TestFilled) ? 1 : (!PosFilled && TestFilled) ? -1 : 0;
                }
            }

            ++TestPos[d];
            n = 0;

            uint32 j;
            for (j = 0; j < Dims[v]; ++j)
            {
                uint32 i;
                for (i = 0; i < Dims[u]; )
                {
                    if (Mask[n])
                    {
                        const int32 CurMask = Mask[n];

                        // calc width
                        uint32 w;
                        for (w = 1; i + w < Dims[u] && Mask[n + w] == CurMask; ++w)
                        {
                            //
                        }

                        // calc height
                        uint32 h;
                        bool done = false;
                        for (h = 1; j + h < Dims[v]; ++h)
                        {
                            for (uint32 k = 0; k < w; ++k)
                            {
                                if (Mask[n + k + h * Dims[u]] != CurMask)
                                {
                                    done = true;
                                    break;
                                }
                            }
                            if (done)
                            {
                                break;
                            }
                        }

                        TestPos[u] = i;
                        TestPos[v] = j;
                        FVector du{ 0,0,0 };
                        du[u] = w;
                        FVector dv{ 0,0,0 };
                        dv[v] = h;

                        {
                            MeshData.Vertices.Add(TestPos * Scale);
                            MeshData.Vertices.Add((TestPos + du) * Scale);
                            MeshData.Vertices.Add((TestPos + du + dv) * Scale);
                            MeshData.Vertices.Add((TestPos + dv) * Scale);

                            if (CurMask > 0)
                            {
                                MeshData.Triangles.Add(index + 3);
                                MeshData.Triangles.Add(index + 1);
                                MeshData.Triangles.Add(index + 0);

                                MeshData.Triangles.Add(index + 3);
                                MeshData.Triangles.Add(index + 2);
                                MeshData.Triangles.Add(index + 1);

                                MeshData.Normals.Add(TestDir);
                                MeshData.Normals.Add(TestDir);
                                MeshData.Normals.Add(TestDir);
                                MeshData.Normals.Add(TestDir);
                            }
                            else
                            {
                                MeshData.Triangles.Add(index + 0);
                                MeshData.Triangles.Add(index + 1);
                                MeshData.Triangles.Add(index + 3);

                                MeshData.Triangles.Add(index + 1);
                                MeshData.Triangles.Add(index + 2);
                                MeshData.Triangles.Add(index + 3);

                                MeshData.Normals.Add(-TestDir);
                                MeshData.Normals.Add(-TestDir);
                                MeshData.Normals.Add(-TestDir);
                                MeshData.Normals.Add(-TestDir);
                            }

                            index += 4;
                        }

                        // clear mask
                        for (uint32 l = 0; l < h; ++l)
                        {
                            for (uint32 k = 0; k < w; ++k)
                            {
                                Mask[n + k + l * Dims[u]] = 0;
                            }
                        }

                        i += w;
                        n += w;
                    }
                    else
                    {
                        ++i;
                        ++n;
                    }
                }
            }
        }
    }
}

int AChunkNormal::GetBlockIndex(int X, int Y, int Z) const
{
    return Z * Size * Size + Y * Size + X;
}
