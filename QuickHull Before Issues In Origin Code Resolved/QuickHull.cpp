// Fill out your copyright notice in the Description page of Project Settings.

#include "QuickHull.h"

#include "time.h"

#define FLT_ZERO 1e-5f

/**
Try To Set Very Small Float To Zero
**/
void TryTruncateFloatToZero(float& Value)
{
	if (Value < FLT_ZERO && Value > -FLT_ZERO)
		Value = 0;
}

/**
Adding Index to array if Index is next index relatively to array's last index
Otherwise inserting Index to start of array at Delta position
**/
void AddOrInsert(TArray<uint32>& IndicesArray,const uint32& Index, int& Delta)
{
	if (IndicesArray.Num() == 0 || (Index - IndicesArray.Last() == 1))
		IndicesArray.Add(Index);
	else if ((Index - IndicesArray.Last()) > 1)
	{
		IndicesArray.Insert(Index, Delta);
		Delta++;
	}
}
/**
Search First Sliced Face Or Face From Behind Of SlicePlane Having Edge Adjacent To SlicePlane 
Returns false If Slice Plane don't intersects Convex Hull
**/
bool SearchFirstSlicedFace(const TArray<FVector>& OriginVertices, const TArray<Face>& OriginFaces, const FPlane& SlicePlane, SlicedFace& slicedFace)
{
	TArray<uint32> AboveIndices, BelowIndices, PlaneIndices;
	int AboveDelta = 0, BelowDelta = 0;

	float Distance;
	for (int i = 0; i < OriginFaces.Num(); i++)
	{
		AboveIndices.Empty();
		BelowIndices.Empty();
		PlaneIndices.Empty();
		AboveDelta = 0;
		BelowDelta = 0;
		for (int j = 0; j < OriginFaces[i].Indices.Num(); j++)
		{
			Distance = SlicePlane.PlaneDot(OriginVertices[OriginFaces[i].Indices[j]]);
			TryTruncateFloatToZero(Distance);
			//point is on SlicePlane
			if (Distance == 0)
			{
				//three points of face is on SlicePlane
				if (PlaneIndices.Num() == 2)
				{
					return false;
				}
				AddOrInsert(AboveIndices, j, AboveDelta);
				AddOrInsert(BelowIndices, j, BelowDelta);
				PlaneIndices.Add(j);
			}
			else if (Distance < 0)
			{
				AddOrInsert(BelowIndices, j, BelowDelta);
			}
			else if (Distance > 0)
			{
				AddOrInsert(AboveIndices, j, AboveDelta);
			}
		}
		//Face intersects SlicePlane 
		if (AboveIndices.Num() > 0 && BelowIndices.Num() > 0)
		{
			//SlicePlane intersects face at two edges
			if (PlaneIndices.Num() == 0 && AboveIndices.Num() > 0 && BelowIndices.Num() > 0)
			{
				slicedFace = SlicedFace(OriginFaces[i]);
				slicedFace.LeftEdgeIndex = BelowIndices.Last();
				slicedFace.LeftSlicePoint = FMath::LinePlaneIntersection(
					OriginVertices[slicedFace.Indices[slicedFace.LeftEdgeIndex]],//left edge below point
					OriginVertices[slicedFace.Indices[AboveIndices[0]]],//left edge above point
					SlicePlane);
				slicedFace.RightEdgeIndex = AboveIndices.Last();
				slicedFace.RightSlicePoint = FMath::LinePlaneIntersection(
					OriginVertices[slicedFace.Indices[slicedFace.RightEdgeIndex]],//right edge above point
					OriginVertices[slicedFace.Indices[BelowIndices[0]]],//right edge below point
					SlicePlane);
				return true;
			}
			//SlicePlane intersects face in vertex and edge
			else if (PlaneIndices.Num() == 1 && AboveIndices.Num() > 1 && BelowIndices.Num() > 1)
			{
				slicedFace = SlicedFace(OriginFaces[i]);
				//SlicePlane intersects face in vertex at left
				if (AboveIndices[0] == BelowIndices.Last())
				{
					slicedFace.LeftVertexIsOnSlicePlane = true;
					slicedFace.LeftEdgeIndex = BelowIndices.Last(1);
					slicedFace.LeftSlicePoint = OriginVertices[slicedFace.Indices[AboveIndices[0]]];
					slicedFace.RightEdgeIndex = AboveIndices.Last();
					slicedFace.RightSlicePoint = FMath::LinePlaneIntersection(
						OriginVertices[slicedFace.Indices[slicedFace.RightEdgeIndex]],//right edge above point
						OriginVertices[slicedFace.Indices[BelowIndices[0]]],//right edge below point
						SlicePlane);
				}
				//SlicePlane intersects face in vertex at right
				else if (AboveIndices.Last() == BelowIndices[0])
				{
					slicedFace.LeftEdgeIndex = BelowIndices.Last();
					slicedFace.LeftSlicePoint = FMath::LinePlaneIntersection(
						OriginVertices[slicedFace.Indices[slicedFace.LeftEdgeIndex]],//left edge below point
						OriginVertices[slicedFace.Indices[AboveIndices[0]]],//left edge above point
						SlicePlane);
					slicedFace.RightVertexIsOnSlicePlane = true;
					slicedFace.RightEdgeIndex = AboveIndices.Last();
					slicedFace.RightSlicePoint = OriginVertices[slicedFace.Indices[slicedFace.RightEdgeIndex]];
				}
				return true;
			}
			else if (PlaneIndices.Num() == 2)
			{
				//face has edge adjacent to SlicePlane and other part is behind or in front of SlicePlane 
				if (AboveIndices.Num() == 2 || BelowIndices.Num() == 2)
				{
					//neighbour face is at left plane index
					//if left plane index was found before right plane index - adjacent to SlicePlane edge index is PlaneIndices[0] , otherwise - PlaneIndices[1]
					uint32 AdjEdgeIndex = (PlaneIndices[1] - PlaneIndices[0] == 1 ? PlaneIndices[0] : PlaneIndices[1]);
					Face NeighbourFace = OriginFaces[OriginFaces[i].Neighbours[AdjEdgeIndex]];
					//index of neighbour's edge adjacent to SlicePlane
					uint32 NeighAdjEdgeInd = *NeighbourFace.Neighbours.FindKey(i);
					//distance of previous neighbour vertex relatively to adjacent edge
					uint32 NeighLeftEdgeInd = NeighAdjEdgeInd == 0 ? NeighbourFace.Indices.Num() - 1 : NeighAdjEdgeInd - 1;
					float NeighPrevPointDist = SlicePlane.PlaneDot(OriginVertices[NeighbourFace.Indices[NeighLeftEdgeInd]]);
					TryTruncateFloatToZero(NeighPrevPointDist);
					//NeighbourFace and SlicePlane are complanar
					if (NeighPrevPointDist == 0)
					{
						return false;
					}
					else
					{
						//Convex Hull has edge on SlicePlane and other part is at one side from SlicePlane
						if ((AboveIndices.Num() > 2 && NeighPrevPointDist > 0) || (BelowIndices.Num() > 2 && NeighPrevPointDist < 0))
						{
							return false;
						}
						//current face is in front of SlicePlane - slicedFace is its neighbour
						else if (AboveIndices.Num() > 0 && NeighPrevPointDist < 0)
						{
							slicedFace = SlicedFace(NeighbourFace);
							slicedFace.EdgeOnSlicePlane = true;
							slicedFace.EdgeOnSlicePlaneIndex = NeighAdjEdgeInd;
							slicedFace.LeftVertexIsOnSlicePlane = true;
							slicedFace.LeftEdgeIndex = NeighLeftEdgeInd;
							slicedFace.LeftSlicePoint = OriginVertices[slicedFace.Indices[slicedFace.EdgeOnSlicePlaneIndex]];
							slicedFace.RightVertexIsOnSlicePlane = true;
							slicedFace.RightEdgeIndex = NeighAdjEdgeInd == slicedFace.Neighbours.Num() - 1 ? 0 : NeighAdjEdgeInd + 1;
							slicedFace.RightSlicePoint = OriginVertices[slicedFace.Indices[slicedFace.RightEdgeIndex]];
							return true;
						}
						//current face is slicedFace
						else if (BelowIndices.Num() > 0 && NeighPrevPointDist > 0)
						{
							slicedFace = SlicedFace(OriginFaces[i]);
							slicedFace.EdgeOnSlicePlane = true;
							slicedFace.EdgeOnSlicePlaneIndex = AdjEdgeIndex;
							slicedFace.LeftVertexIsOnSlicePlane = true;
							slicedFace.LeftEdgeIndex = AdjEdgeIndex == 0 ? slicedFace.Indices.Num() - 1 : AdjEdgeIndex - 1;
							slicedFace.LeftSlicePoint = OriginVertices[slicedFace.Indices[slicedFace.EdgeOnSlicePlaneIndex]];
							slicedFace.RightVertexIsOnSlicePlane = true;
							slicedFace.RightEdgeIndex = AdjEdgeIndex == slicedFace.Neighbours.Num() - 1 ? 0 : AdjEdgeIndex + 1;
							slicedFace.RightSlicePoint = OriginVertices[slicedFace.Indices[slicedFace.RightEdgeIndex]];
							return true;
						}
					}
				}
				//SlicePlane intersects face in two vertices not on one edge
				else
				{
					//AboveIndices[0] == BelowIndices.Last()
					//AboveIndices.Last() == BelowIndices[0]
					slicedFace = SlicedFace(OriginFaces[i]);
					slicedFace.LeftVertexIsOnSlicePlane = true;
					slicedFace.LeftEdgeIndex = BelowIndices.Last(1);
					slicedFace.LeftSlicePoint = OriginVertices[slicedFace.Indices[AboveIndices[0]]];
					slicedFace.RightVertexIsOnSlicePlane = true;
					slicedFace.RightEdgeIndex = BelowIndices[0];
					slicedFace.RightSlicePoint = OriginVertices[slicedFace.Indices[BelowIndices[0]]];
					return true;
				}
			}
		}		
	}
	return false;
}

/**
Search Sliced Faces in CW order from behind of SlicePlane
**/
void SearchSlicedFaces(const TArray<FVector>& OriginVertices, const TArray<Face>& OriginFaces, const FPlane& SlicePlane, TArray<SlicedFace>& SlicedFaces)
{
	uint32 CurrentFaceIndex = SlicedFaces[0].Number;
	uint32 NextFaceIndex = SlicedFaces[0].Neighbours[SlicedFaces[0].RightEdgeIndex];
	SlicedFace CurrentSlicedFace = SlicedFaces[0];
	SlicedFace NextSlicedFace;
	float Distance;
	while (NextFaceIndex != SlicedFaces[0].Number)
	{		
		NextSlicedFace = SlicedFace(OriginFaces[NextFaceIndex]);
		NextSlicedFace.LeftVertexIsOnSlicePlane = CurrentSlicedFace.RightVertexIsOnSlicePlane;
		NextSlicedFace.LeftEdgeIndex = *NextSlicedFace.Neighbours.FindKey(CurrentFaceIndex);
		NextSlicedFace.LeftSlicePoint = CurrentSlicedFace.RightSlicePoint;

		//Index of next point index after left sliced edge
		uint32 NextPointIndex = NextSlicedFace.LeftEdgeIndex == NextSlicedFace.Indices.Num() - 2 ? 0 :
			(NextSlicedFace.LeftEdgeIndex == NextSlicedFace.Indices.Num() - 1 ? 1 : NextSlicedFace.LeftEdgeIndex + 2);		
		Distance = SlicePlane.PlaneDot(OriginVertices[NextSlicedFace.Indices[NextPointIndex]]);
		TryTruncateFloatToZero(Distance);

		int AbovePointsCount = 0;
		//Search First point behind or on SlicePlane
		while (Distance > 0)
		{
			NextPointIndex = NextPointIndex == NextSlicedFace.Indices.Num() - 1 ? 0 : NextPointIndex + 1;
			Distance = SlicePlane.PlaneDot(OriginVertices[NextSlicedFace.Indices[NextPointIndex]]);
			TryTruncateFloatToZero(Distance);
			AbovePointsCount++;
		}

		//Next point is behind from SlicePlane
		if (Distance < 0)
		{
			//NextSlicedFace has one point on SlicePlane and other part is behind from SlicePlane - search NextSlicedFace's neighbour
			if (NextSlicedFace.LeftVertexIsOnSlicePlane && AbovePointsCount == 0)
			{
				NextSlicedFace.RightVertexIsOnSlicePlane = true;
				NextSlicedFace.RightEdgeIndex = NextSlicedFace.LeftEdgeIndex == NextSlicedFace.Indices.Num() - 1 ? 0 : NextSlicedFace.LeftEdgeIndex + 1;
				NextSlicedFace.RightSlicePoint = NextSlicedFace.LeftSlicePoint;				
			}
			else
			{
				if (AbovePointsCount == 0)
					NextSlicedFace.RightEdgeIndex = NextSlicedFace.LeftEdgeIndex == NextSlicedFace.Indices.Num() - 1 ? 0 : NextSlicedFace.LeftEdgeIndex + 1;
				else
					NextSlicedFace.RightEdgeIndex = NextPointIndex == 0 ? NextSlicedFace.Indices.Num() - 1 : NextPointIndex - 1;
				NextSlicedFace.RightSlicePoint = FMath::LinePlaneIntersection(
					OriginVertices[NextSlicedFace.Indices[NextSlicedFace.RightEdgeIndex]],//point from above of SlicePlane (at RightEdge)										
					OriginVertices[NextSlicedFace.Indices[NextPointIndex]],//point from below of SlicePlane (next point after RightEdge)
					SlicePlane);

				//Add NextSlicedFace to SlicedFaces array
				SlicedFaces.Add(NextSlicedFace);
			}
		}
		//Next Point Is on SlicePlane
		else if (Distance == 0)
		{
			if (NextSlicedFace.LeftVertexIsOnSlicePlane && AbovePointsCount == 0)
			{
				NextSlicedFace.EdgeOnSlicePlane = true;
				NextSlicedFace.EdgeOnSlicePlaneIndex = NextSlicedFace.LeftEdgeIndex == NextSlicedFace.Indices.Num() - 1 ? 0 : NextSlicedFace.LeftEdgeIndex + 1;
			}
			NextSlicedFace.RightVertexIsOnSlicePlane = true;
			NextSlicedFace.RightEdgeIndex = NextPointIndex;
			NextSlicedFace.RightSlicePoint = OriginVertices[NextSlicedFace.Indices[NextSlicedFace.RightEdgeIndex]];

			//Add NextSlicedFace to SlicedFaces array
			SlicedFaces.Add(NextSlicedFace);			
		}
		CurrentSlicedFace = NextSlicedFace;
		CurrentFaceIndex = CurrentSlicedFace.Number;
		NextFaceIndex = CurrentSlicedFace.Neighbours[CurrentSlicedFace.RightEdgeIndex];
		
	}
}

void CheckFace(const Face& face)
{
	uint32 VertexIndex;
	uint32 NeighbourIndex;
	
	for (int i = 0; i < face.Indices.Num(); i++)
	{
		VertexIndex = face.Indices[i];
		NeighbourIndex = face.Neighbours[i];
	}
}

bool SetSlicedFaces(TArray<Face>& Faces, const TArray<SlicedFace>& SlicedFaces)
{	
	for (int i = 0; i < SlicedFaces.Num(); i++)
	{
		 Faces[SlicedFaces[i].Number].IsSlicedFace = true;
	}
	return false;
}

void BuildSlicedHulls(TArray<FVector>& FirstHalfVertices, TArray<Face>& FirstHalfFaces,//First half is behind plane
	TArray<FVector>& SecondHalfVertices, TArray<Face>& SecondHalfFaces, const TArray<SlicedFace>& SlicedFaces)
{
	Face FirstSliceFace;
	FirstSliceFace.Number = FirstHalfFaces.Num();	
	//temp array of neighbour faces numbers for FirstSliceFace
	TArray<long> FirstFaceNeighbours;
	Face SecondSliceFace;
	SecondSliceFace.Number = SecondHalfFaces.Num();	
	
	//FirstHalfFaces.Add(FirstSliceFace);
	//SecondHalfFaces.Add(SecondSliceFace);
	

	//Slice SlicedFaces
	//Reorder Indices And Neighbours if face intersects by SlicePlane 
	//If face has adjacent edge to SlicePlane - just change neighbours at adjacent edge
	for (int i = 0; i < SlicedFaces.Num(); i++)
	{
		SlicedFace slicedFace = SlicedFaces[i];
		if (slicedFace.EdgeOnSlicePlane)
		{
			//Set for slicedFace's neighbour at adjacent edge slicedFace as unfound neighbour
			Face& Neighbour = FirstHalfFaces[slicedFace.Neighbours[slicedFace.EdgeOnSlicePlaneIndex]];
			uint32 NeighEdgeIndex = *Neighbour.Neighbours.FindKey(slicedFace.Number);
			Neighbour.Neighbours[NeighEdgeIndex] = -1;
			Neighbour.AllNeighboursFound = false;

			//Set slicedFaces's neighbour at adjacent edge as FirstSliceFace
			FirstHalfFaces[slicedFace.Number].Neighbours[slicedFace.EdgeOnSlicePlaneIndex] = FirstSliceFace.Number;
			
			//Insert Index and Neighbour to FirstSliceFace
			//Index is RightEdgeIndex
			//Neighbour is slicedFace
			FirstSliceFace.Indices.Insert(slicedFace.Indices[slicedFace.RightEdgeIndex], 0);
			FirstFaceNeighbours.Insert(slicedFace.Number, 0);
#ifdef WITH_DEBUG
			CheckFace(FirstHalfFaces[slicedFace.Number]);				
#endif
			//Same operations for second half
			Face& SecNeighbour = SecondHalfFaces[slicedFace.Number];			
			SecNeighbour.Neighbours[slicedFace.EdgeOnSlicePlaneIndex] = -1;
			SecNeighbour.AllNeighboursFound = false;
			
			SecondHalfFaces[Neighbour.Number].Neighbours[NeighEdgeIndex] = SecondSliceFace.Number;

			//Add Index and Neighbour to SecondSliceFace
			//Index is left slice point index of slicedFace
			//Neighbour is Neighbour of slicedFace at adjacent edge to SlicePlane
 			SecondSliceFace.Indices.Add(slicedFace.Indices[slicedFace.EdgeOnSlicePlaneIndex]);
			SecondSliceFace.Neighbours.Add(SecondSliceFace.Neighbours.Num(), Neighbour.Number);
#ifdef WITH_DEBUG
			CheckFace(SecondHalfFaces[Neighbour.Number]);
#endif
		}
		else
		{
			Face& FirstHalfFace = FirstHalfFaces[slicedFace.Number];
			Face& SecondHalfFace = SecondHalfFaces[slicedFace.Number];

			//Index of LeftSlicePoint in array of vertices for FirstHalfFace
			uint32 First_L_Vert_Ind;
			//Index of RightSlicePoint in array of vertices for SecondHalfFace = First_L_Vert_Ind
			uint32 Second_R_Vert_Ind;
			
			//Add LeftSlicePoint of first SlicedFace to vertices array
			if (i == 0 && !slicedFace.LeftVertexIsOnSlicePlane)
			{
				First_L_Vert_Ind = FirstHalfVertices.Num();
				FirstHalfVertices.Add(slicedFace.LeftSlicePoint);
				SecondHalfVertices.Add(slicedFace.LeftSlicePoint);
			}
			else if (i == 0 && slicedFace.LeftVertexIsOnSlicePlane)
				//Index of next vertex relatively to SlicedFaces[i].LeftEdgeIndex		
				First_L_Vert_Ind = slicedFace.Indices[slicedFace.LeftEdgeIndex == slicedFace.Neighbours.Num() - 1 ? 0 : slicedFace.LeftEdgeIndex + 1];
			else if (i > 0)
			{
				Face& PrevFace = FirstHalfFaces[SlicedFaces[i - 1].Number];
				//Index of RightSlicePoint of previous SlicedFace
				//If previous SlicedFace has adjacent edge to SlicePlane - Index of its RightSlicePoint is its RightEdgeIndex
				First_L_Vert_Ind = PrevFace.Indices[SlicedFaces[i - 1].EdgeOnSlicePlane ? SlicedFaces[i - 1].RightEdgeIndex : 1];
			}
			Second_R_Vert_Ind = First_L_Vert_Ind;

			//Index of RightSlicePoint in array of vertices for FirstHalfFace
			uint32 First_R_Vert_Ind;
			//Index of LeftSlicePoint in array of vertices for SecondHalfFace = First_R_Vert_Ind
			uint32 Second_L_Vert_Ind;
				
			//Add RightSlicePoint to vertices array if SlicedFace isn't last one
			if (i<SlicedFaces.Num()-1&&!slicedFace.RightVertexIsOnSlicePlane)
			{
				First_R_Vert_Ind = FirstHalfVertices.Num();
				FirstHalfVertices.Add(slicedFace.RightSlicePoint);
				SecondHalfVertices.Add(slicedFace.RightSlicePoint);
			}
			else if(i < SlicedFaces.Num() - 1 && slicedFace.RightVertexIsOnSlicePlane)
				//Index of current vertex relatively to SlicedFaces[i].RightEdgeIndex	
				First_R_Vert_Ind = slicedFace.Indices[slicedFace.RightEdgeIndex];
			//First_R_Vert_Ind of last SlicedFace is First_L_Vert_Ind of first SlicedFace
			else if (i == SlicedFaces.Num() - 1)
			{
				Face& FirstFace = FirstHalfFaces[SlicedFaces[0].Number];
				//If first SlicedFace has adjacent edge to SlicePlane - Index of its LeftSlicePoint is its EdgeOnSlicePlaneIndex
				First_R_Vert_Ind = FirstFace.Indices[SlicedFaces[0].EdgeOnSlicePlane ? SlicedFaces[0].EdgeOnSlicePlaneIndex : 0];
			}
			Second_L_Vert_Ind = First_R_Vert_Ind;

			//Insert Index and Neighbour to FirstSliceFace
			//Index is Right Slice Point of slicedFace index (First_R_Vert_Ind)
			//Neighbour is slicedFace
			FirstSliceFace.Indices.Insert(First_R_Vert_Ind, 0);
			FirstFaceNeighbours.Insert(slicedFace.Number, 0);

			//Add Index and Neighbour to SecondSliceFace
			//Index is left slice point index of slicedFace
			//Neighbour is slicedFace
			SecondSliceFace.Indices.Add(Second_R_Vert_Ind);
			SecondSliceFace.Neighbours.Add(SecondSliceFace.Neighbours.Num(), slicedFace.Number);
			
			//For FirstHalfFace:
			//Set for SlicedFace's neighbours at edges from above of SlicePlane slicedFace as unfound neighbour
			//First edge from above index is next edge index after LeftEdgeIndex
			uint32 CurrentEdgeIndex = slicedFace.LeftEdgeIndex == slicedFace.Neighbours.Num() - 1 ? 0 : slicedFace.LeftEdgeIndex + 1;
			while (CurrentEdgeIndex != slicedFace.RightEdgeIndex)
			{
				Face& Neighbour = FirstHalfFaces[slicedFace.Neighbours[CurrentEdgeIndex]];
				if (!Neighbour.IsSlicedFace)
				{
					uint32 NeighEdgeIndex = *Neighbour.Neighbours.FindKey(slicedFace.Number);
					Neighbour.Neighbours[NeighEdgeIndex] = -1;
					Neighbour.AllNeighboursFound = false;
				}

				CurrentEdgeIndex = CurrentEdgeIndex == slicedFace.Neighbours.Num() - 1 ? 0 : CurrentEdgeIndex + 1;
			}

			//For SecondHalfFace:			
			//CurrentEdgeIndex == slicedFace.RightEdgeIndex at this moment			
			//If not RightVertexIsOnSlicePlane Set CurrentEdgeIndex as next index after slicedFace.RightEdgeIndex
			if(!slicedFace.RightVertexIsOnSlicePlane)
				CurrentEdgeIndex = CurrentEdgeIndex == slicedFace.Neighbours.Num() - 1 ? 0 : CurrentEdgeIndex + 1;
			
			uint32 FinishEdgeIndex;
			//FinishEdgeIndex for setting slicedFace as notfound neighbour for faces from below of SlicePlane
			//If not LeftVertexIsOnSlicePlane FinishEdgeIndex is LeftEdgeIndex
			//Otherwise FinishEdgeIndex is index of next edge after LeftEdge
			if (!slicedFace.LeftVertexIsOnSlicePlane)
				FinishEdgeIndex = slicedFace.LeftEdgeIndex;
			else
				FinishEdgeIndex = slicedFace.LeftEdgeIndex == slicedFace.Neighbours.Num() - 1 ? 0 : slicedFace.LeftEdgeIndex + 1;
			//Set for SlicedFace's neighbours at edges from below of SlicePlane slicedFace as unfound neighbour
			while (CurrentEdgeIndex != FinishEdgeIndex)
			{
				Face& Neighbour = SecondHalfFaces[slicedFace.Neighbours[CurrentEdgeIndex]];
				if (!Neighbour.IsSlicedFace)
				{
					uint32 NeighEdgeIndex = *Neighbour.Neighbours.FindKey(slicedFace.Number);
					Neighbour.Neighbours[NeighEdgeIndex] = -1;
					Neighbour.AllNeighboursFound = false;
				}

				CurrentEdgeIndex = CurrentEdgeIndex == slicedFace.Neighbours.Num() - 1 ? 0 : CurrentEdgeIndex + 1;
			}

			//For FirstHalfFace:
			//Reorder Indices and neighbours for FirstHalfFace
			//First Index is First_L_Vert_Ind, second Index is First_R_Vert_Ind, first Neigbour is FirstSliceFace
			TArray<uint32> NewIndices;
			TMap<uint32, long> NewNeighbours;
			NewIndices.Add(First_L_Vert_Ind);
			NewNeighbours.Add(NewNeighbours.Num(), FirstSliceFace.Number);

			NewIndices.Add(First_R_Vert_Ind);
			//Second Neighbour is neighbour at RightEdge
			//Set CurrentEdgeIndex as RightEdgeIndex
			CurrentEdgeIndex = slicedFace.RightEdgeIndex;
			NewNeighbours.Add(NewNeighbours.Num(), slicedFace.Neighbours[CurrentEdgeIndex]);
						
			//Add Indices and neighbours from below of SlicePlane
			do {				
				CurrentEdgeIndex = CurrentEdgeIndex == slicedFace.Neighbours.Num() - 1 ? 0 : CurrentEdgeIndex + 1;				
				NewIndices.Add(slicedFace.Indices[CurrentEdgeIndex]);
				NewNeighbours.Add(NewNeighbours.Num(), slicedFace.Neighbours[CurrentEdgeIndex]);
			} while (CurrentEdgeIndex!= slicedFace.LeftEdgeIndex);
			FirstHalfFace.Indices = NewIndices;
			FirstHalfFace.Neighbours = NewNeighbours;
			FirstHalfFace.AllNeighboursFound = true;
#ifdef WITH_DEBUG
			CheckFace(FirstHalfFace);
#endif
			//For SecondHalfFace:
			//Reorder Indices and neighbours for SecondHalfFace
			//First Index is Second_L_Vert_Ind, second Index is Second_R_Vert_Ind, first Neigbour is SecondSliceFace
			NewIndices.Empty();
			NewNeighbours.Empty();
			NewIndices.Add(Second_L_Vert_Ind);
			NewNeighbours.Add(NewNeighbours.Num(), SecondSliceFace.Number);

			NewIndices.Add(Second_R_Vert_Ind);			
			//CurrentEdgeIndex == slicedFace.LeftEdgeIndex at this moment
			//If LeftVertexIsOnSlicePlane set CurrentEdgeIndex as next edge index after LeftEdge		
			if (slicedFace.LeftVertexIsOnSlicePlane)
			{
				CurrentEdgeIndex = CurrentEdgeIndex == slicedFace.Neighbours.Num() - 1 ? 0 : CurrentEdgeIndex + 1;
			}
			//Second Neighbour is neighbour at CurrentEdge
			NewNeighbours.Add(NewNeighbours.Num(), slicedFace.Neighbours[CurrentEdgeIndex]);

			//FinishEdgeIndex for reordering Indices and Neighbours: 
			//If RightVertexIsOnSlicePlane set FinishEdgeIndex as previous edge index from RightEdge
			//Othrewise FinishEdgeIndex is RightEdgeIndex
			FinishEdgeIndex = 
				slicedFace.RightVertexIsOnSlicePlane ? (slicedFace.RightEdgeIndex == 0 ? slicedFace.Neighbours.Num() - 1 : slicedFace.RightEdgeIndex - 1) : slicedFace.RightEdgeIndex;
			
			//Add Indices and neighbours from above of SlicePlane
			do {				
				CurrentEdgeIndex = CurrentEdgeIndex == slicedFace.Neighbours.Num() - 1 ? 0 : CurrentEdgeIndex + 1;				
				NewIndices.Add(slicedFace.Indices[CurrentEdgeIndex]);
				NewNeighbours.Add(NewNeighbours.Num(), slicedFace.Neighbours[CurrentEdgeIndex]);
			} while (CurrentEdgeIndex != FinishEdgeIndex);
			SecondHalfFace.Indices = NewIndices;
			SecondHalfFace.Neighbours = NewNeighbours;
			SecondHalfFace.AllNeighboursFound = true;
#ifdef WITH_DEBUG
			CheckFace(SecondHalfFace);
#endif
		}
	}
	//Fill FirstSliceFace.Neighbours
	for (int i = 0; i < FirstFaceNeighbours.Num(); i++)
	{
		FirstSliceFace.Neighbours.Add(i, FirstFaceNeighbours[i]);
	}
	FirstSliceFace.AllNeighboursFound = true;
	SecondSliceFace.AllNeighboursFound = true;
	CheckFace(FirstSliceFace);
	CheckFace(SecondSliceFace);
}

bool SliceConvexHull(const TArray<FVector>& OriginVertices, const TArray<Face>& OriginFaces, const FPlane& SlicePlane,
	TArray<FVector>& FirstHalfVertices, TArray<Face>& FirstHalfFaces,//First half is behind plane
	TArray<FVector>& SecondHalfVertices, TArray<Face>& SecondHalfFaces)
{
	
	TArray<SlicedFace> SlicedFaces;

	

	//Find first sliced face or face from behind of SlicePlane having edge on SlicePlane
	SlicedFace slicedFace;
	bool FirstSlicedFaceFound = SearchFirstSlicedFace(OriginVertices, OriginFaces, SlicePlane, slicedFace);
	if (!FirstSlicedFaceFound)
		return false;
	else
	{
		SlicedFaces.Add(slicedFace);
		SearchSlicedFaces(OriginVertices, OriginFaces, SlicePlane, SlicedFaces);
		FirstHalfVertices = OriginVertices;
		FirstHalfFaces = OriginFaces;
		SetSlicedFaces(FirstHalfFaces, SlicedFaces);
		SecondHalfVertices = OriginVertices;		
		SecondHalfFaces = FirstHalfFaces;

		BuildSlicedHulls(FirstHalfVertices, FirstHalfFaces, SecondHalfVertices, SecondHalfFaces, SlicedFaces);
	}

	/*uint32 VertexIndex, NeighbourIndex;
	for (int i = 0; i < SlicedFaces.Num(); i++)
	{
		slicedFace = SlicedFaces[i];
		for (int j = 0; j < slicedFace.Indices.Num(); j++)
		{
			VertexIndex = slicedFace.Indices[j];
			NeighbourIndex = slicedFace.Neighbours[j];
		}
	}*/
	return true;

}




//Check vectors for collinearity
bool PointsAreOnOneLine(FVector A, FVector B, FVector C)
{
	FVector AB = B - A;
	FVector AC = C - A;
	if (AB.IsZero() || AC.IsZero())
		return true;

	float cos = CosBetweenVerts(AB, AC);

	if (cos == 1 || cos == -1)
		return true;

	return false;
}


float CosBetweenPlanes(const FPlane& A, const FPlane& B)
{
	FVector ANorm = (FVector)A;
	FVector BNorm = (FVector)B;
	return CosBetweenVerts(A, B);
}

float CosBetweenVerts(const FVector& A, const FVector& B)
{
	return ((A | B) / (A.Size()*B.Size()));
}

template <typename T>
int GetIndex(const TArray<T> Array, const T& Elem)
{
	for (int i = 0; i < Array.Num(); i++)
	{
		if (Array[i] == Elem)
			return i;
	}
	return -1;
}

template <typename T>
bool Contains3DArray(const TArray<TArray<T>> Array, const T& Elem)
{
	for (int i = 0; i < Array.Num(); i++)
	{
		if (Array[i].Contains(Elem))
			return true;
	}
	return false;
}

void CheckNeighbours(Face& F)
{
	for (int i = 0; i < F.Neighbours.Num(); i++)
	{
		if (F.Neighbours[i] == -1)
			return;
	}
	F.AllNeighboursFound = true;
}

void FillTempVertices(TArray<BYTE>& TempIndices, const TArray<FVector>& HullVertices)
{
	for (BYTE i = 0; i < HullVertices.Num(); i++)
		TempIndices.Add(i);
}

/**
Remove indices of found face and excess indices
**/
void RemoveTempVertices(TArray<BYTE>& TempVertIndices, const TArray<BYTE>& PlaneIndices, const TArray<BYTE>& ExcessIndices)
{
	if (TempVertIndices.Num() > 0)
	{
		for (int i = 0; i < PlaneIndices.Num(); i++)
		{
			TempVertIndices.Remove(PlaneIndices[i]);
		}
		for (int i = 0; i < ExcessIndices.Num(); i++)
		{
			TempVertIndices.Remove(ExcessIndices[i]);
		}
	}
}


bool ComplanarPlanes(const FPlane& A, const FPlane& B)
{
	FPlane C = B * (-1);
	return (A == B) || (A == C);
}

qh_vertex_t const* TArray_To_Pointer_Array(const TArray<FVector>& Vertices)
{
	qh_vertex_t* vertices=new qh_vertex_t[Vertices.Num()];
	for (int i = 0; i < Vertices.Num(); i++)
	{
		vertices[i] = Vertices[i];
	}
	return vertices;
}