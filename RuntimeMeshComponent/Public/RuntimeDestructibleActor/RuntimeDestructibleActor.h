// Copyright 2016-2018 Chris Conway (Koderz). All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"

//#include "PhysXPublic.h"//nvidia::apex::Vertex and other types

//#include "Rendering/SkeletalMeshLODModel.h"//FSoftSkinVertex

//#include "DestructibleActorJoint.h"

#include "RuntimeDestructibleActor.generated.h"

class URuntimeMesh;
class URuntimeMeshComponent;


class URMC_DestructibleMesh;
class URMC_DestructibleComponent;

class UFractureChunkComponent; 
class AFractureChunkActor;
class UIsleComponent;
class AIsleActor;

class UPhysicsConstraintComponent;

struct FSoftSkinVertex;
struct FSkeletalMaterial;

struct FRuntimeMeshTangent;
struct FRuntimeMeshCollisionConvexMesh;

//struct FBodyInstance::FInitBodySpawnParams;


#if WITH_APEX

// Forward declares
namespace nvidia
{
	namespace apex
	{
		class ExplicitHierarchicalMesh;	
		class RenderMeshAssetAuthoring;

		class DestructibleActorJoint;		

		struct Vertex;
		struct VertexColor;
		struct VertexUV;
		struct IntPair;
	};
	
};
namespace physx
{
	class PxVec3;
	class PxRigidActor;
	class PxAggregate;
	class PxRigidBody;
	class PxRigidDynamic;
	class PxGeometry;
	class PxShape;
	class PxMaterial;
	class PxTransform;
	class PxFixedJoint;
	struct PxContactPair;
	struct PxFilterData;
};
#endif // WITH_APEX

USTRUCT(BlueprintType)
/**
Fake URuntimeMeshComponent with 4 triangles and collision convex meshes of overlapped chunks
For overlapped chunks physics simulation
**/
struct FIsle
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, DisplayName = "Isle Component")
	URuntimeMeshComponent* IsleComponent;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, DisplayName = "Parent Chunk Index")
	int32 ParentIndex;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite,  DisplayName = "Children Chunks Indices")
	TSet<int32> ChildrenIndices;
	
	///**
	//Number of attached chunks
	//**/
	//UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, DisplayName = "Chunks Count")
	//int32 ChunksCount;

	//Constructor
	FIsle()
	{
		ParentIndex = -1;
		//ChunksCount = 0;		
	}	
};

USTRUCT(BlueprintType)
struct FFractureChunk
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, /*Category = "Runtime Destructible Actor Chunks",*/ DisplayName = "Fracture Chunk")
	UFractureChunkComponent* FractureChunkComponent;	
	
	//Constructor
	FFractureChunk()
	{
	}
};

USTRUCT(BlueprintType)
struct FTempVertex
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, DisplayName = "Position")
	FVector						Position;
	
	// Normal, TangentZ
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, DisplayName = "Normal")	
	FVector						Normal;

	// Tangent, TangentX, U-direction
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, DisplayName = "Tangent")
	FRuntimeMeshTangent			Tangent;

	// Binormal, TangentY, V-direction
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, DisplayName = "Binormal")	
	FVector						Binormal;
	
	// UVs, Texture coordinates
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, DisplayName = "UVs (TexCoords)")
	TArray<FVector2D> UVs;

	// VertexColor
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, DisplayName = "Color")
	FColor						Color;	

	FTempVertex()
	{
		UVs.AddDefaulted(MAX_TEXCOORDS);
	}


};

USTRUCT(BlueprintType)
struct FTempSection
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, DisplayName = "Section Index")
	int32 SectionIndex;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, DisplayName = "Vertices")
	TArray<FTempVertex> Vertices;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, DisplayName = "Indices")
	TArray<int32> Indices;


};

USTRUCT(BlueprintType)
struct FTempConvexHull
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, DisplayName = "Vertices")
	TArray<FVector> Vertices;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, DisplayName = "Planes")
	TArray<FPlane> Planes;


};

USTRUCT(BlueprintType)
struct FTempChunk
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, DisplayName = "Chunk Index")
	int32 ChunkIndex;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, DisplayName = "Sections")
	TArray<FTempSection> Sections;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, DisplayName = "Convex Hulls")
	TArray<FTempConvexHull> ConvexHulls;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, DisplayName = "Overlaps")
	TSet<int32> Overlaps;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, DisplayName = "Children")
	TSet<int32> Children;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, DisplayName = "Parent Index")
	int32 ParentIndex;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, DisplayName = "Isle Index")
	int32 IsleIndex;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, DisplayName = "Depth")
	int32 Depth;
	   	

	FTempChunk()
	{
		IsleIndex = INDEX_NONE;
	}

};


/**
Helper struct for CopyDestructibleMeshDataToRuntimeMeshComponent() in construction script handle 
**/
USTRUCT(BlueprintType)
struct FPostEditChangeHelper
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(BlueprintReadOnly)
	bool bFullRebuild = true;

	/*UPROPERTY(BlueprintReadOnly)
	bool bChangeSimulatePhysicsAtDepth = true;*/
};



UCLASS()
class RUNTIMEMESHCOMPONENT_API ARuntimeDestructibleActor : public AActor
{
	GENERATED_BODY()

public:
	ARuntimeDestructibleActor(const FObjectInitializer& ObjectInitializer);
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Scene Component", DisplayName ="Scene")
	USceneComponent* SceneComponent;		

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Transient, Category = "Destructible Mesh Component", DisplayName = "Destructible Mesh")
	URMC_DestructibleComponent* RMC_DestructibleMesh;	
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Origin Destructible Mesh", DisplayName = "Origin Destructible Mesh")
	URMC_DestructibleMesh* OriginDestructibleMesh;
	
	UPROPERTY(EditDefaultsOnly, Transient, BlueprintReadOnly, Category = "Fracture Chunks", DisplayName = "Fracture Chunks")
	TArray<UFractureChunkComponent*> FractureChunks;

	UPROPERTY(EditDefaultsOnly, Transient, BlueprintReadOnly, Category = "Fracture Chunk Isles", DisplayName = "Fracture Chunk Isles")
	TArray<UIsleComponent*> Isles;

	UPROPERTY(EditDefaultsOnly, Transient, BlueprintReadOnly, Category = "Fracture Chunk Actors", DisplayName = "Fracture Chunk Actors")
	TArray<AFractureChunkActor*> FractureChunkActors;

	UPROPERTY(EditDefaultsOnly, Transient, BlueprintReadOnly, Category = "Isle Actors", DisplayName = "Isle Actors")
	TArray<AIsleActor*> IsleActors;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Fractured Chunks Count", DisplayName = "Fractured Chunks Count")
	int32 FracturedChunksCount;	



	/**
	
	**/
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Temporary Chunks", DisplayName = "Temporary Chunks")
	TArray<FTempChunk> TempChunks;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Materials", DisplayName = "Override Materials")
	TArray <UMaterialInterface*> OverrideMaterials;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Materials", DisplayName = "Origin Materials")
	TArray <FSkeletalMaterial> OriginMaterials;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Variables", DisplayName = "Override Materials?")
	bool bOverrideMaterials;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Variables", DisplayName = "Copy Collision?")
	bool bCopyCollision;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Variables", DisplayName = "Create Adjacency Buffer?")
	bool bCreateAdjBuffer;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Variables", DisplayName = "UVs Number")
	int32 NumUV;

	/**
	Wether chunks at depth level should simulate physics
	**/
	UPROPERTY(EditAnywhere, BlueprintReadOnly, EditFixedSize, Category = "Simulate Physics At Depth", DisplayName = "Simulate Physics At Depth")
	TArray<bool> SimulatePhysicsAtDepth;
	
	UPROPERTY(BlueprintReadOnly, Category = "PostEditChange Helper", DisplayName = "PostEditChange Helper")
	FPostEditChangeHelper PostEditChangeHelper;	

	//UPROPERTY(BlueprintReadOnly, Transient)
	TArray<TArray<int32>> SectionIndices;

	//UPROPERTY(BlueprintReadOnly, Transient)
	TArray<TArray<int32>> ConvexHullIndices;	
	
	//Temp array for line traces
	//TArray<int32> SingleChunks;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

#if WITH_EDITOR
	virtual void PostEditChangeProperty(struct FPropertyChangedEvent& Event) override;

	//for TArrays:
	virtual void PostEditChangeChainProperty(struct FPropertyChangedChainEvent& ChainEvent) override;

	virtual void OnConstruction(const FTransform& Transform) override;
#endif

private:
	//virtual void Serialize(FArchive& Ar) override;

public:	


	// Called every frame
	virtual void Tick(float DeltaTime) override;
		
#if WITH_EDITOR	/*&& WITH_APEX*/

	/** 
	Copy chunk from a destructible mesh to a runtime mesh 
	Source static mesh's ChunkIndex = 0
	**/
	UFUNCTION(BlueprintCallable)
		bool CopyDestructibleMeshChunkToRuntimeMesh(int32 ChunkIndex = 0, bool bOverrideMats = true, bool bCopyCollis = false, bool bCreateAdjBuff = false);
	
	/** 
	Copy all chunks data from a destructible mesh component to FTempChunk Array	
	**/
	UFUNCTION(BlueprintCallable)
		static bool CopyDestructibleMeshDataToRuntimeMeshComponent(ARuntimeDestructibleActor* RuntimeDestructibleActor, bool bOverrideMats = true, bool bCopyCollis = false, bool bCreateAdjBuff = false);

#endif //WITH_EDITOR
	
	UFUNCTION(BlueprintCallable)
		static void ApplyRadiusDamage(ARuntimeDestructibleActor* RuntimeDestructibleActor, float BaseDamage, const FVector Impulse,
			const FVector HurtOrigin, float DamageRadius, float ImpulseStrength, bool bFullDamage);

	UFUNCTION(BlueprintCallable)
		static void TestAttachment(ARuntimeDestructibleActor* Actor);

//#endif //WITH_EDITOR && WITH_APEX

//#if WITH_APEX
private:

#if	WITH_EDITOR
	//This function uses FSoftSkinVertex - editor only type
	static void BuildAdjacencyBuffer(const TArray<FTempVertex>& RM_TempVertexBuffer, const uint32 NumUV, const TArray<int32>& RM_IndexBuffer, TArray<uint32>& OutRM_AdjacencyIndexBuffer);
#endif //WITH_EDITOR

	static void GetChunkConvexHullVertices(TArray<TArray<FVector>>& DM_ConvexHullsToVertices, TArray<TArray<FPlane>>& DM_ConvexHullsToPlanes, nvidia::apex::ExplicitHierarchicalMesh& DM_HierarchicalMesh, int32 ChunkIdx);

	static bool GetChunkCollision(nvidia::apex::ExplicitHierarchicalMesh& DM_HierarchicalMesh, TArray<TArray<FVector>>& DM_ConvexHullsToVertices, TArray<TArray<FPlane>>& DM_ConvexHullsToPlanes, int32 ChunkIdx);
	/** Cet Chunks Collision**/
	static bool GetChunksCollision(nvidia::apex::ExplicitHierarchicalMesh& DM_HierarchicalMesh, TArray<TArray<TArray<FVector>>>& DM_ChunksToConvexHullsToVertices, TArray<TArray<TArray<FPlane>>>& DM_ChunksToConvexHullsToPlanes);

	static void GetChunkBuffers(TArray<TArray<FTempVertex>>& DM_SectionsToVertexBuffers, TArray<TArray<int32>>& DM_SectionsToIndexBuffers,
		TArray<int32>& DM_SectionIndices, nvidia::apex::ExplicitHierarchicalMesh& DM_HierarchicalMesh, nvidia::apex::RenderMeshAssetAuthoring* RenderMeshAssetAuthor, int32 ChunkIdx);

	static void GetChunkRenderData(TArray<TArray<FTempVertex>>& DM_SectionsToVertexBuffers, TArray<TArray<int32>>& DM_SectionsToIndexBuffers,
		TArray<int32>& DM_SectionIndices, nvidia::apex::ExplicitHierarchicalMesh& DM_HierarchicalMesh, int32 ChunkIdx);
	/**
	Create Vertex and Index Buffers for sections of DestructibleMesh parts(chunks)
	**/
	static void GetChunksRenderData(TArray<TArray<TArray<FTempVertex>>>& DM_PartsToSectionsToVertexBuffers, TArray<TArray<TArray<int32>>>& DM_PartsToSectionsToIndexBuffers,
		TArray<TArray<int32>>&  DM_PartsToSectionIndices, nvidia::apex::ExplicitHierarchicalMesh& DM_HierarchicalMesh);




	////////////////////////////////////////////////////////////////////////////////FTempChunk/////////////////////////////////////////////////////////////////////////

	static void GetChunkConvexHullVerticesAndPlanes(FTempChunk& DM_Chunk, nvidia::apex::ExplicitHierarchicalMesh& DM_HierarchicalMesh, int32 ChunkIdx);

	static bool GetChunkCollision(nvidia::apex::ExplicitHierarchicalMesh& DM_HierarchicalMesh, FTempChunk& DM_Chunk, int32 ChunkIdx);
	/** Cet Chunks Collision**/
	static bool GetChunksCollision(nvidia::apex::ExplicitHierarchicalMesh& DM_HierarchicalMesh, TArray<FTempChunk>& TempChunks);

	static void GetChunkBuffers(FTempChunk& DM_Chunk, nvidia::apex::ExplicitHierarchicalMesh& DM_HierarchicalMesh, nvidia::apex::RenderMeshAssetAuthoring* RenderMeshAssetAuthor, int32 ChunkIdx);

	static void GetChunkRenderData(FTempChunk& DM_Chunk, nvidia::apex::ExplicitHierarchicalMesh& DM_HierarchicalMesh, int32 ChunkIdx);
	/**
	Create Vertex and Index Buffers for sections of DestructibleMesh parts(chunks)
	**/
	static void GetChunksRenderData(TArray<FTempChunk>& TempChunks, nvidia::apex::ExplicitHierarchicalMesh& DM_HierarchicalMesh);

	static void CreateRuntimeMeshRenderData(URuntimeMeshComponent* RuntimeMeshComponent, FTempChunk& DM_Chunk,
		const TArray<UMaterialInterface*>& DM_OverrideMaterials, const TArray<FSkeletalMaterial>& DM_Materials, const uint32 NumUV, bool bOverrideMaterials, bool bCreateAdjBuffer);

	static void CreateRuntimeMeshCollision(URuntimeMeshComponent* RuntimeMeshComponent, FTempChunk& DM_Chunk);

	static void FillOverlapsDepthParents(TArray<FTempChunk>& TempChunks, TArray<uint32>& DM_DepthToOverlapsCount, TArray<const nvidia::apex::IntPair*>& DM_DepthToOverlaps, nvidia::apex::ExplicitHierarchicalMesh& HierarchicalMesh);

	static void BuildDestructibleActorChunks(ARuntimeDestructibleActor* RuntimeDestructibleActor);

	static void CopyChunkSectionsToIsle(ARuntimeDestructibleActor* RuntimeDestructibleActor, UFractureChunkComponent* Chunk, UIsleComponent* Isle);
	
	static void ClearFractureChunkActors( TArray <AFractureChunkActor*>& FractureActors);

	static void ClearIsleActors(TArray <AIsleActor*>& IsleActors);

	static UIsleComponent* InitIsleComponent(USceneComponent* InParent, const wchar_t* IsleName);

	static UFractureChunkComponent* InitFractureChunkComponent(ARuntimeDestructibleActor* RuntimeDestructibleActor, USceneComponent* InParent, const wchar_t* ChunkName, FTempChunk& TempChunk);

	static void CreateFractureChunkFromTempChunk(ARuntimeDestructibleActor* RuntimeDestructibleActor, FTempChunk& TempChunk, UFractureChunkComponent* FractureChunk);

	/**Recursive search of Chunk's overlaps and attaching them to Isle**/
	static void SearchOverlapsToAttach(ARuntimeDestructibleActor* RuntimeDestructibleActor, UIsleComponent* Isle, FTempChunk& TempChunk);	

	static void CreatePhysicsConstraint(AActor* RootActor, AActor* TargetActor);

	static AIsleActor* SpawnIsleActor(ARuntimeDestructibleActor* RuntimeDestructibleActor, const FTransform Transform, bool bSimulatePhysics = false, FName CollisionProfileName = TEXT("NoCollision"));

	static AFractureChunkActor* SpawnFractureChunkActor(ARuntimeDestructibleActor* RuntimeDestructibleActor, FTempChunk& TempChunk);

	static AFractureChunkActor* SpawnFractureChunkActor(ARuntimeDestructibleActor* RuntimeDestructibleActor);

	static void AddTestChunk(ARuntimeDestructibleActor* RuntimeDestructibleActor);

	static void FractureTestChunk(ARuntimeDestructibleActor* RuntimeDestructibleActor);

	static void DamageChunk(ARuntimeDestructibleActor* RuntimeDestructibleActor, UFractureChunkComponent* FractureChunk, const FTransform& ChunkTransform, float BaseDamage, const FVector& Impulse,
		const FVector& LocalHurtOrigin, const FVector& WorldHurtOrigin, float DamageRadius, float ImpulseStrength, bool bFullDamage);

	static void DamageIsle(ARuntimeDestructibleActor* RuntimeDestructibleActor, UIsleComponent* Isle, const FTransform& IsleTransform, float BaseDamage, const FVector& Impulse,
		const FVector& LocalHurtOrigin, const FVector& WorldHurtOrigin, float DamageRadius, float ImpulseStrength, bool bFullDamage);

	static int32 SearchNewIsles(ARuntimeDestructibleActor* RuntimeDestructibleActor, UIsleComponent* SourceIsle, const FTransform& IsleTransform,
		TArray<int32>& SectionsToRemove, TArray<int32>& ConvexCollisionSectionsToRemove,
		float BaseDamage, const FVector& Impulse, const FVector& HurtOrigin, float DamageRadius, float ImpulseStrength, bool bFullDamage);

	static void SearchDamagedChunks(ARuntimeDestructibleActor* RuntimeDestructibleActor, UIsleComponent* Isle, const FTransform& IsleTransform, int32 ConvexHullId,
		TArray<int32>& SectionsToRemove, TArray<int32>& ConvexCollisionSectionsToRemove, TSet<int32>& CheckedCollisionSections,
		float BaseDamage, const FVector& Impulse, const FVector& LocalHurtOrigin, const FVector& WorldHurtOrigin, float DamageRadius, float ImpulseStrength, bool bFullDamage);

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////



	static inline FVector PxVec3_To_FVector(const physx::PxVec3& InVertex);

	static inline FColor VertexColor_To_FColor(const nvidia::apex::VertexColor& InColor);

	static inline FVector2D VertexUV_To_FVector2D(const nvidia::apex::VertexUV& InVertexUV);

#if	WITH_EDITOR
	//FSoftSkinVertex is editor only type
	static inline FSoftSkinVertex NvVertex_To_FSoftSkinVertex(const nvidia::apex::Vertex& InVertex);

	static inline FSoftSkinVertex FTempVertex_To_FSoftSkinVertex(const FTempVertex& InVertex);
#endif //WITH_EDITOR

	static inline FTempVertex NvVertex_To_FTempVertex(const nvidia::apex::Vertex& InVertex);

	static void CreateRuntimeMeshCollision(URuntimeMeshComponent* RuntimeMeshComponent, TArray<TArray<FVector>>& DM_ConvexHullsToVertices, const TArray<TArray<FPlane>>& DM_ConvexHullsToPlanes);

	/**Creating RuntimeMesh Sections, tesselation triangles (if need) and setting materials **/
	static void CreateRuntimeMeshRenderData(URuntimeMeshComponent* RuntimeMeshComponent, const TArray<TArray<FTempVertex>>& DM_PartSectionsToVertexBuffers, const TArray<TArray<int32>>& DM_PartSectionsToIndexBuffers,
		const TArray<int32>& DM_PartSectionIndices, const TArray<UMaterialInterface*>& DM_OverrideMaterials, const TArray<FSkeletalMaterial>& DM_Materials, const uint32 NumUV, bool bOverrideMaterials, bool bCreateAdjBuffer);

	static UFractureChunkComponent* InitFakeFractureChunkComponent(ARuntimeDestructibleActor* RuntimeDestructibleActor);

	static UFractureChunkComponent* InitFractureChunkComponent(ARuntimeDestructibleActor* RuntimeDestructibleActor, nvidia::apex::ExplicitHierarchicalMesh& HierarchicalMesh, uint32 ChunkIndex, const wchar_t* ChunkName);

	
	static void SetupChunksAttachments(ARuntimeDestructibleActor* RuntimeDestructibleActor, TArray<uint32>& DM_DepthToOverlapsCount, TArray<const nvidia::apex::IntPair*>& DM_DepthToOverlaps);

	static UIsleComponent* InitFakeIsleComponent(ARuntimeDestructibleActor* RuntimeDestructibleActor, const wchar_t* IsleName);

	

	/**Recursive search of Chunk's overlaps and attaching them to Isle**/
	static void SearchOverlapsToAttach(TArray <UFractureChunkComponent*>& FractureChunks, UIsleComponent* Isle, UFractureChunkComponent* Chunk);

	/**Recursive search of separate overlapped chunks groups and attaching them to new Isle**/
	static void SearchOverlapsToDetach(TArray <UFractureChunkComponent*>& FractureChunks, UIsleComponent* Isle, UFractureChunkComponent* Chunk);

	static void ClearFractureChunks(TArray <FFractureChunk>& FractureChunks);

	static void ClearFractureChunks(TArray <UFractureChunkComponent*>& FractureChunks);

	static void ClearIsles(TArray <UIsleComponent*>& Isles);

	static void ClearIsles(TArray <FIsle>& Isles);

	static void UpdateBodySetup(URuntimeMeshComponent* Component);

	static void AddFakeBoxCollision(UFractureChunkComponent* Chunk);

//#endif  //WITH_APEX

	static void InitFakeIsleData(URuntimeMeshComponent* FakeIsleComponent);

	static void WriteRuntimeMeshDataToTxtFile(const URuntimeMesh* InRuntimeMesh, const char* Path = "E:/Unreal Engine 4 Projects/");

	/**
	Copy runtime mesh render data and collision to other runtime mesh
	Only one UV-channel supporting at this moment
	**/
	static void CloneRuntimeMesh(const URuntimeMesh* FromRuntimeMesh, URuntimeMesh* ToRuntimeMesh);

	//Useful functions 

	void CreateFixedJoint(UPrimitiveComponent* Parent, UPrimitiveComponent* Target);

	void GetActorsInRadius(const FVector Center, float Radius);

	void GetPxScene();

	void GetPxConstraints();

};
