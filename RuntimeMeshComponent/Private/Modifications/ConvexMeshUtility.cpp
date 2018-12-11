// Copyright 2016-2018 Chris Conway (Koderz). All Rights Reserved.

#include "ConvexMeshUtility.h"

#define MIN_HULL_VERT_DISTANCE		(0.1f)
#define MIN_HULL_VALID_DIMENSION	(0.5f)

void AddVertexIfNotPresent(TArray<FVector> &Vertices, const FVector& NewVertex)
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

void RemoveDuplicateVerts(TArray<FVector>& InVerts)
{
	TArray<FVector> BackupVerts = InVerts;
	InVerts.Empty();

	for (int32 i = 0; i < BackupVerts.Num(); i++)
	{
		AddVertexIfNotPresent(InVerts, BackupVerts[i]);
	}
}


float DistanceToLine(const FVector& LineStart, const FVector& LineEnd, const FVector& Point)
{
	const FVector StartToEnd = LineEnd - LineStart;
	const FVector PointToStart = LineStart - Point;

	const FVector Cross = StartToEnd ^ PointToStart;
	return Cross.Size() / StartToEnd.Size();
}

bool EnsureHullIsValid(TArray<FVector>& InVerts)
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

bool IsNewCombination(TArray<int> Combination, TArray<TArray<int>>& CheckedCombinations)
{
	const int Size = 3;

	int ArraySize = CheckedCombinations.Num();
	if (ArraySize == 0)
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

bool PointsOfPlane(TArray<TArray<FVector>>PlanesAndTheirPoints, FVector A, FVector B, FVector C)
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
bool PointsOnOneLine(FVector A, FVector B, FVector C)
{
	FVector AB = B - A;
	FVector AC = C - A;
	if (AB.IsZero() || AC.IsZero())
		return true;

	float Coef = 0;
	if (AB.X != 0)
	{
		Coef = AC.X / AB.X;
	}
	else if (AB.Y != 0)
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
bool PlaneIsValid(FVector A, FVector B, FVector C, TArray<FVector> HullVertices, TArray<TArray<FVector>>& PlanesAndTheirPoints, TArray<FPlane>& Planes, bool& AllPointsAreOnPlane)
{

	if (PointsOnOneLine(A, B, C))
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
				if ((Distance > 0 && !PointsShouldBeInFrontOfPlane) || (Distance < 0 && PointsShouldBeInFrontOfPlane))
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

//Detecting Plains Of Mesh From Array Of Points
bool GetPlanesFromHull(TArray<FVector> HullVertices, TArray<FPlane>& Planes, TArray<TArray<FVector>>& PlanesAndTheirPoints)
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
	for (int a = 0; a < HullVertices.Num() - 2; a++)
	{
		if (AllPointsAreOnPlane)
			break;
		for (int b = a + 1; b < HullVertices.Num() - 1; b++)
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
	return true;
}

//Rewrite PointsOfPlane into two-points array
void FindSlicePoints(const TArray<FVector>& FrontPoints, const TArray<FVector>& BehindPoints, TArray<FVector>& PointsOfPlane, const FPlane& SlicePlane)
{
	for (int i = 0; i < FrontPoints.Num(); i++)
	{
		for (int j = 0; j < BehindPoints.Num(); j++)
		{
			PointsOfPlane.AddUnique(FMath::LinePlaneIntersection(FrontPoints[i], BehindPoints[j], SlicePlane));
		}
	}
	int I = 0, J = 0;
	FVector Temp;
	float MaxLength = 0;
	for (int i = 0; i < PointsOfPlane.Num() - 1; i++)
	{
		for (int j = i + 1; j < PointsOfPlane.Num(); j++)
		{
			Temp = PointsOfPlane[i] - PointsOfPlane[j];
			float TempLength = Temp.Size();

			if (MaxLength < TempLength)
			{
				MaxLength = TempLength;
				I = i;
				J = j;
			}
		}
	}
	TArray<FVector> NewPoints;
	NewPoints.Add(PointsOfPlane[I]);
	NewPoints.Add(PointsOfPlane[J]);
	PointsOfPlane.Empty();
	PointsOfPlane = NewPoints;
}

void AddUniqueVector(const TArray<FVector>& InVertices, TArray<FVector>& OutVertices)
{
	for (int i = 0; i < InVertices.Num(); i++)
	{
		OutVertices.AddUnique(InVertices[i]);
	}
}

bool SliceHull(const TArray<TArray<FVector>>& PlanesAndTheirPoints, const FPlane& SlicePlane, TArray<FVector>& OneHalfVertices, TArray<FVector>& OtherHalfVertices, bool CreateOtherHalf)
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
			else if (Distance < 0)
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
			FindSlicePoints(FrontPoints, BehindPoints, PointsOfPlane, SlicePlane);
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
