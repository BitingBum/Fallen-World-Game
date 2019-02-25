// Copyright 2016-2018 Chris Conway (Koderz). All Rights Reserved.

#include "RuntimeDestructibleActor.h"

#include "RuntimeMesh.h"
#include "RuntimeMeshComponent.h"

#include "Engine/SkeletalMesh.h"
#include "RMC_DestructibleMesh.h"
#include "RMC_DestructibleComponent.h"
#include "RMC_DestructibleFractureSettings.h"

#include "PhysXPublic.h"

#include "Rendering/SkeletalMeshRenderData.h"
#include "Rendering/SkeletalMeshLODModel.h"
#include "Rendering/SkeletalMeshModel.h"

#if WITH_EDITOR
#include "ThirdPartyBuildOptimizationHelper.h"
#endif

// Sets default values
ARuntimeDestructibleActor::ARuntimeDestructibleActor(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	SceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Scene"));

	SourceRuntimeMesh = CreateDefaultSubobject<URuntimeMeshComponent>(TEXT("Source Runtime Mesh"));

	RMC_DestructibleMesh = CreateDefaultSubobject<URMC_DestructibleComponent>(TEXT("RMC Destructible Mesh"));

	RootComponent = SceneComponent;
	SourceRuntimeMesh->SetupAttachment(RootComponent);	
	RMC_DestructibleMesh->SetupAttachment(RootComponent);
}

// Called when the game starts or when spawned
void ARuntimeDestructibleActor::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void ARuntimeDestructibleActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);	
}

#if WITH_EDITOR	&& WITH_APEX

int32 BoneIdxToChunkIdx(int32 BoneIndex)
{
	return --BoneIndex;
}
int32 ChunkIdxToBoneIdx(int32 ChunkIndex)
{
	return ++ChunkIndex;
}

void ARuntimeDestructibleActor::BuildAdjacencyBuffer(const TArray<FSoftSkinVertex>& RM_SoftVertexBuffer, const uint32 NumUV, const TArray<int32>& RM_IndexBuffer, TArray<uint32>& OutRM_AdjacencyIndexBuffer)
{
	TArray<uint32> RM_Indices;
	RM_Indices.AddDefaulted(RM_IndexBuffer.Num());
	for (int i = 0; i < RM_IndexBuffer.Num(); i++)
	{
		RM_Indices[i] = RM_IndexBuffer[i];
	}	
		
	BuildOptimizationThirdParty::NvTriStripHelper::BuildSkeletalAdjacencyIndexBuffer(RM_SoftVertexBuffer, NumUV, RM_Indices, OutRM_AdjacencyIndexBuffer);
}

void GetChunkConvexHullVertices(TArray<TArray<FVector>>& DM_ConvexHullsToVertices, ExplicitHierarchicalMesh& DM_HierarchicalMesh, int32 ChunkIdx)
{
	int32_t* partIndex = DM_HierarchicalMesh.partIndex(ChunkIdx);
	if (partIndex != nullptr)
	{
		uint32_t convexHullCount = DM_HierarchicalMesh.convexHullCount(*partIndex);
		const ExplicitHierarchicalMesh::ConvexHull** ConvexHulls = DM_HierarchicalMesh.convexHulls(*partIndex);
		
		for (uint32_t Hull_Idx = 0; Hull_Idx < convexHullCount; Hull_Idx++)
		{
			const ExplicitHierarchicalMesh::ConvexHull* ConvexHull = ConvexHulls[Hull_Idx];
			uint32_t VertCount = ConvexHull->getVertexCount();
			TArray<FVector> DM_Vertices;
			for (uint32_t VertIdx = 0; VertIdx < VertCount; VertIdx++)
			{
				const PxVec3& Vertex = ConvexHull->getVertex(VertIdx);
				DM_Vertices.Add(P2UVector(Vertex));
			}
			DM_ConvexHullsToVertices.Add(DM_Vertices);
		}		
	}
}

bool ARuntimeDestructibleActor::GetChunkCollision(ExplicitHierarchicalMesh& DM_HierarchicalMesh, TArray<TArray<FVector>>& DM_ConvexHullsToVertices, int32 ChunkIdx)
{	
	uint32_t chunkCount = DM_HierarchicalMesh.chunkCount();	
	if (ChunkIdx<0 || (uint32_t)ChunkIdx>chunkCount - 1)
		return false;
	DM_ConvexHullsToVertices.Empty();
	GetChunkConvexHullVertices(DM_ConvexHullsToVertices, DM_HierarchicalMesh, ChunkIdx);	
	
	return true;
}

bool ARuntimeDestructibleActor::GetChunksCollision(ExplicitHierarchicalMesh& DM_HierarchicalMesh, TArray<TArray<TArray<FVector>>>& DM_ChunksToConvexHullsToVertices)
{
	DM_ChunksToConvexHullsToVertices.Empty();
	
	uint32_t chunkCount = DM_HierarchicalMesh.chunkCount();	
	for (uint32_t ChunkIdx = 0; ChunkIdx < chunkCount; ChunkIdx++)
	{
		TArray<TArray<FVector>> DM_ConvexHullsToVertices;
		GetChunkConvexHullVertices(DM_ConvexHullsToVertices, DM_HierarchicalMesh, ChunkIdx);
		DM_ChunksToConvexHullsToVertices.Add(DM_ConvexHullsToVertices);
	}
	return true;
}


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
		TArray<TArray<Vertex>> DM_PartSectionsToVertexBuffers;

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

		// Create Source RuntimeMeshComponent render data
		CreateRuntimeMeshRenderData(SourceRuntimeMesh, DM_PartSectionsToVertexBuffers, DM_PartSectionsToIndexBuffers, DM_PartSectionIndices,
			DM_OverrideMaterials, DM_Materials, NumUV, bOverrideMaterials, bCreateAdjBuffer);

		if (bCopyCollision)
		{
			/** Array of vertices of convex hulls of every chunk**/
			TArray<TArray<FVector>> DM_ChunkConvexHullsToVertices;

			//Get Chunk's Collision	
			GetChunkCollision(DM_HierarchicalMesh, DM_ChunkConvexHullsToVertices, ChunkIndex);			

			// Create Source RuntimeMeshComponent collision
			CreateRuntimeMeshCollision(SourceRuntimeMesh, DM_ChunkConvexHullsToVertices);
		}
		return true;
	}
	return false;
}

void ARuntimeDestructibleActor::GetChunkBuffers(TArray<TArray<Vertex>>& DM_SectionsToVertexBuffers, TArray<TArray<int32>>& DM_SectionsToIndexBuffers,
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
			TArray<Vertex>& DM_SectionVertexBuffer = DM_SectionsToVertexBuffers[LocalSectionIdx];
			//Final section's index buffer
			TArray<int32>& DM_SectionIndexBuffer = DM_SectionsToIndexBuffers[LocalSectionIdx];

			if (DM_SectionNonReducedVertexBuffer.Num() > 0)
			{
				TArray<PxU32> ReducedSectionIndices;
				int32 NonReducedVertCount = DM_SectionNonReducedVertexBuffer.Num();
				ReducedSectionIndices.SetNumUninitialized(NonReducedVertCount);

				const PxU32 ReducedSectionVertexCount = RenderMeshAssetAuthor->createReductionMap(ReducedSectionIndices.GetData(), DM_SectionNonReducedVertexBuffer.GetData(), NULL, (PxU32)NonReducedVertCount,
					PxVec3(0.0001f), 0.001f, 1.0f / 256.01f);

				DM_SectionVertexBuffer.Init(Vertex(), ReducedSectionVertexCount);

				DM_SectionIndexBuffer.Init(int32(), NonReducedVertCount);

				for (int32 OldIndex = 0; OldIndex < NonReducedVertCount; ++OldIndex)
				{
					const int32 NewIndex = ReducedSectionIndices[OldIndex];
					DM_SectionIndexBuffer[OldIndex] = NewIndex;
					DM_SectionVertexBuffer[NewIndex] = DM_SectionNonReducedVertexBuffer[OldIndex];	// This will copy over several times, but with the same (or close enough) data					
				}
			}
		}
	}
}


void ARuntimeDestructibleActor::GetChunkRenderData(TArray<TArray<Vertex>>& DM_SectionsToVertexBuffers, TArray<TArray<int32>>& DM_SectionsToIndexBuffers,
	TArray<int32>& DM_SectionIndices, ExplicitHierarchicalMesh& DM_HierarchicalMesh, int32 ChunkIdx)
{
	uint32 ChunkCount = DM_HierarchicalMesh.chunkCount();
	if (ChunkIdx<0 || (uint32)ChunkIdx>ChunkCount - 1)
		return;

	apex::RenderMeshAssetAuthoring* RenderMeshAssetAuthor = static_cast<apex::RenderMeshAssetAuthoring*>(apex::GetApexSDK()->createAssetAuthoring(RENDER_MESH_AUTHORING_TYPE_NAME));

	GetChunkBuffers(DM_SectionsToVertexBuffers, DM_SectionsToIndexBuffers, DM_SectionIndices, DM_HierarchicalMesh, RenderMeshAssetAuthor, ChunkIdx);
}

void ARuntimeDestructibleActor::GetChunksRenderData(TArray<TArray<TArray<Vertex>>>& DM_PartsToSectionsToVertexBuffers, TArray<TArray<TArray<int32>>>& DM_PartsToSectionsToIndexBuffers,
	TArray<TArray<int32>>& DM_PartsToSectionIndices, ExplicitHierarchicalMesh& DM_HierarchicalMesh)
{
	apex::RenderMeshAssetAuthoring* RenderMeshAssetAuthor = static_cast<apex::RenderMeshAssetAuthoring*>(apex::GetApexSDK()->createAssetAuthoring(RENDER_MESH_AUTHORING_TYPE_NAME));

	uint32 ChunkCount = DM_HierarchicalMesh.chunkCount();

	DM_PartsToSectionsToVertexBuffers.Init(TArray<TArray<Vertex>>(), ChunkCount);
	DM_PartsToSectionsToIndexBuffers.Init(TArray<TArray<int32>>(), ChunkCount);
	DM_PartsToSectionIndices.Init(TArray<int32>(), ChunkCount);

	for (uint32 ChunkIndex = 0; ChunkIndex < ChunkCount; ++ChunkIndex)
	{		
		TArray<TArray<Vertex>>& DM_SectionsToVertexBuffers = DM_PartsToSectionsToVertexBuffers[ChunkIndex];			
		TArray<TArray<int32>>& DM_SectionsToIndexBuffers = DM_PartsToSectionsToIndexBuffers[ChunkIndex];
		TArray<int32>& DM_SectionIndices = DM_PartsToSectionIndices[ChunkIndex];
		GetChunkBuffers(DM_SectionsToVertexBuffers, DM_SectionsToIndexBuffers, DM_SectionIndices, DM_HierarchicalMesh, RenderMeshAssetAuthor, ChunkIndex);		
	}
}

FVector ARuntimeDestructibleActor::PxVec3_To_FVector(const PxVec3& InVertex)
{
	//In URMC_DestructibleMesh::CreateSubmeshFromSMSection function vertices positions, normals, tangents and binormals Ys are multiplied by -1
	return FVector(InVertex.x, InVertex.y * -1, InVertex.z) ;
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

/**Creating RuntimeMesh Sections, tesselation triangles (if need) and setting materials **/
void ARuntimeDestructibleActor::CreateRuntimeMeshRenderData(URuntimeMeshComponent* RuntimeMeshComponent, const TArray<TArray<Vertex>>& DM_PartSectionsToVertexBuffers, const TArray<TArray<int32>>& DM_PartSectionsToIndexBuffers,
	const TArray<int32>& DM_PartSectionIndices, const TArray<UMaterialInterface*>& DM_OverrideMaterials, const TArray<FSkeletalMaterial>& DM_Materials, const uint32 NumUV, bool bOverrideMaterials, bool bCreateAdjBuffer)
{
	URuntimeMesh* RuntimeMesh = RuntimeMeshComponent->GetOrCreateRuntimeMesh();
	RuntimeMesh->ClearAllMeshSections();

	int32 SectionNum = DM_PartSectionsToVertexBuffers.Num();
	for (int32 LocalSectionIdx = 0; LocalSectionIdx < SectionNum; LocalSectionIdx++)
	{
		const TArray<Vertex>& DM_SectionVertexBuffer = DM_PartSectionsToVertexBuffers[LocalSectionIdx];
		TArray<FSoftSkinVertex> RM_SectionSoftVertexBuffer;
		if (bCreateAdjBuffer)
		{
			RM_SectionSoftVertexBuffer.Init(FSoftSkinVertex(), DM_SectionVertexBuffer.Num());
		}

		const TArray<int32>& RM_IndexBuffer = DM_PartSectionsToIndexBuffers[LocalSectionIdx];

		TArray<FVector> RM_Vertices;		
		TArray<FVector> RM_Normals;
		TArray<FVector2D> RM_UV0;
		TArray<FColor> RM_Colors;
		TArray<FRuntimeMeshTangent> RM_Tangents;
		TArray<FVector> RM_BiNormals;

		for (int i = 0; i < DM_SectionVertexBuffer.Num(); i++)
		{
			const Vertex& Vert = DM_SectionVertexBuffer[i];
			
			if (bCreateAdjBuffer)
			{
				FSoftSkinVertex SoftSkinVert = NvVertex_To_FSoftSkinVertex(Vert);

				RM_Vertices.Add(SoftSkinVert.Position);
				RM_Normals.Add(SoftSkinVert.TangentZ);
				RM_UV0.Add(SoftSkinVert.UVs[0]);
				RM_Colors.Add(SoftSkinVert.Color);
				RM_Tangents.Add(SoftSkinVert.TangentX);
				RM_BiNormals.Add(SoftSkinVert.TangentY);

				RM_SectionSoftVertexBuffer.Add(SoftSkinVert);
			}
			else
			{
				RM_Vertices.Add(PxVec3_To_FVector(Vert.position));
				RM_Normals.Add(PxVec3_To_FVector(Vert.normal));
				RM_UV0.Add(VertexUV_To_FVector2D(Vert.uv[0]));
				RM_Colors.Add(VertexColor_To_FColor(Vert.color));
				RM_Tangents.Add(PxVec3_To_FVector(Vert.tangent));
				RM_BiNormals.Add(PxVec3_To_FVector(Vert.binormal));
			}

		}

		RuntimeMesh->CreateMeshSection(LocalSectionIdx, RM_Vertices, RM_IndexBuffer, RM_Normals, RM_UV0, RM_Colors, RM_Tangents);

		//Set Materials
		if (bOverrideMaterials)
			RuntimeMeshComponent->SetMaterial(LocalSectionIdx, DM_OverrideMaterials[DM_PartSectionIndices[LocalSectionIdx]]);
		else
			RuntimeMeshComponent->SetMaterial(LocalSectionIdx, DM_Materials[DM_PartSectionIndices[LocalSectionIdx]].MaterialInterface);
		
		if (bCreateAdjBuffer)
		{
			TArray<uint32> RM_AdjacencyIndexBuffer;
			BuildAdjacencyBuffer(RM_SectionSoftVertexBuffer, NumUV, RM_IndexBuffer, RM_AdjacencyIndexBuffer);
			RuntimeMesh->SetSectionTessellationTriangles(LocalSectionIdx, RM_AdjacencyIndexBuffer);
		}
	}
}

void ARuntimeDestructibleActor::CreateRuntimeMeshCollision(URuntimeMeshComponent* RuntimeMeshComponent, const TArray<TArray<FVector>>& DM_ConvexHullsToVertices)
{
	URuntimeMesh* RuntimeMesh = RuntimeMeshComponent->GetOrCreateRuntimeMesh();
	RuntimeMesh->ClearAllConvexCollisionSections();
	for (int32 i = 0; i < DM_ConvexHullsToVertices.Num(); i++)
	{
		RuntimeMesh->AddConvexCollisionSection(DM_ConvexHullsToVertices[i]);
	}
}

bool ARuntimeDestructibleActor::CopyDestructibleMeshToRuntimeMeshComponent(URMC_DestructibleComponent* DestructibleMeshComponent,
	URuntimeMeshComponent* RuntimeMeshComponent, TArray <URuntimeMeshComponent*>& Chunks, bool bOverrideMaterials, bool bCopyCollision, bool bCreateAdjBuffer)
{
	if (DestructibleMeshComponent != nullptr)
	{
		URMC_DestructibleMesh* DestructibleMesh = DestructibleMeshComponent->GetDestructibleMesh();
		if (DestructibleMesh != nullptr)
		{
			uint32 NumUV;//TexCoordsNum						

			const FSkeletalMeshLODRenderData& LODRenderData = DestructibleMesh->GetResourceForRendering()->LODRenderData[0];
			
			NumUV = LODRenderData.GetNumTexCoords();//TexCoordsNum

			const TArray <UMaterialInterface*>& DM_OverrideMaterials = DestructibleMeshComponent->GetMaterials();			

			const TArray <FSkeletalMaterial>& DM_Materials = DestructibleMesh->Materials;		
			
			/**
			Vertex Buffers for sections of DestructibleMesh parts(chunks)
			**/
			TArray<TArray<TArray<Vertex>>> DM_PartsToSectionsToVertexBuffers;

			/**
			Index Buffers for sections of DestructibleMesh parts(chunks)
			**/
			TArray<TArray<TArray<int32>>> DM_PartsToSectionsToIndexBuffers;

			/**
			Section Indices of DestructibleMesh parts(chunks)
			**/
			TArray<TArray<int32>> DM_PartsToSectionIndices;
			
			URMC_DestructibleFractureSettings* DM_FractureSettings = DestructibleMesh->FractureSettings;
			DestructibleAssetAuthoring*	DM_ApexDestructibleAssetAuthoring = DM_FractureSettings->ApexDestructibleAssetAuthoring;
			const DestructibleAsset& DM_DestructibleAsset = *DestructibleMesh->ApexDestructibleAsset;			

			ExplicitHierarchicalMesh& DM_HierarchicalMesh = DM_ApexDestructibleAssetAuthoring->getExplicitHierarchicalMesh();

			if(!DM_HierarchicalMesh.chunkCount())
				DM_ApexDestructibleAssetAuthoring->importDestructibleAssetToRootMesh(DM_DestructibleAsset);

			GetChunksRenderData(DM_PartsToSectionsToVertexBuffers, DM_PartsToSectionsToIndexBuffers, DM_PartsToSectionIndices, DM_HierarchicalMesh);
						
			// Create Source RuntimeMeshComponent render data
			CreateRuntimeMeshRenderData(RuntimeMeshComponent, DM_PartsToSectionsToVertexBuffers[0], DM_PartsToSectionsToIndexBuffers[0], DM_PartsToSectionIndices[0],
				DM_OverrideMaterials, DM_Materials, NumUV, bOverrideMaterials, bCreateAdjBuffer);
			
			if (bCopyCollision)
			{			
				/** Array of vertices of convex hulls of every chunk**/
				TArray<TArray<TArray<FVector>>> DM_ChunksToConvexHullsToVertices;

				//Get Chunk's Collision	
				GetChunksCollision(DM_HierarchicalMesh, DM_ChunksToConvexHullsToVertices);

				const TArray<TArray<FVector>>& DM_ConvexHullsToVertices = DM_ChunksToConvexHullsToVertices[0];

				// Create Source RuntimeMeshComponent collision
				CreateRuntimeMeshCollision(RuntimeMeshComponent, DM_ConvexHullsToVertices);
			}
			return true;
		}
	}
	return false;
}
#endif // WITH_EDITOR && WITH_APEX


//bool ARuntimeDestructibleActor::CopyDestructibleMeshToRuntimeMeshComponent(URMC_DestructibleComponent* DestructibleMeshComponent,
//	URuntimeMeshComponent* RuntimeMeshComponent, TArray <URuntimeMeshComponent*>& Chunks, int32 LODIndex, bool bOverrideMaterials, bool bCopyCollision, bool bCreateAdjBuffer)
//{
//#if WITH_EDITOR	
//	if (DestructibleMeshComponent != nullptr)
//	{
//		URMC_DestructibleMesh* DestructibleMesh = DestructibleMeshComponent->GetDestructibleMesh();
//		if (DestructibleMesh != nullptr)
//		{
//			uint32 NumUV;//TexCoordsNum			
//
//			TArray<int32> DM_IndexBuffer;//Overall Indices
//
//			const FSkeletalMeshLODRenderData& LODRenderData = DestructibleMesh->GetResourceForRendering()->LODRenderData[LODIndex];
//
//			const TArray <UMaterialInterface*>& DM_OverrideMaterials = DestructibleMeshComponent->GetMaterials();
//			TMap <int32, TArray <UMaterialInterface*>> RM_BonesToSectionsToOverrideMaterials;
//
//			const TArray <FSkeletalMaterial>& DM_Materials = DestructibleMesh->Materials;
//			TMap <int32, TArray <FSkeletalMaterial>> RM_BonesToSectionsToMaterials;
//
//			NumUV = LODRenderData.GetNumTexCoords();//TexCoordsNum
//
//			const FRawStaticIndexBuffer16or32Interface* DM_OriginIndexBuffer = LODRenderData.MultiSizeIndexContainer.GetIndexBuffer();
//			int32 IndicesNum = DM_OriginIndexBuffer->Num();
//			DM_IndexBuffer.AddDefaulted(IndicesNum);
//			for (int i = 0; i < IndicesNum; i++)
//			{
//				DM_IndexBuffer[i] = DM_OriginIndexBuffer->Get(i);
//			}
//
//			FSkeletalMeshModel* DM_Model = DestructibleMesh->GetImportedModel();
//
//			const FSkeletalMeshLODModel& DM_ModelLod = DM_Model->LODModels[LODIndex];
//			const TArray<FSkelMeshSection>& DM_Sections = DM_ModelLod.Sections;//Skeletal Mesh Sections (overall chunks vertex data allocated in this sections, one section can contain 256 bones(chunks) ) 
//
//			/**
//			Overall TMap of SoftVertices rigidly weighted to bones
//			Key - bone index of DestructibleMesh,
//			Value: Key - origin section index, Value - TMap of SoftVertices rigidly weighted to bone (Key - Original Vertex Index in Overall IndexBuffer)
//			If Value.Num() > 1 - Chunk is multi-sectional
//			**/
//			TMap <int32, TMap<int32, TMap<int32, FSoftSkinVertex>>> DM_BonesToSectionsToIndicesToSoftVertices;
//
//			/**Key - bone index, Value - TArray of TArrays of Runtime Mesh IndexBuffers relative to SoftVertices rigidly weighted to bone (if Value.Num() > 1 - Chunk is multi-sectional) **/
//			TMap <int32, TArray<TArray<int32>>> RM_BonesToSectionsToIndexBuffers;
//
//			/**Key - bone index, Value - Runtime Mesh AdjacencyIndexBuffer to SoftVertices rigidly weighted to bone **/
//			TMap <int32, TArray<uint32>> RM_BonesToAdjacencyIndexBuffers;
//
//			/**
//			Bounds in DM_IndexBuffer of each destructible mesh section
//			innerarray[0] - start index
//			innerarray[1] - finish index
//			**/
//			TArray<TArray<int32>> DM_SectionsToIndexBufferBounds;
//
//
//			int32 DM_SectionsNum = DM_ModelLod.Sections.Num();
//			DM_SectionsToIndexBufferBounds.AddDefaulted(DM_SectionsNum);
//			for (int32 i = 0; i < DM_SectionsToIndexBufferBounds.Num(); i++)
//			{
//				DM_SectionsToIndexBufferBounds[i].AddDefaulted(2);
//			}
//			//Search RenderData for chunks
//			for (int32 SectionIndex = 0; SectionIndex < DM_SectionsNum; SectionIndex++)//First chunk data is in first section locaed expected 
//			{
//				const TArray<FSoftSkinVertex>& DM_SectionSoftVertices = DM_Sections[SectionIndex].SoftVertices;//chunks of current section vertex data (position, tangent, normal, binormal, influence bones and weights)
//				const TArray<FBoneIndexType>& DM_BoneMap = DM_Sections[SectionIndex].BoneMap;
//
//				int32 VertIndexOffset = DM_Sections[SectionIndex].BaseVertexIndex;//Vertices offset
//
//				uint8 LocalBoneIndex;
//
//				//get vertices rigidly weighted to bones
//				for (int32 i = 0; i < DM_SectionSoftVertices.Num(); i++)
//				{
//					if (DM_SectionSoftVertices[i].GetRigidWeightBone(LocalBoneIndex))
//					{
//						int32 OriginBoneIndex = DM_BoneMap[LocalBoneIndex];
//						/**
//						Key - origin section index,
//						Value - TMap of SoftVertices rigidly weighted to bone
//						*Key - Original Vertex Index in Overall IndexBuffer
//						If Array.Num() > 1 - Chunk is multi-sectional
//						**/
//						TMap<int32, TMap<int32, FSoftSkinVertex>>& DM_SectionsToIndicesToSoftVertices = DM_BonesToSectionsToIndicesToSoftVertices.FindOrAdd(OriginBoneIndex);
//
//						TMap<int32, FSoftSkinVertex>& DM_IndicesToSoftVertices = DM_SectionsToIndicesToSoftVertices.FindOrAdd(SectionIndex);
//
//						DM_IndicesToSoftVertices.Add(i + VertIndexOffset, DM_SectionSoftVertices[i]);
//					}
//				}
//
//				//Fill Section bounds
//				//Start and finish indices in DM_IndexBuffer
//				int32 StartIndex = DM_Sections[SectionIndex].BaseIndex;//Indices Offset
//				int32 FinishIndex = StartIndex + (DM_Sections[SectionIndex].NumTriangles * 3);
//				DM_SectionsToIndexBufferBounds[SectionIndex][0] = StartIndex;
//				DM_SectionsToIndexBufferBounds[SectionIndex][1] = FinishIndex;
//			}
//			//At this moment SoftVertices (with their origin indices) of each chunk are separated by bones they're rigidly weighted to and sections they're located in
//
//			/**Bones Indices extracted from DM_BonesToSectionsToIndicesToSoftVertices**/
//			TArray<int32> BoneIndices;
//			DM_BonesToSectionsToIndicesToSoftVertices.GenerateKeyArray(BoneIndices);
//
//			//Generate Values and Keys arrays from DM_BonesToSectionsToIndicesToSoftVertices TMap<int32, TMap<int32, FSoftSkinVertex>>
//			//Keys arrays will be used to faster generation of indexbuffers of each chunk
//			//And Fill Material Maps
//			TMap <int32, TArray<TMap<int32, FSoftSkinVertex>>> DM_BonesToSectionsToIndicesToSoftVerticesValues;
//			TMap <int32, TArray<int32>> DM_BonesToSectionIndicesKeys;
//			{
//				for (int32 i = 0; i < BoneIndices.Num(); i++)
//				{
//					TArray<TMap<int32, FSoftSkinVertex>> DM_SectionsToIndicesToSoftVertices;
//					DM_BonesToSectionsToIndicesToSoftVertices[BoneIndices[i]].GenerateValueArray(DM_SectionsToIndicesToSoftVertices);
//					DM_BonesToSectionsToIndicesToSoftVerticesValues.Add(BoneIndices[i], DM_SectionsToIndicesToSoftVertices);
//
//					//TArray of origin section indices of current chunk
//					TArray<int32> DM_SectionIndices;
//					DM_BonesToSectionsToIndicesToSoftVertices[BoneIndices[i]].GenerateKeyArray(DM_SectionIndices);
//					DM_BonesToSectionIndicesKeys.Add(BoneIndices[i], DM_SectionIndices);
//
//					if (bOverrideMaterials)
//					{
//						//TMap <int32, TArray <UMaterialInterface*>> RM_BonesToSectionsToOverrideMaterials;
//						TArray <UMaterialInterface*> RM_SectionsToMaterials;
//						for (int32 j = 0; j < DM_SectionIndices.Num(); j++)
//						{
//							RM_SectionsToMaterials.Add(DM_OverrideMaterials[DM_Sections[DM_SectionIndices[j]].MaterialIndex]);
//						}
//						RM_BonesToSectionsToOverrideMaterials.Add(BoneIndices[i], RM_SectionsToMaterials);
//					}
//					else
//					{
//						//TMap <int32, TArray <FSkeletalMaterial>> RM_BonesToSectionsToMaterials;
//						TArray <FSkeletalMaterial> RM_SectionsToMaterials;
//						for (int32 j = 0; j < DM_SectionIndices.Num(); j++)
//						{
//							RM_SectionsToMaterials.Add(DM_Materials[DM_Sections[DM_SectionIndices[j]].MaterialIndex]);
//						}
//						RM_BonesToSectionsToMaterials.Add(BoneIndices[i], RM_SectionsToMaterials);
//					}
//
//				}
//			}
//			DM_BonesToSectionsToIndicesToSoftVertices.Empty();
//
//			//Correction Vert indices
//			{
//				/**Key - bone index, Value - TArray of TMaps of origin indices of SoftVertices and indices recalculated relative to chunk**/
//				TMap<int32, TArray<TMap<int32, int32>>> BonesToSectionsToOldToNewIndices;
//
//				for (int32 i = 0; i < BoneIndices.Num(); i++)
//				{
//					const TArray<TMap<int32, FSoftSkinVertex>>& DM_SectionsToIndicesToSoftVertices = DM_BonesToSectionsToIndicesToSoftVerticesValues[BoneIndices[i]];
//					TArray<TMap<int32, int32>>& SectionsToOldToNewIndices = BonesToSectionsToOldToNewIndices.Add(BoneIndices[i]);
//					for (int32 j = 0; j < DM_SectionsToIndicesToSoftVertices.Num(); j++)
//					{
//						TMap<int32, int32> OldToNewIndices;
//						TArray<int32> OldKeys;
//						DM_SectionsToIndicesToSoftVertices[j].GenerateKeyArray(OldKeys);
//						for (int32 k = 0; k < OldKeys.Num(); k++)
//						{
//							OldToNewIndices.Add(OldKeys[k], k);
//						}
//						SectionsToOldToNewIndices.Add(OldToNewIndices);
//					}
//				}
//
//				//Initialize TMap <int32, TArray<TArray<int32>>> RM_BonesToSectionsToIndexBuffers by default values;
//				{
//					for (int32 j = 0; j < BoneIndices.Num(); j++)
//					{
//						TArray<TArray<int32>>& RM_SectionsToIndexBuffers = RM_BonesToSectionsToIndexBuffers.Add(BoneIndices[j]);
//						const TArray<TMap<int32, FSoftSkinVertex>>& DM_SectionsToIndicesToSoftVertices = DM_BonesToSectionsToIndicesToSoftVerticesValues[BoneIndices[j]];
//						RM_SectionsToIndexBuffers.AddDefaulted(DM_SectionsToIndicesToSoftVertices.Num());
//					}
//				}
//
//				//get triangles rigidly weighted to bones (generate indexbuffers for each chunk)			
//				for (int32 i = 0; i < DM_IndexBuffer.Num(); i += 3)
//				{
//					//check if DM_BonesToIndicesToSoftVertices contains first index of triangle
//					//and if it is - add all triangle with recalculated indices to indexbuffer of chunk 
//					for (int32 j = 0; j < BoneIndices.Num(); j++)
//					{
//						int32 LocalSectionIndex = GetSectionIndexByIndexBufferBounds(i, BoneIndices[j], DM_BonesToSectionIndicesKeys, DM_SectionsToIndexBufferBounds);
//
//						if (LocalSectionIndex != -1)
//						{
//							const TArray<TMap<int32, FSoftSkinVertex>>& DM_SectionsToIndicesToSoftVertices = DM_BonesToSectionsToIndicesToSoftVerticesValues[BoneIndices[j]];
//							const TArray<TMap<int32, int32>>& SectionsToOldToNewIndices = BonesToSectionsToOldToNewIndices[BoneIndices[j]];
//							TArray<TArray<int32>>& RM_SectionsToIndexBuffers = RM_BonesToSectionsToIndexBuffers[BoneIndices[j]];
//
//							const TMap<int32, FSoftSkinVertex>& DM_IndicesToSoftVertices = DM_SectionsToIndicesToSoftVertices[LocalSectionIndex];
//							if (DM_IndicesToSoftVertices.Contains(DM_IndexBuffer[i]))
//							{
//								//Chunk IndexBuffer
//								TArray<int32>& RM_IndexBuffer = RM_SectionsToIndexBuffers[LocalSectionIndex];
//								for (int32 l = 0; l < 3; l++)
//								{
//									int32 OldIndex = DM_IndexBuffer[i + l];
//
//									RM_IndexBuffer.Add(SectionsToOldToNewIndices[LocalSectionIndex][OldIndex]);
//								}
//
//							}
//						}
//						//bool TriIsFound = false;
//						//const TArray<TMap<int32, FSoftSkinVertex>>& DM_SectionsToIndicesToSoftVertices = DM_BonesToSectionsToIndicesToSoftVerticesValues[BoneIndices[j]];
//						//const TArray<TMap<int32, int32>>& SectionsToOldToNewIndices = BonesToSectionsToOldToNewIndices[BoneIndices[j]];
//						//TArray<TArray<int32>>& RM_SectionsToIndexBuffers = RM_BonesToSectionsToIndexBuffers[BoneIndices[j]];
//						//for (int32 k = 0; k < DM_SectionsToIndicesToSoftVertices.Num(); k++)
//						//{
//						//	const TMap<int32, FSoftSkinVertex>& DM_IndicesToSoftVertices = DM_SectionsToIndicesToSoftVertices[k];
//						//	if (DM_IndicesToSoftVertices.Contains(DM_IndexBuffer[i]))
//						//	{
//						//		//Chunk IndexBuffer
//						//		TArray<int32>& RM_IndexBuffer = RM_SectionsToIndexBuffers[k];
//						//		for (int32 l = 0; l < 3; l++)
//						//		{
//						//			int32 OldIndex = DM_IndexBuffer[i + l];			
//						//			 
//						//			RM_IndexBuffer.Add(SectionsToOldToNewIndices[k][OldIndex]);																		
//						//		}
//						//		TriIsFound = true;
//						//	}
//						//	if (TriIsFound)
//						//		break;
//						//}
//						//if (TriIsFound)
//						//	break;
//					}
//				}
//			}
//
//
//			URuntimeMesh* RuntimeMesh = RuntimeMeshComponent->GetOrCreateRuntimeMesh();
//			RuntimeMesh->ClearAllMeshSections();
//
//			TArray<TArray<FSoftSkinVertex>> DM_SectionsToSoftVertices;
//			TArray<TArray<int32>> RM_SectionsToIndexBuffer;
//
//			TArray <UMaterialInterface*> RM_SectionsToOverrideMaterials;
//			TArray <FSkeletalMaterial> RM_SectionsToMaterials;
//
//			for (int i = 0; i < DM_BonesToSectionsToIndicesToSoftVerticesValues[1].Num(); i++)
//			{
//				TArray<FSoftSkinVertex> SoftVertices;
//				TArray<int32>IndexBuffer;
//				DM_BonesToSectionsToIndicesToSoftVerticesValues[1][i].GenerateValueArray(SoftVertices);
//				DM_SectionsToSoftVertices.Add(SoftVertices);
//
//				RM_SectionsToIndexBuffer.Add(RM_BonesToSectionsToIndexBuffers[1][i]);
//
//				if (bOverrideMaterials)
//					RM_SectionsToOverrideMaterials.Add(RM_BonesToSectionsToOverrideMaterials[1][i]);
//				else
//					RM_SectionsToMaterials.Add(RM_BonesToSectionsToMaterials[1][i]);
//			}
//
//
//
//
//			for (int32 i = 0; i < DM_SectionsToSoftVertices.Num(); i++)
//			{
//				if (DM_SectionsToSoftVertices[i].Num() && RM_SectionsToIndexBuffer.Num())
//				{
//					TArray<FSoftSkinVertex>& RM_SoftVertexBuffer = DM_SectionsToSoftVertices[i];
//					TArray<int32>& RM_IndexBuffer = RM_SectionsToIndexBuffer[i];
//
//					TArray<FVector> RM_Vertices;
//					TArray<FVector> RM_Normals;
//					TArray<FVector2D> RM_UV0;
//					TArray<FColor> RM_Colors;
//					TArray<FRuntimeMeshTangent> RM_Tangents;
//					TArray<FVector> RM_BiNormals;
//
//					for (int i = 0; i < RM_SoftVertexBuffer.Num(); i++)
//					{
//						const FSoftSkinVertex& SoftSkinVert = RM_SoftVertexBuffer[i];
//
//						RM_Vertices.Add(SoftSkinVert.Position);
//						RM_Normals.Add(SoftSkinVert.TangentZ);
//						RM_UV0.Add(SoftSkinVert.UVs[0]);
//						RM_Colors.Add(SoftSkinVert.Color);
//						RM_Tangents.Add(SoftSkinVert.TangentX);
//						RM_BiNormals.Add(SoftSkinVert.TangentY);
//					}
//
//					RuntimeMesh->CreateMeshSection(i, RM_Vertices, RM_IndexBuffer, RM_Normals, RM_UV0, RM_Colors, RM_Tangents);
//
//
//
//					/*if (bCreateAdjBuffer)
//					{
//						BuildAdjacencyBuffer(RM_SoftVertexBuffer, NumUV, RM_IndexBuffer, RM_AdjacencyIndexBuffer);
//						RuntimeMesh->SetSectionTessellationTriangles(i, RM_AdjacencyIndexBuffer);
//					}*/
//
//					if (bOverrideMaterials)
//						RuntimeMeshComponent->SetMaterial(i, RM_SectionsToOverrideMaterials[i]);
//					else
//						RuntimeMeshComponent->SetMaterial(i, RM_SectionsToMaterials[i].MaterialInterface);
//				}
//			}
//			if (bCopyCollision)
//			{
//				RuntimeMesh->ClearAllConvexCollisionSections();
//
//				/** Array of vertices of convex hulls of every chunk**/
//				TArray<TArray<TArray<FVector>>> DM_ChunksToConvexHullsToVertices;
//
//				//Get Chunk's Collision	
//				GetChunksCollision(DestructibleMeshComponent, DM_ChunksToConvexHullsToVertices);
//
//				const TArray<TArray<FVector>>& DM_ConvexHullsToVertices = DM_ChunksToConvexHullsToVertices[0];
//				for (int32 i = 0; i < DM_ConvexHullsToVertices.Num(); i++)
//				{
//					RuntimeMesh->AddConvexCollisionSection(DM_ConvexHullsToVertices[i]);
//				}
//			}
//			return true;
//		}
//	}
//
//	return false;
//#endif
//	return false;
//}

