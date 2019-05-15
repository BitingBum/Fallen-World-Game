// Copyright 2016-2018 Chris Conway (Koderz). All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "RuntimeMeshComponent.h"
#include "IsleComponent.generated.h"

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
	TSet<int32> ChildrenIndices;

	UPROPERTY(BlueprintReadOnly, Category = "Owning Actor", DisplayName = "Owning Actor")
	TWeakObjectPtr<AActor> OwningActor;

	//Constructor
	UIsleComponent();
	
};
