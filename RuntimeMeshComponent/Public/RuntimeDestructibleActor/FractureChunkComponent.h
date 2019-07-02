// Copyright 2016-2018 Chris Conway (Koderz). All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "RuntimeMeshComponent.h"
#include "FractureChunkComponent.generated.h"

/**
 * 
 */
UCLASS(HideCategories = (Object, LOD), Meta = (BlueprintSpawnableComponent))
class RUNTIMEMESHCOMPONENT_API UFractureChunkComponent : public URuntimeMeshComponent
{
	GENERATED_UCLASS_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fracture Chunk Parameters", DisplayName = "Chunk Index")
	int32 ChunkIndex;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fracture Chunk Parameters", DisplayName = "Parent Chunk Index")
	int32 ParentIndex;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fracture Chunk Parameters", DisplayName = "Parent Isle Index")
	int32 IsleIndex;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fracture Chunk Parameters", DisplayName = "Depth Level")
	int32 DepthLevel;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fracture Chunk Parameters", DisplayName = "Damage Taken")
	int32 DamageTaken;

	/**
	Wether chunk is replaced by smaller children chunks
	**/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fracture Chunk Parameters", DisplayName = "Is Fractured")
	bool IsFractured;

	/**
	Wether chunk is separated from overlapping chunks
	**/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fracture Chunk Parameters", DisplayName = "Is Fractured")
	bool IsSeparated;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fracture Chunk Parameters", DisplayName = "Children Chunks Indices")
	TSet<int32> ChildrenChunks;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fracture Chunk Parameters", DisplayName = "Children Isles Indices")
	TSet<int32> ChildrenIsles;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fracture Chunk Parameters", DisplayName = "Overlap Chunks Indices")
	TSet<int32> OverlapIndices;	

	UPROPERTY(BlueprintReadOnly, Category = "Owning Actor", DisplayName = "Owning Actor")
	TWeakObjectPtr<AActor> OwningActor;

	UFractureChunkComponent();

	

	//	/* Serializes this component */
	//virtual void Serialize(FArchive& Ar) override;
};
