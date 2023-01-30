// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ProceduralMeshComponent.h"
#include "VoxelMeshComponent.generated.h"

struct VOXELTEST_API FVoxel
{
	bool Cached = false;
	bool Filled = false;
};

/**
 * 
 */
UCLASS(hidecategories = (Object, LOD), meta = (BlueprintSpawnableComponent), ClassGroup = Rendering)
class VOXELTEST_API UVoxelMeshComponent : public UProceduralMeshComponent
{
	GENERATED_BODY()
	
public:
	UPROPERTY(EditAnywhere)
	uint32 SizeX = 32;
	UPROPERTY(EditAnywhere)
	uint32 SizeY = 32;
	UPROPERTY(EditAnywhere)
	uint32 SizeZ = 32;
	UPROPERTY(EditAnywhere)
	double NoiseScale = 0.1;
	UPROPERTY(EditAnywhere)
	double Scale = 100;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool DoCull = false;

	TArray<FVoxel> Voxels;

	UFUNCTION(BlueprintCallable)
	void RefreshMesh();

	FVoxel& GetVoxel(uint32 X, uint32 Y, uint32 Z);

	virtual void OnRegister() override;

	TOptional<FPlane> GetCameraPlane();

	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	float RefreshTimer = 0;
};
