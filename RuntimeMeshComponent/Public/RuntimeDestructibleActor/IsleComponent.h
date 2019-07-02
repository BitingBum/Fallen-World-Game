// Copyright 2016-2018 Chris Conway (Koderz). All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "RuntimeMeshComponent.h"
#include "IsleComponent.generated.h"

USTRUCT(BlueprintType)
struct FIndices
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<int32> Indices;

	//Constructor
	FIndices()
	{
	}
};

/**
 * 
 */
UCLASS(HideCategories = (Object, LOD), Meta = (BlueprintSpawnableComponent))
class RUNTIMEMESHCOMPONENT_API UIsleComponent : public URuntimeMeshComponent
{
	GENERATED_UCLASS_BODY()
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Isle Parameters", DisplayName = "Isle Index")
	int32 IsleIndex;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Isle Parameters", DisplayName = "Parent Chunk Index")
	int32 ParentIndex;

	/**
	Number of attached chunks
	**/
	/*UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Isle Parameters", DisplayName = "Chunks Count")
	int32 ChunksCount;*/

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Isle Parameters", DisplayName = "Children Chunks Indices")
	TSet<int32> ChildrenChunks;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Isle Parameters", DisplayName = "Children Chunks To Sections")
	TMap<int32, FIndices> ChildrenToSections;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Isle Parameters", DisplayName = "Children Chunks To Collision Sections")
	TMap<int32, FIndices> ChildrenToCollisionSections;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Isle Parameters", DisplayName = "Sections To Children Chunks")
	TMap<int32, int32> SectionsToChildren;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Isle Parameters", DisplayName = "Collision Sections To Children Chunks")
	TMap<int32, int32> CollisionSectionsToChildren;

	UPROPERTY(BlueprintReadOnly, Category = "Owning Actor", DisplayName = "Owning Actor")
	TWeakObjectPtr<AActor> OwningActor;

	//Constructor
	UIsleComponent();
	
};
