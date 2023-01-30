// Fill out your copyright notice in the Description page of Project Settings.


#include "VoxelMeshComponent.h"

struct FQuad
{
    FQuad(const FVector& a, const FVector& b, const FVector& c, const FVector& d)
    :Corners{a,b,c,d}
    {}

    FVector Corners[4];
};

void UVoxelMeshComponent::OnRegister()
{
    PrimaryComponentTick.bCanEverTick = true;
    Super::OnRegister();
    Voxels.Empty();
    Voxels.SetNum(SizeX * SizeY * SizeZ);
    RefreshMesh();
}

void UVoxelMeshComponent::RefreshMesh()
{
    TOptional<FPlane> CameraPlane = GetCameraPlane();
    const auto f = [this, &CameraPlane](uint32 x, uint32 y, uint32 z) {
        const FVoxel& V = GetVoxel(x, y, z);
        if (DoCull && V.Filled && CameraPlane.IsSet())
        {
            FPlane Plane = CameraPlane.GetValue();
            FVector Pos = FVector{ static_cast<double>(x), static_cast<double>(y), static_cast<double>(z)} * Scale;
            return Plane.PlaneDot(Pos) < 0;
        }
        return V.Filled;
    };

    TArray<FVector> Vertices;
    TArray<FVector> Normals;
    TArray<int> Triangles;
    int index = 0;

    const FUintVector Dims{ SizeX, SizeY, SizeZ };
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
                    const bool PosFilled = (0 <= TestPos[d]) ? f(TestPos[0], TestPos[1], TestPos[2]) : false;
                    const bool TestFilled = (TestPos[d] < Dims[d] - 1) ? f(TestPos[0] + TestDir[0], TestPos[1] + TestDir[1], TestPos[2] + TestDir[2]) : false;
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
                        for (w = 1; i + w < Dims[u] && Mask[n+w] == CurMask; ++w)
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
                                if (Mask[n+k+h*Dims[u]] != CurMask)
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
                        FVector du{0,0,0};
                        du[u] = w;
                        FVector dv{0,0,0};
                        dv[v] = h;

                        {
                            Vertices.Add(TestPos * Scale);
                            Vertices.Add((TestPos + du) * Scale);
                            Vertices.Add((TestPos + du + dv) * Scale);
                            Vertices.Add((TestPos + dv) * Scale);

                            if (CurMask > 0)
                            {
                                Triangles.Add(index + 3);
                                Triangles.Add(index + 1);
                                Triangles.Add(index + 0);

                                Triangles.Add(index + 3);
                                Triangles.Add(index + 2);
                                Triangles.Add(index + 1);

                                Normals.Add(TestDir);
                                Normals.Add(TestDir);
                                Normals.Add(TestDir);
                                Normals.Add(TestDir);
                            }
                            else
                            {
                                Triangles.Add(index + 0);
                                Triangles.Add(index + 1);
                                Triangles.Add(index + 3);

                                Triangles.Add(index + 1);
                                Triangles.Add(index + 2);
                                Triangles.Add(index + 3);

                                Normals.Add(-TestDir);
                                Normals.Add(-TestDir);
                                Normals.Add(-TestDir);
                                Normals.Add(-TestDir);
                            }

                            index += 4;
                        }

                        // clear mask
                        for (uint32 l = 0; l < h; ++l)
                        {
                            for (uint32 k = 0; k < w; ++k)
                            {
                                Mask[n+k+l*Dims[u]] = 0;
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

    ClearMeshSection(0);
    CreateMeshSection(0, Vertices, Triangles, Normals, {}, {}, {}, false);
}

FVoxel& UVoxelMeshComponent::GetVoxel(uint32 X, uint32 Y, uint32 Z)
{
    auto& V = Voxels[X + (Y * SizeX) + (Z * SizeX * SizeY)];
    if (!V.Cached)
    {
        V.Filled = FMath::PerlinNoise3D(FVector{ static_cast<double>(X), static_cast<double>(Y), static_cast<double>(Z) } * NoiseScale) < 0;
        V.Cached = true;
    }
    return V;
}

TOptional<FPlane> UVoxelMeshComponent::GetCameraPlane()
{
    if (UWorld* World = GetWorld())
    {
        if (auto* PlayerController = World->GetFirstPlayerController())
        {
            APlayerCameraManager* PlayerCamera = PlayerController->PlayerCameraManager;
            AActor* CameraFocus = PlayerCamera->GetViewTarget();
            FVector CameraLocation = PlayerCamera->GetCameraLocation();
            FVector PlaneLocation = CameraFocus->GetActorLocation();
            FVector PlaneNormal = (CameraLocation - PlaneLocation).GetSafeNormal(0.01);
            return FPlane(PlaneLocation, PlaneNormal);
        }
    }
    return FPlane();
}

void UVoxelMeshComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    RefreshTimer -= DeltaTime;
    if (RefreshTimer <= 0)
    {
        RefreshMesh();
        RefreshTimer = 0.03;
    }
}
