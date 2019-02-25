// Copyright 2016-2018 Chris Conway (Koderz). All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"

#include "RuntimeDestructibleActor.generated.h"

class URuntimeMesh;
class URuntimeMeshComponent;

class URMC_DestructibleMesh;
class URMC_DestructibleComponent;

struct FSoftSkinVertex;
struct FSkeletalMaterial;

#if WITH_APEX

// Forward declares
namespace nvidia
{
	namespace apex
	{
		class ExplicitHierarchicalMesh;	
		class RenderMeshAssetAuthoring;
		struct Vertex;
		struct VertexColor;
		struct VertexUV;
	};
	
};
namespace physx
{
	class PxVec3;
};
#endif // WITH_APEX

USTRUCT(BlueprintType)
struct FFractureChunks
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Runtime Destructible Actor Chunks", DisplayName = "Fracture Chunks")
	TArray <URuntimeMeshComponent*> Chunks;

	int32 GetChunksCount() const
	{
		return Chunks.Num();
	}
	//Constructor
	FFractureChunks()
	{
	}
};

UCLASS()
class RUNTIMEMESHCOMPONENT_API ARuntimeDestructibleActor : public AActor
{
	GENERATED_UCLASS_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Runtime Destructible Actor", DisplayName ="Scene")
	USceneComponent* SceneComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Runtime Destructible Actor", DisplayName = "Source Runtime Mesh")
	URuntimeMeshComponent* SourceRuntimeMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Runtime Destructible Actor", DisplayName = "Fracture Chunks")
	FFractureChunks FractureChunks;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Runtime Destructible Actor", DisplayName = "RMC Destructible Mesh")
	URMC_DestructibleComponent* RMC_DestructibleMesh;
	

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

#if	WITH_EDITOR	&& WITH_APEX
	/** 
	Copy chunk from a destructible mesh to a runtime mesh 
	Source static mesh's ChunkIndex = 0
	**/
	UFUNCTION(BlueprintCallable)
		bool CopyDestructibleMeshChunkToRuntimeMesh(int32 ChunkIndex = 0, bool bOverrideMaterials = true, bool bCopyCollision = false, bool bCreateAdjBuffer = false);
	
	/** 
	Copy all chunks from a destructible mesh component to a runtime mesh components
	RuntimeMeshComponent - top chunk in DestructibleMeshComponent hierarchy
	Chunks - other chunks
	**/
	UFUNCTION(BlueprintCallable)
		static bool CopyDestructibleMeshToRuntimeMeshComponent(URMC_DestructibleComponent* DestructibleMeshComponent,
			URuntimeMeshComponent* RuntimeMeshComponent, TArray <URuntimeMeshComponent*>& Chunks, bool bOverrideMaterials = true, bool bCopyCollision = false, bool bCreateAdjBuffer = false);
	
private:
	static void BuildAdjacencyBuffer(const TArray<FSoftSkinVertex>& RM_SoftVertexBuffer, const uint32 NumUV, const TArray<int32>& RM_IndexBuffer, TArray<uint32>& OutRM_AdjacencyIndexBuffer);

	static bool GetChunkCollision(nvidia::apex::ExplicitHierarchicalMesh& DM_HierarchicalMesh, TArray<TArray<FVector>>& DM_ConvexHullsToVertices, int32 ChunkIdx);
	/** Cet Chunks Collision**/
	static bool GetChunksCollision(nvidia::apex::ExplicitHierarchicalMesh& DM_HierarchicalMesh, TArray<TArray<TArray<FVector>>>& DM_ChunksToConvexHullsToVertices);

	static void GetChunkBuffers(TArray<TArray<nvidia::apex::Vertex>>& DM_SectionsToVertexBuffers, TArray<TArray<int32>>& DM_SectionsToIndexBuffers,
		TArray<int32>& DM_SectionIndices, nvidia::apex::ExplicitHierarchicalMesh& DM_HierarchicalMesh, nvidia::apex::RenderMeshAssetAuthoring* RenderMeshAssetAuthor, int32 ChunkIdx);

	static void GetChunkRenderData(TArray<TArray<nvidia::apex::Vertex>>& DM_SectionsToVertexBuffers, TArray<TArray<int32>>& DM_SectionsToIndexBuffers,
		TArray<int32>& DM_SectionIndices, nvidia::apex::ExplicitHierarchicalMesh& DM_HierarchicalMesh, int32 ChunkIdx);
	/**
	Create Vertex and Index Buffers for sections of DestructibleMesh parts(chunks)
	**/
	static void GetChunksRenderData(TArray<TArray<TArray<nvidia::apex::Vertex>>>& DM_PartsToSectionsToVertexBuffers, TArray<TArray<TArray<int32>>>& DM_PartsToSectionsToIndexBuffers,
		TArray<TArray<int32>>&  DM_PartsToSectionIndices, nvidia::apex::ExplicitHierarchicalMesh& DM_HierarchicalMesh);

	static inline FVector PxVec3_To_FVector(const physx::PxVec3& InVertex);

	static inline FColor VertexColor_To_FColor(const nvidia::apex::VertexColor& InColor);

	static inline FVector2D VertexUV_To_FVector2D(const nvidia::apex::VertexUV& InVertexUV);

	static inline FSoftSkinVertex NvVertex_To_FSoftSkinVertex(const nvidia::apex::Vertex& InVertex);

	static void CreateRuntimeMeshCollision(URuntimeMeshComponent* RuntimeMeshComponent, const TArray<TArray<FVector>>& DM_ConvexHullsToVertices);

	/**Creating RuntimeMesh Sections, tesselation triangles (if need) and setting materials **/
	static void CreateRuntimeMeshRenderData(URuntimeMeshComponent* RuntimeMeshComponent, const TArray<TArray<nvidia::apex::Vertex>>& DM_PartSectionsToVertexBuffers, const TArray<TArray<int32>>& DM_PartSectionsToIndexBuffers,
		const TArray<int32>& DM_PartSectionIndices, const TArray<UMaterialInterface*>& DM_OverrideMaterials, const TArray<FSkeletalMaterial>& DM_Materials, const uint32 NumUV, bool bOverrideMaterials, bool bCreateAdjBuffer);

		
#endif  //WITH_EDITOR && WITH_APEX
};
