// Copyright 2016-2018 Chris Conway (Koderz). All Rights Reserved.

#include "RuntimeDestructibleActor.h"
#include "FractureChunkComponent.h"
#include "FractureChunkActor.h"
#include "IsleComponent.h"
#include "IsleActor.h"
#include "ConstraintActor.h"

#include "RuntimeMesh.h"
#include "RuntimeMeshData.h"
#include "RuntimeMeshComponent.h"
#include "RuntimeMeshLibrary.h"

#include "Engine/SkeletalMesh.h"
#include "RMC_DestructibleMesh.h"
#include "RMC_DestructibleComponent.h"
#include "RMC_DestructibleFractureSettings.h"

#include "WorldCollision.h"

#include "PhysicsEngine/PhysicsConstraintComponent.h"

//#include "PhysXPublic.h"
//#include "RenderMeshAsset.h"//nvidia::apex::Vertex

#include "Rendering/SkeletalMeshRenderData.h"
#include "Rendering/SkeletalMeshLODModel.h"//FSoftSkinVertex
#include "Rendering/SkeletalMeshModel.h"

#if WITH_EDITOR
#include "ThirdPartyBuildOptimizationHelper.h"
#endif
//////////////////////////////////////////////////////////////////////////////////////////////////////////
#include "ApexSharedUtils.h"// struct IntPair

///////////////////////////////////////////////////////////////////////////////////////

#include <fstream>

#include <sstream>
using std::stringstream;
#include <iostream>
#include <iomanip>

using std::cout;
using std::endl;
using std::cin;
//using namespace std;

#include <string>
using namespace std;


#define DEBUGMESSAGE(x, ...) if(GEngine){GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Red, FString::Printf(TEXT(x), __VA_ARGS__));}


// Sets default values
ARuntimeDestructibleActor::ARuntimeDestructibleActor(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	SceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Scene"));

	RMC_DestructibleMesh = CreateDefaultSubobject<URMC_DestructibleComponent>(TEXT("RMC Destructible Mesh"));

	RootComponent = SceneComponent;
	RMC_DestructibleMesh->SetupAttachment(RootComponent);	
}

// Called when the game starts or when spawned
void ARuntimeDestructibleActor::BeginPlay()
{
	Super::BeginPlay();		

	RMC_DestructibleMesh->DestroyComponent();

	//Creating fracturechunk and isle components inside RuntimeDestructibleActor either in construction script or in BeginPlay causes unexpected physics behaviour
	//Also Epic developers recommending to use physics constraints instead of AttachToComponent at runtime
	//But my experiments with creating constraints were unsuccessful
	//Conclusion: Spawn Actors of each chunk and isle and setup hierarchy by attaching components
	
	BuildDestructibleActorChunks(this);

	//Set all isles and fracturechunks owning actor
	for (int32 i = 0; i < FractureChunks.Num(); i++)
	{
		FractureChunks[i]->OwningActor = this;
	}
	for (int32 i = 0; i < Isles.Num(); i++)
	{
		Isles[i]->OwningActor = this;
	}

	/*FractureChunks[0]->SetVisibility(false);
	FractureChunks[0]->SetSimulatePhysics(false);
	FractureChunks[0]->SetCollisionProfileName(TEXT("NoCollision"));*/


	//UIsleComponent* Isle0 = Isles[0];
	//
	//Isle0->SetSimulatePhysics(true);
	////Isle->SetCollisionProfileName(TEXT("Destructible"));	
	//Isle0->SetCollisionProfileName(TEXT("OverlapAllDynamic"));
	////Isle->SetCollisionProfileName(TEXT("BlockAllDynamic"));
	//Isle0->SetVisibility(false);
	////Isle->DetachFromComponent(FDetachmentTransformRules(EDetachmentRule(EDetachmentRule::KeepRelative), true));					
	//Isle0->GetBodyInstance()->SetMassOverride(0, true);

	
	//UWorld* World = GetWorld();

	//AIsleActor* IsleActor = World->SpawnActor<AIsleActor>(AIsleActor::StaticClass(), GetTransform());

	//UIsleComponent* Isle = IsleActor->IsleComponent;

	//Isle->SetCollisionUseComplexAsSimple(false);

	//Isle->SetSimulatePhysics(true);
	////Isle->SetCollisionProfileName(TEXT("OverlapAllDynamic"));
	//Isle->SetCollisionProfileName(TEXT("NoCollision"));

	//Isle->SetVisibility(false);
	//Isle->SetCastShadow(false);

	//Isle->GetBodyInstance()->bAutoWeld = true;

	//Isle->GetBodyInstance()->SetMassOverride(0, true);

	//CloneRuntimeMesh(Isle0->GetOrCreateRuntimeMesh(), Isle->GetOrCreateRuntimeMesh());

	//IsleActors.Add(IsleActor);	
	
	for (int32& i : FractureChunks[1]->ChildrenIndices)
	{
		//FractureChunks[i]->SetVisibility(true);
		//FractureChunks[i]->SetCollisionProfileName(TEXT("BlockAllDynamic"));

		//FractureChunks[i]->DetachFromComponent(FDetachmentTransformRules(EDetachmentRule(EDetachmentRule::KeepWorld), true));
		//FractureChunks[i]->DestroyComponent();

		//AFractureChunkActor* FractureChunkActor = World->SpawnActor<AFractureChunkActor>(AFractureChunkActor::StaticClass(), GetTransform());

		//UFractureChunkComponent* FractureChunk = FractureChunkActor->FractureChunkComponent;

		//FractureChunk->SetCollisionUseComplexAsSimple(false);

		//FractureChunk->SetSimulatePhysics(false);
		////FractureChunk->SetCollisionProfileName(TEXT("BlockAllDynamic"));
		//FractureChunk->SetCollisionProfileName(TEXT("NoCollision"));

		//FractureChunk->SetVisibility(true);
		//FractureChunk->SetCastShadow(false);

		//FractureChunk->GetBodyInstance()->bAutoWeld = true;
		//
		//CloneRuntimeMesh(FractureChunks[i]->GetOrCreateRuntimeMesh(), FractureChunk->GetOrCreateRuntimeMesh());

		//FractureChunk->AttachToComponent(Isle, FAttachmentTransformRules(EAttachmentRule(EAttachmentRule::KeepWorld), true));
		////CreatePhysicsConstraint(IsleActor, FractureChunkActor);
		//
		//FractureChunkActors.Add(FractureChunkActor);

	}	

}

// Called every frame
void ARuntimeDestructibleActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);	
}


#if WITH_EDITOR
void ARuntimeDestructibleActor::PostEditChangeProperty(struct FPropertyChangedEvent& Event)
{
	FName PropertyName = (Event.Property != NULL) ? Event.Property->GetFName() : NAME_None;

	//FName Name = GET_MEMBER_NAME_CHECKED(ARuntimeDestructibleActor, RMC_DestructibleMesh)

	if (PropertyName == GET_MEMBER_NAME_CHECKED(ARuntimeDestructibleActor, OriginDestructibleMesh))
	{
		this->PostEditChangeHelper.bFullRebuild = true;
		//this->PostEditChangeHelper.bChangeSimulatePhysicsAtDepth = true;
	}


	Super::PostEditChangeProperty(Event);
}

//for TArrays:
void ARuntimeDestructibleActor::PostEditChangeChainProperty(struct FPropertyChangedChainEvent& ChainEvent)
{	
	FName PropertyName = (ChainEvent.Property != NULL) ? ChainEvent.Property->GetFName() : NAME_None;

	if (PropertyName == GET_MEMBER_NAME_CHECKED(ARuntimeDestructibleActor, SimulatePhysicsAtDepth))
	{
		this->PostEditChangeHelper.bFullRebuild = false;
		//this->PostEditChangeHelper.bChangeSimulatePhysicsAtDepth = false;
	}

	Super::PostEditChangeChainProperty(ChainEvent);
}

void ARuntimeDestructibleActor::OnConstruction(const FTransform& Transform)
{
	if (this->PostEditChangeHelper.bFullRebuild)
	{		
		this->RMC_DestructibleMesh->SetDestructibleMesh(this->OriginDestructibleMesh);
		CopyDestructibleMeshDataToRuntimeMeshComponent(this, true, true, false);
	}
}
#endif


#if WITH_EDITOR	/*&& WITH_APEX*/

bool ARuntimeDestructibleActor::CopyDestructibleMeshChunkToRuntimeMesh(int32 ChunkIndex, bool bOverrideMaterials, bool bCopyCollision, bool bCreateAdjBuffer)
{
	if (RMC_DestructibleMesh == nullptr || ChunkIndex < 0)
		return false;

	URMC_DestructibleMesh* DestructibleMesh = RMC_DestructibleMesh->GetDestructibleMesh();
	if (DestructibleMesh != nullptr)
	{
		uint32 NumUV;//TexCoordsNum			

		const FSkeletalMeshLODRenderData& LODRenderData = DestructibleMesh->GetResourceForRendering()->LODRenderData[0];

		NumUV = LODRenderData.GetNumTexCoords();//TexCoordsNum

		const TArray <UMaterialInterface*>& DM_OverrideMaterials = RMC_DestructibleMesh->GetMaterials();

		const TArray <FSkeletalMaterial>& DM_Materials = DestructibleMesh->Materials;

		/**
		Vertex Buffers for sections of DestructibleMesh part(chunk)
		**/
		TArray<TArray<FTempVertex>> DM_PartSectionsToVertexBuffers;

		/**
		Index Buffers for sections of DestructibleMesh part(chunk)
		**/
		TArray<TArray<int32>> DM_PartSectionsToIndexBuffers;

		/**
		Section Indices of DestructibleMesh part(chunk)
		**/
		TArray<int32> DM_PartSectionIndices;

		URMC_DestructibleFractureSettings* DM_FractureSettings = DestructibleMesh->FractureSettings;
		DestructibleAssetAuthoring*	DM_ApexDestructibleAssetAuthoring = DM_FractureSettings->ApexDestructibleAssetAuthoring;
		const DestructibleAsset& DM_DestructibleAsset = *DestructibleMesh->ApexDestructibleAsset;

		ExplicitHierarchicalMesh& DM_HierarchicalMesh = DM_ApexDestructibleAssetAuthoring->getExplicitHierarchicalMesh();

		if (!DM_HierarchicalMesh.chunkCount())
			DM_ApexDestructibleAssetAuthoring->importDestructibleAssetToRootMesh(DM_DestructibleAsset);

		GetChunkRenderData(DM_PartSectionsToVertexBuffers, DM_PartSectionsToIndexBuffers, DM_PartSectionIndices, DM_HierarchicalMesh, ChunkIndex);

		//Clear Chunks
		ClearFractureChunks(FractureChunks);

		wstring ChunkName = L"Chunk_" + to_wstring(ChunkIndex);

		UFractureChunkComponent* RM_Chunk = NewObject<UFractureChunkComponent>(RootComponent, ChunkName.data());

		RM_Chunk->SetRelativeLocation(RootComponent->GetComponentLocation());
		RM_Chunk->SetRelativeRotation(RootComponent->GetComponentRotation());
		//Chunks[ChunkIndex]->SetupAttachment(RuntimeDestructibleActor->RootComponent);

		RM_Chunk->SetCollisionUseComplexAsSimple(false);
		RM_Chunk->SetSimulatePhysics(false);

		// Create Source RuntimeMeshComponent render data
		CreateRuntimeMeshRenderData(RM_Chunk, DM_PartSectionsToVertexBuffers, DM_PartSectionsToIndexBuffers, DM_PartSectionIndices,
			DM_OverrideMaterials, DM_Materials, NumUV, bOverrideMaterials, bCreateAdjBuffer);

		if (bCopyCollision)
		{
			/** Array of vertices of convex hulls of chunk**/
			TArray<TArray<FVector>> DM_ChunkConvexHullsToVertices;

			/** Array of planes of convex hulls of chunk**/
			TArray<TArray<FPlane>> DM_ChunkToConvexHullsToPlanes;

			//Get Chunk's Collision	
			GetChunkCollision(DM_HierarchicalMesh, DM_ChunkConvexHullsToVertices, DM_ChunkToConvexHullsToPlanes, ChunkIndex);

			// Create Source RuntimeMeshComponent collision
			CreateRuntimeMeshCollision(RM_Chunk, DM_ChunkConvexHullsToVertices, DM_ChunkToConvexHullsToPlanes);
		}

		UFractureChunkComponent* FractureChunk = RM_Chunk;
		FractureChunks.Add(FractureChunk);

		return true;
	}
	return false;
}


bool ARuntimeDestructibleActor::CopyDestructibleMeshDataToRuntimeMeshComponent(ARuntimeDestructibleActor* RuntimeDestructibleActor, bool bOverrideMaterials, bool bCopyCollision, bool bCreateAdjBuffer)
{
	//DEBUGMESSAGE("Copy Component")
	if (RuntimeDestructibleActor && RuntimeDestructibleActor->PostEditChangeHelper.bFullRebuild)
	{		
		RuntimeDestructibleActor->PostEditChangeHelper.bFullRebuild = false;

		URMC_DestructibleComponent* DestructibleMeshComponent = RuntimeDestructibleActor->RMC_DestructibleMesh;

		if (DestructibleMeshComponent != nullptr)
		{
			URMC_DestructibleMesh* DestructibleMesh = DestructibleMeshComponent->GetDestructibleMesh();
			if (DestructibleMesh != nullptr)
			{
				const FSkeletalMeshLODRenderData& LODRenderData = DestructibleMesh->GetResourceForRendering()->LODRenderData[0];

				RuntimeDestructibleActor->NumUV = LODRenderData.GetNumTexCoords();//TexCoordsNum

				RuntimeDestructibleActor->OverrideMaterials = DestructibleMeshComponent->GetMaterials();

				RuntimeDestructibleActor->OriginMaterials = DestructibleMesh->Materials;

				RuntimeDestructibleActor->bOverrideMaterials = bOverrideMaterials;
				RuntimeDestructibleActor->bCopyCollision = bCopyCollision;
				RuntimeDestructibleActor->bCreateAdjBuffer = bCreateAdjBuffer;

				URMC_DestructibleFractureSettings* DM_FractureSettings = DestructibleMesh->FractureSettings;
				DestructibleAssetAuthoring*	DM_ApexDestructibleAssetAuthoring = DM_FractureSettings->ApexDestructibleAssetAuthoring;
				const DestructibleAsset& DM_DestructibleAsset = *DestructibleMesh->ApexDestructibleAsset;
				/////////////////////////////////////////////////////////////////////////////////////////////////////////
				uint32_t DepthCount = DM_DestructibleAsset.getDepthCount();

				//Fill SimulatePhysicsAtDepth array
				RuntimeDestructibleActor->SimulatePhysicsAtDepth.Init(true, DepthCount);				

				TArray<const IntPair*> DM_DepthToOverlaps;
				TArray<uint32> DM_DepthToOverlapsCount;
				for (uint32_t Depth = 0; Depth < DepthCount; Depth++)
				{
					DM_DepthToOverlapsCount.Add(DM_DestructibleAsset.getCachedOverlapCountAtDepth(Depth));
					DM_DepthToOverlaps.Add(DM_DestructibleAsset.getCachedOverlapsAtDepth(Depth));
				}

				/////////////////////////////////////////////////////////////////////////////////////////////////////////
				ExplicitHierarchicalMesh& DM_HierarchicalMesh = DM_ApexDestructibleAssetAuthoring->getExplicitHierarchicalMesh();

				if (!DM_HierarchicalMesh.chunkCount())
					DM_ApexDestructibleAssetAuthoring->importDestructibleAssetToRootMesh(DM_DestructibleAsset);
				

				///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

				TArray <UFractureChunkComponent*>& FractureChunks = RuntimeDestructibleActor->FractureChunks;
				TArray <UIsleComponent*>& Isles = RuntimeDestructibleActor->Isles;

				//Clear Chunks
				ClearFractureChunks(FractureChunks);
				//Clear Isles
				ClearIsles(Isles);
				//Clear FractureChunkActors
				ClearFractureChunkActors(RuntimeDestructibleActor->FractureChunkActors);
				//Clear IsleActors
				ClearIsleActors(RuntimeDestructibleActor->IsleActors);

				TArray<FTempChunk>& TempChunks = RuntimeDestructibleActor->TempChunks;
				GetChunksRenderData(TempChunks, DM_HierarchicalMesh);

				if (bCopyCollision)
					GetChunksCollision(DM_HierarchicalMesh, TempChunks);

				FillOverlapsDepthParents(TempChunks, DM_DepthToOverlapsCount, DM_DepthToOverlaps, DM_HierarchicalMesh);

				//BuildDestructibleActorChunks(RuntimeDestructibleActor, TempChunks, DM_OverrideMaterials, DM_Materials, NumUV, bCopyCollision, bOverrideMaterials, bCreateAdjBuffer);

				
				///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////	

				return true;
			}
		}
	}
	return false;
}

#endif //WITH_EDITOR

void ARuntimeDestructibleActor::ApplyRadiusDamage(ARuntimeDestructibleActor* RuntimeDestructibleActor, float BaseDamage, const FVector& HurtOrigin, float DamageRadius, float ImpulseStrength, bool bFullDamage)
{
	FCollisionQueryParams SphereParams(SCENE_QUERY_STAT(ApplyRadialDamage), false);

	TArray<FOverlapResult> Overlaps;
	if (UWorld* World = GEngine->GetWorldFromContextObject(RuntimeDestructibleActor, EGetWorldErrorMode::LogAndReturnNull))
	{
		World->OverlapMultiByObjectType(Overlaps, HurtOrigin, FQuat::Identity, FCollisionObjectQueryParams(FCollisionObjectQueryParams::InitType::AllObjects), FCollisionShape::MakeSphere(DamageRadius), SphereParams);
	}

	TMap<AActor*, TArray<FHitResult> > OverlapComponentMap;
	for (int32 Idx = 0; Idx < Overlaps.Num(); ++Idx)
	{
		FOverlapResult const& Overlap = Overlaps[Idx];
		AActor* const OverlapActor = Overlap.GetActor();
		UPrimitiveComponent* const OverlapComp = Overlap.GetComponent();

		UFractureChunkComponent* FractureChunk = Cast<UFractureChunkComponent>(OverlapComp);
		if (FractureChunk != nullptr)
		{
			FractureChunk->SetSimulatePhysics(true);			
		}
	}
}

void ARuntimeDestructibleActor::TestAttachment(ARuntimeDestructibleActor* Actor)
{
	static int32 Count = 0;
	if (Count % 2 == 0)
	{
		Actor->FractureChunks[3]->SetSimulatePhysics(false);
		Actor->FractureChunks[3]->AttachToComponent(Actor->FractureChunks[2], FAttachmentTransformRules(EAttachmentRule(EAttachmentRule::KeepWorld), true));
	}
	else
	{
		Actor->FractureChunks[3]->SetSimulatePhysics(true);
	}
	Count++;
}



//#endif //WITH_EDITOR && WITH_APEX



//#if WITH_APEX

int32 BoneIdxToChunkIdx(int32 BoneIndex)
{
	return --BoneIndex;
}
int32 ChunkIdxToBoneIdx(int32 ChunkIndex)
{
	return ++ChunkIndex;
}

#if	WITH_EDITOR
void ARuntimeDestructibleActor::BuildAdjacencyBuffer(const TArray<FTempVertex>& RM_TempVertexBuffer, const uint32 NumUV, const TArray<int32>& RM_IndexBuffer, TArray<uint32>& OutRM_AdjacencyIndexBuffer)
{
	TArray<FSoftSkinVertex> RM_SoftSkinVertices;
	TArray<uint32> RM_Indices;
	
	RM_SoftSkinVertices.AddDefaulted(RM_TempVertexBuffer.Num());
	RM_Indices.AddDefaulted(RM_IndexBuffer.Num());

	for (int i = 0; i < RM_TempVertexBuffer.Num(); i++)
	{
		RM_SoftSkinVertices[i] = FTempVertex_To_FSoftSkinVertex(RM_TempVertexBuffer[i]);
	}

	for (int i = 0; i < RM_IndexBuffer.Num(); i++)
	{
		RM_Indices[i] = RM_IndexBuffer[i];
	}


	BuildOptimizationThirdParty::NvTriStripHelper::BuildSkeletalAdjacencyIndexBuffer(RM_SoftSkinVertices, NumUV, RM_Indices, OutRM_AdjacencyIndexBuffer);
}
#endif //WITH_EDITOR

void ARuntimeDestructibleActor::GetChunkConvexHullVertices(TArray<TArray<FVector>>& DM_ConvexHullsToVertices, TArray<TArray<FPlane>>& DM_ConvexHullsToPlanes, ExplicitHierarchicalMesh& DM_HierarchicalMesh, int32 ChunkIdx)
{
	int32_t* partIndex = DM_HierarchicalMesh.partIndex(ChunkIdx);
	if (partIndex != nullptr)
	{
		uint32_t convexHullCount = DM_HierarchicalMesh.convexHullCount(*partIndex);
		const ExplicitHierarchicalMesh::ConvexHull** ConvexHulls = DM_HierarchicalMesh.convexHulls(*partIndex);

		for (uint32_t Hull_Idx = 0; Hull_Idx < convexHullCount; Hull_Idx++)
		{
			const ExplicitHierarchicalMesh::ConvexHull* ConvexHull = ConvexHulls[Hull_Idx];
			//Get convex hull vertices
			uint32_t VertCount = ConvexHull->getVertexCount();
			TArray<FVector> DM_Vertices;
			for (uint32_t VertIdx = 0; VertIdx < VertCount; VertIdx++)
			{
				const PxVec3& Vertex = ConvexHull->getVertex(VertIdx);
				DM_Vertices.Add(P2UVector(Vertex));
			}
			//Get convex hull plane
			uint32_t PlanesCount = ConvexHull->getPlaneCount();
			TArray<FPlane> DM_Planes;
			for (uint32_t PlaneIdx = 0; PlaneIdx < PlanesCount; PlaneIdx++)
			{
				const PxPlane& Plane = ConvexHull->getPlane(PlaneIdx);
				DM_Planes.Add(P2UPlane(Plane));
			}
			DM_ConvexHullsToVertices.Add(DM_Vertices);
			DM_ConvexHullsToPlanes.Add(DM_Planes);
		}
	}
}

bool ARuntimeDestructibleActor::GetChunkCollision(ExplicitHierarchicalMesh& DM_HierarchicalMesh, TArray<TArray<FVector>>& DM_ConvexHullsToVertices, TArray<TArray<FPlane>>& DM_ConvexHullsToPlanes, int32 ChunkIdx)
{
	uint32_t chunkCount = DM_HierarchicalMesh.chunkCount();
	if (ChunkIdx<0 || (uint32_t)ChunkIdx>chunkCount - 1)
		return false;
	DM_ConvexHullsToVertices.Empty();
	DM_ConvexHullsToPlanes.Empty();
	GetChunkConvexHullVertices(DM_ConvexHullsToVertices, DM_ConvexHullsToPlanes, DM_HierarchicalMesh, ChunkIdx);

	return true;
}

bool ARuntimeDestructibleActor::GetChunksCollision(ExplicitHierarchicalMesh& DM_HierarchicalMesh, TArray<TArray<TArray<FVector>>>& DM_ChunksToConvexHullsToVertices, TArray<TArray<TArray<FPlane>>>& DM_ChunksToConvexHullsToPlaness)
{
	DM_ChunksToConvexHullsToVertices.Empty();
	DM_ChunksToConvexHullsToPlaness.Empty();

	uint32_t chunkCount = DM_HierarchicalMesh.chunkCount();
	for (uint32_t ChunkIdx = 0; ChunkIdx < chunkCount; ChunkIdx++)
	{
		TArray<TArray<FVector>> DM_ConvexHullsToVertices;
		TArray<TArray<FPlane>> DM_ConvexHullsToPlanes;
		GetChunkConvexHullVertices(DM_ConvexHullsToVertices, DM_ConvexHullsToPlanes, DM_HierarchicalMesh, ChunkIdx);
		DM_ChunksToConvexHullsToVertices.Add(DM_ConvexHullsToVertices);
		DM_ChunksToConvexHullsToPlaness.Add(DM_ConvexHullsToPlanes);
	}
	return true;
}


void ARuntimeDestructibleActor::GetChunkBuffers(TArray<TArray<FTempVertex>>& DM_SectionsToVertexBuffers, TArray<TArray<int32>>& DM_SectionsToIndexBuffers,
	TArray<int32>& DM_SectionIndices, ExplicitHierarchicalMesh& DM_HierarchicalMesh, RenderMeshAssetAuthoring* RenderMeshAssetAuthor, int32 ChunkIdx)
{
	int32_t* PartIndex = DM_HierarchicalMesh.partIndex(ChunkIdx);
	if (PartIndex != nullptr)
	{
		const uint32 PartTriangleCount = DM_HierarchicalMesh.meshTriangleCount(*PartIndex);
		ExplicitRenderTriangle* PartTriangles = DM_HierarchicalMesh.meshTriangles(*PartIndex);

		uint32 PartVertexCount = 0;

		//Non-reduced vertex buffers
		TArray<TArray<Vertex>> DM_SectionsToNonReducedVertexBuffers;

		//Copy Part vertices to section arrays
		//Part have as many sections as Part's triangles have different submesh indices
		//Triangles are ordered by submesh indices
		int32 CurrentSubmeshIdx = -1;
		for (uint32 PartTriangleIndex = 0; PartTriangleIndex < PartTriangleCount; ++PartTriangleIndex)
		{
			ExplicitRenderTriangle& Triangle = PartTriangles[PartTriangleIndex];
			//Add new Sections
			if (Triangle.submeshIndex != CurrentSubmeshIdx)
			{
				CurrentSubmeshIdx = Triangle.submeshIndex;
				DM_SectionsToVertexBuffers.AddDefaulted();
				DM_SectionsToNonReducedVertexBuffers.AddDefaulted();
				DM_SectionsToIndexBuffers.AddDefaulted();
				DM_SectionIndices.Add(CurrentSubmeshIdx);
			}
			TArray<Vertex>& DM_SectionNonReducedVertexBuffer = DM_SectionsToNonReducedVertexBuffers[DM_SectionsToNonReducedVertexBuffers.Num() - 1];

			for (int32 i = 0; i < 3; i++)
			{
				DM_SectionNonReducedVertexBuffer.Add(Triangle.vertices[i]);
			}
		}

		//Reduce Part section vertices and create index buffers
		int32 SectionCount = DM_SectionsToNonReducedVertexBuffers.Num();
		for (int32 LocalSectionIdx = 0; LocalSectionIdx < SectionCount; LocalSectionIdx++)
		{
			TArray<Vertex>& DM_SectionNonReducedVertexBuffer = DM_SectionsToNonReducedVertexBuffers[LocalSectionIdx];
			//Final section's vertex buffer
			TArray<FTempVertex>& DM_SectionVertexBuffer = DM_SectionsToVertexBuffers[LocalSectionIdx];
			//Final section's index buffer
			TArray<int32>& DM_SectionIndexBuffer = DM_SectionsToIndexBuffers[LocalSectionIdx];

			if (DM_SectionNonReducedVertexBuffer.Num() > 0)
			{
				TArray<PxU32> ReducedSectionIndices;
				int32 NonReducedVertCount = DM_SectionNonReducedVertexBuffer.Num();
				ReducedSectionIndices.SetNumUninitialized(NonReducedVertCount);

				const PxU32 ReducedSectionVertexCount = RenderMeshAssetAuthor->createReductionMap(ReducedSectionIndices.GetData(), DM_SectionNonReducedVertexBuffer.GetData(), NULL, (PxU32)NonReducedVertCount,
					PxVec3(0.0001f), 0.001f, 1.0f / 256.01f);

				DM_SectionVertexBuffer.Init(FTempVertex(), ReducedSectionVertexCount);

				DM_SectionIndexBuffer.Init(int32(), NonReducedVertCount);

				for (int32 OldIndex = 0; OldIndex < NonReducedVertCount; ++OldIndex)
				{
					const int32 NewIndex = ReducedSectionIndices[OldIndex];
					DM_SectionIndexBuffer[OldIndex] = NewIndex;
					DM_SectionVertexBuffer[NewIndex] = NvVertex_To_FTempVertex(DM_SectionNonReducedVertexBuffer[OldIndex]);	// This will copy over several times, but with the same (or close enough) data					
				}
			}
		}
	}
}


void ARuntimeDestructibleActor::GetChunkRenderData(TArray<TArray<FTempVertex>>& DM_SectionsToVertexBuffers, TArray<TArray<int32>>& DM_SectionsToIndexBuffers,
	TArray<int32>& DM_SectionIndices, ExplicitHierarchicalMesh& DM_HierarchicalMesh, int32 ChunkIdx)
{
	uint32 ChunkCount = DM_HierarchicalMesh.chunkCount();
	if (ChunkIdx<0 || (uint32)ChunkIdx>ChunkCount - 1)
		return;

	apex::RenderMeshAssetAuthoring* RenderMeshAssetAuthor = static_cast<apex::RenderMeshAssetAuthoring*>(apex::GetApexSDK()->createAssetAuthoring(RENDER_MESH_AUTHORING_TYPE_NAME));

	GetChunkBuffers(DM_SectionsToVertexBuffers, DM_SectionsToIndexBuffers, DM_SectionIndices, DM_HierarchicalMesh, RenderMeshAssetAuthor, ChunkIdx);
}

void ARuntimeDestructibleActor::GetChunksRenderData(TArray<TArray<TArray<FTempVertex>>>& DM_PartsToSectionsToVertexBuffers, TArray<TArray<TArray<int32>>>& DM_PartsToSectionsToIndexBuffers,
	TArray<TArray<int32>>& DM_PartsToSectionIndices, ExplicitHierarchicalMesh& DM_HierarchicalMesh)
{
	apex::RenderMeshAssetAuthoring* RenderMeshAssetAuthor = static_cast<apex::RenderMeshAssetAuthoring*>(apex::GetApexSDK()->createAssetAuthoring(RENDER_MESH_AUTHORING_TYPE_NAME));

	uint32 ChunkCount = DM_HierarchicalMesh.chunkCount();

	DM_PartsToSectionsToVertexBuffers.Init(TArray<TArray<FTempVertex>>(), ChunkCount);
	DM_PartsToSectionsToIndexBuffers.Init(TArray<TArray<int32>>(), ChunkCount);
	DM_PartsToSectionIndices.Init(TArray<int32>(), ChunkCount);

	for (uint32 ChunkIndex = 0; ChunkIndex < ChunkCount; ++ChunkIndex)
	{
		TArray<TArray<FTempVertex>>& DM_SectionsToVertexBuffers = DM_PartsToSectionsToVertexBuffers[ChunkIndex];
		TArray<TArray<int32>>& DM_SectionsToIndexBuffers = DM_PartsToSectionsToIndexBuffers[ChunkIndex];
		TArray<int32>& DM_SectionIndices = DM_PartsToSectionIndices[ChunkIndex];
		GetChunkBuffers(DM_SectionsToVertexBuffers, DM_SectionsToIndexBuffers, DM_SectionIndices, DM_HierarchicalMesh, RenderMeshAssetAuthor, ChunkIndex);
	}
}








void ARuntimeDestructibleActor::GetChunkConvexHullVertices(FTempChunk& DM_Chunk, ExplicitHierarchicalMesh& DM_HierarchicalMesh, int32 ChunkIdx)
{
	int32_t* partIndex = DM_HierarchicalMesh.partIndex(ChunkIdx);
	if (partIndex != nullptr)
	{
		uint32_t convexHullCount = DM_HierarchicalMesh.convexHullCount(*partIndex);
		const ExplicitHierarchicalMesh::ConvexHull** ConvexHulls = DM_HierarchicalMesh.convexHulls(*partIndex);

		DM_Chunk.ConvexHulls.AddDefaulted(convexHullCount);

		for (uint32_t Hull_Idx = 0; Hull_Idx < convexHullCount; Hull_Idx++)
		{
			const ExplicitHierarchicalMesh::ConvexHull* PxConvexHull = ConvexHulls[Hull_Idx];
			FTempConvexHull& ConvexHull = DM_Chunk.ConvexHulls[Hull_Idx];

			//Get convex hull vertices
			uint32_t VertCount = PxConvexHull->getVertexCount();
			TArray<FVector>& DM_Vertices = ConvexHull.Vertices;
			for (uint32_t VertIdx = 0; VertIdx < VertCount; VertIdx++)
			{
				const PxVec3& Vertex = PxConvexHull->getVertex(VertIdx);
				DM_Vertices.Add(P2UVector(Vertex));
			}
			//Get convex hull planes
			uint32_t PlanesCount = PxConvexHull->getPlaneCount();
			TArray<FPlane>& DM_Planes = ConvexHull.Planes;;
			for (uint32_t PlaneIdx = 0; PlaneIdx < PlanesCount; PlaneIdx++)
			{
				const PxPlane& Plane = PxConvexHull->getPlane(PlaneIdx);
				DM_Planes.Add(P2UPlane(Plane));
			}
		}
	}
}

bool ARuntimeDestructibleActor::GetChunkCollision(ExplicitHierarchicalMesh& DM_HierarchicalMesh, FTempChunk& DM_Chunk, int32 ChunkIdx)
{
	uint32_t chunkCount = DM_HierarchicalMesh.chunkCount();
	if (ChunkIdx<0 || (uint32_t)ChunkIdx>chunkCount - 1)
		return false;
	for (int32 ConvexIdx = 0; ConvexIdx < DM_Chunk.ConvexHulls.Num(); ConvexIdx++)
	{
		DM_Chunk.ConvexHulls[ConvexIdx].Vertices.Empty();
		DM_Chunk.ConvexHulls[ConvexIdx].Planes.Empty();
	}
	GetChunkConvexHullVertices(DM_Chunk, DM_HierarchicalMesh, ChunkIdx);

	return true;
}
/** Cet Chunks Collision**/
bool ARuntimeDestructibleActor::GetChunksCollision(ExplicitHierarchicalMesh& DM_HierarchicalMesh, TArray<FTempChunk>& TempChunks)
{
	//Clear chunks convex hulls data
	for (int32 ChunkIdx = 0; ChunkIdx < TempChunks.Num(); ChunkIdx++)
	{
		for (int32 ConvexIdx = 0; ConvexIdx < TempChunks[ChunkIdx].ConvexHulls.Num(); ConvexIdx++)
		{
			TempChunks[ChunkIdx].ConvexHulls[ConvexIdx].Vertices.Empty();
			TempChunks[ChunkIdx].ConvexHulls[ConvexIdx].Planes.Empty();
		}
	}

	uint32_t chunkCount = DM_HierarchicalMesh.chunkCount();
	for (uint32_t ChunkIdx = 0; ChunkIdx < chunkCount; ChunkIdx++)
	{
		GetChunkConvexHullVertices(TempChunks[ChunkIdx], DM_HierarchicalMesh, ChunkIdx);
	}
	return true;
}

void ARuntimeDestructibleActor::GetChunkBuffers(FTempChunk& DM_Chunk, ExplicitHierarchicalMesh& DM_HierarchicalMesh, RenderMeshAssetAuthoring* RenderMeshAssetAuthor, int32 ChunkIdx)
{
	int32_t* PartIndex = DM_HierarchicalMesh.partIndex(ChunkIdx);
	if (PartIndex != nullptr)
	{
		const uint32 PartTriangleCount = DM_HierarchicalMesh.meshTriangleCount(*PartIndex);
		ExplicitRenderTriangle* PartTriangles = DM_HierarchicalMesh.meshTriangles(*PartIndex);

		uint32 PartVertexCount = 0;

		//Non-reduced vertex buffers
		TArray<TArray<Vertex>> DM_SectionsToNonReducedVertexBuffers;

		//Copy Part vertices to section arrays
		//Part have as many sections as Part's triangles have different submesh indices
		//Triangles are ordered by submesh indices
		int32 CurrentSubmeshIdx = -1;
		int32 SectionIdx = -1;
		for (uint32 PartTriangleIndex = 0; PartTriangleIndex < PartTriangleCount; ++PartTriangleIndex)
		{
			ExplicitRenderTriangle& Triangle = PartTriangles[PartTriangleIndex];
			//Add new Sections
			if (Triangle.submeshIndex != CurrentSubmeshIdx)
			{
				CurrentSubmeshIdx = Triangle.submeshIndex;
				SectionIdx = DM_Chunk.Sections.AddDefaulted();
				DM_SectionsToNonReducedVertexBuffers.AddDefaulted();
				DM_Chunk.Sections[SectionIdx].SectionIndex = CurrentSubmeshIdx;
			}
			TArray<Vertex>& DM_SectionNonReducedVertexBuffer = DM_SectionsToNonReducedVertexBuffers[SectionIdx];

			for (int32 i = 0; i < 3; i++)
			{
				DM_SectionNonReducedVertexBuffer.Add(Triangle.vertices[i]);
			}
		}

		//Reduce Part section vertices and create index buffers
		int32 SectionCount = DM_SectionsToNonReducedVertexBuffers.Num();
		for (int32 LocalSectionIdx = 0; LocalSectionIdx < SectionCount; LocalSectionIdx++)
		{
			TArray<Vertex>& DM_SectionNonReducedVertexBuffer = DM_SectionsToNonReducedVertexBuffers[LocalSectionIdx];
			//Final section's vertex buffer
			TArray<FTempVertex>& DM_SectionVertexBuffer = DM_Chunk.Sections[LocalSectionIdx].Vertices;
			//Final section's index buffer
			TArray<int32>& DM_SectionIndexBuffer = DM_Chunk.Sections[LocalSectionIdx].Indices;

			if (DM_SectionNonReducedVertexBuffer.Num() > 0)
			{
				TArray<PxU32> ReducedSectionIndices;
				int32 NonReducedVertCount = DM_SectionNonReducedVertexBuffer.Num();
				ReducedSectionIndices.SetNumUninitialized(NonReducedVertCount);

				const PxU32 ReducedSectionVertexCount = RenderMeshAssetAuthor->createReductionMap(ReducedSectionIndices.GetData(), DM_SectionNonReducedVertexBuffer.GetData(), NULL, (PxU32)NonReducedVertCount,
					PxVec3(0.0001f), 0.001f, 1.0f / 256.01f);

				DM_SectionVertexBuffer.Init(FTempVertex(), ReducedSectionVertexCount);

				DM_SectionIndexBuffer.Init(int32(), NonReducedVertCount);

				for (int32 OldIndex = 0; OldIndex < NonReducedVertCount; ++OldIndex)
				{
					const int32 NewIndex = ReducedSectionIndices[OldIndex];
					DM_SectionIndexBuffer[OldIndex] = NewIndex;
					DM_SectionVertexBuffer[NewIndex] = NvVertex_To_FTempVertex(DM_SectionNonReducedVertexBuffer[OldIndex]);	// This will copy over several times, but with the same (or close enough) data					
				}
			}
		}
	}
}

void ARuntimeDestructibleActor::GetChunksRenderData(TArray<FTempChunk>& TempChunks, ExplicitHierarchicalMesh& DM_HierarchicalMesh)
{
	TempChunks.Empty();

	apex::RenderMeshAssetAuthoring* RenderMeshAssetAuthor = static_cast<apex::RenderMeshAssetAuthoring*>(apex::GetApexSDK()->createAssetAuthoring(RENDER_MESH_AUTHORING_TYPE_NAME));

	uint32 ChunkCount = DM_HierarchicalMesh.chunkCount();

	TempChunks.Init(FTempChunk(), ChunkCount);
	for (uint32 ChunkIndex = 0; ChunkIndex < ChunkCount; ++ChunkIndex)
	{
		TempChunks[ChunkIndex].ChunkIndex = ChunkIndex;
		GetChunkBuffers(TempChunks[ChunkIndex], DM_HierarchicalMesh, RenderMeshAssetAuthor, ChunkIndex);
	}
}

void ARuntimeDestructibleActor::GetChunkRenderData(FTempChunk& DM_Chunk, ExplicitHierarchicalMesh& DM_HierarchicalMesh, int32 ChunkIdx)
{
	uint32 ChunkCount = DM_HierarchicalMesh.chunkCount();
	if (ChunkIdx<0 || (uint32)ChunkIdx>ChunkCount - 1)
		return;

	apex::RenderMeshAssetAuthoring* RenderMeshAssetAuthor = static_cast<apex::RenderMeshAssetAuthoring*>(apex::GetApexSDK()->createAssetAuthoring(RENDER_MESH_AUTHORING_TYPE_NAME));

	GetChunkBuffers(DM_Chunk, DM_HierarchicalMesh, RenderMeshAssetAuthor, ChunkIdx);
}

/**Creating RuntimeMesh Sections, tesselation triangles (if need) and setting materials **/
void ARuntimeDestructibleActor::CreateRuntimeMeshRenderData(URuntimeMeshComponent* RuntimeMeshComponent, FTempChunk& TempChunk, const TArray<UMaterialInterface*>& DM_OverrideMaterials, const TArray<FSkeletalMaterial>& DM_Materials, const uint32 NumUV, bool bOverrideMaterials, bool bCreateAdjBuffer)
{
	URuntimeMesh* RuntimeMesh = RuntimeMeshComponent->GetOrCreateRuntimeMesh();
	RuntimeMesh->ClearAllMeshSections();

	int32 SectionNum = TempChunk.Sections.Num();
	for (int32 LocalSectionIdx = 0; LocalSectionIdx < SectionNum; LocalSectionIdx++)
	{
		const TArray<FTempVertex>& DM_SectionVertexBuffer = TempChunk.Sections[LocalSectionIdx].Vertices;

		const TArray<int32>& RM_IndexBuffer = TempChunk.Sections[LocalSectionIdx].Indices;

		TArray<FVector> RM_Vertices;
		TArray<FVector> RM_Normals;
		TArray<FVector2D> RM_UV0;
		TArray<FColor> RM_Colors;
		TArray<FRuntimeMeshTangent> RM_Tangents;
		TArray<FVector> RM_BiNormals;

		for (int i = 0; i < DM_SectionVertexBuffer.Num(); i++)
		{
			const FTempVertex& Vert = DM_SectionVertexBuffer[i];

			RM_Vertices.Add(Vert.Position);
			RM_Normals.Add(Vert.Normal);
			RM_UV0.Add(Vert.UVs[0]);
			RM_Colors.Add(Vert.Color);
			RM_Tangents.Add(Vert.Tangent);
			RM_BiNormals.Add(Vert.Binormal);
		}

		RuntimeMesh->CreateMeshSection(LocalSectionIdx, RM_Vertices, RM_IndexBuffer, RM_Normals, RM_UV0, RM_Colors, RM_Tangents);

		//Set Materials
		if (bOverrideMaterials)
			RuntimeMeshComponent->SetMaterial(LocalSectionIdx, DM_OverrideMaterials[TempChunk.Sections[LocalSectionIdx].SectionIndex]);
		else
			RuntimeMeshComponent->SetMaterial(LocalSectionIdx, DM_Materials[TempChunk.Sections[LocalSectionIdx].SectionIndex].MaterialInterface);

		if (bCreateAdjBuffer)
		{
			/*TArray<uint32> RM_AdjacencyIndexBuffer;
#if	WITH_EDITOR
			BuildAdjacencyBuffer(DM_SectionVertexBuffer, NumUV, RM_IndexBuffer, RM_AdjacencyIndexBuffer);
#endif		
			RuntimeMesh->SetSectionTessellationTriangles(LocalSectionIdx, RM_AdjacencyIndexBuffer);*/

			FRuntimeMeshSectionPtr Section = RuntimeMesh->GetRuntimeMeshData()->GetMeshSection(LocalSectionIdx);
			
			RuntimeMesh->SetSectionTessellationTriangles(LocalSectionIdx, URuntimeMeshLibrary::GenerateTessellationIndexBuffer(Section->GetSectionMeshAccessor()));
		}
	}
}

void ARuntimeDestructibleActor::CreateRuntimeMeshCollision(URuntimeMeshComponent* RuntimeMeshComponent, FTempChunk& TempChunk)
{
	URuntimeMesh* RuntimeMesh = RuntimeMeshComponent->GetOrCreateRuntimeMesh();
	RuntimeMesh->ClearAllConvexCollisionSections();
	for (int32 i = 0; i < TempChunk.ConvexHulls.Num(); i++)
	{
		FRuntimeMeshCollisionConvexMesh ConvexMesh;
		FRuntimeMeshData::CreateConvexMeshFromBuffers(ConvexMesh, TempChunk.ConvexHulls[i].Vertices, TempChunk.ConvexHulls[i].Planes);
		RuntimeMesh->AddConvexCollisionSectionAtLast(ConvexMesh);
	}
}









FVector ARuntimeDestructibleActor::PxVec3_To_FVector(const PxVec3& InVertex)
{
	//In URMC_DestructibleMesh::CreateSubmeshFromSMSection function vertices positions, normals, tangents and binormals Ys are multiplied by -1
	return FVector(InVertex.x, InVertex.y * -1, InVertex.z);
}

FColor ARuntimeDestructibleActor::VertexColor_To_FColor(const VertexColor& InColor)
{
	return FLinearColor(InColor.r, InColor.g, InColor.b, InColor.a).ToFColor(true);
}

FVector2D ARuntimeDestructibleActor::VertexUV_To_FVector2D(const VertexUV& InVertexUV)
{
	//In URMC_DestructibleMesh::CreateSubmeshFromSMSection function v = -Y + 1.0
	//Y = -v + 1.0
	return FVector2D(InVertexUV.u, -InVertexUV.v + 1.0);
}

#if	WITH_EDITOR
FSoftSkinVertex ARuntimeDestructibleActor::NvVertex_To_FSoftSkinVertex(const Vertex& InVertex)
{
	FSoftSkinVertex SoftSkinVertex;
	SoftSkinVertex.Position = PxVec3_To_FVector(InVertex.position);
	SoftSkinVertex.TangentX = PxVec3_To_FVector(InVertex.tangent);
	SoftSkinVertex.TangentY = PxVec3_To_FVector(InVertex.binormal);
	SoftSkinVertex.TangentZ = PxVec3_To_FVector(InVertex.normal);
	SoftSkinVertex.Color = VertexColor_To_FColor(InVertex.color);
	for (int32 UVIdx = 0; UVIdx < MAX_TEXCOORDS; UVIdx++)
	{
		SoftSkinVertex.UVs[UVIdx] = VertexUV_To_FVector2D(InVertex.uv[UVIdx]);
	}

	return SoftSkinVertex;
}

FSoftSkinVertex ARuntimeDestructibleActor::FTempVertex_To_FSoftSkinVertex(const FTempVertex& InVertex)
{
	FSoftSkinVertex SoftSkinVertex;
	SoftSkinVertex.Position = InVertex.Position;
	SoftSkinVertex.TangentX = InVertex.Tangent.TangentX;
	SoftSkinVertex.TangentY = InVertex.Binormal;
	SoftSkinVertex.TangentZ = InVertex.Normal;
	SoftSkinVertex.Color = InVertex.Color;
	for (int32 UVIdx = 0; UVIdx < MAX_TEXCOORDS; UVIdx++)
	{
		SoftSkinVertex.UVs[UVIdx] = InVertex.UVs[UVIdx];
	}

	return SoftSkinVertex;
}
#endif //WITH_EDITOR

FTempVertex ARuntimeDestructibleActor::NvVertex_To_FTempVertex(const nvidia::apex::Vertex& InVertex)
{
	FTempVertex TempVertex;
	TempVertex.Position = PxVec3_To_FVector(InVertex.position);
	TempVertex.Tangent = PxVec3_To_FVector(InVertex.tangent);
	TempVertex.Binormal = PxVec3_To_FVector(InVertex.binormal);
	TempVertex.Normal = PxVec3_To_FVector(InVertex.normal);
	TempVertex.Color = VertexColor_To_FColor(InVertex.color);
	for (int32 UVIdx = 0; UVIdx < MAX_TEXCOORDS; UVIdx++)
	{
		TempVertex.UVs[UVIdx] = VertexUV_To_FVector2D(InVertex.uv[UVIdx]);
	}
	return TempVertex;
}

/**Creating RuntimeMesh Sections, tesselation triangles (if need) and setting materials **/
void ARuntimeDestructibleActor::CreateRuntimeMeshRenderData(URuntimeMeshComponent* RuntimeMeshComponent, const TArray<TArray<FTempVertex>>& DM_PartSectionsToVertexBuffers, const TArray<TArray<int32>>& DM_PartSectionsToIndexBuffers,
	const TArray<int32>& DM_PartSectionIndices, const TArray<UMaterialInterface*>& DM_OverrideMaterials, const TArray<FSkeletalMaterial>& DM_Materials, const uint32 NumUV, bool bOverrideMaterials, bool bCreateAdjBuffer)
{
	URuntimeMesh* RuntimeMesh = RuntimeMeshComponent->GetOrCreateRuntimeMesh();
	RuntimeMesh->ClearAllMeshSections();

	int32 SectionNum = DM_PartSectionsToVertexBuffers.Num();
	for (int32 LocalSectionIdx = 0; LocalSectionIdx < SectionNum; LocalSectionIdx++)
	{
		const TArray<FTempVertex>& DM_SectionVertexBuffer = DM_PartSectionsToVertexBuffers[LocalSectionIdx];

		const TArray<int32>& RM_IndexBuffer = DM_PartSectionsToIndexBuffers[LocalSectionIdx];

		TArray<FVector> RM_Vertices;
		TArray<FVector> RM_Normals;
		TArray<FVector2D> RM_UV0;
		TArray<FColor> RM_Colors;
		TArray<FRuntimeMeshTangent> RM_Tangents;
		TArray<FVector> RM_BiNormals;

		for (int i = 0; i < DM_SectionVertexBuffer.Num(); i++)
		{
			const FTempVertex& Vert = DM_SectionVertexBuffer[i];

			RM_Vertices.Add(Vert.Position);
			RM_Normals.Add(Vert.Normal);
			RM_UV0.Add(Vert.UVs[0]);
			RM_Colors.Add(Vert.Color);
			RM_Tangents.Add(Vert.Tangent);
			RM_BiNormals.Add(Vert.Binormal);
		}

		RuntimeMesh->CreateMeshSection(LocalSectionIdx, RM_Vertices, RM_IndexBuffer, RM_Normals, RM_UV0, RM_Colors, RM_Tangents);

		//Set Materials
		if (bOverrideMaterials)
			RuntimeMeshComponent->SetMaterial(LocalSectionIdx, DM_OverrideMaterials[DM_PartSectionIndices[LocalSectionIdx]]);
		else
			RuntimeMeshComponent->SetMaterial(LocalSectionIdx, DM_Materials[DM_PartSectionIndices[LocalSectionIdx]].MaterialInterface);

		if (bCreateAdjBuffer)
		{
			/*TArray<uint32> RM_AdjacencyIndexBuffer;
#if	WITH_EDITOR
			BuildAdjacencyBuffer(DM_SectionVertexBuffer, NumUV, RM_IndexBuffer, RM_AdjacencyIndexBuffer);
#endif
			RuntimeMesh->SetSectionTessellationTriangles(LocalSectionIdx, RM_AdjacencyIndexBuffer);*/
			FRuntimeMeshSectionPtr Section = RuntimeMesh->GetRuntimeMeshData()->GetMeshSection(LocalSectionIdx);

			RuntimeMesh->SetSectionTessellationTriangles(LocalSectionIdx, URuntimeMeshLibrary::GenerateTessellationIndexBuffer(Section->GetSectionMeshAccessor()));
		}
	}
}

void ARuntimeDestructibleActor::CreateRuntimeMeshCollision(URuntimeMeshComponent* RuntimeMeshComponent, TArray<TArray<FVector>>& DM_ConvexHullsToVertices, const TArray<TArray<FPlane>>& DM_ConvexHullsToPlanes)
{
	URuntimeMesh* RuntimeMesh = RuntimeMeshComponent->GetOrCreateRuntimeMesh();
	RuntimeMesh->ClearAllConvexCollisionSections();
	for (int32 i = 0; i < DM_ConvexHullsToVertices.Num(); i++)
	{
		FRuntimeMeshCollisionConvexMesh ConvexMesh;
		FRuntimeMeshData::CreateConvexMeshFromBuffers(ConvexMesh, DM_ConvexHullsToVertices[i], DM_ConvexHullsToPlanes[i]);
		RuntimeMesh->AddConvexCollisionSectionAtLast(ConvexMesh);
	}
}






/**
Create hierarchy of fracture chunks and isles
**/
void ARuntimeDestructibleActor::BuildDestructibleActorChunks(ARuntimeDestructibleActor* RuntimeDestructibleActor)
{		
	TArray <UFractureChunkComponent*>& FractureChunks = RuntimeDestructibleActor->FractureChunks;
	TArray <UIsleComponent*>& Isles = RuntimeDestructibleActor->Isles;
	
	TArray<FTempChunk>& TempChunks = RuntimeDestructibleActor->TempChunks;

	wstring ChunkStr = L"Chunk_";
	wstring ChunkIdx;
	wstring ChunkName;

	wstring IsleStr = L"Isle_";
	wstring IsleIdx;
	wstring IsleName;

	//Attach Chunks either to patent chunk or to parent isle
	for (int32 ChunkIndex = 0; ChunkIndex < TempChunks.Num(); ChunkIndex++)
	{
		FTempChunk& TempChunk = TempChunks[ChunkIndex];
		
		ChunkIdx = to_wstring(TempChunk.ChunkIndex);
		ChunkName = ChunkStr + ChunkIdx;

		//UFractureChunkComponent* ParentChunk = FractureChunks[TempChunk.ParentIndex];		


		//if chunk has overlaps - it must be attached to isle
		if (TempChunk.Overlaps.Num() > 0)
		{
			//if chunk not attached to isle yet - it is first chunk of overlapping group
			//So create new Isle and search other chunks from this group
			//Otherwise - chunk already attached to isle
			if (TempChunk.IsleIndex == INDEX_NONE)
			{
				IsleIdx = to_wstring(Isles.Num());
				IsleName = IsleStr + IsleIdx;				

				AIsleActor* IsleActor = SpawnIsleActor(RuntimeDestructibleActor);

				UIsleComponent* Isle = IsleActor->IsleComponent;

				Isle->ParentIndex = TempChunk.ParentIndex;
				TempChunk.IsleIndex = Isles.Num() - 1;				

				AFractureChunkActor* FractureChunkActor = SpawnFractureChunkActor(RuntimeDestructibleActor, TempChunk);

				//Parent is either TempChunk.ParentIndex (if ParentIndex != -1) or RootComponent (if TempChunk has no parent (ParentIndex == -1))
				UFractureChunkComponent* Chunk = FractureChunkActor->FractureChunkComponent;

				Chunk->AttachToComponent(Isle, FAttachmentTransformRules(EAttachmentRule(EAttachmentRule::KeepWorld), true));

				//if root isle
				if (TempChunk.ParentIndex == INDEX_NONE)
				{
					if(RuntimeDestructibleActor->SimulatePhysicsAtDepth[0])
						Isle->SetSimulatePhysics(true);
					Isle->SetCollisionProfileName(TEXT("OverlapAllDynamic"));
					Chunk->SetCollisionProfileName(TEXT("Destructible"));
				}
				//if not root isle - attach to parent chunk
				else
				{
					UFractureChunkComponent* ParentChunk = FractureChunks[TempChunk.ParentIndex];
					Isle->AttachToComponent(ParentChunk, FAttachmentTransformRules(EAttachmentRule(EAttachmentRule::KeepWorld), true));
				}				
				
				Isle->ChildrenIndices.Add(Chunk->ChunkIndex);

				//Search Overlapped Chunks
				SearchOverlapsToAttach(RuntimeDestructibleActor, Isle, TempChunk);				
			}
		}
		//if chunk has no overlaps - attach it to parent chunk
		else
		{	
			AFractureChunkActor* FractureChunkActor = SpawnFractureChunkActor(RuntimeDestructibleActor, TempChunk);

			//Parent is either TempChunk.ParentIndex (if ParentIndex != -1) or RootComponent (if TempChunk has no parent (ParentIndex == -1))
			UFractureChunkComponent* Chunk = FractureChunkActor->FractureChunkComponent;

			//if root chunk
			if (TempChunk.ParentIndex == INDEX_NONE)
			{
				if (RuntimeDestructibleActor->SimulatePhysicsAtDepth[0])
					Chunk->SetSimulatePhysics(true);
				Chunk->SetCollisionProfileName(TEXT("Destructible"));
				Chunk->SetVisibility(true);
				Chunk->SetCastShadow(true);
			}
			//if not root chunk - attach to parent chunk
			else
			{
				UFractureChunkComponent* ParentChunk = FractureChunks[Chunk->ParentIndex];
				Chunk->AttachToComponent(ParentChunk, FAttachmentTransformRules(EAttachmentRule(EAttachmentRule::KeepWorld), true));
			}			
		}
	}
}

AIsleActor* ARuntimeDestructibleActor::SpawnIsleActor(ARuntimeDestructibleActor* RuntimeDestructibleActor)
{
	TArray <UIsleComponent*>& Isles = RuntimeDestructibleActor->Isles;
	TArray <AIsleActor*>& IsleActors = RuntimeDestructibleActor->IsleActors;

	UWorld* World = RuntimeDestructibleActor->GetWorld();

	AIsleActor* IsleActor = World->SpawnActor<AIsleActor>(AIsleActor::StaticClass(), RuntimeDestructibleActor->GetTransform());

	UIsleComponent* Isle = IsleActor->IsleComponent;

	Isle->SetCollisionUseComplexAsSimple(false);

	Isle->SetSimulatePhysics(false);	
	Isle->SetCollisionProfileName(TEXT("NoCollision"));

	Isle->SetVisibility(false);
	Isle->SetCastShadow(false);

	Isle->GetBodyInstance()->bAutoWeld = true;

	Isle->GetBodyInstance()->SetMassOverride(0, true);

	InitFakeIsleData(Isle);

	IsleActors.Add(IsleActor);
	Isles.Add(Isle);

	return IsleActor;
}

AFractureChunkActor* ARuntimeDestructibleActor::SpawnFractureChunkActor(ARuntimeDestructibleActor* RuntimeDestructibleActor, FTempChunk& TempChunk)
{
	TArray <UFractureChunkComponent*>& FractureChunks = RuntimeDestructibleActor->FractureChunks;	
	TArray <AFractureChunkActor*>& FractureChunkActors = RuntimeDestructibleActor->FractureChunkActors;
	
	UWorld* World = RuntimeDestructibleActor->GetWorld();

	AFractureChunkActor* FractureChunkActor = World->SpawnActor<AFractureChunkActor>(AFractureChunkActor::StaticClass(), RuntimeDestructibleActor->GetTransform());

	UFractureChunkComponent* FractureChunk = FractureChunkActor->FractureChunkComponent;

	FractureChunk->SetCollisionUseComplexAsSimple(false);

	FractureChunk->SetSimulatePhysics(false);
	FractureChunk->SetCollisionProfileName(TEXT("NoCollision"));

	FractureChunk->SetVisibility(false);
	FractureChunk->SetCastShadow(false);

	FractureChunk->GetBodyInstance()->bAutoWeld = true;

	CreateFractureChunkFromTempChunk(RuntimeDestructibleActor, TempChunk, FractureChunk);

	//FractureChunk->AttachToComponent(Isle, FAttachmentTransformRules(EAttachmentRule(EAttachmentRule::KeepWorld), true));
	//CreatePhysicsConstraint(IsleActor, FractureChunkActor);

	FractureChunkActors.Add(FractureChunkActor);
	FractureChunks.Add(FractureChunk);

	return FractureChunkActor;
}

void ARuntimeDestructibleActor::SearchOverlapsToAttach(ARuntimeDestructibleActor* RuntimeDestructibleActor, UIsleComponent* Isle, FTempChunk& InTempChunk)
{
	TArray<FTempChunk>& TempChunks = RuntimeDestructibleActor->TempChunks;

	for (int32& OverlapIdx : InTempChunk.Overlaps)
	{
		FTempChunk& TempChunk = TempChunks[OverlapIdx];
		//If Chunk already attached to Isle - continue
		if (TempChunk.IsleIndex != INDEX_NONE)
			continue;

		TempChunk.IsleIndex = InTempChunk.IsleIndex;

		wstring ChunkStr = L"Chunk_";
		wstring ChunkIdx;
		wstring ChunkName;

		ChunkIdx = to_wstring(TempChunk.ChunkIndex);
		ChunkName = ChunkStr + ChunkIdx;

		AFractureChunkActor* FractureChunkActor = SpawnFractureChunkActor(RuntimeDestructibleActor, TempChunk);

		//Parent is either TempChunk.ParentIndex (if ParentIndex != -1) or RootComponent (if TempChunk has no parent (ParentIndex == -1))
		UFractureChunkComponent* Chunk = FractureChunkActor->FractureChunkComponent;

		Chunk->AttachToComponent(Isle, FAttachmentTransformRules(EAttachmentRule(EAttachmentRule::KeepWorld), true));

		//if root isle
		if(Isle->ParentIndex == INDEX_NONE)
		{			
			Chunk->SetCollisionProfileName(TEXT("Destructible"));
		}				

		Isle->ChildrenIndices.Add(Chunk->ChunkIndex);		

		//Search Overlapped Chunks
		SearchOverlapsToAttach(RuntimeDestructibleActor, Isle, TempChunk);
	}
}

void ARuntimeDestructibleActor::CreateFractureChunkFromTempChunk(ARuntimeDestructibleActor* RuntimeDestructibleActor, FTempChunk& TempChunk, UFractureChunkComponent* FractureChunk)
{
	const TArray<UMaterialInterface*>& OverrideMaterials = RuntimeDestructibleActor->OverrideMaterials;
	const TArray<FSkeletalMaterial>& OriginMaterials = RuntimeDestructibleActor->OriginMaterials;
	const uint32 NumUV = (uint32)RuntimeDestructibleActor->NumUV;
	bool bCopyCollision = RuntimeDestructibleActor->bCopyCollision;
	bool bOverrideMaterials = RuntimeDestructibleActor->bOverrideMaterials;
	bool bCreateAdjBuffer = RuntimeDestructibleActor->bCreateAdjBuffer;
	
	CreateRuntimeMeshRenderData(FractureChunk, TempChunk, OverrideMaterials, OriginMaterials, NumUV, bOverrideMaterials, bCreateAdjBuffer);

	if (bCopyCollision)
		CreateRuntimeMeshCollision(FractureChunk, TempChunk);

	FractureChunk->ChunkIndex = TempChunk.ChunkIndex;
	for (int32 i : TempChunk.Overlaps)
	{
		FractureChunk->OverlapIndices.Add(i);
	}
	//FractureChunk->OverlapIndices = TempChunk.Overlaps;
	for (int32 i : TempChunk.Children)
	{
		FractureChunk->ChildrenIndices.Add(i);
	}
	//FractureChunk->ChildrenIndices = TempChunk.Children;
	FractureChunk->ParentIndex = TempChunk.ParentIndex;
	FractureChunk->DepthLevel = TempChunk.Depth;
	FractureChunk->IsleIndex = TempChunk.IsleIndex;

}

UIsleComponent* ARuntimeDestructibleActor::InitFakeIsleComponent(USceneComponent* InParent, const wchar_t* IsleName)
{
	UIsleComponent* Isle = NewObject<UIsleComponent>(InParent, IsleName);
	Isle->SetCollisionUseComplexAsSimple(false);

	InitFakeIsleData(Isle);

	Isle->RegisterComponent();

	//Isle->SetupAttachment(InParent);
	Isle->AttachToComponent(InParent, FAttachmentTransformRules(EAttachmentRule(EAttachmentRule::KeepWorld), true));

	Isle->SetWorldLocation(InParent->GetComponentLocation());
	Isle->SetWorldRotation(InParent->GetComponentRotation());

	Isle->SetSimulatePhysics(false);
	Isle->SetCollisionProfileName(TEXT("NoCollision"));

	Isle->SetVisibility(false);
	Isle->SetCastShadow(false);

	return Isle;
}

UFractureChunkComponent* ARuntimeDestructibleActor::InitFractureChunkComponent(ARuntimeDestructibleActor* RuntimeDestructibleActor, USceneComponent* InParent, const wchar_t* ChunkName, FTempChunk& TempChunk,
	const TArray<UMaterialInterface*>& DM_OverrideMaterials, const TArray<FSkeletalMaterial>& DM_Materials, const uint32 NumUV, bool bCopyCollision, bool bOverrideMaterials, bool bCreateAdjBuffer)
{
	UFractureChunkComponent* FractureChunk = NewObject<UFractureChunkComponent>(InParent, ChunkName);
	FractureChunk->SetCollisionUseComplexAsSimple(false);

	//////////////////////////////////////////////////////////////////////////////////

	//TArray<AFractureChunkActor*> FractureChunkActors = RuntimeDestructibleActor->FractureChunkActors;

	//UWorld* World = RuntimeDestructibleActor->GetWorld();	

	//AFractureChunkActor* FractureChunkActor = World->SpawnActor<AFractureChunkActor>(AFractureChunkActor::StaticClass(), RuntimeDestructibleActor->GetTransform());

	//UFractureChunkComponent* FractureChunk = FractureChunkActor->FractureChunkComponent;
	//	
	//FractureChunk->SetCollisionUseComplexAsSimple(false);

	//FractureChunk->SetSimulatePhysics(false);
	//FractureChunk->SetCollisionProfileName(TEXT("NoCollision"));

	//FractureChunk->SetVisibility(false);
	//FractureChunk->SetCastShadow(false);

	//FractureChunk->GetBodyInstance()->bAutoWeld = true;		

	////FractureChunk->AttachToComponent(InParent, FAttachmentTransformRules(EAttachmentRule(EAttachmentRule::KeepWorld), true));

	//FractureChunkActors.Add(FractureChunkActor);
	//////////////////////////////////////////////////////////////////////////////////

	//CreateFractureChunkFromTempChunk(TempChunk, FractureChunk, DM_OverrideMaterials, DM_Materials, NumUV, bCopyCollision, bOverrideMaterials, bCreateAdjBuffer);

	FractureChunk->RegisterComponent();

	//FractureChunk->SetupAttachment(InParent);
	FractureChunk->AttachToComponent(InParent, FAttachmentTransformRules(EAttachmentRule(EAttachmentRule::KeepWorld), true));

	FractureChunk->SetWorldLocation(InParent->GetComponentLocation());
	FractureChunk->SetWorldRotation(InParent->GetComponentRotation());

	FractureChunk->SetSimulatePhysics(false);
	FractureChunk->SetCollisionProfileName(TEXT("NoCollision"));

	FractureChunk->SetVisibility(false);
	FractureChunk->SetCastShadow(false);

	return FractureChunk;
}

void ARuntimeDestructibleActor::ClearFractureChunkActors(TArray <AFractureChunkActor*>& FractureActors)
{
	for (int32 i = 0; i < FractureActors.Num(); i++)
	{
		FractureActors[i]->Destroy();
	}
	FractureActors.Empty();
}

void ARuntimeDestructibleActor::ClearIsleActors(TArray <AIsleActor*>& IsleActors)
{
	for (int32 i = 0; i < IsleActors.Num(); i++)
	{
		IsleActors[i]->Destroy();
	}
	IsleActors.Empty();
}

/**Fill chunks overlaps indices and parent indices**/
void ARuntimeDestructibleActor::FillOverlapsDepthParents(TArray<FTempChunk>& TempChunks, TArray<uint32>& DM_DepthToOverlapsCount, TArray<const IntPair*>& DM_DepthToOverlaps, ExplicitHierarchicalMesh& HierarchicalMesh)
{
	//Fill chunks overlaps indices
	for (int32 Depth = 0; Depth < DM_DepthToOverlapsCount.Num(); Depth++)
	{
		for (uint32 PairIdx = 0; PairIdx < DM_DepthToOverlapsCount[Depth]; PairIdx++)
		{
			const IntPair& Pair = DM_DepthToOverlaps[Depth][PairIdx];
			TempChunks[Pair.i0].Depth = Depth;
			TempChunks[Pair.i0].Overlaps.Add(Pair.i1);
		}
	}

	for (int32 ChunkIdx = 0; ChunkIdx < TempChunks.Num(); ChunkIdx++)
	{
		FTempChunk& TempChunk = TempChunks[ChunkIdx];

		int32_t* ParentIndex = HierarchicalMesh.parentIndex(ChunkIdx);

		if (ParentIndex)
		{
			TempChunk.ParentIndex = *ParentIndex;
			if (*ParentIndex == INDEX_NONE)
			{
				TempChunk.Depth = 0;
			}
			else
			{
				FTempChunk& ParentChunk = TempChunks[*ParentIndex];

				TempChunk.Depth = ParentChunk.Depth + 1;

				ParentChunk.Children.Add(TempChunk.ChunkIndex);
			}
		}
		//if ParentIndex==nullptr - set PArentIndex to INDEX_NONE(-1)
		else
			TempChunks[ChunkIdx].ParentIndex = INDEX_NONE;
	}
}

void ARuntimeDestructibleActor::CreatePhysicsConstraint(AIsleActor* RootIsleActor, AFractureChunkActor* TargetFractureChunkActor)
{

	AConstraintActor* ConstraintActor = RootIsleActor->GetWorld()->SpawnActor<AConstraintActor>(AConstraintActor::StaticClass(), RootIsleActor->GetTransform());


	//set up the constraint instance with all the desired values
	FConstraintInstance ConstraintInstance;

	ConstraintInstance.ProfileInstance.LinearLimit.bSoftConstraint = false;
	ConstraintInstance.SetDisableCollision(true);	
	

	ConstraintInstance.SetLinearXMotion(ELinearConstraintMotion::LCM_Locked);
	ConstraintInstance.SetLinearYMotion(ELinearConstraintMotion::LCM_Locked);
	ConstraintInstance.SetLinearZMotion(ELinearConstraintMotion::LCM_Locked);

	ConstraintInstance.SetLinearLimitSize(0);
	ConstraintInstance.SetLinearXLimit(ELinearConstraintMotion::LCM_Locked, 0);
	ConstraintInstance.SetLinearYLimit(ELinearConstraintMotion::LCM_Locked, 0);
	ConstraintInstance.SetLinearZLimit(ELinearConstraintMotion::LCM_Locked, 0);	

	ConstraintInstance.ProfileInstance.LinearLimit.bSoftConstraint = false;
	ConstraintInstance.ProfileInstance.LinearLimit.Stiffness = 0;
	ConstraintInstance.ProfileInstance.LinearLimit.Damping = 0;
	

	ConstraintInstance.AngularRotationOffset = FRotator::ZeroRotator;

	ConstraintInstance.SetAngularSwing1Motion(EAngularConstraintMotion::ACM_Locked);
	ConstraintInstance.SetAngularSwing2Motion(EAngularConstraintMotion::ACM_Locked);
	ConstraintInstance.SetAngularTwistMotion(EAngularConstraintMotion::ACM_Locked);

	ConstraintInstance.SetAngularDOFLimitScale(0, 0, 0);
	ConstraintInstance.SetAngularSwing1Limit(EAngularConstraintMotion::ACM_Locked, 0);
	ConstraintInstance.SetAngularSwing2Limit(EAngularConstraintMotion::ACM_Locked, 0);
	ConstraintInstance.SetAngularTwistLimit(EAngularConstraintMotion::ACM_Locked, 0);


	ConstraintInstance.ProfileInstance.ConeLimit.bSoftConstraint = false;

	ConstraintInstance.ProfileInstance.TwistLimit.bSoftConstraint = false;

	ConstraintInstance.ProfileInstance.ConeLimit.Stiffness = 0;
	ConstraintInstance.ProfileInstance.ConeLimit.Damping = 0;
	ConstraintInstance.ProfileInstance.TwistLimit.Stiffness = 0;
	ConstraintInstance.ProfileInstance.TwistLimit.Damping = 0;

	//UPhysicsConstraintComponent* ConstraintComp = NewObject<UPhysicsConstraintComponent>(RootIsleActor);
	//if (!ConstraintComp)
	//{
	//	//UE_LOG constraint UObject could not be created!
	//	return;
	//}

		
	UPhysicsConstraintComponent* ConstraintComp = ConstraintActor->ConstraintComponent;	
	
	
	ConstraintComp->ConstraintInstance = ConstraintInstance;
	//~~~~~~~~~~~~~~~~~~~~~~~~

	//Set World Location
	ConstraintComp->SetWorldLocation(RootIsleActor->GetActorLocation());

	//Attach to Root!
	//ConstraintComp->AttachToComponent(RootIsleActor->GetRootComponent(), FAttachmentTransformRules(EAttachmentRule(EAttachmentRule::KeepWorld), true));

	ConstraintComp->ConstraintActor1 = RootIsleActor;

	ConstraintComp->ConstraintActor2 = TargetFractureChunkActor;

	
	UpdateBodySetup(TargetFractureChunkActor->FractureChunkComponent);

	//~~~ Init Constraint ~~~
	ConstraintComp->SetConstrainedComponents(RootIsleActor->IsleComponent, NAME_None, TargetFractureChunkActor->FractureChunkComponent, NAME_None);

	

	//ConstraintComp->RegisterComponent();
}











UIsleComponent* ARuntimeDestructibleActor::InitFakeIsleComponent(ARuntimeDestructibleActor* RuntimeDestructibleActor, const wchar_t* IsleName)
{
	UIsleComponent* Isle = NewObject<UIsleComponent>(RuntimeDestructibleActor->RootComponent, IsleName);
	Isle->SetCollisionUseComplexAsSimple(false);

	Isle->SetupAttachment(RuntimeDestructibleActor->RootComponent);

	Isle->SetWorldLocation(RuntimeDestructibleActor->RootComponent->GetComponentLocation());
	Isle->SetWorldRotation(RuntimeDestructibleActor->RootComponent->GetComponentRotation());

	Isle->SetSimulatePhysics(false);
	Isle->SetCollisionProfileName(TEXT("NoCollision"));

	Isle->SetVisibility(false);
	Isle->SetCastShadow(false);

	return Isle;
}

UFractureChunkComponent* ARuntimeDestructibleActor::InitFakeFractureChunkComponent(ARuntimeDestructibleActor* RuntimeDestructibleActor)
{
	UFractureChunkComponent* Chunk = NewObject<UFractureChunkComponent>(RuntimeDestructibleActor->RootComponent);
	Chunk->SetCollisionUseComplexAsSimple(false);

	Chunk->SetupAttachment(RuntimeDestructibleActor->RootComponent);

	Chunk->SetWorldLocation(RuntimeDestructibleActor->RootComponent->GetComponentLocation());
	Chunk->SetWorldRotation(RuntimeDestructibleActor->RootComponent->GetComponentRotation());

	Chunk->SetSimulatePhysics(false);
	Chunk->SetCollisionProfileName(TEXT("NoCollision"));

	Chunk->SetVisibility(false);
	Chunk->SetCastShadow(false);

	return Chunk;
}

UFractureChunkComponent* ARuntimeDestructibleActor::InitFractureChunkComponent(ARuntimeDestructibleActor* RuntimeDestructibleActor, ExplicitHierarchicalMesh& HierarchicalMesh, uint32 ChunkIndex, const wchar_t* ChunkName)
{
	TArray <UFractureChunkComponent*>& FractureChunks = RuntimeDestructibleActor->FractureChunks;

	int32_t* ParentIndex = HierarchicalMesh.parentIndex(ChunkIndex);

	if (ParentIndex)
	{
		UFractureChunkComponent* Chunk = NewObject<UFractureChunkComponent>(RuntimeDestructibleActor->RootComponent, ChunkName);

		Chunk->ChunkIndex = ChunkIndex;
		Chunk->ParentIndex = *ParentIndex;
		Chunk->IsleIndex = INDEX_NONE;
		Chunk->IsFractured = false;
		Chunk->SetCollisionUseComplexAsSimple(false);
		if (*ParentIndex == INDEX_NONE)
		{
			Chunk->DepthLevel = 0;

			Chunk->SetupAttachment(RuntimeDestructibleActor->RootComponent);

			Chunk->SetWorldLocation(RuntimeDestructibleActor->RootComponent->GetComponentLocation());
			Chunk->SetWorldRotation(RuntimeDestructibleActor->RootComponent->GetComponentRotation());

			Chunk->SetSimulatePhysics(true);
			Chunk->SetCollisionProfileName(TEXT("Destructible"));
			//Chunk->SetCollisionProfileName(TEXT("OverlapAllDynamic"));
			//Chunk->SetVisibility(false);
		}
		else
		{
			UFractureChunkComponent* ParentChunk = FractureChunks[*ParentIndex];

			Chunk->DepthLevel = ParentChunk->DepthLevel + 1;

			Chunk->SetSimulatePhysics(false);
			Chunk->SetCollisionProfileName(TEXT("NoCollision"));
			//Chunk->SetCollisionProfileName(TEXT("OverlapAllDynamic"));
			//Chunk->SetCollisionProfileName(TEXT("BlockAllDynamic"));
			//Chunk->SetCollisionProfileName(TEXT("Destructible"));
			Chunk->SetVisibility(false);
			Chunk->SetCastShadow(false);

			ParentChunk->ChildrenIndices.Add(Chunk->ChunkIndex);
		}
		return Chunk;
	}
	return nullptr;
}



void ARuntimeDestructibleActor::SetupChunksAttachments(ARuntimeDestructibleActor* RuntimeDestructibleActor, TArray<uint32>& DM_DepthToOverlapsCount, TArray<const IntPair*>& DM_DepthToOverlaps)
{
	TArray <UFractureChunkComponent*>& FractureChunks = RuntimeDestructibleActor->FractureChunks;

	TArray<UIsleComponent*>& Isles = RuntimeDestructibleActor->Isles;

	//Fill chunks overlaps indices
	for (int32 Depth = 0; Depth < DM_DepthToOverlapsCount.Num(); Depth++)
	{
		for (uint32 PairIdx = 0; PairIdx < DM_DepthToOverlapsCount[Depth]; PairIdx++)
		{
			const IntPair& Pair = DM_DepthToOverlaps[Depth][PairIdx];
			FractureChunks[Pair.i0]->OverlapIndices.Add(Pair.i1);
		}
	}

	wstring IsleStr = L"Isle_";
	wstring IsleIdx;
	wstring IsleName;

	//Attach Chunks either to patent chunk or to parent isle
	for (int32 ChunkIdx = 0; ChunkIdx < FractureChunks.Num(); ChunkIdx++)
	{
		UFractureChunkComponent* Chunk = FractureChunks[ChunkIdx];
		if (Chunk->OverlapIndices.Num() > 0)
		{
			if (Chunk->IsleIndex == INDEX_NONE)
			{
				IsleIdx = to_wstring(Isles.Num());
				IsleName = IsleStr + IsleIdx;

				UIsleComponent* NewIsle = InitFakeIsleComponent(RuntimeDestructibleActor, IsleName.data());
				InitFakeIsleData(NewIsle);
				NewIsle->GetBodyInstance()->bAutoWeld = true;
				NewIsle->ParentIndex = Chunk->ParentIndex;				

				//NewIsle Setup Attachment 
				if (NewIsle->ParentIndex != INDEX_NONE)
				{
					NewIsle->SetupAttachment(FractureChunks[NewIsle->ParentIndex]);
					NewIsle->SetWorldLocation(FractureChunks[NewIsle->ParentIndex]->GetComponentLocation());
					NewIsle->SetWorldRotation(FractureChunks[NewIsle->ParentIndex]->GetComponentRotation());
				}
				else
				{
					NewIsle->SetWorldLocation(RuntimeDestructibleActor->RootComponent->GetComponentLocation());
					NewIsle->SetWorldRotation(RuntimeDestructibleActor->RootComponent->GetComponentRotation());
				}

				//Attach Chunk to New Isle
				Chunk->IsleIndex = Isles.Num();
				Chunk->SetupAttachment(NewIsle);
				Chunk->SetWorldLocation(NewIsle->GetComponentLocation());
				Chunk->SetWorldRotation(NewIsle->GetComponentRotation());

				NewIsle->ChildrenIndices.Add(Chunk->ChunkIndex);

				//Add Chunk's Collision to New Isle
				/*for (auto& Pair : Chunk->GetRuntimeMesh()->GetRuntimeMeshData()->GetConvexCollisionSections())
				{
					NewIsleMesh->AddConvexCollisionSectionAtLast(*Chunk->GetRuntimeMesh()->GetRuntimeMeshData()->GetConvexCollisionSection(Pair.Key));
				}*/

				//Search Overlapped Chunks
				SearchOverlapsToAttach(RuntimeDestructibleActor->FractureChunks, NewIsle, Chunk);

				
				//Add NewIsle to Isles
				Isles.Add(NewIsle);

			}
		}
		else if (Chunk->ParentIndex != INDEX_NONE)
		{
			UFractureChunkComponent* ParentChunk = FractureChunks[Chunk->ParentIndex];
			Chunk->SetupAttachment(ParentChunk);
			Chunk->SetWorldLocation(ParentChunk->GetComponentLocation());
			Chunk->SetWorldRotation(ParentChunk->GetComponentRotation());
		}
	}

}



/**Recursive search of Chunk's overlaps and attaching them to Isle**/
void ARuntimeDestructibleActor::SearchOverlapsToAttach(TArray <UFractureChunkComponent*>& FractureChunks, UIsleComponent* Isle, UFractureChunkComponent* InChunk)
{
	for (int32& OverlapIdx : InChunk->OverlapIndices)
	{
		UFractureChunkComponent* Chunk = FractureChunks[OverlapIdx];
		//If Chunk already attached to Isle - continue
		if (Chunk->IsleIndex != INDEX_NONE)
			continue;

		//Attach Chunk to New Isle
		Chunk->IsleIndex = InChunk->IsleIndex;
		Chunk->SetupAttachment(Isle);
		Chunk->SetWorldLocation(FractureChunks[Isle->ParentIndex]->GetComponentLocation());
		Chunk->SetWorldRotation(FractureChunks[Isle->ParentIndex]->GetComponentRotation());

		Isle->ChildrenIndices.Add(Chunk->ChunkIndex);

		//Add Chunk's Collision to Isle
		/*URuntimeMesh* IsleMesh = Isle.IsleComponent->GetRuntimeMesh();
		for (auto& Pair : Chunk->GetRuntimeMesh()->GetRuntimeMeshData()->GetConvexCollisionSections())
		{
			IsleMesh->AddConvexCollisionSectionAtLast(*Chunk->GetRuntimeMesh()->GetRuntimeMeshData()->GetConvexCollisionSection(Pair.Key));
		}*/

		//Search Overlapped Chunks
		SearchOverlapsToAttach(FractureChunks, Isle, Chunk);
	}
}

/**Recursive search of separate overlapped chunks groups and attaching them to new Isle**/
void ARuntimeDestructibleActor::SearchOverlapsToDetach(TArray <UFractureChunkComponent*>& FractureChunks, UIsleComponent* Isle, UFractureChunkComponent* Chunk)
{

}

void ARuntimeDestructibleActor::ClearIsles(TArray <UIsleComponent*>& Isles)
{
	for (int32 IsleIndex = 0; IsleIndex < Isles.Num(); IsleIndex++)
	{
		if (Isles[IsleIndex] != nullptr)
		{
			//Isles[IsleIndex]->DetachFromParent();
			Isles[IsleIndex]->DestroyComponent();
			//delete Isles[IsleIndex];
		}
	}
	Isles.Empty();
}

void ARuntimeDestructibleActor::ClearIsles(TArray <FIsle>& Isles)
{
	for (int32 IsleIndex = 0; IsleIndex < Isles.Num(); IsleIndex++)
	{
		if (Isles[IsleIndex].IsleComponent != nullptr)
		{
			//Isles[IsleIndex].IsleComponent->DetachFromParent();
			Isles[IsleIndex].IsleComponent->DestroyComponent();
			//delete Isles[IsleIndex].IsleComponent;
		}
	}
	Isles.Empty();
}

void ARuntimeDestructibleActor::ClearFractureChunks(TArray <FFractureChunk>& FractureChunks)
{
	for (int32 ChunkIndex = 0; ChunkIndex < FractureChunks.Num(); ChunkIndex++)
	{
		if (FractureChunks[ChunkIndex].FractureChunkComponent != nullptr)
		{
			//FractureChunks[ChunkIndex].FractureChunkComponent->DetachFromParent();
			FractureChunks[ChunkIndex].FractureChunkComponent->DestroyComponent();
			//delete FractureChunks[ChunkIndex].FractureChunkComponent;
		}
	}
	FractureChunks.Empty();
}

void ARuntimeDestructibleActor::ClearFractureChunks(TArray <UFractureChunkComponent*>& FractureChunks)
{
	for (int32 ChunkIndex = 0; ChunkIndex < FractureChunks.Num(); ChunkIndex++)
	{
		if (FractureChunks[ChunkIndex] != nullptr)
		{
			//FractureChunks[ChunkIndex]->DetachFromParent();
			FractureChunks[ChunkIndex]->DestroyComponent();
			//delete FractureChunks[ChunkIndex];
		}
	}
	FractureChunks.Empty();
}


void ARuntimeDestructibleActor::UpdateBodySetup(URuntimeMeshComponent* Component)
{
	FBodyInstance* BI = Component->GetBodyInstance();
	if (!BI->BodySetup.IsValid())
	{
		UBodySetup* BS = Component->GetRuntimeMesh()->GetBodySetup();
		if (BS == nullptr)
		{
			Component->UpdateCollision(true);
		}
		//BI->BodySetup = BS;
	}
	bool IsValid = BI->BodySetup.IsValid();
	//check(IsValid);
}



void ARuntimeDestructibleActor::AddFakeBoxCollision(UFractureChunkComponent* Chunk)
{
	auto Section = Chunk->GetSectionReadonly(0);

	const int32 NumBaseVerts = Section->NumVertices();

	FVector Centroid = FVector::ZeroVector;

	// Build vertex buffer 
	for (int32 BaseVertIndex = 0; BaseVertIndex < NumBaseVerts; BaseVertIndex++)
	{
		FRuntimeMeshAccessorVertex BaseVert = Section->GetVertex(BaseVertIndex);
		Centroid += BaseVert.Position;
	}
	Centroid /= NumBaseVerts;

	FRuntimeMeshCollisionBox Box(5);
	Box.Center = Centroid;
	Chunk->AddCollisionBox(Box);
}

//#endif //WITH_APEX










/**
Create fake isle render data and collision (both are tetrahedrons)
**/
void ARuntimeDestructibleActor::InitFakeIsleData(URuntimeMeshComponent* FakeIsleComponent)
{
	URuntimeMesh* RuntimeMesh = FakeIsleComponent->GetOrCreateRuntimeMesh();


	TArray<FVector> Vertices0 = { FVector(-258.956,-0,50), FVector(-308.956,-0,50), FVector(-358.956,-0,50), FVector(-258.956,-0,0), FVector(-358.956,-0,0), FVector(-308.956,-0,0),
		FVector(-258.956,-0,-50), FVector(-358.956,-0,-50), FVector(-308.956,-0,-50), FVector(-258.956,-0,50), FVector(-308.956,-0,50), FVector(-308.956,-50,50), FVector(-358.956,-50,50),
		FVector(-358.956,-0,50), FVector(-258.956,-0,0), FVector(-258.956,-0,-50), FVector(-258.956,-50,0), FVector(-258.956,-50,50), FVector(-258.956,-0,50), FVector(-358.956,-50,50),
		FVector(-358.956,-0,50), FVector(-358.956,-0,0), FVector(-358.956,-0,-50), FVector(-358.956,-0,-50), FVector(-358.956,-50,-50), FVector(-308.956,-0,-50), FVector(-258.956,-0,-50),
		FVector(-258.956,-100,50), FVector(-258.956,-50,50), FVector(-308.956,-100,50), FVector(-358.956,-100,50), FVector(-258.956,-100,0), FVector(-258.956,-100,50), FVector(-358.956,-100,50),
		FVector(-358.956,-100,0), FVector(-258.956,-50,-50), FVector(-258.956,-100,-50), FVector(-308.956,-100,-50), FVector(-258.956,-50,-50), FVector(-308.956,-50,-50), FVector(-258.956,-100,-50),
		FVector(-358.956,-50,-50), FVector(-358.956,-50,0), FVector(-358.956,-100,-50), FVector(-358.956,-100,-50), FVector(-258.956,-100,50), FVector(-308.956,-100,50), FVector(-358.956,-100,0),
		FVector(-358.956,-100,50), FVector(-258.956,-100,-50), FVector(-258.956,-100,0), FVector(-308.956,-100,0), FVector(-308.956,-100,-50), FVector(-358.956,-100,-50) };

	TArray<FVector> Normals0 = { FVector4(0,1,0,1), FVector4(0,1,0,1), FVector4(0,1,0,1), FVector4(0,1,0,1), FVector4(0,1,0,1), FVector4(0,1,0,1), FVector4(0,1,0,1), FVector4(0,1,0,1), FVector4(0,1,0,1),
		FVector4(0,0,1,1), FVector4(0,0,1,1), FVector4(0,0,1,1), FVector4(0,0,1,1), FVector4(0,0,1,1), FVector4(1,0,0,1), FVector4(1,0,0,1), FVector4(1,0,0,1), FVector4(1,0,0,1), FVector4(1,0,0,1),
		FVector4(-1,0,0,1), FVector4(-1,0,0,1), FVector4(-1,0,0,1), FVector4(-1,0,0,1), FVector4(0,0,-1,1), FVector4(0,0,-1,1), FVector4(0,0,-1,1), FVector4(0,0,-1,1), FVector4(0,0,1,1), FVector4(0,0,1,1),
		FVector4(0,0,1,1), FVector4(0,0,1,1), FVector4(1,0,0,1), FVector4(1,0,0,1), FVector4(-1,0,0,1), FVector4(-1,0,0,1), FVector4(1,0,0,1), FVector4(1,0,0,1), FVector4(0,0,-1,1), FVector4(0,0,-1,1),
		FVector4(0,0,-1,1), FVector4(0,0,-1,1), FVector4(-1,0,0,1), FVector4(-1,0,0,1), FVector4(-1,0,0,1), FVector4(0,0,-1,1), FVector4(0,-1,0,1), FVector4(0,-1,0,1), FVector4(0,-1,0,1), FVector4(0,-1,0,1),
		FVector4(0,-1,0,1), FVector4(0,-1,0,1), FVector4(0,-1,0,1), FVector4(0,-1,0,1), FVector4(0,-1,0,1) };

	TArray<FVector2D> UV00 = { FVector2D(0,1), FVector2D(0.5,1), FVector2D(1,1), FVector2D(0,0.5), FVector2D(1,0.5), FVector2D(0.5,0.5), FVector2D(0,0), FVector2D(1,0), FVector2D(0.5,0), FVector2D(1,1),
		FVector2D(0.5,1), FVector2D(0.5,0.5), FVector2D(0,0.5), FVector2D(0,1), FVector2D(0.5,1), FVector2D(1,1), FVector2D(0.5,0.5), FVector2D(0,0.5), FVector2D(0,1), FVector2D(1,0.5), FVector2D(1,1),
		FVector2D(0.5,1), FVector2D(0,1), FVector2D(1,1), FVector2D(1,0.5), FVector2D(0.5,1), FVector2D(0,1), FVector2D(1,0), FVector2D(1,0.5), FVector2D(0.5,0), FVector2D(0,0), FVector2D(0.5,0), FVector2D(0,0),
		FVector2D(1,0), FVector2D(0.5,0), FVector2D(1,0.5), FVector2D(1,0), FVector2D(0.5,0), FVector2D(0,0.5), FVector2D(0.5,0.5), FVector2D(0,0), FVector2D(0,0.5), FVector2D(0.5,0.5), FVector2D(0,0),
		FVector2D(1,0), FVector2D(1,1), FVector2D(0.5,1), FVector2D(0,0.5), FVector2D(0,1), FVector2D(1,0), FVector2D(1,0.5), FVector2D(0.5,0.5), FVector2D(0.5,0), FVector2D(0,0) };

	TArray<FColor> Colors0 = { FColor(255,255,255,255), FColor(255,255,255,255), FColor(255,255,255,255), FColor(255,255,255,255), FColor(255,255,255,255), FColor(255,255,255,255), FColor(255,255,255,255),
		FColor(255,255,255,255), FColor(255,255,255,255), FColor(255,255,255,255), FColor(255,255,255,255), FColor(255,255,255,255), FColor(255,255,255,255), FColor(255,255,255,255), FColor(255,255,255,255),
		FColor(255,255,255,255), FColor(255,255,255,255), FColor(255,255,255,255), FColor(255,255,255,255), FColor(255,255,255,255), FColor(255,255,255,255), FColor(255,255,255,255), FColor(255,255,255,255),
		FColor(255,255,255,255), FColor(255,255,255,255), FColor(255,255,255,255), FColor(255,255,255,255), FColor(255,255,255,255), FColor(255,255,255,255), FColor(255,255,255,255), FColor(255,255,255,255),
		FColor(255,255,255,255), FColor(255,255,255,255), FColor(255,255,255,255), FColor(255,255,255,255), FColor(255,255,255,255), FColor(255,255,255,255), FColor(255,255,255,255), FColor(255,255,255,255),
		FColor(255,255,255,255), FColor(255,255,255,255), FColor(255,255,255,255), FColor(255,255,255,255), FColor(255,255,255,255), FColor(255,255,255,255), FColor(255,255,255,255), FColor(255,255,255,255),
		FColor(255,255,255,255), FColor(255,255,255,255), FColor(255,255,255,255), FColor(255,255,255,255), FColor(255,255,255,255), FColor(255,255,255,255), FColor(255,255,255,255) };

	TArray<FRuntimeMeshTangent> Tangents0 = { FVector(-1,0,0), FVector(-1,0,0), FVector(-1,0,0), FVector(-1,0,0), FVector(-1,0,0), FVector(-1,0,0), FVector(-1,0,0), FVector(-1,0,0), FVector(-1,0,0), FVector(1,0,0),
		FVector(1,0,0), FVector(1,0,0), FVector(1,0,0), FVector(1,0,0), FVector(0,0,-1), FVector(0,0,-1), FVector(0,0,-1), FVector(0,0,-1), FVector(0,0,-1), FVector(0,0,1), FVector(0,0,1), FVector(0,0,1),
		FVector(0,0,1), FVector(-1,0,0), FVector(-1,0,0), FVector(-1,0,0), FVector(-1,0,0), FVector(1,0,0), FVector(1,0,0), FVector(1,0,0), FVector(1,0,0), FVector(0,0,-1), FVector(0,0,-1), FVector(0,0,1),
		FVector(0,0,1), FVector(0,0,-1), FVector(0,0,-1), FVector(-1,0,0), FVector(-1,0,0), FVector(-1,0,0), FVector(-1,0,0), FVector(0,0,1), FVector(0,0,1), FVector(0,0,1), FVector(-1,0,0), FVector(1,0,0),
		FVector(1,0,0), FVector(1,0,0), FVector(1,0,0), FVector(1,0,0), FVector(1,0,0), FVector(1,0,0), FVector(1,0,0), FVector(1,0,0) };

	TArray<int32> Indices0 = { 0, 1, 3, 1, 2, 5, 3, 5, 6, 2, 4, 5, 5, 8, 6, 5, 4, 8, 4, 7, 8, 28, 10, 9, 28, 11, 10, 11, 13, 10, 11, 12, 13, 35, 14, 15, 35, 16, 14, 16, 18, 14, 16, 17, 18, 1, 5, 3, 19, 21, 20,
		19, 42, 21, 42, 22, 21, 42, 41, 22, 24, 25, 23, 24, 39, 25, 39, 26, 25, 39, 38, 26, 27, 11, 28, 27, 29, 11, 29, 12, 11, 29, 30, 12, 31, 17, 16, 31, 32, 17, 33, 42, 19, 33, 34, 42, 36, 16, 35, 36, 31, 16,
		37, 38, 39, 37, 40, 38, 34, 41, 42, 34, 43, 41, 44, 37, 39, 44, 39, 24, 50, 46, 45, 50, 51, 46, 51, 48, 46, 51, 47, 48, 49, 51, 50, 49, 52, 51, 52, 47, 51, 52, 53, 47 };





	TArray<FVector> ConvexVertices0 = { FVector(-358.956,-100,50), FVector(-258.956,-100,50), FVector(-258.956,-100,-50), FVector(-358.956,-100,-50), FVector(-358.956,0,-50), FVector(-358.956,0,50),
		FVector(-258.956,0,-50), FVector(-258.956,0,50) };

	FBox ConvexBox0 = FBox(FVector(-358.956, -100, -50), FVector(-258.956, 0, 50));

	TArray<int32> ConvexIndices0 = { 1, 0, 5, 0, 4, 5, 5, 4, 6, 5, 6, 7, 6, 1, 7, 1, 5, 7, 1, 6, 2, 6, 4, 2, 0, 1, 2, 0, 2, 3, 2, 4, 3, 4, 0, 3 };

	TArray<FPlane> ConvexPlanes0 = { FPlane(0,-1,0,100), FPlane(-1,0,0,358.956), FPlane(0,0,-1,50), FPlane(-0,1,-0,-0), FPlane(1,-0,-0,-258.956), FPlane(-0,-0,1,50), FPlane(0,0,0,-0), FPlane(0,0,0,-0) };



	TArray<FVector> Vertices;
	TArray<FVector> Normals;
	TArray<FRuntimeMeshTangent> Tangents;
	TArray<FColor> VertexColors;
	TArray<FVector2D> TextureCoordinates;
	TArray<int32> Triangles;

	
	// First vertex
	Vertices.Add(FVector(0, 100, 0));
	Normals.Add(FVector(0, 0, 1));
	Tangents.Add(FRuntimeMeshTangent(0, -1, 0));
	VertexColors.Add(FColor::White);
	TextureCoordinates.Add(FVector2D(0, 0));

	// Second vertex
	Vertices.Add(FVector(100, 100, 0));
	Normals.Add(FVector(0, 0, 1));
	Tangents.Add(FRuntimeMeshTangent(0, -1, 0));
	VertexColors.Add(FColor::White);
	TextureCoordinates.Add(FVector2D(0, 1));

	// Third vertex
	Vertices.Add(FVector(100, 0, 0));
	Normals.Add(FVector(0, 0, 1));
	Tangents.Add(FRuntimeMeshTangent(0, -1, 0));
	VertexColors.Add(FColor::White);
	TextureCoordinates.Add(FVector2D(1, 1));

	// Fourth vertex
	Vertices.Add(FVector(0, 0, 0));
	Normals.Add(FVector(0, 0, 1));
	Tangents.Add(FRuntimeMeshTangent(0, -1, 0));
	VertexColors.Add(FColor::White);
	TextureCoordinates.Add(FVector2D(1, 0));

	Triangles.Add(0);
	Triangles.Add(1);
	Triangles.Add(2);
	Triangles.Add(0);
	Triangles.Add(2);
	Triangles.Add(3);


	TArray<FVector> ConvexVertices;
	ConvexVertices.Add(FVector(0, 0, 0));
	ConvexVertices.Add(FVector(100, 0, 0));
	ConvexVertices.Add(FVector(0, 100, 0));
	ConvexVertices.Add(FVector(100, 100, 0));
	ConvexVertices.Add(FVector(0, 0, 100));
	ConvexVertices.Add(FVector(100, 0, 100));
	ConvexVertices.Add(FVector(0, 100, 100));
	ConvexVertices.Add(FVector(100, 100, 100));

	FRuntimeMeshCollisionConvexMesh ConvexMesh;
	ConvexMesh.VertexBuffer = ConvexVertices0;
	ConvexMesh.BoundingBox = ConvexBox0;
	ConvexMesh.IndexBuffer = ConvexIndices0;
	ConvexMesh.Planes = ConvexPlanes0;


	TArray<FVector> Vertices1;
	TArray<FVector> Normals1;
	TArray<FRuntimeMeshTangent> Tangents1;
	TArray<FColor> VertexColors1;
	TArray<FVector2D> TextureCoordinates1;
	TArray<int32> Triangles1;

	Vertices1.Add(FVector(0, 0, 0));
	Vertices1.Add(FVector(5, 0, 0));
	Vertices1.Add(FVector(0, 5, 0));
	Vertices1.Add(FVector(0, 0, 5));

	Triangles1 = {0,1,2, 0,2,3, 0,3,1, 1,3,2};

	RuntimeMesh->ClearAllMeshSections();

	//RuntimeMesh->CreateMeshSection(0, Vertices, Triangles, Normals, TextureCoordinates, VertexColors, Tangents);
	//RuntimeMesh->CreateMeshSection(0, Vertices1, Triangles1, Normals1, TextureCoordinates1, VertexColors1, Tangents1);
	RuntimeMesh->CreateMeshSection(0, Vertices0, Indices0, Normals0, UV00, Colors0, Tangents0);


	RuntimeMesh->ClearAllConvexCollisionSections();

	//RuntimeMesh->AddConvexCollisionSection(ConvexVertices);
	//RuntimeMesh->AddConvexCollisionSection(Vertices1);
	RuntimeMesh->AddConvexCollisionSectionAtLast(ConvexMesh);
}

/**
Write Runtime Mesh vertices, triangles and collision to txt files
**/
void ARuntimeDestructibleActor::WriteRuntimeMeshDataToTxtFile(const URuntimeMesh* InRuntimeMesh, const char* Path)
{
	if (InRuntimeMesh)
	{
		const char* DataFile = "Runtime Mesh Data.txt";

		stringstream iostr;
		ofstream FileSteram;
		iostr << Path << DataFile;
		FileSteram.open(iostr.str());

		//Write section vertices and indices
		FRuntimeMeshDataPtr InMeshDataPtr = InRuntimeMesh->GetRuntimeMeshData();
		if (&InMeshDataPtr != nullptr)
		{
			for (int32 SectionIndex = 0; SectionIndex < InRuntimeMesh->GetNumSections(); SectionIndex++)
			{
				if (!InRuntimeMesh->DoesSectionExist(SectionIndex))
				{
					continue;
				}

				auto SourceMeshData = InMeshDataPtr->GetSectionReadonly(SectionIndex);

				stringstream Vertstream;
				stringstream Normalstream;
				stringstream UV0stream;
				stringstream Colorstream;
				stringstream Tangentstream;

				Vertstream << "TArray<FVector> Vertices" << SectionIndex << " = {";
				Normalstream << "TArray<FVector> Normals" << SectionIndex << " = {";
				UV0stream << "TArray<FVector2D> UV0" << SectionIndex << " = {";
				Colorstream << "TArray<FColor> Colors" << SectionIndex << " = {";
				Tangentstream << "TArray<FRuntimeMeshTangent> Tangents" << SectionIndex << " = {";

				//Write section vertices
				int32 NumBaseVerts = SourceMeshData->NumVertices();
				for (int32 BaseVertIndex = 0; BaseVertIndex < NumBaseVerts; BaseVertIndex++)
				{
					const FRuntimeMeshAccessorVertex& BaseVert = SourceMeshData->GetVertex(BaseVertIndex);

					const FVector& Vertex = BaseVert.Position;
					const FVector4& Normal = BaseVert.Normal;
					const FVector2D& UV0 = BaseVert.UVs[0];
					const FColor& Color = BaseVert.Color;
					const FVector& Tangent = BaseVert.Tangent;

					Vertstream << "FVector(" << Vertex.X << "," << Vertex.Y << "," << Vertex.Z << ")";
					Normalstream << "FVector4(" << Normal.X << "," << Normal.Y << "," << Normal.Z << "," << Normal.W << ")";
					UV0stream << "FVector2D(" << UV0.X << "," << UV0.Y << ")";
					Colorstream << "FColor(" << (int)Color.R << "," << (int)Color.G << "," << (int)Color.B << "," << (int)Color.A << ")";
					Tangentstream << "FVector(" << Tangent.X << "," << Tangent.Y << "," << Tangent.Z << ")";

					if (BaseVertIndex < NumBaseVerts - 1)
					{
						Vertstream << ", ";
						Normalstream << ", ";
						UV0stream << ", ";
						Colorstream << ", ";
						Tangentstream << ", ";
					}
				}

				Vertstream << "};\r\n";
				Normalstream << "};\r\n";
				UV0stream << "};\r\n";
				Colorstream << "};\r\n";
				Tangentstream << "};\r\n";

				FileSteram << Vertstream.rdbuf();
				FileSteram << Normalstream.rdbuf();
				FileSteram << UV0stream.rdbuf();
				FileSteram << Colorstream.rdbuf();
				FileSteram << Tangentstream.rdbuf();

				//Write section indices
				int32 NumBaseIndices = SourceMeshData->NumIndices();

				stringstream Indexstream;
				Indexstream << "TArray<int32> Indices" << SectionIndex << " = {";

				for (int32 BaseIndex = 0; BaseIndex < NumBaseIndices; BaseIndex++)
				{
					int32 Index = SourceMeshData->GetIndex(BaseIndex);
					Indexstream << Index;
					if (BaseIndex < NumBaseIndices - 1)
						Indexstream << ", ";
				}
				Indexstream << "};\r\n";

				FileSteram << Indexstream.rdbuf();

				FileSteram << "\r\n" << "\r\n";
			}
		}


		//Write collision
		FRuntimeMeshDataRef InMeshDataRef = InRuntimeMesh->GetRuntimeMeshData();
		if (&InMeshDataRef != nullptr)
		{


			int32 MapSize = InMeshDataRef->NumConvexCollisionSections();
			for (int32 ConvexIndex = 0; ConvexIndex < MapSize; ConvexIndex++)
			{
				stringstream ConvexVertstream;
				stringstream ConvexBoxstream;
				stringstream ConvexIndexstream;
				stringstream ConvexPlanestream;

				ConvexVertstream << "TArray<FVector> ConvexVertices" << ConvexIndex << " = {";
				ConvexIndexstream << "TArray<int32> ConvexIndices" << ConvexIndex << " = {";
				ConvexPlanestream << "TArray<FPlane> ConvexPlanes" << ConvexIndex << " = {";

				FRuntimeMeshCollisionConvexMesh* ConvexCollisionSection = InMeshDataRef->GetConvexCollisionSection(ConvexIndex);

				const TArray<FVector>& VertexBuffer = ConvexCollisionSection->VertexBuffer;
				for (int32 VertIndex = 0; VertIndex < VertexBuffer.Num(); VertIndex++)
				{
					const FVector& Vertex = VertexBuffer[VertIndex];
					ConvexVertstream << "FVector(" << Vertex.X << "," << Vertex.Y << "," << Vertex.Z << ")";
					if (VertIndex < VertexBuffer.Num() - 1)
					{
						ConvexVertstream << ", ";
					}
				}

				const FBox& BoundingBox = ConvexCollisionSection->BoundingBox;
				ConvexBoxstream << "FBox ConvexBox" << ConvexIndex << " = FBox(FVector(" << BoundingBox.Min.X << "," << BoundingBox.Min.Y << "," << BoundingBox.Min.Z << "), " <<
					"FVector(" << BoundingBox.Max.X << "," << BoundingBox.Max.Y << "," << BoundingBox.Max.Z << "));\r\n";

				const TArray<int32>& IndexBuffer = ConvexCollisionSection->IndexBuffer;
				for (int32 i = 0; i < IndexBuffer.Num(); i++)
				{
					ConvexIndexstream << IndexBuffer[i];
					if (i < IndexBuffer.Num() - 1)
					{
						ConvexIndexstream << ", ";
					}
				}

				const TArray<FPlane>& Planes = ConvexCollisionSection->Planes;
				for (int32 PlaneIndex = 0; PlaneIndex < Planes.Num(); PlaneIndex++)
				{
					const FPlane& Plane = Planes[PlaneIndex];
					ConvexPlanestream << "FPlane(" << Plane.X << "," << Plane.Y << "," << Plane.Z << "," << Plane.W << ")";
					if (PlaneIndex < Planes.Num() - 1)
					{
						ConvexPlanestream << ", ";
					}
				}
				ConvexVertstream << "};\r\n";
				ConvexIndexstream << "};\r\n";
				ConvexPlanestream << "};\r\n";

				FileSteram << ConvexVertstream.rdbuf();
				FileSteram << ConvexBoxstream.rdbuf();
				FileSteram << ConvexIndexstream.rdbuf();
				FileSteram << ConvexPlanestream.rdbuf();

				FileSteram << "\r\n" << "\r\n";
			}
			FileSteram << "\r\n" << "\r\n";
		}

		FileSteram.close();
	}
}

/**
Copy runtime mesh render data and collision to other runtime mesh
Only one UV-channel supporting at this moment
**/
void ARuntimeDestructibleActor::CloneRuntimeMesh(const URuntimeMesh* FromRuntimeMesh, URuntimeMesh* ToRuntimeMesh)
{
	if (FromRuntimeMesh)
	{
		ToRuntimeMesh->ClearAllMeshSections();
		ToRuntimeMesh->ClearAllConvexCollisionSections();

		//Write section vertices and indices
		FRuntimeMeshDataPtr InMeshDataPtr = FromRuntimeMesh->GetRuntimeMeshData();
		if (&InMeshDataPtr != nullptr)
		{
			for (int32 SectionIndex = 0; SectionIndex < FromRuntimeMesh->GetNumSections(); SectionIndex++)
			{
				if (!FromRuntimeMesh->DoesSectionExist(SectionIndex))
				{
					continue;
				}

				auto SourceMeshData = InMeshDataPtr->GetSectionReadonly(SectionIndex);

				TArray<FVector> Vertices;
				TArray<FVector> Normals;
				TArray<FVector2D> UV0;
				TArray<FColor> Colors;
				TArray<FRuntimeMeshTangent> Tangents;

				//Write section vertices
				int32 NumBaseVerts = SourceMeshData->NumVertices();
				Vertices.AddDefaulted(NumBaseVerts);
				Normals.AddDefaulted(NumBaseVerts);
				UV0.AddDefaulted(NumBaseVerts);
				Colors.AddDefaulted(NumBaseVerts);
				Tangents.AddDefaulted(NumBaseVerts);

				for (int32 BaseVertIndex = 0; BaseVertIndex < NumBaseVerts; BaseVertIndex++)
				{
					const FRuntimeMeshAccessorVertex& BaseVert = SourceMeshData->GetVertex(BaseVertIndex);

					Vertices[BaseVertIndex] = BaseVert.Position;
					Normals[BaseVertIndex] = BaseVert.Normal;
					UV0[BaseVertIndex] = BaseVert.UVs[0];
					Colors[BaseVertIndex] = BaseVert.Color;
					Tangents[BaseVertIndex] = BaseVert.Tangent;
				}

				//Write section indices
				int32 NumBaseIndices = SourceMeshData->NumIndices();

				TArray<int32> Indices;
				Indices.AddDefaulted(NumBaseIndices);

				for (int32 BaseIndex = 0; BaseIndex < NumBaseIndices; BaseIndex++)
				{
					Indices[BaseIndex] = SourceMeshData->GetIndex(BaseIndex);
				}

				ToRuntimeMesh->CreateMeshSection(SectionIndex, Vertices, Indices, Normals, UV0, Colors, Tangents);

			}
		}

		//Write collision
		FRuntimeMeshDataRef InMeshDataRef = FromRuntimeMesh->GetRuntimeMeshData();
		if (&InMeshDataRef != nullptr)
		{

			int32 MapSize = InMeshDataRef->NumConvexCollisionSections();
			for (int32 ConvexIndex = 0; ConvexIndex < MapSize; ConvexIndex++)
			{

				TArray<FVector> ConvexVertices;
				TArray<int32> ConvexIndices;
				TArray<FPlane> ConvexPlanes;

				FRuntimeMeshCollisionConvexMesh* ConvexCollisionSection = InMeshDataRef->GetConvexCollisionSection(ConvexIndex);

				const TArray<FVector>& VertexBuffer = ConvexCollisionSection->VertexBuffer;

				const FBox& BoundingBox = ConvexCollisionSection->BoundingBox;

				const TArray<int32>& IndexBuffer = ConvexCollisionSection->IndexBuffer;

				const TArray<FPlane>& Planes = ConvexCollisionSection->Planes;


				FRuntimeMeshCollisionConvexMesh ConvexMesh;

				for (int32 VertIndex = 0; VertIndex < VertexBuffer.Num(); VertIndex++)
				{
					const FVector& Vertex = VertexBuffer[VertIndex];
					ConvexMesh.VertexBuffer.Add(FVector(Vertex.X, Vertex.Y, Vertex.Z));
				}

				ConvexMesh.BoundingBox = FBox(FVector(BoundingBox.Min.X, BoundingBox.Min.Y, BoundingBox.Min.Z),
					FVector(BoundingBox.Max.X, BoundingBox.Max.Y, BoundingBox.Max.Z));

				for (int32 i = 0; i < IndexBuffer.Num(); i++)
				{
					ConvexMesh.IndexBuffer.Add(IndexBuffer[i]);
				}

				for (int32 PlaneIndex = 0; PlaneIndex < Planes.Num(); PlaneIndex++)
				{
					const FPlane& Plane = Planes[PlaneIndex];
					ConvexMesh.Planes.Add(FPlane(Plane.X, Plane.Y, Plane.Z, Plane.W));
				}

				ToRuntimeMesh->AddConvexCollisionSectionAtLast(ConvexMesh);
			}
		}
	}
}

void LoadStaticMeshAssetToStaticMeshComponent()
{
	//UStaticMeshComponent* Cube = NewObject<UStaticMeshComponent>(RuntimeDestructibleActor->RootComponent, TEXT("Cube"));
	//Cube->RegisterComponent();
	UStaticMesh* Mesh = LoadObject<UStaticMesh>(nullptr, TEXT("StaticMesh'/Engine/BasicShapes/Cube.Cube'"));
	//Cube->SetStaticMesh(Mesh);
	//Cube->AttachToComponent(FractureChunks[1], FAttachmentTransformRules(EAttachmentRule(EAttachmentRule::KeepRelative), true));
	//Cube->SetSimulatePhysics(false);

	//FVector Loc = FractureChunks[1]->GetComponentLocation();
	//Loc.Y += 100;
	//Cube->SetWorldLocation(Loc);
}