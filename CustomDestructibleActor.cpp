// Fill out your copyright notice in the Description page of Project Settings.

#include "CustomDestructibleActor.h"

//#include "RawMesh/Public/RawMesh.h"

#include "Runtime/CoreUObject/Public/UObject/ConstructorHelpers.h"

#include "Runtime/CoreUObject/Public/UObject/UObjectGlobals.h"
#include "Runtime/Core/Public/Containers/Array.h"


//"E:/Program Files/Epic Games/UE_4.20/Engine/Source/Runtime/Engine/Classes/AI/Navigation/NavCollisionBase.h"
#include "PhysXPublic.h"//P2UVector
#include "ThirdParty/PhysX3/PxShared/include/foundation/PxVec3.h"//PxVec3
#include "PhysicsPublic.h"
#include "PhysXIncludes.h"

//E:\Program Files\Epic Games\UE_4.20\Engine\Source\ThirdParty\PhysX3\PhysX_3.4\Include\geometry\PxTriangleMesh.h
#include "ThirdParty/PhysX3/PhysX_3.4/Include/geometry/PxTriangleMesh.h"

//E:\Program Files\Epic Games\UE_4.20\Engine\Source\ThirdParty\PhysX3\PxShared\include\foundation\PxSimpleTypes.h
//ThirdParty/PhysX3/PhysX_3.4/Include/foundation/PxSimpleTypes.h
#include "ThirdParty/PhysX3/PxShared/Include/foundation/PxSimpleTypes.h"

#include "Engine/StaticMesh.h"//UStaticMesh Class
//E:\Program Files\Epic Games\UE_4.20\Engine\Source\Runtime\Engine\Classes\Components\StaticMeshComponent.h
#include "Components/StaticMeshComponent.h"
//E:\Program Files\Epic Games\UE_4.20\Engine\Source\Runtime\Engine\Classes\PhysicsEngine\BodyInstance.h
#include "PhysicsEngine/BodyInstance.h"

#include "PhysicsEngine/BodySetup.h"


#include "RuntimeMeshComponent.h"
#include "RuntimeMesh.h"

//E:\ProgramFiles\EpicGames\UE_4.20\Engine\Source\ThirdParty\PhysX3\PhysX_3.4\Include\geometry\PxConvexMesh.h
#include "ThirdParty/PhysX3/PhysX_3.4/Include/geometry/PxConvexMesh.h"

#include "time.h"
#define QUICKHULL_IMPLEMENTATION

#include "QuickHull.h"

#define MIN_HULL_VERT_DISTANCE		(0.1f)
#define MIN_HULL_VALID_DIMENSION	(0.5f)




// Sets default values
ACustomDestructibleActor::ACustomDestructibleActor()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;


	BaseStaticMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>("Base Static Mesh Component");
	static ConstructorHelpers::FObjectFinder<UStaticMesh> SphereVisualAsset(TEXT("/Game/StarterContent/Shapes/Shape_Cube.Shape_Cube"));

	BaseStaticMeshComponent->SetStaticMesh(SphereVisualAsset.Object);



	SubStaticMeshComponents.Add(CreateDefaultSubobject<UStaticMeshComponent>("Sub Static Mesh Component 0"));


	RootComponent = BaseStaticMeshComponent;
	SubStaticMeshComponents[0]->SetupAttachment(BaseStaticMeshComponent);
	BaseStaticMeshComponent->SetSimulatePhysics(true);
}

// Called when the game starts or when spawned
void ACustomDestructibleActor::BeginPlay()
{
	Super::BeginPlay();

}

// Called every frame
void ACustomDestructibleActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}
//int32 ACustomDestructibleActor::AddConvexCollisionSection(FKConvexElem ConvexShape)
//{
//	check(IsInGameThread());
//	return 2;
//}


//void ACustomDestructibleActor::CopyCollisionFromStaticMesh(UStaticMesh* StaticMesh, URuntimeMesh* RuntimeMesh)
//{
//	// Clear any existing collision hulls
//	RuntimeMesh->ClearAllConvexCollisionSections();
//
//	if (StaticMesh->BodySetup != nullptr)
//	{
//		// Iterate over all convex hulls on static mesh..
//		const int32 NumConvex = StaticMesh->BodySetup->AggGeom.ConvexElems.Num();
//		for (int ConvexIndex = 0; ConvexIndex < NumConvex; ConvexIndex++)
//		{
//			// Copy convex verts
//			FKConvexElem MeshConvex = StaticMesh->BodySetup->AggGeom.ConvexElems[ConvexIndex];
//
//			//RuntimeMesh->AddConvexCollisionSection(MeshConvex.VertexData);
//
//			/////////////////////////////////////////////////////////////////////////////////////////////
//			int32 I=ACustomDestructibleActor::AddConvexCollisionSection(MeshConvex);
//			/////////////////////////////////////////////////////////////////////////////////////////////
//		}
//
//	}
//}
//
//int32 ACustomDestructibleActor::AddConvexCollisionSection(/*TArray<FVector> ConvexVerts*/FKConvexElem ConvexShape)
//{
//	SCOPE_CYCLE_COUNTER(STAT_RuntimeMesh_AddConvexCollisionSection);
//
//	FRuntimeMeshScopeLock Lock(SyncRoot);	
//
//	int32 NewIndex = 0;
//	while (ConvexCollisionSections.Contains(NewIndex))
//	{
//		NewIndex++;
//	}
//
//	//auto& NewConvexShape = ConvexCollisionShapes.Add(NewIndex);
//	auto& ConvexSection = ConvexCollisionSections.Add(NewIndex);
//
//	ConvexShape.GetPlanes();
//
//	ConvexSection.VertexBuffer = ConvexShape.VertexData;
//	ConvexSection.BoundingBox = ConvexShape.ElemBox;
//
//	MarkCollisionDirty();
//
//	return NewIndex;
//}
//
//void ACustomDestructibleActor::SetCollisionConvexMeshes(const TArray<FKConvexElem>& ConvexMeshes)
//{
//	SCOPE_CYCLE_COUNTER(STAT_RuntimeMesh_ClearAllConvexCollisionSections);
//
//	FRuntimeMeshScopeLock Lock(SyncRoot);
//	
//
//	ConvexCollisionSections.Empty();
//	ConvexCollisionShapes.Empty();
//
//	int32 Index = 0;
//	for (const auto& ConvexShape : ConvexMeshes)
//	{
//		auto& NewConvexShape = ConvexCollisionShapes.FindOrAdd(Index);
//		auto& ConvexSection = ConvexCollisionSections.FindOrAdd(Index++);
//
//		NewConvexShape = ConvexShape;
//
//		ConvexSection.VertexBuffer = NewConvexShape.VertexData;
//		ConvexSection.BoundingBox = NewConvexShape.ElemBox;
//	}
//
//	MarkCollisionDirty();
//}
//
//
//int32 ACustomDestructibleActor::CompareBoxPlane(const FBox& InBox, const FPlane& InPlane)
//{
//	return 0;
//}
//
//void ACustomDestructibleActor::SliceConvexElem(FKConvexElem ConvexSection, const FPlane& SlicePlane, TArray<FKConvexElem>& OutConvexShapes)
//{
//	FPlane SlicePlaneFlipped = SlicePlane.Flip();
//
//	// Get set of planes that make up hull
//	TArray<FPlane> ConvexPlanes;
//	ConvexSection.GetPlanes(ConvexPlanes);
//
//	if (ConvexPlanes.Num() >= 4)
//	{
//		// Add on the slicing plane (need to flip as it culls geom in the opposite sense to our geom culling code)
//		ConvexPlanes.Add(SlicePlaneFlipped);
//
//		// Create output convex based on new set of planes
//		FKConvexElem SlicedElem;
//		bool bSuccess = SlicedElem.HullFromPlanes(ConvexPlanes, ConvexSection.VertexData);
//		if (bSuccess)
//		{
//			OutConvexShapes.Add(SlicedElem);
//		}
//	}
//}
//
//FKConvexElem* CreateFKConvexElem(FRuntimeMeshCollisionConvexMesh* ConvexSection)
//{
//	FKConvexElem SlicedElem;
//	bool bSuccess = SlicedElem.HullFromPlanes(ConvexSection->ConvexPlanes, ConvexSection->VertexBuffer);
//	if (bSuccess)
//	{
//		return &SlicedElem;
//	}
//	return nullptr;
//}
//
//void ACustomDestructibleActor::SliceRuntimeMeshConvexCollision(URuntimeMesh* InRuntimeMesh, URuntimeMesh* OutOtherHalf, FVector PlanePosition, FVector PlaneNormal)
//{
//	bool bCreateOtherHalf = OutOtherHalf != nullptr;
//
//	PlaneNormal.Normalize();
//	FPlane SlicePlane(PlanePosition, PlaneNormal);
//
//
//	// Array of sliced collision shapes
//	TArray<FKConvexElem> SlicedCollision;
//	TArray<FKConvexElem> OtherSlicedCollision;
//
//	
//	//UBodySetup* BodySetup = InRuntimeMesh->GetBodySetup();
//
//	FRuntimeMeshDataRef OldInRuntimeMeshData = InRuntimeMesh->GetRuntimeMeshData();
//
//
//
//	if (&OldInRuntimeMeshData != nullptr)
//	{
//		for (int32 ConvexIndex = 0; ConvexIndex </* OldInRuntimeMeshData->NumConvexCollisionSections()*/ 1; ConvexIndex++)
//		{
//			FKConvexElem* ConvexCollisionSection;
//
//			int32 BoxCompare = CompareBoxPlane(ConvexCollisionSection->ElemBox, SlicePlane);
//
//			// If box totally clipped, add to other half (if desired)
//			if (BoxCompare == -1)
//			{
//				if (bCreateOtherHalf)
//				{
//					OtherSlicedCollision.Add(*ConvexCollisionSection);
//				}
//			}
//			// If box totally valid, just keep mesh as is
//			else if (BoxCompare == 1)
//			{
//				SlicedCollision.Add(*ConvexCollisionSection);
//			}
//			// Need to actually slice the convex shape
//			else
//			{
//				SliceConvexElem(ConvexCollisionSection, SlicePlane, SlicedCollision);
//
//				// Slice again to get the other half of the collision, if desired
//				if (bCreateOtherHalf)
//				{
//					SliceConvexElem(ConvexCollisionSection, SlicePlane.Flip(), OtherSlicedCollision);
//				}
//			}
//		}
//
//		// Update collision of runtime mesh
//		InRuntimeMesh->SetCollisionConvexMeshes(SlicedCollision);
//
//		// Set collision for other mesh
//		if (bCreateOtherHalf)
//		{
//			OutOtherHalf->SetCollisionConvexMeshes(OtherSlicedCollision);
//		}
//	}
//}
//
//FRuntimeMeshCollisionConvexMesh* ACustomDestructibleActor::GetConvexCollisionSection(int32 Index)
//{
//
//	if (ConvexCollisionSections.Contains(Index))
//		return ConvexCollisionSections.Find(Index);
//	return nullptr;
//
//}
//
//struct FRuntimeConvexCollisionMesh_OLD
//{
//	TArray<FVector> VertexData;
//	FBox ElemBox;
//	FTransform Transform;
//	physx::PxConvexMesh*   ConvexMesh;
//	physx::PxConvexMesh*   ConvexMeshNegX;
//
//	friend FArchive& operator <<(FArchive& Ar, FRuntimeConvexCollisionMesh_OLD& Section)
//	{
//		Ar << Section.VertexData;
//		Ar << Section.ElemBox;
//		Ar << Section.Transform;
//		Ar << Section.ConvexMesh;
//		Ar << Section.ConvexMeshNegX;
//		
//		return Ar;
//	}
//};

void ACustomDestructibleActor::AddVertexIfNotPresent(TArray<FVector> &Vertices, const FVector& NewVertex)
{
	bool bIsPresent = 0;

	for (int32 i = 0; i < Vertices.Num() && !bIsPresent; i++)
	{
		const float DiffSqr = (NewVertex - Vertices[i]).SizeSquared();
		if (DiffSqr < MIN_HULL_VERT_DISTANCE * MIN_HULL_VERT_DISTANCE)
		{
			bIsPresent = true;
		}
	}

	if (!bIsPresent)
	{
		Vertices.Add(NewVertex);
	}
}

void ACustomDestructibleActor::RemoveDuplicateVerts(TArray<FVector>& InVerts)
{
	TArray<FVector> BackupVerts = InVerts;
	InVerts.Empty();

	for (int32 i = 0; i < BackupVerts.Num(); i++)
	{
		AddVertexIfNotPresent(InVerts, BackupVerts[i]);
	}
}


float ACustomDestructibleActor::DistanceToLine(const FVector& LineStart, const FVector& LineEnd, const FVector& Point)
{
	const FVector StartToEnd = LineEnd - LineStart;
	const FVector PointToStart = LineStart - Point;

	const FVector Cross = StartToEnd ^ PointToStart;
	return Cross.Size() / StartToEnd.Size();
}

bool ACustomDestructibleActor::EnsureHullIsValid(TArray<FVector>& InVerts)
{
	RemoveDuplicateVerts(InVerts);

	if (InVerts.Num() < 3)
	{
		return false;
	}

	// Take any vert. In this case - the first one.
	const FVector FirstVert = InVerts[0];

	// Now find vert furthest from this one.
	float FurthestDistSqr = 0.f;
	int32 FurthestVertIndex = INDEX_NONE;
	for (int32 i = 1; i < InVerts.Num(); i++)
	{
		const float TestDistSqr = (InVerts[i] - FirstVert).SizeSquared();
		if (TestDistSqr > FurthestDistSqr)
		{
			FurthestDistSqr = TestDistSqr;
			FurthestVertIndex = i;
		}
	}

	// If smallest dimension is too small - hull is invalid.
	if (FurthestVertIndex == INDEX_NONE || FurthestDistSqr < FMath::Square(MIN_HULL_VALID_DIMENSION))
	{
		return false;
	}

	// Now find point furthest from line defined by these 2 points.
	float ThirdPointDist = 0.f;
	int32 ThirdPointIndex = INDEX_NONE;
	for (int32 i = 1; i < InVerts.Num(); i++)
	{
		if (i != FurthestVertIndex)
		{
			const float TestDist = DistanceToLine(FirstVert, InVerts[FurthestVertIndex], InVerts[i]);
			if (TestDist > ThirdPointDist)
			{
				ThirdPointDist = TestDist;
				ThirdPointIndex = i;
			}
		}
	}

	// If this dimension is too small - hull is invalid.
	if (ThirdPointIndex == INDEX_NONE || ThirdPointDist < MIN_HULL_VALID_DIMENSION)
	{
		return false;
	}

	// Now we check each remaining point against this plane.

	// First find plane normal.
	const FVector Dir1 = InVerts[FurthestVertIndex] - InVerts[0];
	const FVector Dir2 = InVerts[ThirdPointIndex] - InVerts[0];
	FVector PlaneNormal = Dir1 ^ Dir2;
	const bool bNormalizedOk = PlaneNormal.Normalize();
	if (!bNormalizedOk)
	{
		return false;
	}

	// Now iterate over all remaining vertices.
	float MaxThickness = 0.f;
	for (int32 i = 1; i < InVerts.Num(); i++)
	{
		if ((i != FurthestVertIndex) && (i != ThirdPointIndex))
		{
			const float PointPlaneDist = FMath::Abs((InVerts[i] - InVerts[0]) | PlaneNormal);
			MaxThickness = FMath::Max(PointPlaneDist, MaxThickness);
		}
	}

	if (MaxThickness < MIN_HULL_VALID_DIMENSION)
	{
		return false;
	}

	return true;
}

bool ACustomDestructibleActor::IsNewCombination(TArray<int> Combination, TArray<TArray<int>>& CheckedCombinations)
{
	const int Size = 3;	

	int ArraySize = CheckedCombinations.Num();
	if (ArraySize==0)
	{		
		CheckedCombinations.Add(Combination);
		return true;
	}
	else
	{		
		for (int i = 0; i < CheckedCombinations.Num(); i++)
		{			
			bool AFound, BFound, CFound;
			AFound = BFound = CFound = false;
			for (int j = 0; j < Size; j++)
			{
					
				if (!AFound&&CheckedCombinations[i][j] == Combination[0])
				{
					AFound = true;
				}
				if (!BFound&&CheckedCombinations[i][j] == Combination[1])
				{
					BFound = true;
				}
				if (!CFound&&CheckedCombinations[i][j] == Combination[2])
				{
					CFound = true;
				}
			}		
			if (AFound&&BFound&&CFound)
				return false;
		}
		CheckedCombinations.Add(Combination);
		return true;
	}
	return false;
}

bool ACustomDestructibleActor::PointsOfPlane(TArray<TArray<FVector>>PlanesAndTheirPoints, FVector A, FVector B, FVector C)
{
	int ArraySize = PlanesAndTheirPoints.Num();
	if (!ArraySize)
	{
		return false;
	}
	else
	{
		for (int i = 0; i < PlanesAndTheirPoints.Num(); i++)
		{			
			bool AFound, BFound, CFound;
			AFound = BFound = CFound = false;
			for (int j = 0; j < PlanesAndTheirPoints[i].Num(); j++)
			{
				if (!AFound&&PlanesAndTheirPoints[i][j] == A)
				{					
					AFound = true;
				}
				if (!BFound&&PlanesAndTheirPoints[i][j] == B)
				{					
					BFound = true;
				}
				if (!CFound&&PlanesAndTheirPoints[i][j] == C)
				{					
					CFound = true;
				}

				if (AFound&&BFound&&CFound)
					return true;
			}
		}
	}
	return false;
}


//Check vectors for collinearity
bool ACustomDestructibleActor::PointsAreOnOneLine(FVector A, FVector B, FVector C)
{
	FVector AB = B - A;
	FVector AC = C - A;
	if (AB.IsZero() || AC.IsZero())
		return true;

	float Coef=0;
	if (AB.X != 0)
	{
		Coef = AC.X / AB.X;
	}
	else if(AB.Y != 0)
	{
		Coef = AC.Y / AB.Y;
	}
	else if (AB.Z != 0)
	{
		Coef = AC.Z / AB.Z;
	}

	FVector Temp = AB * Coef;
	if (AC == Temp)
		return true;	
		
	return false;
}

//return true and add array of points to planes and points array if all points are on one side of plane
//if point is on the plane - adding it to plane vertices
bool ACustomDestructibleActor::PlaneIsValid(FVector A, FVector B, FVector C, TArray<FVector> HullVertices, TArray<TArray<FVector>>& PlanesAndTheirPoints, TArray<FPlane>& Planes,bool& AllPointsAreOnPlane)
{
	
	if (PointsAreOnOneLine(A,B,C))
		return false;

	TArray<FVector> PlaneVertices;
	PlaneVertices.Add(A);
	PlaneVertices.Add(B);
	PlaneVertices.Add(C);

	FPlane Plane = FPlane(A, B, C);

	float Distance = 0;

	bool FirstPointOutOfOlaneDetected = false;
	bool PointsShouldBeInFrontOfPlane = false;

	for (int i = 0; i < HullVertices.Num(); i++)
	{
		if (HullVertices[i] == A || HullVertices[i] == B || HullVertices[i] == C)
			continue;		
		
		Distance = Plane.PlaneDot(HullVertices[i]);
		
		//Point On The Plane
		if (Distance == 0)
		{
			PlaneVertices.Add(HullVertices[i]);
		}
		else
		{
			//detecting position of first point not on plane
			//plane is valid only if all points are on one side of plane
			if (!FirstPointOutOfOlaneDetected)
			{
				FirstPointOutOfOlaneDetected = true;
				if (Distance > 0)
					PointsShouldBeInFrontOfPlane = true;
			}
			else
			{
				if ((Distance > 0 && !PointsShouldBeInFrontOfPlane)||(Distance < 0 && PointsShouldBeInFrontOfPlane))
					return false;
			}
		}		
	}
	//all points are on plane
	//if so use doublesided geometry for convex
	if (!FirstPointOutOfOlaneDetected)
	{
		AllPointsAreOnPlane = true;
	}

	//all points are in front of plane	
	if (FirstPointOutOfOlaneDetected&&PointsShouldBeInFrontOfPlane)
	{
		Plane.Flip();
	}


	PlanesAndTheirPoints.Add(PlaneVertices);
	Planes.Add(Plane);
	return true;
}

float ACustomDestructibleActor::DegreesAngleBetweenVerts(const FVector& A, const FVector& B)
{
	return (FMath::Acos((A|B) / (A.Size()*B.Size())))*180/PI;
}

float ACustomDestructibleActor::CosAngleBetweenVerts(const FVector& A, const FVector& B)
{
	return ((A | B) / (A.Size()*B.Size()));
}

float ACustomDestructibleActor::SinAngleBetweenVerts(const FVector& A, const FVector& B)
{
	return (FMath::Sqrt(1-FMath::Square(CosAngleBetweenVerts(A,B))));
}

template<typename T> int  ACustomDestructibleActor::GetIndex(const TArray<T>& Array, T& Elem)
{	
	for (int i = 0; i < Array.Num(); i++)
	{
		if (Array[i] == Elem)
			return i;
	}
	return -1;
}

void ACustomDestructibleActor::FindEdgesOfLine(const FVector& A, const FVector& B,const TArray<FVector>& LinePoints, TArray<FVector>& NewVertices, TArray<FVector>& ExcessPoints, int& NewAIndex, int& NewBIndex)
{
	int I = 0, J = 0;
	FindEdgeIndices(I, J, LinePoints);
	FVector IVec = LinePoints[I];
	FVector JVec = LinePoints[J];
	if (NewAIndex == I && NewBIndex != J)
		NewBIndex = J;
	else if (NewAIndex == J && NewBIndex != I)
		NewBIndex = I;
	else if (NewBIndex == I && NewAIndex != J)
		NewAIndex = J;
	else if (NewBIndex == J && NewAIndex != I)
		NewAIndex = I;
	else if (I != NewAIndex && I != NewBIndex && J != NewAIndex && J != NewBIndex)
	{
		FVector IA = A - IVec;
		FVector IB = B - IVec;
		if (IA.Size() < IB.Size())
		{
			NewAIndex = I;
			NewBIndex = J;
		}
		else
		{
			NewAIndex = J;
			NewBIndex = I;
		}
	}


	
	/*if (NewVertices.Num() == 0)			
		NewVertices.Add(LinePoints[NewAIndex]);
	NewVertices.Add(LinePoints[NewBIndex]);*/
	
	for (int i = 0; i < LinePoints.Num(); i++)
	{
		if (i == NewAIndex || i == NewBIndex)
			continue;
		ExcessPoints.Add(LinePoints[i]);
	}
	
}



bool ACustomDestructibleActor::IsLastEdge(TArray<FVector>& NewVertices, TArray<FVector>& ExcessVertices, const TArray<FVector>& PlaneVertices, const FPlane& Plane)
{
	TArray<FVector> TempPoints;
	
	FVector AB = NewVertices.Last() - NewVertices[0];
	for (int i = 0; i < PlaneVertices.Num(); i++)
	{
		if (!NewVertices.Contains(PlaneVertices[i]))
		{
			TempPoints.Add(PlaneVertices[i]);			
		}
	}
	if (TempPoints.Num() == 0)
		return true;
	else
	{
		FVector BC;
		FVector DotProd;
		for (int i = 0; i < TempPoints.Num(); i++)
		{
			BC = TempPoints[i] - NewVertices.Last();
			DotProd = AB ^ BC;

			if (Plane.PlaneDot(DotProd) < 0)
			{
				return false;
			}
		}
	}
	for (int i = 0; i < TempPoints.Num(); i++)
	{
		ExcessVertices.AddUnique(TempPoints[i]);
	}


	return true;
}


void ACustomDestructibleActor::FindEdges(TArray<FVector>& NewVertices, TArray<FVector>& ExcessVertices, const TArray<FVector>& PlaneVertices, const FPlane& Plane)
{
	int A = 0, B, C;
	bool EdgeFound = false;
	bool AllEdgesFound = false;
	FVector AB;
	FVector BC;
	//NewVertices array except for first point
	//For checking if last Edge Found suddenly
	TArray<FVector> TempPoints;
	TArray<FVector> LinePoints;
	while (!AllEdgesFound)
		//for (A = 0; A < PlaneVertices.Num(); A++)
	{
		EdgeFound = false;
		if (ExcessVertices.Contains(PlaneVertices[A]))
		{
			A++;
			continue;
		}
		bool AIsLast = true;
		for (B = 0; B < PlaneVertices.Num(); B++)
		{			
			if (B == A || ExcessVertices.Contains(PlaneVertices[B]) 
				//if edges >= 2 (3 points sorted) check if next edge is last
				||(NewVertices.Num()<3? NewVertices.Contains(PlaneVertices[B]):TempPoints.Contains(PlaneVertices[B])))
				continue;
			AIsLast = false;
			bool InvalidPointFound = false;

			AB = PlaneVertices[B] - PlaneVertices[A];
			bool BIsLast = true;
			for (C = 0; C < PlaneVertices.Num(); C++)
			{
				
				if (C == A || C == B || ExcessVertices.Contains(PlaneVertices[C]) || NewVertices.Contains(PlaneVertices[C]))
					continue;
				BIsLast = false;
				BC = PlaneVertices[C] - PlaneVertices[B];
				FVector DotProd = AB ^ BC;
				//Points are on one line
				if (DotProd.IsZero()&&!InvalidPointFound)
				{
					if (LinePoints.Num() == 0)
					{
						LinePoints.Add(PlaneVertices[A]);
						LinePoints.Add(PlaneVertices[B]);
					}
					LinePoints.AddUnique(PlaneVertices[C]);
				}
				else
				{
					float Distance = Plane.PlaneDot(DotProd);

					//Point C is invalid (at right from AB)
					//we need points at left from AB or on AB
					if (Distance < 0)
					{
						InvalidPointFound = true;						
						break;
					}
				}
			}
			if (InvalidPointFound)
			{
				if (LinePoints.Num())
					LinePoints.Empty();
				continue;
			}
			else
			{
				EdgeFound = true;
				if (LinePoints.Num())
				{
					int I = 0, J=1;
					FindEdgesOfLine(LinePoints[I],LinePoints[J],LinePoints, NewVertices, ExcessVertices,I,J);
					A = GetIndex(PlaneVertices, LinePoints[I]);
					B = GetIndex(PlaneVertices, LinePoints[J]);
					LinePoints.Empty();
				}
				

				if (NewVertices.Num() > 0 && NewVertices[0] == PlaneVertices[B])
				{
					AllEdgesFound = true;
					break;
				}

				if (NewVertices.Num() == 0)
					NewVertices.Add(PlaneVertices[A]);
				NewVertices.Add(PlaneVertices[B]);
				TempPoints.Add(PlaneVertices[B]);
				//}
				break;

			}

		}
		if (!AllEdgesFound)
		{
			//Next Edge First point is PlaneVertices[B]
			if (EdgeFound)
				A = B;
			else
			{
				//PlaneVertices[A] not belongs to any edge so it is Excess point
				if (NewVertices.Num() == 0)
					ExcessVertices.AddUnique(PlaneVertices[A]);
				//PlaneVertices[B] not belongs to any edge so it is Excess point
				/*else
					ExcessVertices.AddUnique(PlaneVertices[B]);*/
				A++;
			}
			if (NewVertices.Num() >= 3)
			{
				//AllEdgesFound = IsLastEdge(NewVertices,ExcessVertices,PlaneVertices,Plane);
				AllEdgesFound = NewVertices.Num() + ExcessVertices.Num() == PlaneVertices.Num();
				if (AllEdgesFound)
					break;
			}
		}
		else
		{
			break;
		}
		
	}
}


//Sorting Points Of Plane Counter-Clockwise And Remove Excess Points
void ACustomDestructibleActor::SortPointsOfPlane(TArray<FVector>& PlaneVertices,const FPlane& Plane)
{
	TArray<FVector> NewVertices;
	TArray<FVector> ExcessVertices;
	//Find First Edge
	FindEdges(NewVertices, ExcessVertices, PlaneVertices, Plane);
	PlaneVertices.Empty();
	PlaneVertices = NewVertices;
	NewVertices.Empty();
	ExcessVertices.Empty();
}

//Detecting Plains Of Mesh From Array Of Points
bool ACustomDestructibleActor::GetPlanesFromHull(TArray<FVector> HullVertices, TArray<FPlane>& Planes, TArray<TArray<FVector>>& PlanesAndTheirPoints)
{
	if (!EnsureHullIsValid(HullVertices))
		return false;

	Planes.Empty();
	PlanesAndTheirPoints.Empty();
	
	
	//Planes are searched from triangles
	//Array of indices of points of checked triangles
	TArray<TArray<int>> CheckedCombinations;

	const int Size = 3;
	bool AllPointsAreOnPlane = false;
	for (int a = 0; a < HullVertices.Num()-2; a++)
	{
		if (AllPointsAreOnPlane)
			break;
		for (int b = a+1; b < HullVertices.Num()-1; b++)
		{
			if (AllPointsAreOnPlane)
				break;
			for (int c = b + 1; c < HullVertices.Num(); c++)
			{
				if (AllPointsAreOnPlane)
					break;
				TArray<int> Combination;
				Combination.Add(a);
				Combination.Add(b);
				Combination.Add(c);
				FVector A = HullVertices[a];
				FVector B = HullVertices[b];
				FVector C = HullVertices[c];
				
				if (IsNewCombination(Combination, CheckedCombinations)
					&&
					!PointsOfPlane(PlanesAndTheirPoints, A, B, C))
				{
					PlaneIsValid(A, B, C, HullVertices, PlanesAndTheirPoints, Planes, AllPointsAreOnPlane);
				}
			}
		}
	}
	for (int i = 0; i < PlanesAndTheirPoints.Num(); i++)
	{
		//TArray<FVector> TempPoints = PlanesAndTheirPoints[i];

		//Sort points of plane in counter-clockwise order and remove excess points
		SortPointsOfPlane(PlanesAndTheirPoints[i], Planes[i]);

		/*TempPoints = PlanesAndTheirPoints[i];
		FVector Temp;
		for (int j = 0; j < TempPoints.Num(); j++)
		{
			Temp = TempPoints[j];
		}*/
	}
	return true;
}

//Find Indices Of Edge points of line
void ACustomDestructibleActor::FindEdgeIndices(int& I, int& J, const TArray<FVector>& Points)
{
	I = 0, J = 0;
	FVector Temp;
	float MaxLength = 0;
	for (int i = 0; i < Points.Num() - 1; i++)
	{
		for (int j = i + 1; j < Points.Num(); j++)
		{
			Temp = Points[i] - Points[j];
			float TempLength = Temp.Size();

			if (MaxLength < TempLength)
			{
				MaxLength = TempLength;
				I = i;
				J = j;
			}
		}
	}
}

//Rewrite PointsOfPlane into two-points array
void ACustomDestructibleActor::FindSlicePoints(const TArray<FVector>& FrontPoints, const TArray<FVector>& BehindPoints, TArray<FVector>& PointsOfPlane, const FPlane& SlicePlane)
{
	for (int i = 0; i < FrontPoints.Num(); i++)
	{
		for (int j = 0; j < BehindPoints.Num(); j++)
		{
			PointsOfPlane.AddUnique(FMath::LinePlaneIntersection(FrontPoints[i], BehindPoints[j], SlicePlane));
		}
	}
	int I = 0, J = 0;
	FindEdgeIndices(I, J, PointsOfPlane);
	TArray<FVector> NewPoints;
	NewPoints.Add(PointsOfPlane[I]);
	NewPoints.Add(PointsOfPlane[J]);
	PointsOfPlane.Empty();
	PointsOfPlane = NewPoints;
}

void ACustomDestructibleActor::AddUniqueVector(const TArray<FVector>& InVertices, TArray<FVector>& OutVertices)
{
	for (int i = 0; i < InVertices.Num(); i++)
	{
		OutVertices.AddUnique(InVertices[i]);
	}
}

bool ACustomDestructibleActor::SliceHull(const TArray<TArray<FVector>>& PlanesAndTheirPoints,const FPlane& SlicePlane, TArray<FVector>& OneHalfVertices, TArray<FVector>& OtherHalfVertices, bool CreateOtherHalf)
{
	OneHalfVertices.Empty();
	OtherHalfVertices.Empty();

	for (int i = 0; i < PlanesAndTheirPoints.Num(); i++)
	{
		TArray<FVector> FrontPoints;
		TArray<FVector> BehindPoints;
		TArray<FVector> PointsOfPlane;
		float Distance;
		for (int j = 0; j < PlanesAndTheirPoints[i].Num(); j++)
		{
			Distance = SlicePlane.PlaneDot(PlanesAndTheirPoints[i][j]);
			if (Distance == 0)
			{				
				PointsOfPlane.Add(PlanesAndTheirPoints[i][j]);
			}
			else if (Distance > 0)
			{
				FrontPoints.Add(PlanesAndTheirPoints[i][j]);
			}
			else if(Distance < 0)
			{
				BehindPoints.Add(PlanesAndTheirPoints[i][j]);
			}
		}
		//All points are in front of slice plane or on it
		if (BehindPoints.Num() == 0)
		{
			if (PointsOfPlane.Num())
			{
				FrontPoints.Append(PointsOfPlane);
				PointsOfPlane.Empty();
			}			
			AddUniqueVector(FrontPoints, OneHalfVertices);
			
		}
		//All points are behind slice plane or on it
		if (FrontPoints.Num() == 0 && CreateOtherHalf)
		{
			if (PointsOfPlane.Num())
			{
				BehindPoints.Append(PointsOfPlane);
				PointsOfPlane.Empty();
			}			
			AddUniqueVector(BehindPoints, OtherHalfVertices);
			
		}

		if (FrontPoints.Num() > 0 && BehindPoints.Num() > 0)
		{
			FindSlicePoints(FrontPoints,BehindPoints,PointsOfPlane,SlicePlane);
		}

		AddUniqueVector(FrontPoints, OneHalfVertices);
		AddUniqueVector(PointsOfPlane, OneHalfVertices);

		if (CreateOtherHalf)
		{
			AddUniqueVector(BehindPoints, OtherHalfVertices);
			AddUniqueVector(PointsOfPlane, OtherHalfVertices);
		}

		BehindPoints.Empty();
		FrontPoints.Empty();
		PointsOfPlane.Empty();
	}

	return true;
}

void ACustomDestructibleActor::TestHalfSlice()
{
	//FRuntimeMeshCollisionConvexMesh A;
	//FRuntimeConvexCollisionMesh_OLD B;
	////B.



	UStaticMesh* BaseStaticMesh = BaseStaticMeshComponent->GetStaticMesh();

	TArray<FVector> ConvexVerts;
	TArray<FVector> Vertices;
	
	

	ConvexVerts.Add(FVector(-22, -30, -14));
	ConvexVerts.Add(FVector(7, 76, -83));
	ConvexVerts.Add(FVector(86, 100, -14));
	ConvexVerts.Add(FVector(-48, -44, 26));
	ConvexVerts.Add(FVector(-97, -95, 41));
	ConvexVerts.Add(FVector(26, -4, 52));
	ConvexVerts.Add(FVector(19, -44, 52));
	ConvexVerts.Add(FVector(93, -97, -38));
	ConvexVerts.Add(FVector(-18, -95, -73));
	ConvexVerts.Add(FVector(49, -76, -74));
	ConvexVerts.Add(FVector(-23, -93, -72));
	ConvexVerts.Add(FVector(-88, 68, 92));
	ConvexVerts.Add(FVector(21, -80, 13));
	ConvexVerts.Add(FVector(-63, -48, 35));
	ConvexVerts.Add(FVector(-27, -47, 70));
	ConvexVerts.Add(FVector(-76, 31, 42));
	ConvexVerts.Add(FVector(15, -96, 75));
	ConvexVerts.Add(FVector(33, 84, 49));
	ConvexVerts.Add(FVector(-56, -45, -22));
	ConvexVerts.Add(FVector(-18, -37, -47));
	ConvexVerts.Add(FVector(68, 81, -75));
	ConvexVerts.Add(FVector(-33, 79, -3));
	ConvexVerts.Add(FVector(-96, -33, 11));
	ConvexVerts.Add(FVector(-56, -14, 63));
	ConvexVerts.Add(FVector(-42, 86, 48));
	ConvexVerts.Add(FVector(13, -53, 86));
	ConvexVerts.Add(FVector(90, -76, 33));
	ConvexVerts.Add(FVector(-38, 63, 44));
	ConvexVerts.Add(FVector(-95, 72, 69));
	ConvexVerts.Add(FVector(-1, -95, -10));
	ConvexVerts.Add(FVector(-75, 49, -96));
	ConvexVerts.Add(FVector(-85, -85, 52));
	ConvexVerts.Add(FVector(-90, -66, -81));
	ConvexVerts.Add(FVector(-62, 85, 47));
	ConvexVerts.Add(FVector(24, -13, 24));
	ConvexVerts.Add(FVector(-23, -71, 93));
	ConvexVerts.Add(FVector(0, -25, 42));
	ConvexVerts.Add(FVector(-57, -20, -3));
	ConvexVerts.Add(FVector(24, 26, -52));
	ConvexVerts.Add(FVector(20, 67, 86));
	ConvexVerts.Add(FVector(-50, -34, 92));
	ConvexVerts.Add(FVector(71, -2, -87));
	ConvexVerts.Add(FVector(36, 30, -5));
	ConvexVerts.Add(FVector(-42, -41, -28));
	ConvexVerts.Add(FVector(61, 79, 78));
	ConvexVerts.Add(FVector(-15, 76, 86));
	ConvexVerts.Add(FVector(14, -31, 81));
	ConvexVerts.Add(FVector(-21, 59, -53));
	ConvexVerts.Add(FVector(75, -97, 33));
	ConvexVerts.Add(FVector(94, 30, -30));
	ConvexVerts.Add(FVector(68, -31, -24));
	ConvexVerts.Add(FVector(50, -32, 16));
	ConvexVerts.Add(FVector(94, -16, -64));	ConvexVerts.Add(FVector(-78, -3, 7));
	ConvexVerts.Add(FVector(-63, -79, -29));
	ConvexVerts.Add(FVector(-72, 55, -99));
	ConvexVerts.Add(FVector(-72, -79, -57));
	ConvexVerts.Add(FVector(69, 34, -92));
	ConvexVerts.Add(FVector(44, -58, 35));
	ConvexVerts.Add(FVector(82, 21, 15));
	ConvexVerts.Add(FVector(-21, 10, -58));
	ConvexVerts.Add(FVector(-17, -55, -95));
	ConvexVerts.Add(FVector(-74, -41, -73));
	ConvexVerts.Add(FVector(48, -7, -74));
	ConvexVerts.Add(FVector(78, -43, -11));
	ConvexVerts.Add(FVector(94, 28, 95));
	ConvexVerts.Add(FVector(-1, -52, 87));
	ConvexVerts.Add(FVector(-97, -9, 95));
	ConvexVerts.Add(FVector(99, 2, 66));
	ConvexVerts.Add(FVector(-90, 67, 67));
	ConvexVerts.Add(FVector(-44, 43, 10));
	ConvexVerts.Add(FVector(49, -90, 22));
	ConvexVerts.Add(FVector(-86, -87, -36));
	ConvexVerts.Add(FVector(29, 41, 48));
	ConvexVerts.Add(FVector(-61, 8, 97));
	ConvexVerts.Add(FVector(-4, -82, 70));
	ConvexVerts.Add(FVector(39, -94, 97));
	ConvexVerts.Add(FVector(-89, -36, -24));
	ConvexVerts.Add(FVector(90, 27, -93));
	ConvexVerts.Add(FVector(-93, -82, 4));
	ConvexVerts.Add(FVector(70, 98, 46));
	ConvexVerts.Add(FVector(86, -81, 22));
	ConvexVerts.Add(FVector(-64, -41, 35));
	ConvexVerts.Add(FVector(-24, -75, 43));
	ConvexVerts.Add(FVector(-32, -65, 44));
	ConvexVerts.Add(FVector(-94, -7, 26));
	ConvexVerts.Add(FVector(-54, 94, 13));
	ConvexVerts.Add(FVector(93, 80, -56));
	ConvexVerts.Add(FVector(50, -1, 9));
	ConvexVerts.Add(FVector(-99, 93, 85));
	ConvexVerts.Add(FVector(-53, -61, 43));
	ConvexVerts.Add(FVector(-57, -56, -66));
	ConvexVerts.Add(FVector(-86, 78, -85));
	ConvexVerts.Add(FVector(25, -54, -86));
	ConvexVerts.Add(FVector(-85, 71, 29));
	ConvexVerts.Add(FVector(92, -75, -82));
	ConvexVerts.Add(FVector(21, 73, -46));
	ConvexVerts.Add(FVector(87, -56, 11));
	ConvexVerts.Add(FVector(32, -45, -65));
	ConvexVerts.Add(FVector(-51, -42, 30));
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////	   
	srand((unsigned)time(0));

	TArray<Face> Faces;

	TArray<FVector> FirstHalfVertices;
	TArray<FVector> SecondHalfVertices;
	TArray<Face> FirstHalfFaces;
	TArray<Face> SecondHalfFaces;
	//FPlane SlicePlane = FPlane(FVector(-100, 0, 50), FVector(100, 0, 50), FVector(-100, 100, 50));//for 3d octagon
	//FPlane SlicePlane = FPlane(FVector(0, 0, 200), FVector(0, 200, 0), FVector(-200, 0, 0));//for cube
	//FPlane SlicePlane = FPlane(FVector(-100, 0, 200), FVector(200, -100, 0), FVector(200, 100, 0));//for icosahedron
	FPlane SlicePlane = FPlane(FVector(-100, 0, 0), FVector(100, 0, 0), FVector(0, 100, 0));
	//SlicePlane = SlicePlane.Flip();

	int32 PlanesCount = 0;
	clock_t time, time1;
	double Time;

	//qh_mesh_t QHMesh = qh_quickhull3d(ConvexVerts, ConvexVerts.Num());	

	time = clock();
	PlanesCount = qh_quickhull3d(ConvexVerts, ConvexVerts.Num(), Faces);
	time = clock() - time;
	Time = (double)time / CLOCKS_PER_SEC;


	time1 = clock();
	SliceConvexHull(ConvexVerts, Faces, SlicePlane, FirstHalfVertices, FirstHalfFaces, SecondHalfVertices, SecondHalfFaces);
	time1 = clock() - time1;
	//////////////////////////////////////////////////SEARCHING ISSUES////////////////////////////////////////////////////////////
	//for (int i = 0; i < 100; i++)
	//{
	//	
	//	ConvexVerts.Empty();
	//	Faces.Empty();
	//	FirstHalfVertices.Empty();
	//	SecondHalfVertices.Empty();
	//	FirstHalfFaces.Empty();
	//	SecondHalfFaces.Empty();
	//	int n = 100;
	//	int max = 100, min = -100;
	//	float x, y, z;
	//	for (int i = 0; i < n; ++i) {
	//		x = rand() % (max - min + 1) + min;
	//		y = rand() % (max - min + 1) + min;
	//		z = rand() % (max - min + 1) + min;
	//		ConvexVerts.Add(FVector(x, y, z));
	//	}
	//	WriteVertices(ConvexVerts);

	//	/*PlanesCount = qh_quickhull3d(TestVerts, TestVerts.Num(), Faces);
	//	for (float& w = SlicePlane.W; w > 0; w -= 0.1)
	//	{

	//		SliceConvexHull(ConvexVerts, Faces, SlicePlane, FirstHalfVertices, FirstHalfFaces, SecondHalfVertices, SecondHalfFaces);

	//		FirstHalfVertices.Empty();
	//		SecondHalfVertices.Empty();
	//		FirstHalfFaces.Empty();
	//		SecondHalfFaces.Empty();
	//	}*/

	//	time = clock();

	//	PlanesCount = qh_quickhull3d(ConvexVerts, ConvexVerts.Num(), Faces);
	//	time = clock() - time;
	//	Time = (double)time / CLOCKS_PER_SEC;

	//	time1 = clock();
	//	SliceConvexHull(ConvexVerts, Faces, SlicePlane, FirstHalfVertices, FirstHalfFaces, SecondHalfVertices, SecondHalfFaces);
	//	time1 = clock() - time1;
	//}

	//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	
	double Time1 = (double)time1 / CLOCKS_PER_SEC;

	float F = false;

	if (BaseStaticMesh != nullptr&&PlanesCount > 0 && FirstHalfVertices.Num() && SecondHalfVertices.Num() && FirstHalfFaces.Num() && SecondHalfFaces.Num() && Time&&Time1&&F)
	{
		//BaseStaticMesh->dirty
		//FKConvexElem El= BaseStaticMesh->BodySetup->AggGeom.ConvexElems[0];
		//FKConvexElem
		//TMap<int32, int16> Map;
		//Map.
		//URuntimeMesh* RM=NewObject<URuntimeMesh>();







		/*RM->AddConvexCollisionSection(ConvexVerts);
		RM->CookCollisionNow();

		RM->AddConvexCollisionSection(ConvexVerts);
		*/

		//FBox Box = FBox(ConvexVerts);


		/*FKConvexElem* ConvexElem = new FKConvexElem();
		ConvexElem->VertexData = ConvexVerts;
		ConvexElem->UpdateElemBox();
		PxConvexMesh* PXCM = ConvexElem->GetConvexMesh();*/


		// Get set of planes that make up hull
		/*TArray<FPlane> ConvexPlanes;
		ConvexElem->GetPlanes(ConvexPlanes);
		if(ConvexPlanes.Num())
		FPlane FirstPlane = ConvexPlanes[0];*/

		//UBodySetup* StaticMeshBS=NewObject<UBodySetup>();
		//UBodySetup* RMBS = RM->GetBodySetup();
		//RMBS = StaticMeshBS;
		//StaticMeshBS->CopyBodyPropertiesFrom(RM->GetBodySetup());
		////RM->SetBasicBodySetupParameters(StaticMeshBS);
		//
		//URuntimeMeshComponent* RMC = NewObject<URuntimeMeshComponent>();
		//RMC->SetSimulatePhysics(true);
		//RMC->SetMobility(EComponentMobility::Movable);

		PxTriangleMesh* TempTriMesh = BaseStaticMeshComponent->BodyInstance.BodySetup.Get()->TriMeshes[0];

		check(TempTriMesh);
		int32 TriNumber = TempTriMesh->getNbTriangles();
		const PxVec3* PVertices = TempTriMesh->getVertices();
		const void* Triangles = TempTriMesh->getTriangles();
		int32 I0, I1, I2;
		PxU16* P16BitIndices = nullptr;
		PxU32* P32BitIndices = nullptr;
		bool Is16BitIndices = true;
		/*if (TempTriMesh->getTriangleMeshFlags() & PxTriangleMeshFlag::e16_BIT_INDICES)
			P16BitIndices = (PxU16*)Triangles;
		else
		{
			P32BitIndices = (PxU32*)Triangles;
			Is16BitIndices = false;
		}*/

		FVector BoundingBoxSize;
		float MeshHeight;


		BoundingBoxSize = BaseStaticMesh->GetBoundingBox().GetSize();
		MeshHeight = BoundingBoxSize.Z;
		float Middle = MeshHeight / 2;
		TArray<FVector> Positions;
		TArray<uint32> Indices;


		for (int32 TriIndex = 0; TriIndex < TriNumber; ++TriIndex) {
			/*if (Is16BitIndices)
			{
				I0 = P16BitIndices[(TriIndex * 3) + 0];
				I1 = P16BitIndices[(TriIndex * 3) + 1];
				I2 = P16BitIndices[(TriIndex * 3) + 2];
			}
			else
			{
				I0 = P32BitIndices[(TriIndex * 3) + 0];
				I1 = P32BitIndices[(TriIndex * 3) + 1];
				I2 = P32BitIndices[(TriIndex * 3) + 2];
			}*/

			if (TempTriMesh->getTriangleMeshFlags() & PxTriangleMeshFlag::e16_BIT_INDICES)
			{
				PxU16* P16BitIndices = (PxU16*)Triangles;
				I0 = P16BitIndices[(TriIndex * 3) + 0];
				I1 = P16BitIndices[(TriIndex * 3) + 1];
				I2 = P16BitIndices[(TriIndex * 3) + 2];
			}
			else
			{
				Is16BitIndices = false;
				PxU32* P32BitIndices = (PxU32*)Triangles;
				I0 = P32BitIndices[(TriIndex * 3) + 0];
				I1 = P32BitIndices[(TriIndex * 3) + 1];
				I2 = P32BitIndices[(TriIndex * 3) + 2];
			}


			FVector V0 = P2UVector(PVertices[I0]);
			FVector V1 = P2UVector(PVertices[I1]);
			FVector V2 = P2UVector(PVertices[I2]);
			if (V0.Z <= Middle || V1.Z <= Middle || V2.Z <= Middle)
			{
				if (V0.Z > Middle)
					V0.Z = Middle;
				if (V1.Z > Middle)
					V1.Z = Middle;
				if (V2.Z > Middle)
					V2.Z = Middle;
				Positions.Add(V0);
				Positions.Add(V1);
				Positions.Add(V2);
				/*Indices.Add(I0);
				Indices.Add(I1);
				Indices.Add(I2);*/
			}
		}

		//BaseStaticMesh->RenderData->LODResources[0].IndexBuffer.SetIndices(Indices, Is16BitIndices ?EIndexBufferStride::Force16Bit: EIndexBufferStride::Force32Bit);


		BaseStaticMesh->RenderData->LODResources[0].VertexBuffers.PositionVertexBuffer.Init(Positions);

		//BaseStaticMesh->RenderData->LODResources[0].IndexBuffer.InitRHI();
		BaseStaticMesh->RenderData->LODResources[0].VertexBuffers.PositionVertexBuffer.InitRHI();




		////////////////////////////////////////////////////////////////////////


		//FVector BoundingBoxSize;
		//float MeshHeight;


		//BoundingBoxSize = BaseStaticMesh->GetBoundingBox().GetSize();
		//MeshHeight = BoundingBoxSize.Z;
		//float Middle = MeshHeight / 2;
		//TArray<FVector> Positions;
		//TArray<FVector> OriginPositions;
		//TArray<FColor> Colors;
		//TArray<uint32> Indices;

		///////////////////////////////////////////////////////////////////////////////

		////FStaticMeshVertexBuffer* NewStaticMeshVBuffer;

		//FPositionVertexBuffer* PositionVBuffer = &BaseStaticMesh->RenderData->LODResources[0].VertexBuffers.PositionVertexBuffer;

		//FStaticMeshVertexBuffer* StaticMeshVBuffer = &BaseStaticMesh->RenderData->LODResources[0].VertexBuffers.StaticMeshVertexBuffer;
		//FStaticMeshVertexBuffer::FTangentsVertexBuffer* TangentsVBuffer = &StaticMeshVBuffer->TangentsVertexBuffer;
		//FStaticMeshVertexBuffer::FTexcoordVertexBuffer* TexCoordVBuffer = &StaticMeshVBuffer->TexCoordVertexBuffer;
		//FRawStaticIndexBuffer* IndicesVBuffer = &BaseStaticMesh->RenderData->LODResources[0].IndexBuffer;

		//FColorVertexBuffer* ColorVBuffer = &BaseStaticMesh->RenderData->LODResources[0].VertexBuffers.ColorVertexBuffer;

		////TArray <FStaticMeshBuildVertex> BuildVertices;

		//if (PositionVBuffer&&IndicesVBuffer)
		//{
		//	const int32 nbIndices = IndicesVBuffer->GetNumIndices();
		//	IndicesVBuffer->GetCopy(Indices);

		//	for (int32 index = 0; index < nbIndices; index += 3)
		//	{
		//		int32 I0 = Indices[index];
		//		int32 I1 = Indices[index + 1];
		//		int32 I2 = Indices[index + 2];
		//		FVector V0 = PositionVBuffer->VertexPosition(I0);
		//		FVector V1 = PositionVBuffer->VertexPosition(I1);
		//		FVector V2 = PositionVBuffer->VertexPosition(I2);
		//		if (V0.Z <= Middle || V1.Z <= Middle || V2.Z <= Middle)
		//		{
		//			if (V0.Z > Middle)
		//				V0.Z = Middle;
		//			if (V1.Z > Middle)
		//				V1.Z = Middle;
		//			if (V2.Z > Middle)
		//				V2.Z = Middle;
		//			Positions.Add(V0);
		//			Positions.Add(V1);
		//			Positions.Add(V2);
		//		}
		//	}
		//}


		//if (PositionVBuffer)
		//{
		//	const int32 nbVertices = PositionVBuffer->GetNumVertices();
		//	const int32 nbIndices = vIndBuffer->GetNumIndices();

		//	for (int32 index = 0; index < nbVertices; index++)
		//	{
		//		if (nbIndices == nbVertices)
		//		{
		//			Positions.Add(PositionVBuffer->VertexPosition(index));
		//		}
		//		FVector position;
		//		FColor color;
		//		position = PositionVBuffer->VertexPosition(index);
		//		if (ColorVBuffer)
		//			color = ColorVBuffer->VertexColor(index);

		//		if (position.Z <= Middle)
		//		{
		//			/*FStaticMeshBuildVertex BuildVertex;
		//			BuildVertex.Position=position;
		//			BuildVertex.Color = color;
		//			BuildVertex.TangentX = StaticMeshVBuffer->VertexTangentX(index);
		//			BuildVertex.TangentY = StaticMeshVBuffer->VertexTangentY(index);
		//			BuildVertex.TangentZ = StaticMeshVBuffer->VertexTangentZ(index);

		//			for (int i = 0; i < MAX_STATIC_TEXCOORDS; i++)
		//			{
		//				BuildVertex.UVs[i] = StaticMeshVBuffer->GetVertexUV(index, i);
		//			}


		//			BuildVertices.Add(BuildVertex);*/
		//			Positions.Add(position);
		//			
		//			/*if (ColorVBuffer)
		//				Colors.Add(color);*/
		//		}

		//	}
		//}

		/*BaseStaticMesh->RenderData->LODResources[0].VertexBuffers.PositionVertexBuffer.Init(Positions);
		BaseStaticMesh->RenderData->LODResources[0].VertexBuffers.PositionVertexBuffer.InitRHI();*/


		/*BaseStaticMesh->RenderData->LODResources[0].VertexBuffers.StaticMeshVertexBuffer.Init(BuildVertices, MAX_STATIC_TEXCOORDS);
		BaseStaticMesh->RenderData->LODResources[0].VertexBuffers.StaticMeshVertexBuffer.InitRHI();


		BaseStaticMesh->Build();*/







		//FRawMesh RawMesh;

		//float Vertices = BaseStaticMesh->GetNumVertices(0);
		//BaseStaticMesh->SourceModels[0].LoadRawMesh(RawMesh);

		//BoundingBoxSize = BaseStaticMesh->GetBoundingBox().GetSize();
		//MeshHeight = BoundingBoxSize.Z;
		//float Middle = MeshHeight / 2;
		//float rawVertices = RawMesh.VertexPositions.Num();
		//for (int vertIndex = 0; vertIndex < rawVertices; vertIndex++) {

		//	if (RawMesh.VertexPositions[vertIndex].Z > Middle)
		//	{
		//		/*RawMesh.VertexPositions[vertIndex].X = 0.f;
		//		RawMesh.VertexPositions[vertIndex].Y = 0.f;*/
		//		RawMesh.VertexPositions[vertIndex].Z = Middle;
		//	}
		//}


		//FString pathPackage = FString("/Game/MyStaticMeshes/");
		//FString absolutePathPackage = FPaths::GameContentDir() + "/MyStaticMeshes/";

		///*FString pathPackage = FString("/Game/");
		//FString absolutePathPackage = FPaths::GameContentDir();*/

		//FPackageName::RegisterMountPoint(*pathPackage, *absolutePathPackage);

		//UPackage* Package = CreatePackage(nullptr, *pathPackage);

		//// Create Static Mesh
		//FString ObjectName = FString("MyObject");

		//FName StaticMeshName = MakeUniqueObjectName(Package, UStaticMesh::StaticClass(), FName(*ObjectName));
		//UStaticMesh* NewStaticMesh = NewObject<UStaticMesh>(Package, StaticMeshName, RF_Public | RF_Standalone|RF_Transient);//



		//new(NewStaticMesh->SourceModels) FStaticMeshSourceModel();
		/*BaseStaticMesh->SourceModels[0].SaveRawMesh(RawMesh);
		BaseStaticMesh->Build();*/
		//FStaticMeshSourceModel& SrcModel = NewStaticMesh->SourceModels[0];

		//NewStaticMesh->SourceModels[0].BuildSettings.bRecomputeNormals = true;
		//NewStaticMesh->SourceModels[0].BuildSettings.bRecomputeTangents = true;
		//NewStaticMesh->SourceModels[0].BuildSettings.bUseMikkTSpace = false;
		//NewStaticMesh->SourceModels[0].BuildSettings.bGenerateLightmapUVs = true;
		//NewStaticMesh->SourceModels[0].BuildSettings.bBuildAdjacencyBuffer = false;
		//NewStaticMesh->SourceModels[0].BuildSettings.bBuildReversedIndexBuffer = false;
		//NewStaticMesh->SourceModels[0].BuildSettings.bUseFullPrecisionUVs = false;
		//NewStaticMesh->SourceModels[0].BuildSettings.bUseHighPrecisionTangentBasis = false;



		//

		//// Processing the StaticMesh and Marking it as not saved
		//NewStaticMesh->ImportVersion = EImportStaticMeshVersion::LastVersion;
		//NewStaticMesh->CreateBodySetup();		
		//NewStaticMesh->SetLightingGuid();	

		//BaseStaticMesh->PostEditChange();
		//Package->MarkPackageDirty();
		//BaseStaticMeshComponent->SetStaticMesh(BaseStaticMesh);
		//	/*delete NewStaticMesh;
		//	delete BaseStaticMesh;
		//	delete Package;*/
		//}

		// Object Details
		//FString ObjectName = FString("MyObject");
		//
		//TArray<FVector> Vertices;
		//Vertices.Add(FVector(86.6, 75, 0));
		//Vertices.Add(FVector(-86.6, 75, 0));
		//Vertices.Add(FVector(2.13, 25, 175));
		//Vertices.Add(FVector(2.13, -75, 0));
		//int numberOfVertices = Vertices.Num();

		//struct Face {
		//	unsigned int v1;
		//	unsigned int v2;
		//	unsigned int v3;
		//	short materialID;
		//	FVector2D uvCoords1;
		//	FVector2D uvCoords2;
		//	FVector2D uvCoords3;
		//};
		//TArray<Face> Faces;
		//Face oneFace;
		//oneFace = { 1,3,0,  0,  FVector2D(0,0), FVector2D(1, 0), FVector2D(0.5, 1) };
		//Faces.Add(oneFace);
		//oneFace = { 0,2,1,  1,  FVector2D(0,0), FVector2D(1, 0), FVector2D(0.5, 1) };
		//Faces.Add(oneFace);
		//oneFace = { 3,2,0,  0,  FVector2D(0,0), FVector2D(1, 0), FVector2D(0.5, 1) };
		//Faces.Add(oneFace);
		//oneFace = { 1,2,3,  1,  FVector2D(0,0), FVector2D(1, 0), FVector2D(0.5, 1) };
		//Faces.Add(oneFace);
		//int numberOfFaces = Faces.Num();

		//TArray<FStaticMaterial> Materials; //This should contain the real Materials, this is just an example
		//Materials.Add(FStaticMaterial());
		//Materials.Add(FStaticMaterial());
		//int numberOfMaterials = Materials.Num();

		//// Create Package
		//FString pathPackage = FString("/Game/MyStaticMeshes/");
		//FString absolutePathPackage = FPaths::GameContentDir() + "/MyStaticMeshes/";

		//FPackageName::RegisterMountPoint(*pathPackage, *absolutePathPackage);

		//UPackage * Package = CreatePackage(nullptr, *pathPackage);

		//// Create Static Mesh
		//FName StaticMeshName = MakeUniqueObjectName(Package, UStaticMesh::StaticClass(), FName(*ObjectName));
		//UStaticMesh* myStaticMesh = NewObject<UStaticMesh>(Package, StaticMeshName, RF_Public | RF_Standalone);

		//if (myStaticMesh != NULL)
		//{
		//	FRawMesh myRawMesh = FRawMesh();
		//	FColor WhiteVertex = FColor(255, 255, 255, 255);
		//	FVector EmptyVector = FVector(0, 0, 0);

		//	// Vertices
		//	for (int vertIndex = 0; vertIndex < numberOfVertices; vertIndex++) {
		//		myRawMesh.VertexPositions.Add(Vertices[vertIndex]);
		//	}
		//	// Faces and UV/Normals
		//	for (int faceIndex = 0; faceIndex < numberOfFaces; faceIndex++) {
		//		myRawMesh.WedgeIndices.Add(Faces[faceIndex].v1);
		//		myRawMesh.WedgeIndices.Add(Faces[faceIndex].v2);
		//		myRawMesh.WedgeIndices.Add(Faces[faceIndex].v3);

		//		myRawMesh.WedgeColors.Add(WhiteVertex);
		//		myRawMesh.WedgeColors.Add(WhiteVertex);
		//		myRawMesh.WedgeColors.Add(WhiteVertex);

		//		myRawMesh.WedgeTangentX.Add(EmptyVector);
		//		myRawMesh.WedgeTangentX.Add(EmptyVector);
		//		myRawMesh.WedgeTangentX.Add(EmptyVector);

		//		myRawMesh.WedgeTangentY.Add(EmptyVector);
		//		myRawMesh.WedgeTangentY.Add(EmptyVector);
		//		myRawMesh.WedgeTangentY.Add(EmptyVector);

		//		myRawMesh.WedgeTangentZ.Add(EmptyVector);
		//		myRawMesh.WedgeTangentZ.Add(EmptyVector);
		//		myRawMesh.WedgeTangentZ.Add(EmptyVector);

		//		// Materials
		//		myRawMesh.FaceMaterialIndices.Add(Faces[faceIndex].materialID);

		//		myRawMesh.FaceSmoothingMasks.Add(0xFFFFFFFF); // Phong

		//		for (int UVIndex = 0; UVIndex < MAX_MESH_TEXTURE_COORDS; UVIndex++)
		//		{
		//			myRawMesh.WedgeTexCoords[UVIndex].Add(Faces[faceIndex].uvCoords1);
		//			myRawMesh.WedgeTexCoords[UVIndex].Add(Faces[faceIndex].uvCoords2);
		//			myRawMesh.WedgeTexCoords[UVIndex].Add(Faces[faceIndex].uvCoords3);
		//		}
		//	}

		//	// Saving mesh in the StaticMesh
		//	new(myStaticMesh->SourceModels) FStaticMeshSourceModel();
		//	myStaticMesh->SourceModels[0].SaveRawMesh(myRawMesh);

		//	FStaticMeshSourceModel& SrcModel = myStaticMesh->SourceModels[0];

		//	// Model Configuration
		//	/*myStaticMesh->SourceModels[0].BuildSettings.bRecomputeNormals = true;
		//	myStaticMesh->SourceModels[0].BuildSettings.bRecomputeTangents = true;
		//	myStaticMesh->SourceModels[0].BuildSettings.bUseMikkTSpace = false;
		//	myStaticMesh->SourceModels[0].BuildSettings.bGenerateLightmapUVs = true;
		//	myStaticMesh->SourceModels[0].BuildSettings.bBuildAdjacencyBuffer = false;
		//	myStaticMesh->SourceModels[0].BuildSettings.bBuildReversedIndexBuffer = false;
		//	myStaticMesh->SourceModels[0].BuildSettings.bUseFullPrecisionUVs = false;
		//	myStaticMesh->SourceModels[0].BuildSettings.bUseHighPrecisionTangentBasis = false;*/

		//	// Assign the Materials to the Slots (optional

		//	for (int32 MaterialID = 0; MaterialID < numberOfMaterials; MaterialID++) {
		//		myStaticMesh->StaticMaterials.Add(Materials[MaterialID]);
		//		myStaticMesh->SectionInfoMap.Set(0, MaterialID, FMeshSectionInfo(MaterialID));
		//	}

		//	// Processing the StaticMesh and Marking it as not saved
		//	/*myStaticMesh->ImportVersion = EImportStaticMeshVersion::LastVersion;
		//	myStaticMesh->CreateBodySetup();
		//	myStaticMesh->SetLightingGuid();
		//	myStaticMesh->PostEditChange();*/
		//	Package->MarkPackageDirty();

		//	UE_LOG(LogTemp, Log, TEXT("Static Mesh created: %s"), &ObjectName);
		//	BaseStaticMeshComponent->SetStaticMesh(myStaticMesh);
	};
}


void ACustomDestructibleActor::AddSubStaticMeshComponent(UStaticMeshComponent * SubStaticMeshComponent)
{
	if (SubStaticMeshComponent != nullptr)
	{
		SubStaticMeshComponents.Add(SubStaticMeshComponent);
		SubStaticMeshComponents.Last()->SetupAttachment(BaseStaticMeshComponent);
	}
}


	//Tetrahedron 5 Planes
	/*ConvexVerts.Add(FVector(100, 100, 0));
	ConvexVerts.Add(FVector(100, -100, 0));
	ConvexVerts.Add(FVector(-100, 100, 0));
	ConvexVerts.Add(FVector(-100, -100, 0));

	ConvexVerts.Add(FVector(0, 0, 200));*/

	//Cube 6 Planes
	///////////////////////////////////////////////////////////////////////////////////////////////////
	//ConvexVerts.Add(FVector(0, 50, 0));//Test Point

	//ConvexVerts.Add(FVector(100, 100, 0));//0
	//ConvexVerts.Add(FVector(0, 100, 0));//Test Point
	//ConvexVerts.Add(FVector(-100, 100, 0));//1
	//ConvexVerts.Add(FVector(-100, 0, 0));//Test Point
	//ConvexVerts.Add(FVector(100, -100, 0));//2

	//ConvexVerts.Add(FVector(0, -50, 0));//Test Point
	//ConvexVerts.Add(FVector(50, -50, 0));//Test Point

	//ConvexVerts.Add(FVector(-100, -100, 0));//3
	//ConvexVerts.Add(FVector(0, -100, 0));//Test Point
	//ConvexVerts.Add(FVector(100, 0, 0));//Test Point

	//ConvexVerts.Add(FVector(100, 100, 200));//4
	//ConvexVerts.Add(FVector(-100, 100, 200));//5
	//ConvexVerts.Add(FVector(100, -100, 200));//6
	//ConvexVerts.Add(FVector(-100, -100, 200));//7



	///////////////////////////////////////////////////////////////////////////////////////////////////

	///*ConvexVerts.Add(FVector(0, 0, 100));
	//ConvexVerts.Add(FVector(0, 0, 150));*/




	//Octahedron 8 Planes
	/*ConvexVerts.Add(FVector(0, 100, 0));
	ConvexVerts.Add(FVector(-50, 0, 50));
	ConvexVerts.Add(FVector(50, 0, 50));
	ConvexVerts.Add(FVector(50, 0, -50));
	ConvexVerts.Add(FVector(-50, 0, -50));
	ConvexVerts.Add(FVector(0, -100, 0));*/

	//Icosahedron 20 Planes
	/*ConvexVerts.Add(FVector(0, -200, 100));
	ConvexVerts.Add(FVector(0, 200, 100));
	ConvexVerts.Add(FVector(0, 200, -100));
	ConvexVerts.Add(FVector(0, -200, -100));

	ConvexVerts.Add(FVector(100, 0, 200));
	ConvexVerts.Add(FVector(-100, 0, 200));
	ConvexVerts.Add(FVector(-100, 0, -200));
	ConvexVerts.Add(FVector(100, 0, -200));

	ConvexVerts.Add(FVector(200, 100, 0));
	ConvexVerts.Add(FVector(-200, 100, 0));
	ConvexVerts.Add(FVector(-200, -100, 0));
	ConvexVerts.Add(FVector(200, -100, 0));*/

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//Test Convex Vertices 28 verts 52 faces
	/*ConvexVerts.Add(FVector(-18.6691, -6.67934, 129.556));
	ConvexVerts.Add(FVector(21.9192, -5.55088, 156.914));
	ConvexVerts.Add(FVector(19.3267, -12.3315, 156.05));
	ConvexVerts.Add(FVector(-4.09947, 0.725687, 181.385));
	ConvexVerts.Add(FVector(5.77256, 11.7316, 118.364));
	ConvexVerts.Add(FVector(-8.90948, -21.341, 139.322));
	ConvexVerts.Add(FVector(12.7255, -9.49271, 116.202));
	ConvexVerts.Add(FVector(-12.2251, 10.2057, 118.831));
	ConvexVerts.Add(FVector(2.04344, 8.93487, 173.047));
	ConvexVerts.Add(FVector(21.9192, -20.0007, 145.098));
	ConvexVerts.Add(FVector(-3.96906, -10.267, 178.021));
	ConvexVerts.Add(FVector(-21.423, -0.305896, 141.158));
	ConvexVerts.Add(FVector(-21.423, -20.0007, 145.105));
	ConvexVerts.Add(FVector(21.9192, 2.32199, 145.098));
	ConvexVerts.Add(FVector(-13.5403, -8.17877, 116.202));
	ConvexVerts.Add(FVector(5.72554, -6.67455, 179.536));
	ConvexVerts.Add(FVector(-10.1021, 11.5974, 138.132));
	ConvexVerts.Add(FVector(10.5434, 11.5076, 138.198));
	ConvexVerts.Add(FVector(15.5488, 3.10487, 120.926));
	ConvexVerts.Add(FVector(-6.41552, 4.2489, 176.982));
	ConvexVerts.Add(FVector(-7.61607, -21.1895, 147.888));
	ConvexVerts.Add(FVector(-6.96857, 14.1475, 125.401));
	ConvexVerts.Add(FVector(-18.8746, -12.3534, 156.065));
	ConvexVerts.Add(FVector(6.81343, 2.92093, 177.756));
	ConvexVerts.Add(FVector(21.9192, -6.86482, 134.594));
	ConvexVerts.Add(FVector(8.09672, -18.7315, 132.187));
	ConvexVerts.Add(FVector(3.2318, -11.4099, 176.581));
	ConvexVerts.Add(FVector(-14.975, 9.15705, 138.135));*/



	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//Test Convex Hull 2 50 Verts 26 Faces
	/*ConvexVerts.Add(FVector(69.4633, 7.53672, 111.625));
	ConvexVerts.Add(FVector(69.4633, 7.53672, 45.5883));
	ConvexVerts.Add(FVector(69.4633, 1.08826, 45.5883));
	ConvexVerts.Add(FVector(69.4633, 1.08826, 117.037));
	ConvexVerts.Add(FVector(69.4633, 2.12502, 117.037));
	ConvexVerts.Add(FVector(-69.4633, 1.08826, 45.5883));
	ConvexVerts.Add(FVector(-69.4633, 7.53673, 45.5883));
	ConvexVerts.Add(FVector(-69.4633, 7.53673, 111.591));
	ConvexVerts.Add(FVector(-69.4633, 2.09101, 117.037));
	ConvexVerts.Add(FVector(-69.4633, 1.08826, 117.037));
	ConvexVerts.Add(FVector(-62.1491, 14.8509, 111.591));
	ConvexVerts.Add(FVector(-62.1491, 14.8509, 43.7707));
	ConvexVerts.Add(FVector(-18.9656, 14.8509, 0.600908));
	ConvexVerts.Add(FVector(18.9697, 14.8509, 0.600909));
	ConvexVerts.Add(FVector(62.1491, 14.8509, 43.7811));
	ConvexVerts.Add(FVector(62.1491, 14.8509, 111.626));
	ConvexVerts.Add(FVector(2.88377, 14.8509, 170.899));
	ConvexVerts.Add(FVector(-2.84229, 14.8509, 170.899));
	ConvexVerts.Add(FVector(46.3032, -22.0719, 124.532));
	ConvexVerts.Add(FVector(46.3032, -22.0719, 35.1949));
	ConvexVerts.Add(FVector(19.1715, -22.0719, 8.07186));
	ConvexVerts.Add(FVector(-19.1545, -22.0719, 8.07186));
	ConvexVerts.Add(FVector(-46.3032, -22.0719, 35.221));
	ConvexVerts.Add(FVector(-46.3031, -22.0719, 124.599));
	ConvexVerts.Add(FVector(-2.96047, -22.0719, 167.928));
	ConvexVerts.Add(FVector(2.90753, -22.0719, 167.928));
	ConvexVerts.Add(FVector(-3.86958, -6.46079, 182.63));
	ConvexVerts.Add(FVector(-3.86958, 2.0923, 182.63));
	ConvexVerts.Add(FVector(-2.84231, 3.11959, 182.63));
	ConvexVerts.Add(FVector(2.88379, 3.11959, 182.63));
	ConvexVerts.Add(FVector(3.86958, 2.13367, 182.63));
	ConvexVerts.Add(FVector(3.86958, -6.40755, 182.63));
	ConvexVerts.Add(FVector(2.90753, -7.36962, 182.63));
	ConvexVerts.Add(FVector(-2.96045, -7.36962, 182.63));
	ConvexVerts.Add(FVector(-23.7385, 9.3421, -0.136514));
	ConvexVerts.Add(FVector(-23.7385, -9.2794, -0.136514));
	ConvexVerts.Add(FVector(-19.1545, -13.8635, -0.136514));
	ConvexVerts.Add(FVector(19.1715, -13.8635, -0.136514));
	ConvexVerts.Add(FVector(23.7385, -9.29796, -0.136514));
	ConvexVerts.Add(FVector(23.7385, 9.34465, -0.136514));
	ConvexVerts.Add(FVector(18.9697, 14.1135, -0.136514));
	ConvexVerts.Add(FVector(-18.9656, 14.1135, -0.136514));
	ConvexVerts.Add(FVector(61.9686, -6.4064, 124.531));
	ConvexVerts.Add(FVector(-67.6439, 9.35605, 43.7689));
	ConvexVerts.Add(FVector(-59.0963, -9.27872, 35.2213));
	ConvexVerts.Add(FVector(59.0658, -9.30918, 35.1908));
	ConvexVerts.Add(FVector(67.6562, 9.34378, 43.7812));
	ConvexVerts.Add(FVector(-61.8958, -6.4792, 124.604));
	ConvexVerts.Add(FVector(-61.802, -6.57297, 124.698));
	ConvexVerts.Add(FVector(-59.0217, -9.35331, 35.1467));*/
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//3D Octagon 10 faces
	/*ConvexVerts.Add(FVector(40, 100, 0));
	ConvexVerts.Add(FVector(100, 40, 0));
	ConvexVerts.Add(FVector(100, -40, 0));
	ConvexVerts.Add(FVector(40, -100, 0));
	ConvexVerts.Add(FVector(-40, -100, 0));
	ConvexVerts.Add(FVector(-100, -40, 0));
	ConvexVerts.Add(FVector(-100, 40, 0));
	ConvexVerts.Add(FVector(-40, 100, 0));

	ConvexVerts.Add(FVector(40, 100, 100));
	ConvexVerts.Add(FVector(100, 40, 100));
	ConvexVerts.Add(FVector(100, -40, 100));
	ConvexVerts.Add(FVector(40, -100, 100));
	ConvexVerts.Add(FVector(-40, -100, 100));
	ConvexVerts.Add(FVector(-100, -40, 100));
	ConvexVerts.Add(FVector(-100, 40, 100));
	ConvexVerts.Add(FVector(-40, 100, 100));*/


	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////Create RM section from QuickHull/////////////////////////////////
	//URuntimeMesh RM;
	/*qh_mesh_t Mesh = qh_quickhull3d(vertices, n);
	TArray<FVector>Verts, Normals;
	TArray<int32> Triangles;
	for (unsigned int i = 0; i < Mesh.nvertices; i++)
	{
		qh_vertex_t V = Mesh.vertices[i];
		Verts.Add(FVector(V.x,V.y,V.z));
	}
	for (unsigned int i = 0; i < Mesh.nnormals; i++)
	{
		qh_vertex_t V = Mesh.normals[i];
		Normals.Add(FVector(V.x, V.y, V.z));
	}
	for (unsigned int i = 0; i < Mesh.nindices; i++)
	{

		Triangles.Add(Mesh.indices[i]);
	}
	TArray<FVector2D> UV;
	TArray<FColor> Colors;
	TArray<FRuntimeMeshTangent> Tangents;
	RM.CreateMeshSection(RM.GetNumSections(), Verts, Triangles, Normals,UV,Colors,Tangents);*/
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////ISSUES///////////////////////////////////////////////////////////////
	//Issues with FacesAndNeighboursCorrection Hull with -1 neighbour 20 verts
	/*ConvexVerts.Add(FVector(-87, -53, -5));
	ConvexVerts.Add(FVector(100, 56, 65));
	ConvexVerts.Add(FVector(53, 5, 80));
	ConvexVerts.Add(FVector(5, 30, 89));
	ConvexVerts.Add(FVector(27, -4, 25));
	ConvexVerts.Add(FVector(-48, -45, 8));
	ConvexVerts.Add(FVector(24, -75, 98));
	ConvexVerts.Add(FVector(61, -60, -28));
	ConvexVerts.Add(FVector(-53, 87, 30));
	ConvexVerts.Add(FVector(0, -65, -70));
	ConvexVerts.Add(FVector(24, -16, 21));
	ConvexVerts.Add(FVector(-78, 2, 30));
	ConvexVerts.Add(FVector(23, 89, -13));
	ConvexVerts.Add(FVector(55, 83, 87));
	ConvexVerts.Add(FVector(17, 50, 52));
	ConvexVerts.Add(FVector(70, 1, -15));
	ConvexVerts.Add(FVector(-6, -65, -55));
	ConvexVerts.Add(FVector(-8, -3, 53));
	ConvexVerts.Add(FVector(82, 98, 86));
	ConvexVerts.Add(FVector(-98, 1, 42));*/

	//Issues with FacesAndNeighboursCorrection Hull with -1 neighbour 10 verts
	/*ConvexVerts.Add(FVector(-50, -92, -9));
	ConvexVerts.Add(FVector(-98, 38, 75));
	ConvexVerts.Add(FVector(16, 34, -4));
	ConvexVerts.Add(FVector(15, 66, -45));
	ConvexVerts.Add(FVector(-14, 70, -97));
	ConvexVerts.Add(FVector(-85, -28, -26));
	ConvexVerts.Add(FVector(50, -49, 11));
	ConvexVerts.Add(FVector(-58, 24, -14));
	ConvexVerts.Add(FVector(-89, -51, 0));
	ConvexVerts.Add(FVector(25, 38, -20));*/

	//Issues in FacesAndNeighboursCorrection with 7 neighbour
	//Issue in QuickHull algorythm
	/*ConvexVerts.Add(FVector(-16, -47, 62));
	ConvexVerts.Add(FVector(97, -48, -98));
	ConvexVerts.Add(FVector(11, 2, -60));
	ConvexVerts.Add(FVector(-96, 73, 95));
	ConvexVerts.Add(FVector(-14, -28, -51));
	ConvexVerts.Add(FVector(-25, 8, 66));
	ConvexVerts.Add(FVector(86, -15, -65));
	ConvexVerts.Add(FVector(87, 48, -41));
	ConvexVerts.Add(FVector(64, -21, 7));
	ConvexVerts.Add(FVector(39, -13, 81));*/

	//Issues with SetSlicedFaceAsUnfoundNeighbour after while loop
	/*ConvexVerts.Add(FVector(-62, -34, -20));
	ConvexVerts.Add(FVector(88, -94, 21));
	ConvexVerts.Add(FVector(-70, 37, -68));
	ConvexVerts.Add(FVector(-68, -67, -1));
	ConvexVerts.Add(FVector(77, 35, 12));
	ConvexVerts.Add(FVector(-13, 41, 3));
	ConvexVerts.Add(FVector(-71, 83, -94));
	ConvexVerts.Add(FVector(4, -27, -90));
	ConvexVerts.Add(FVector(82, 74, -44));
	ConvexVerts.Add(FVector(-40, -7, 14));*/

	//Issues with SetSlicedFaceAsUnfoundNeighbour in while loop
	/*ConvexVerts.Add(FVector(-40, -46, 82));
	ConvexVerts.Add(FVector(-48, -40, 89));
	ConvexVerts.Add(FVector(60, -69, 99));
	ConvexVerts.Add(FVector(-40, 17, -2));
	ConvexVerts.Add(FVector(-75, 81, -95));
	ConvexVerts.Add(FVector(-63, 38, 17));
	ConvexVerts.Add(FVector(-70, 78, -33));
	ConvexVerts.Add(FVector(19, 4, 79));
	ConvexVerts.Add(FVector(24, -5, -18));
	ConvexVerts.Add(FVector(5, -34, 39));*/

	//Issues with VerticesAndIndicesCorrection and probably with SearchFirstSlicedFace
	/*ConvexVerts.Add(FVector(-35, 15, -78));
	ConvexVerts.Add(FVector(-70, -82, 65));
	ConvexVerts.Add(FVector(-81, 68, 60));
	ConvexVerts.Add(FVector(20, 70, -100));
	ConvexVerts.Add(FVector(28, 76, -39));
	ConvexVerts.Add(FVector(-93, 32, -43));
	ConvexVerts.Add(FVector(14, -83, -27));
	ConvexVerts.Add(FVector(-14, -42, -67));
	ConvexVerts.Add(FVector(5, 73, -50));
	ConvexVerts.Add(FVector(100, -96, 84));*/


	//Issues with SearchFirstSlicedFace
	/*ConvexVerts.Add(FVector(-100, 62, -35));
	ConvexVerts.Add(FVector(97, 12, 91));
	ConvexVerts.Add(FVector(-24, 66, -7));
	ConvexVerts.Add(FVector(72, 3, 17));
	ConvexVerts.Add(FVector(-55, -83, 20));
	ConvexVerts.Add(FVector(89, -15, 3));
	ConvexVerts.Add(FVector(-92, -25, 87));
	ConvexVerts.Add(FVector(-77, -42, 40));
	ConvexVerts.Add(FVector(-6, -14, -39));
	ConvexVerts.Add(FVector(-38, 38, -51));*/

	//Issues with SetSlicedFaceAsUnfoundNeighbour after while loop
		/*ConvexVerts.Add(FVector(70, -27, 16));
		ConvexVerts.Add(FVector(95, 47, 53));
		ConvexVerts.Add(FVector(73, -62, -58));
		ConvexVerts.Add(FVector(-86, -24, -19));
		ConvexVerts.Add(FVector(-71, 76, -73));
		ConvexVerts.Add(FVector(38, 23, 24));
		ConvexVerts.Add(FVector(-21, 87, -11));
		ConvexVerts.Add(FVector(30, 95, 57));
		ConvexVerts.Add(FVector(94, -89, 60));
		ConvexVerts.Add(FVector(73, -46, 22));
		ConvexVerts.Add(FVector(-85, -49, 40));
		ConvexVerts.Add(FVector(-32, -86, 88));
		ConvexVerts.Add(FVector(38, 28, -90));
		ConvexVerts.Add(FVector(54, 61, -66));
		ConvexVerts.Add(FVector(-51, 42, -54));
		ConvexVerts.Add(FVector(-44, -39, 80));
		ConvexVerts.Add(FVector(-66, 58, -9));
		ConvexVerts.Add(FVector(27, 37, -91));
		ConvexVerts.Add(FVector(-33, -47, -14));
		ConvexVerts.Add(FVector(53, -90, -76));
		ConvexVerts.Add(FVector(67, -16, -9));
		ConvexVerts.Add(FVector(4, -93, -24));
		ConvexVerts.Add(FVector(75, -35, -88));
		ConvexVerts.Add(FVector(85, 56, -73));
		ConvexVerts.Add(FVector(75, -65, -92));
		ConvexVerts.Add(FVector(99, 53, 94));
		ConvexVerts.Add(FVector(46, -73, -76));
		ConvexVerts.Add(FVector(-81, -93, -78));
		ConvexVerts.Add(FVector(47, -3, 72));
		ConvexVerts.Add(FVector(20, 5, 74));
		ConvexVerts.Add(FVector(-50, -39, -17));
		ConvexVerts.Add(FVector(41, 82, -62));
		ConvexVerts.Add(FVector(96, 64, 68));
		ConvexVerts.Add(FVector(-20, 67, -16));
		ConvexVerts.Add(FVector(12, -9, 53));
		ConvexVerts.Add(FVector(95, 89, 8));
		ConvexVerts.Add(FVector(61, 16, -71));
		ConvexVerts.Add(FVector(88, 43, 44));
		ConvexVerts.Add(FVector(-27, -37, -7));
		ConvexVerts.Add(FVector(-36, -62, -12));
		ConvexVerts.Add(FVector(-80, 12, -7));
		ConvexVerts.Add(FVector(85, -31, -62));
		ConvexVerts.Add(FVector(78, 64, 60));
		ConvexVerts.Add(FVector(29, 27, 83));
		ConvexVerts.Add(FVector(31, -89, -43));
		ConvexVerts.Add(FVector(62, 70, -78));
		ConvexVerts.Add(FVector(100, -100, 88));
		ConvexVerts.Add(FVector(-93, 35, 62));
		ConvexVerts.Add(FVector(-69, 0, 12));
		ConvexVerts.Add(FVector(42, -55, -76));
		ConvexVerts.Add(FVector(-19, -21, -80));
		ConvexVerts.Add(FVector(60, 26, 75));
		ConvexVerts.Add(FVector(9, 85, -79));
		ConvexVerts.Add(FVector(65, 64, 19));
		ConvexVerts.Add(FVector(56, -97, 39));
		ConvexVerts.Add(FVector(-99, -69, -38));
		ConvexVerts.Add(FVector(17, 84, -89));
		ConvexVerts.Add(FVector(24, -24, -21));
		ConvexVerts.Add(FVector(40, -66, 83));
		ConvexVerts.Add(FVector(-87, 15, -49));
		ConvexVerts.Add(FVector(-92, 30, 82));
		ConvexVerts.Add(FVector(-93, 6, -48));
		ConvexVerts.Add(FVector(8, 24, 7));
		ConvexVerts.Add(FVector(-84, 44, 41));
		ConvexVerts.Add(FVector(-3, -65, 31));
		ConvexVerts.Add(FVector(-24, 44, 38));
		ConvexVerts.Add(FVector(53, -99, 17));
		ConvexVerts.Add(FVector(-16, 60, 97));
		ConvexVerts.Add(FVector(93, -95, -94));
		ConvexVerts.Add(FVector(-10, -19, 49));
		ConvexVerts.Add(FVector(-66, -43, -15));
		ConvexVerts.Add(FVector(86, -15, 78));
		ConvexVerts.Add(FVector(13, -46, 61));
		ConvexVerts.Add(FVector(96, -22, -78));
		ConvexVerts.Add(FVector(-78, 66, -44));
		ConvexVerts.Add(FVector(6, -9, -78));
		ConvexVerts.Add(FVector(-78, 38, -94));
		ConvexVerts.Add(FVector(89, -1, -40));
		ConvexVerts.Add(FVector(50, -46, -83));
		ConvexVerts.Add(FVector(-11, 58, -86));
		ConvexVerts.Add(FVector(-53, 10, 98));
		ConvexVerts.Add(FVector(39, 26, -66));
		ConvexVerts.Add(FVector(-94, -10, -23));
		ConvexVerts.Add(FVector(92, 78, -30));
		ConvexVerts.Add(FVector(66, 38, -76));
		ConvexVerts.Add(FVector(20, 69, 13));
		ConvexVerts.Add(FVector(-54, 63, 72));
		ConvexVerts.Add(FVector(69, -68, -80));
		ConvexVerts.Add(FVector(-82, -86, -28));
		ConvexVerts.Add(FVector(-99, 45, -9));
		ConvexVerts.Add(FVector(-22, -26, -91));
		ConvexVerts.Add(FVector(-44, -3, 54));
		ConvexVerts.Add(FVector(-21, 65, -69));
		ConvexVerts.Add(FVector(-49, -38, -70));
		ConvexVerts.Add(FVector(-75, 6, -73));
		ConvexVerts.Add(FVector(32, 75, -32));
		ConvexVerts.Add(FVector(79, 77, 77));
		ConvexVerts.Add(FVector(15, 4, -55));
		ConvexVerts.Add(FVector(54, -11, -48));
		ConvexVerts.Add(FVector(74, 63, -44));*/


	//Issues in FacesAndNeighboursCorrection
	//ConvexVerts.Add(FVector(7, -85, 87));
	//ConvexVerts.Add(FVector(-31, 2, -86));
	//ConvexVerts.Add(FVector(56, 25, -17));
	//ConvexVerts.Add(FVector(79, 26, 80));
	//ConvexVerts.Add(FVector(-36, 98, -96));
	//ConvexVerts.Add(FVector(46, 99, -29));
	//ConvexVerts.Add(FVector(-33, -83, -69));
	//ConvexVerts.Add(FVector(-43, 21, 87));
	//ConvexVerts.Add(FVector(42, -40, -40));
	//ConvexVerts.Add(FVector(21, 2, 81));
	//ConvexVerts.Add(FVector(15, -96, -12));
	//ConvexVerts.Add(FVector(18, -52, 45));
	//ConvexVerts.Add(FVector(8, -16, 40));
	//ConvexVerts.Add(FVector(17, -61, -44));
	//ConvexVerts.Add(FVector(10, -22, 30));
	//ConvexVerts.Add(FVector(78, 25, -64));
	//ConvexVerts.Add(FVector(96, -46, -1));
	//ConvexVerts.Add(FVector(96, -4, -77));
	//ConvexVerts.Add(FVector(-93, -8, 12));
	//ConvexVerts.Add(FVector(-42, -62, -85));
	//ConvexVerts.Add(FVector(-45, -17, 3));
	//ConvexVerts.Add(FVector(89, 64, -44));
	//ConvexVerts.Add(FVector(4, 99, 19));
	//ConvexVerts.Add(FVector(27, -9, 73));
	//ConvexVerts.Add(FVector(-29, -64, 60));
	//ConvexVerts.Add(FVector(-40, -22, 17));
	//ConvexVerts.Add(FVector(38, 73, 39));
	//ConvexVerts.Add(FVector(91, 22, -71));
	//ConvexVerts.Add(FVector(22, 69, -86));
	//ConvexVerts.Add(FVector(-22, -12, -67));
	//ConvexVerts.Add(FVector(-93, -9, 67));
	//ConvexVerts.Add(FVector(99, 98, -57));
	//ConvexVerts.Add(FVector(-37, 63, -27));
	//ConvexVerts.Add(FVector(-93, 53, -80));
	//ConvexVerts.Add(FVector(18, 38, -63));
	//ConvexVerts.Add(FVector(22, -92, 4));
	//ConvexVerts.Add(FVector(18, 40, -89));
	//ConvexVerts.Add(FVector(68, 35, 10));
	//ConvexVerts.Add(FVector(-98, -51, -56));
	//ConvexVerts.Add(FVector(-93, 44, -12));
	//ConvexVerts.Add(FVector(95, 89, 45));
	//ConvexVerts.Add(FVector(93, -36, 87));
	//ConvexVerts.Add(FVector(66, 99, -26));
	//ConvexVerts.Add(FVector(55, 77, -39));
	//ConvexVerts.Add(FVector(-24, 17, 13));
	//ConvexVerts.Add(FVector(-48, 18, -4));
	//ConvexVerts.Add(FVector(-37, 11, 10));
	//ConvexVerts.Add(FVector(-67, 33, -48));
	//ConvexVerts.Add(FVector(-74, -98, 16));
	//ConvexVerts.Add(FVector(14, -42, 28));
	//ConvexVerts.Add(FVector(93, -7, -1));
	//ConvexVerts.Add(FVector(-22, 7, -30));
	//ConvexVerts.Add(FVector(-71, 68, 85));
	//ConvexVerts.Add(FVector(-9, -84, 10));
	//ConvexVerts.Add(FVector(27, 87, -98));
	//ConvexVerts.Add(FVector(-73, 75, 97));
	//ConvexVerts.Add(FVector(-48, 2, -66));
	//ConvexVerts.Add(FVector(-2, 39, -37));
	//ConvexVerts.Add(FVector(-47, -20, -80));
	//ConvexVerts.Add(FVector(-62, 20, 87));
	//ConvexVerts.Add(FVector(98, 73, 51));
	//ConvexVerts.Add(FVector(-38, 99, -57));
	//ConvexVerts.Add(FVector(-40, -93, 71));
	//ConvexVerts.Add(FVector(42, -88, 79));
	//ConvexVerts.Add(FVector(-47, -29, 26));
	//ConvexVerts.Add(FVector(6, -14, -46));
	//ConvexVerts.Add(FVector(89, -39, 18));
	//ConvexVerts.Add(FVector(-41, 35, 98));
	//ConvexVerts.Add(FVector(52, 60, 46));
	//ConvexVerts.Add(FVector(16, -53, 48));
	//ConvexVerts.Add(FVector(58, 74, 36));
	//ConvexVerts.Add(FVector(-25, 100, -95));
	//ConvexVerts.Add(FVector(-49, 74, -56));
	//ConvexVerts.Add(FVector(98, -38, 78));
	//ConvexVerts.Add(FVector(-86, 52, 3));
	//ConvexVerts.Add(FVector(-39, -18, -33));
	//ConvexVerts.Add(FVector(18, 41, -68));
	//ConvexVerts.Add(FVector(50, 42, -56));
	//ConvexVerts.Add(FVector(-42, -36, 29));
	//ConvexVerts.Add(FVector(25, 53, -44));
	//ConvexVerts.Add(FVector(97, -74, -10));
	//ConvexVerts.Add(FVector(-29, 89, -84));
	//ConvexVerts.Add(FVector(23, 41, 45));
	//ConvexVerts.Add(FVector(44, -85, 57));
	//ConvexVerts.Add(FVector(0, 97, 32));
	//ConvexVerts.Add(FVector(-1, -82, -37));
	//ConvexVerts.Add(FVector(83, -54, 48));
	//ConvexVerts.Add(FVector(20, 80, 99));
	//ConvexVerts.Add(FVector(-18, 18, 96));
	//ConvexVerts.Add(FVector(-20, 0, -81));
	//ConvexVerts.Add(FVector(-73, -92, -11));
	//ConvexVerts.Add(FVector(50, -54, -87));
	//ConvexVerts.Add(FVector(27, -33, -20));
	//ConvexVerts.Add(FVector(-1, 21, 86));
	//ConvexVerts.Add(FVector(-97, 16, 34));
	//ConvexVerts.Add(FVector(-88, 10, 44));
	//ConvexVerts.Add(FVector(97, -69, 14));
	//ConvexVerts.Add(FVector(29, -42, -21));
	//ConvexVerts.Add(FVector(3, 45, -49));
	//ConvexVerts.Add(FVector(-3, -69, 38));


	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////RESOLVED//////////////////////////////////////////////////////////////////


	

