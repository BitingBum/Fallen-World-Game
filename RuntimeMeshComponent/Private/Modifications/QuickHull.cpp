// Fill out your copyright notice in the Description page of Project Settings.

#include "QuickHull.h"

#include "time.h"

#include <fstream>

#include <sstream>
using std::stringstream;
#include <iostream>
#include <iomanip>

using std::cout;
using std::endl;
using std::cin;
using namespace std;

//#define WITH_DEBUG

void CheckEdges(const TArray<EdgeIndices>& Edges)
{
	int32 VertexIndex0;
	int32 VertexIndex1;
	for (int i = 0; i < Edges.Num(); i++)
	{
		VertexIndex0 = Edges[i].Index0;
		VertexIndex1 = Edges[i].Index1;
	}
}

/**
Check If Triangle is adjacent to SlicePlane in edge or vertex
**/
bool Tri_Is_Adj_To_Plane(int32* SlicedV[3], const float PlaneDist[3])
{	
	for (int i = 0; i < 3; i++)
	{
		if (SlicedV[i] == nullptr && PlaneDist[i] != 0.f)
			return false;
	}
	return true;
}

/**
Check If Triangle is adjacent to SlicePlane in edge and add this Edge to ClipEdges array
**/
bool Tri_Is_Adj_To_Plane_In_Edge(int32* SlicedV[3], const float PlaneDist[3], TArray<EdgeIndices>& ClipEdges)
{		
	for (int32 ThisVert = 0; ThisVert < 3; ThisVert++)
	{
		int32 NextVert = (ThisVert + 1) % 3;
		if (PlaneDist[ThisVert] == 0.f && PlaneDist[NextVert] == 0.f)
		{
			EdgeIndices NewEdge;
			NewEdge.Index1 = *SlicedV[ThisVert];
			NewEdge.Index0 = *SlicedV[NextVert];
			ClipEdges.Add(NewEdge);
			return true;
		}
	}
	return false;
}


/**
Triangulating Clip Edges
If NoCentroid - sort them, remove collinear edges and build triangles only on ClipEdges's vertices
Otherwise - search centroid of ClipEdges's vertices, add it to Indices and Vertices and build triangles from Centroid and ClipEdges
**/
void TriangulateClipEdges(TArray<EdgeIndices>& ClipEdges, TArray<int32>& Indices, TArray<FVector>& Vertices, bool NoCentroid = true)
{
	TArray<int32> FinalIndices;
	//Sort ClipEdges
	for (int32 i = 0; i < ClipEdges.Num() - 2; i++)
	{
		if (ClipEdges[i].Index1 != ClipEdges[i + 1].Index0)
		{
			for (int32 j = i + 2; j < ClipEdges.Num(); j++)
			{
				if (ClipEdges[i].Index1 == ClipEdges[j].Index0)
				{
					Swap(ClipEdges[i + 1], ClipEdges[j]);
					break;
				}
			}
		}
	}
	//Remove Collinear ClipEdges
	for (int32 i = 0; i < ClipEdges.Num() - 1; i++)
	{
		if (PointsAreOnOneLine(Vertices[ClipEdges[i].Index0], Vertices[ClipEdges[i].Index1], Vertices[ClipEdges[i + 1].Index1]))
		{
			ClipEdges[i].Index1 = ClipEdges[i + 1].Index1;
			ClipEdges.RemoveAt(i + 1);
			i--;
		}
	}
	//Check last and first edges for collinearity

	if (PointsAreOnOneLine(Vertices[ClipEdges.Last().Index0], Vertices[ClipEdges.Last().Index1], Vertices[ClipEdges[0].Index1]))
	{
		ClipEdges[0].Index0 = ClipEdges.Last().Index0;
		ClipEdges.RemoveAt(ClipEdges.Num()-1);		
	}
	 
	//Fill FinalIndices
	for (int32 i = 0; i < ClipEdges.Num(); i++)
	{
		FinalIndices.Add(ClipEdges[i].Index0);
	}

	int32 NumFinalIndices = FinalIndices.Num();
	for (int32 Index = 2; Index < NumFinalIndices; Index++)
	{
		Indices.Add(FinalIndices[0]);
		Indices.Add(FinalIndices[Index - 1]);
		Indices.Add(FinalIndices[Index]);
	}
}

bool EdgeAlreadySliced(TArray<SlicedEdge>& SlicedEdges, SlicedEdge& InEdge)
{
	for (int32 i = 0; i < SlicedEdges.Num(); i++)
	{
		if (SlicedEdges[i] == InEdge)
		{
			InEdge = SlicedEdges[i];
			return true;
		}
	}	
	return false;
}

bool SliceConvexHull(const TArray<FVector>& OriginVertices, const TArray<int32>& OriginIndices, const FPlane& SlicePlane,
	TArray<FVector>& FirstHalfVertices, TArray<FVector>& OtherHalfVertices,
	TArray<int32>& FirstHalfIndices, TArray<int32>& OtherHalfIndices,
	bool bCreateOtherHalf)
{
	/** 
	Map of base vert index to sliced vert index
	Key - Vertex Index in OriginVertices
	VAlue - Vertex Index in FirstHalfVertices
	**/
	TMap<int32, int32> BaseToSlicedVertIndex;
	TMap<int32, int32> BaseToOtherSlicedVertIndex;

	TArray<EdgeIndices> FirstClipEdges;
	TArray<EdgeIndices> OtherClipEdges;

	/**
	Array of sliced original edges for prevent duplicates appearance in Vertices and Indices arrays	
	**/
	TArray<SlicedEdge> SlicedEdges;
	bool EdgeIsSlicedAlready;

	const int32 NumBaseVerts = OriginVertices.Num();

	// Distance of each base vert from slice plane
	TArray<float> VertDistance;
	VertDistance.SetNumUninitialized(NumBaseVerts);		
	
	// Build vertex buffer 
	for (int32 BaseVertIndex = 0; BaseVertIndex < NumBaseVerts; BaseVertIndex++)
	{		

		FVector BaseVert = OriginVertices[BaseVertIndex];
		
		// Calculate distance from plane
		VertDistance[BaseVertIndex] = SlicePlane.PlaneDot(BaseVert);

		// See if vert is being kept in this section
		if (VertDistance[BaseVertIndex] <= 0.f)
		{
			int32 SlicedVertIndex = FirstHalfVertices.Add(BaseVert);
			BaseToSlicedVertIndex.Add(BaseVertIndex, SlicedVertIndex);
		}
		// Or add to other half if desired
		if (VertDistance[BaseVertIndex] >= 0.f && bCreateOtherHalf)
		{
			int32 SlicedVertIndex = OtherHalfVertices.Add(BaseVert);
			BaseToOtherSlicedVertIndex.Add(BaseVertIndex, SlicedVertIndex);
		}
		
	}


	// Iterate over base triangles (IE. 3 indices at a time)
	int32 NumBaseIndices = OriginIndices.Num();
	for (int32 BaseIndex = 0; BaseIndex < NumBaseIndices; BaseIndex += 3)
	{
		int32 BaseV[3]; // Triangle vert indices in original mesh
		int32* SlicedV[3]; // Pointers to vert indices in new v buffer
		int32* SlicedOtherV[3]; // Pointers to vert indices in new 'other half' v buffer

								// For each vertex..
		for (int32 i = 0; i < 3; i++)
		{
			// Get triangle vert index
			BaseV[i] = OriginIndices[BaseIndex + i];
			// Look up in sliced v buffer
			SlicedV[i] = BaseToSlicedVertIndex.Find(BaseV[i]);

			// Look up in 'other half' v buffer (if desired)
			if (bCreateOtherHalf)
			{
				SlicedOtherV[i] = BaseToOtherSlicedVertIndex.Find(BaseV[i]);				
			}
		}

		float PlaneDist[3];
		PlaneDist[0] = VertDistance[BaseV[0]];
		PlaneDist[1] = VertDistance[BaseV[1]];
		PlaneDist[2] = VertDistance[BaseV[2]];


		// If all verts survived plane cull, keep the triangle
		if (SlicedV[0] != nullptr && SlicedV[1] != nullptr && SlicedV[2] != nullptr)
		{
			FirstHalfIndices.Add(*SlicedV[0]);
			FirstHalfIndices.Add(*SlicedV[1]);
			FirstHalfIndices.Add(*SlicedV[2]);
			Tri_Is_Adj_To_Plane_In_Edge(SlicedV, PlaneDist, FirstClipEdges);
		}
		// If all verts were removed by plane cull
		else if (SlicedOtherV[0] != nullptr && SlicedOtherV[1] != nullptr && SlicedOtherV[2] != nullptr)
		{
			// If creating other half, add all verts to that
			if (bCreateOtherHalf)
			{
				OtherHalfIndices.Add(*SlicedOtherV[0]);
				OtherHalfIndices.Add(*SlicedOtherV[1]);
				OtherHalfIndices.Add(*SlicedOtherV[2]);
				Tri_Is_Adj_To_Plane_In_Edge(SlicedOtherV, PlaneDist, OtherClipEdges);
			}
		}
		// If partially culled, clip to create 1 or 2 new triangles
		else
		{
			int32 FinalVerts[4];
			int32 NumFinalVerts = 0;

			int32 OtherFinalVerts[4];
			int32 NumOtherFinalVerts = 0;

			EdgeIndices NewClipEdge;
			EdgeIndices NewOtherClipEdge;

			int32 ClippedEdges = 0;			

			for (int32 EdgeIdx = 0; EdgeIdx < 3; EdgeIdx++)
			{

				int32 ThisVert = EdgeIdx;

				// If start vert is inside, add it.
				if (SlicedV[ThisVert] != nullptr)
				{
					check(NumFinalVerts < 4);
					FinalVerts[NumFinalVerts++] = *SlicedV[ThisVert];
				}
				// If not, add to other side
				if (SlicedOtherV[ThisVert] != nullptr && bCreateOtherHalf)
				{
					check(NumOtherFinalVerts < 4);
					OtherFinalVerts[NumOtherFinalVerts++] = *SlicedOtherV[ThisVert];
				}

				
				int32 NextVert = (EdgeIdx + 1) % 3;

				bool VertOnSlicePlane = (SlicedV[EdgeIdx] != nullptr && SlicedOtherV[EdgeIdx] != nullptr);
				bool NextVertOnSlicePlane = (SlicedV[NextVert] != nullptr && SlicedOtherV[NextVert] != nullptr);

				// If vert is on SlicePlane add it to both ClipEdges
				if (VertOnSlicePlane)
				{					
										
					check(ClippedEdges < 2);
					if (SlicedV[NextVert] == nullptr)
						NewClipEdge.Index1 = *SlicedV[ThisVert];
					else
						NewClipEdge.Index0 = *SlicedV[ThisVert];

					if (bCreateOtherHalf)
					{
						if (SlicedOtherV[NextVert] == nullptr)
							NewOtherClipEdge.Index1 = *SlicedOtherV[ThisVert];
						else
							NewOtherClipEdge.Index0 = *SlicedOtherV[ThisVert];
					}

					ClippedEdges++;
				}
				// If start and next vert are on opposite sides, add intersection
				else if ((SlicedV[EdgeIdx] == nullptr) != (SlicedV[NextVert] == nullptr) && !NextVertOnSlicePlane)
				{
					SlicedEdge SlicedEdge;
					SlicedEdge.Index0 = BaseV[ThisVert];
					SlicedEdge.Index1 = BaseV[NextVert];
					EdgeIsSlicedAlready = EdgeAlreadySliced(SlicedEdges, SlicedEdge);

					int32 IntersVertIndex;
					FVector IntersectVert;

					if (!EdgeIsSlicedAlready)
					{
						IntersectVert = FMath::LinePlaneIntersection(OriginVertices[BaseV[ThisVert]], OriginVertices[BaseV[NextVert]], SlicePlane);

						// Add to vertex buffer
						IntersVertIndex = FirstHalfVertices.Add(IntersectVert);

						SlicedEdge.FirstHalfIndex = IntersVertIndex;
					}
					else
						IntersVertIndex = SlicedEdge.FirstHalfIndex;

					// Save vert index for this poly
					check(NumFinalVerts < 4);
					FinalVerts[NumFinalVerts++] = IntersVertIndex;

					// When we make a new edge on the surface of the clip plane, save it off.
					// Add Indices to NewClipEdge in CCW order
					check(ClippedEdges < 2);
					
					//If ThisVert is behind and NextVert is in front of SlicePlane - NewClipEdge Second Index is InterpVertIndex
					//Otherwise NewClipEdge First Index is InterpVertIndex
					if(SlicedV[EdgeIdx] != nullptr)
						NewClipEdge.Index1 = IntersVertIndex;
					else
						NewClipEdge.Index0 = IntersVertIndex;
					

					// If desired, add to the poly for the other half as well
					if (bCreateOtherHalf)
					{
						//If neither ThisVert nor NextVert are on SlicePlane - Add IntersectVert to OtherHalfVertices and OtherFinalVerts and calculate OtherInterpVertIndex
						//Otherwise - OtherInterpVertIndex is either ThisVert's or NextVert's Index
						int32 OtherIntersVertIndex;

						if (!EdgeIsSlicedAlready)
						{														
							OtherIntersVertIndex = OtherHalfVertices.Add(IntersectVert);

							SlicedEdge.OtherHalfIndex = OtherIntersVertIndex;
						}
						else
							OtherIntersVertIndex = SlicedEdge.OtherHalfIndex;

						check(NumOtherFinalVerts < 4);
						OtherFinalVerts[NumOtherFinalVerts++] = OtherIntersVertIndex;
						

						// Add Indices to NewOtherClipEdge in CCW order						
							
						//If ThisVert is in front and NextVert is behind of SlicePlane - NewOtherClipEdge Second Index is OtherInterpVertIndex
						//Otherwise NewOtherClipEdge First Index is OtherInterpVertIndex
						if (SlicedV[EdgeIdx] == nullptr)
							NewOtherClipEdge.Index1 = OtherIntersVertIndex;
						else
							NewOtherClipEdge.Index0 = OtherIntersVertIndex;
					}					

					if (!EdgeIsSlicedAlready)
					{
						SlicedEdges.Add(SlicedEdge);
					}

					ClippedEdges++;
				}
			}

			// Triangulate the clipped polygon.
			for (int32 VertexIndex = 2; VertexIndex < NumFinalVerts; VertexIndex++)
			{
				FirstHalfIndices.Add(FinalVerts[0]);
				FirstHalfIndices.Add(FinalVerts[VertexIndex - 1]);
				FirstHalfIndices.Add(FinalVerts[VertexIndex]);
			}

			// If we are making the other half, triangulate that as well
			if (bCreateOtherHalf)
			{
				for (int32 VertexIndex = 2; VertexIndex < NumOtherFinalVerts; VertexIndex++)
				{
					OtherHalfIndices.Add(OtherFinalVerts[0]);
					OtherHalfIndices.Add(OtherFinalVerts[VertexIndex - 1]);
					OtherHalfIndices.Add(OtherFinalVerts[VertexIndex]);
				}
			}

			check(ClippedEdges != 1); // Should never clip just one edge of the triangle

			// If we created a new edge, save that off here as well
			if (ClippedEdges == 2)
			{
				FirstClipEdges.Add(NewClipEdge);
				if (bCreateOtherHalf)
					OtherClipEdges.Add(NewOtherClipEdge);
			}
		}		
	}
	//CheckEdges(FirstClipEdges);
	//CheckEdges(OtherClipEdges);
	TriangulateClipEdges(FirstClipEdges, FirstHalfIndices, FirstHalfVertices);
	if (bCreateOtherHalf)
		TriangulateClipEdges(OtherClipEdges, OtherHalfIndices, OtherHalfVertices);
	return true;
}


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////OLD////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/**
Try To Set Very Small Float To Zero
**/
void TryTruncateFloatToZero(float& Value)
{
	if (Value != 0 && Value < FLT_ZERO && Value > -FLT_ZERO)
		Value = 0;
}

/**
Adding Index to array if Index is next index relatively to array's last index
Otherwise inserting Index to start of array at Delta position
**/
void AddOrInsert(TArray<uint32>& IndicesArray, const uint32& Index, int& Delta)
{
	if (IndicesArray.Num() == 0 || (Index - IndicesArray.Last() == 1))
		IndicesArray.Add(Index);
	else if ((Index - IndicesArray.Last()) > 1)
	{
		IndicesArray.Insert(Index, Delta);
		Delta++;
	}
}

void SearchFirstNonComplanarFace(int i, const TArray<FVector>& OriginVertices, const TArray<Face>& OriginFaces, const FPlane& SlicePlane, bool& HullIsBehindPlane)
{
	for (int j = 0; j < OriginFaces.Num(); j++)
	{
		if (j == i)
			continue;
		
		float Distance = 0;
		for (int k = 0; k < OriginFaces[j].Indices.Num(); k++)
		{
			Distance = SlicePlane.PlaneDot(OriginVertices[OriginFaces[j].Indices[k]]);
			TryTruncateFloatToZero(Distance);			
			if (Distance < 0)
			{
				HullIsBehindPlane = true;
				return;
			}
			else if (Distance > 0)
			{
				HullIsBehindPlane = false;
				return;
			}
		}		
	}
}

/**
Search First Sliced Face Or Face From Behind Of SlicePlane Having Edge Adjacent To SlicePlane
Returns false If Slice Plane don't intersects Convex Hull
**/
bool SearchFirstSlicedFace(const TArray<FVector>& OriginVertices, const TArray<Face>& OriginFaces, const FPlane& SlicePlane, SlicedFace& slicedFace, bool& HullIsBehindPlane)
{
	bool SlicePlaneIsFace = false;
	uint32 AboveFacesCount = 0, BelowFacesCount = 0;
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
					if (AboveFacesCount > 0)
						HullIsBehindPlane = false;
					else if (BelowFacesCount > 0)
						HullIsBehindPlane = true;
					else if (AboveFacesCount == 0 && BelowFacesCount == 0)
					{
						SearchFirstNonComplanarFace(i, OriginVertices, OriginFaces, SlicePlane, HullIsBehindPlane);
					}
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
						if (AboveIndices.Num() > 2)
							HullIsBehindPlane = false;
						else if (BelowIndices.Num() > 2)
							HullIsBehindPlane = true;
						return false;
					}
					else
					{
						//Convex Hull has edge on SlicePlane and other part is at one side from SlicePlane						
						if (AboveIndices.Num() > 2 && NeighPrevPointDist > 0)
						{
							HullIsBehindPlane = false;
							return false;
						}
						else if (BelowIndices.Num() > 2 && NeighPrevPointDist < 0)
						{
							HullIsBehindPlane = true;
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
		else if (AboveIndices.Num() > 0 && BelowIndices.Num() <= 2)
		{
			AboveFacesCount++;
		}
		else if (AboveIndices.Num() <= 2 && BelowIndices.Num() > 0)
		{
			BelowFacesCount++;
		}
	}
	if (BelowFacesCount > 0 && AboveFacesCount == 0)
		HullIsBehindPlane = true;
	else if (AboveFacesCount > 0 && BelowFacesCount == 0)
		HullIsBehindPlane = false;

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
#ifdef WITH_DEBUG
	CheckSlicedFaces(SlicedFaces);
#endif

	while (NextFaceIndex != SlicedFaces[0].Number)
	{
		//CheckFace(OriginFaces[NextFaceIndex]);
		//CheckVertices(OriginFaces[NextFaceIndex], OriginVertices);

		NextSlicedFace = SlicedFace(OriginFaces[NextFaceIndex]);

		//CheckFace(NextSlicedFace);

		NextSlicedFace.LeftVertexIsOnSlicePlane = CurrentSlicedFace.RightVertexIsOnSlicePlane;
		NextSlicedFace.LeftEdgeIndex = *NextSlicedFace.Neighbours.FindKey(CurrentFaceIndex);
		NextSlicedFace.LeftSlicePoint = CurrentSlicedFace.RightSlicePoint;

		//Index of next point index after left sliced edge
		uint32 NextPointIndex = NextSlicedFace.LeftEdgeIndex == NextSlicedFace.Indices.Num() - 2 ? 0 :
			(NextSlicedFace.LeftEdgeIndex == NextSlicedFace.Indices.Num() - 1 ? 1 : NextSlicedFace.LeftEdgeIndex + 2);
		uint32 PrevPointIndex = NextPointIndex == 0 ? NextSlicedFace.Indices.Num() - 1 : NextPointIndex - 1;

		

		Distance = SlicePlane.PlaneDot(OriginVertices[NextSlicedFace.Indices[NextPointIndex]]);
		TryTruncateFloatToZero(Distance);

		int AbovePointsCount = 0;
		//Search First point behind or on SlicePlane
		while (Distance > 0)
		{			
			AbovePointsCount++;

			NextPointIndex = NextPointIndex == NextSlicedFace.Indices.Num() - 1 ? 0 : NextPointIndex + 1;
			PrevPointIndex = NextPointIndex == 0 ? NextSlicedFace.Indices.Num() - 1 : NextPointIndex - 1;
						
			//uint32 Index = NextSlicedFace.Indices[NextPointIndex];
			FVector F = OriginVertices[NextSlicedFace.Indices[NextPointIndex]];

			Distance = SlicePlane.PlaneDot(F);
			TryTruncateFloatToZero(Distance);
			
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

#ifdef WITH_DEBUG
		CheckSlicedFace(NextSlicedFace);
#endif
	}
}

void CheckVertices(const Face& face, const TArray<FVector>& OriginVertices)
{
	FVector Vertex;
	uint32 Index;
	for (int i = 0; i < face.Indices.Num(); i++)
	{
		Index = face.Indices[i];
		Vertex = OriginVertices[Index];
	}
}

void CheckFace(const Face& face)
{
	uint32 VertexIndex;
	long NeighbourIndex;

//#ifdef WITH_DEBUG

	int Indices[20];
	int Neighbours[20];
	for (int i = 0; i < 20; i++)
	{
		Indices[i] = -5;
		Neighbours[i] = -5;
	}
	for (int i = 0; i < face.Indices.Num(); i++)
	{
		Indices[i] = face.Indices[i];
		Neighbours[i] = face.Neighbours[i];
	}

//#endif // WITH_DEBUG

	for (int i = 0; i < face.Indices.Num(); i++)
	{
		VertexIndex = face.Indices[i];
		NeighbourIndex = face.Neighbours[i];
		if (NeighbourIndex < 0)
		{
			NeighbourIndex++;
			NeighbourIndex--;
		}
	}
}

void CheckFaces(const TArray<Face>& Faces)
{
	uint32 VertexIndex;
	long NeighbourIndex;

	Face face;
	uint32 FacesCount = Faces.Num();
	for (int i = 0; i < Faces.Num(); i++)
	{
		face = Faces[i];
//#ifdef WITH_DEBUG

		int Indices[20];
		int Neighbours[20];
		for (int i = 0; i < 20; i++)
		{
			Indices[i] = -5;
			Neighbours[i] = -5;
		}
		for (int i = 0; i < face.Indices.Num(); i++)
		{ 
			Indices[i] = face.Indices[i];
			Neighbours[i] = face.Neighbours[i];
		}

		if (Indices&&Neighbours&&FacesCount)
		{
			FacesCount++;
			FacesCount--;
		}
//#endif // WITH_DEBUG
		for (int j = 0; j < face.Indices.Num(); j++)
		{
			VertexIndex = face.Indices[j];
			NeighbourIndex = face.Neighbours[j];
			if (VertexIndex == -1 || NeighbourIndex == -1 || NeighbourIndex > Faces.Num())
			{
				NeighbourIndex++;
				NeighbourIndex--;
			}
		}
	}
}

void CheckSlicedFace(const SlicedFace& face)
{
	uint32 VertexIndex;
	long NeighbourIndex;

#ifdef WITH_DEBUG

	int Indices[10];
	int Neighbours[10];
	for (int i = 0; i < 10; i++)
	{
		Indices[i] = -5;
		Neighbours[i] = -5;
	}
	for (int i = 0; i < face.Indices.Num(); i++)
	{
		Indices[i] = face.Indices[i];
		Neighbours[i] = face.Neighbours[i];
	}

#endif // WITH_DEBUG

	for (int i = 0; i < face.Indices.Num(); i++)
	{
		VertexIndex = face.Indices[i];
		NeighbourIndex = face.Neighbours[i];
		if (NeighbourIndex < 0)
		{
			NeighbourIndex++;
			NeighbourIndex--;
		}
	}
}

void CheckSlicedFaces(const TArray<SlicedFace>& Faces)
{
	uint32 VertexIndex;
	long NeighbourIndex;

	SlicedFace Face;
	for (int i = 0; i < Faces.Num(); i++)
	{
		Face = Faces[i];
#ifdef WITH_DEBUG

		int Indices[10];
		int Neighbours[10];
		for (int i = 0; i < 10; i++)
		{
			Indices[i] = -5;
			Neighbours[i] = -5;
		}
		for (int i = 0; i < Face.Indices.Num(); i++)
		{
			Indices[i] = Face.Indices[i];
			Neighbours[i] = Face.Neighbours[i];
		}

#endif // WITH_DEBUG
		for (int j = 0; j < Face.Indices.Num(); j++)
		{
			VertexIndex = Face.Indices[j];
			NeighbourIndex = Face.Neighbours[j];
			if (VertexIndex == -1 || NeighbourIndex == -1)
			{
				NeighbourIndex++;
				NeighbourIndex--;
			}
		}
	}
}

bool HasExcessVerts(const TArray <uint8>& ExcessIndices)
{
	for (int i = 0; i < ExcessIndices.Num(); i++)
	{
		if (ExcessIndices[i] == 0)
			return true;
	}
	return false;
}

bool SetSlicedFaces(TArray<Face>& Faces, const TArray<SlicedFace>& SlicedFaces, bool IsFirstHalf = true)
{
	for (int i = 0; i < SlicedFaces.Num(); i++)
	{
		if (IsFirstHalf)
			Faces[SlicedFaces[i].Number].IsSlicedFace = true;
		else
		{
			if (!SlicedFaces[i].EdgeOnSlicePlane)
				Faces[SlicedFaces[i].Number].IsSlicedFace = true;
			else
			{
				Faces[SlicedFaces[i].Neighbours[SlicedFaces[i].EdgeOnSlicePlaneIndex]].IsSlicedFace = true;
			}
		}
	}
	return false;
}

/**
Set slicedFace for its neighbours as unfound neighbour
**/
void SetSlicedFaceAsUnfoundNeighbour(uint32& CurrentEdgeIndex, const uint32& FinishEdgeIndex, TArray<Face>& HalfFaces, const SlicedFace& slicedFace)
{
	//if CurrentEdgeIndex != FinishEdgeIndex
	while (CurrentEdgeIndex != FinishEdgeIndex)
	{
		Face& Neighbour = HalfFaces[slicedFace.Neighbours[CurrentEdgeIndex]];
		if (!Neighbour.IsSlicedFace)
		{
			const uint32* NeighEdgeIndex = Neighbour.Neighbours.FindKey(slicedFace.Number);
			if (NeighEdgeIndex != nullptr)
			{
				Neighbour.Neighbours[*NeighEdgeIndex] = -1;
				Neighbour.AllNeighboursFound = false;
			}
		}
		CurrentEdgeIndex = CurrentEdgeIndex == slicedFace.Neighbours.Num() - 1 ? 0 : CurrentEdgeIndex + 1;
	}
	//CurrentEdgeIndex==FinishEdgeIndex
	//Set slicedFace as unfound neighbour for neighbour at FinishEdge
#ifdef WITH_DEBUG

	int Indices[10];
	int Neighbours[10];
	for (int i = 0; i < 10; i++)
	{
		Indices[i] = -5;
		Neighbours[i] = -5;
	}
	for (int i = 0; i < slicedFace.Indices.Num(); i++)
	{
		Indices[i] = slicedFace.Indices[i];
		Neighbours[i] = slicedFace.Neighbours[i];
	}

#endif // WITH_DEBUG
	Face& Neighbour = HalfFaces[slicedFace.Neighbours[CurrentEdgeIndex]];
	if (!Neighbour.IsSlicedFace)
	{
		const uint32* NeighEdgeIndex = Neighbour.Neighbours.FindKey(slicedFace.Number);
		if (NeighEdgeIndex != nullptr)
		{
			Neighbour.Neighbours[*NeighEdgeIndex] = -1;
			Neighbour.AllNeighboursFound = false;
		}
	}

}

/**
Reorder Indices and Neighbours of Halfface
**/
void ReorderOtherData(uint32& CurrentEdgeIndex, const uint32& FinishEdgeIndex, TArray<uint32>& NewIndices, TMap<uint32, int64>& NewNeighbours, const SlicedFace& slicedFace, Face& HalfFace)
{
	do {
		CurrentEdgeIndex = CurrentEdgeIndex == slicedFace.Neighbours.Num() - 1 ? 0 : CurrentEdgeIndex + 1;
		NewIndices.Add(slicedFace.Indices[CurrentEdgeIndex]);
		//long NeighbourNum = slicedFace.Neighbours[CurrentEdgeIndex];
		NewNeighbours.Add(NewNeighbours.Num(), slicedFace.Neighbours[CurrentEdgeIndex]);
	} while (CurrentEdgeIndex != FinishEdgeIndex);
	HalfFace.Indices = NewIndices;
	HalfFace.Neighbours = NewNeighbours;
	HalfFace.AllNeighboursFound = true;
}

/**
Recursive serach of invalid faces and vertices
**/
void SearchInvalidValues(const uint32& InvalidIndex, TArray<uint8>& ValidFaces, TArray<uint8>& ValidVertices, TArray<Face>& OriginFaces, TArray<FVector>& OriginVertices,
	TArray<uint32>& AdjacentInvalidFaces)
{
	ValidFaces[InvalidIndex] = 0;

	int32 RemoveIndex = AdjacentInvalidFaces.Find(InvalidIndex);

	if (RemoveIndex != INDEX_NONE)
		AdjacentInvalidFaces.RemoveAt(RemoveIndex);

	Face& CurrentFace = OriginFaces[InvalidIndex];

	if (!CurrentFace.IsSlicedFace)
	{
		CurrentFace.AllNeighboursFound = false;
		for (int i = 0; i < CurrentFace.Neighbours.Num(); i++)
		{
			ValidVertices[CurrentFace.Indices[i]] = 0;

			if (CurrentFace.Neighbours[i] != -1)
			{
				Face& Neighbour = OriginFaces[CurrentFace.Neighbours[i]];
				//set CurrentFace's neighbour at edge[i] as unfound
				CurrentFace.Neighbours[i] = -1;

				if (!Neighbour.IsSlicedFace)
				{

					//set CurrentFace as Neighbour's unfound neighbour
					Neighbour.Neighbours[*Neighbour.Neighbours.FindKey(CurrentFace.Number)] = -1;
					Neighbour.AllNeighboursFound = false;

					SearchInvalidValues(Neighbour.Number, ValidFaces, ValidVertices, OriginFaces, OriginVertices, AdjacentInvalidFaces);
				}
			}
		}
	}
}

/**
If Faces array has invalid faces
Fill Valid Arrays with valid values then search invalid values
**/
void FillValidArrays(TArray<uint8>& ValidFaces, TArray<uint8>& ValidVertices, TArray<Face>& OriginFaces, TArray<FVector>& OriginVertices)
{
	//Array of numbers of faces adjacent to sliced faces from other half
	TArray<uint32> AdjacentInvalidFaces;
	//Search invalid values in both arrays
	//Fill AdjacentInvalidFaces

	for (int i = 0; i < OriginFaces.Num(); i++)
	{
		if (!OriginFaces[i].AllNeighboursFound)
		{
			//OriginFaces[i].Number==i
			Face face = OriginFaces[i];
			AdjacentInvalidFaces.Add(face.Number);
		}
	}
	//invalid face not present
	if (AdjacentInvalidFaces.Num() == 0)
	{
		//If all SlicedFaces have joint vertices at other half and there are no other faces at other half - joint vertices won't be found as invalid vertices
		//So search them now
		//Fill ValidVertices with invalid values
		for (int i = 0; i < OriginVertices.Num(); i++)
		{
			ValidVertices.Add(0);
		}
		//Fill ValidVertices with valid values
		for (int i = 0; i < OriginFaces.Num(); i++)
		{
			for (int j = 0; j < OriginFaces[i].Indices.Num(); j++)
			{
				ValidVertices[OriginFaces[i].Indices[j]] = 1;
			}
		}
	}
	else
	{
		//Fill Valid arrays with valid values
		for (int i = 0; i < OriginFaces.Num(); i++)
		{
			ValidFaces.Add(1);
		}
		for (int i = 0; i < OriginVertices.Num(); i++)
		{
			ValidVertices.Add(1);
		}
		//There could be isolated invalid faces
		//Not isolated faces removing from AdjacentInvalidFaces array during SearchInvalidValues
		while (AdjacentInvalidFaces.Num() > 0)
		{
			uint32 InvalidIndex = AdjacentInvalidFaces[0];
			SearchInvalidValues(InvalidIndex, ValidFaces, ValidVertices, OriginFaces, OriginVertices, AdjacentInvalidFaces);
		}
		//Correction ValidVertices values if SlicedFaces have joint vertices with invalid faces
		for (int i = 0; i < OriginFaces.Num(); i++)
		{
			//ValidFaces[i] = 1;
			if (OriginFaces[i].IsSlicedFace)
			{
				for (int j = 0; j < OriginFaces[i].Indices.Num(); j++)
				{
					ValidVertices[OriginFaces[i].Indices[j]] = 1;
				}
			}
		}
	}
}

/**
Removes Excess Points from original array and corrects face indices
**/
void VerticesAndIndicesCorrection(TArray<FVector>& Vertices, TArray<Face>& Faces, TArray<uint8>& ExcessIndices)
{
	//Key - old index, Value - new index
	TMap<uint32, uint32> ChangedIndices;
	uint32 delta = 0;
	for (int i = 0; i < ExcessIndices.Num(); i++)
	{
		if (ExcessIndices[i] == 0)
		{
			//Remove Excess points from original array
			Vertices.RemoveAt(i - delta);
			delta++;
		}
		else
		{
			/*if (delta > 0)
			{*/
			ChangedIndices.Add(i, i - delta);
			//}
		}
	}

	for (int i = 0; i < Faces.Num(); i++)
	{
		if (Faces[i].AllNeighboursFound)
		{
			for (int j = 0; j < Faces[i].Indices.Num(); j++)
			{
				uint32 Ind = ChangedIndices[Faces[i].Indices[j]];
				Faces[i].Indices[j] = ChangedIndices[Faces[i].Indices[j]];
			}
		}
	}
}

/**
Removes invalid faces from original array and corrects face neighbours
**/
void FacesAndNeighboursCorrection(TArray<Face>& HalfFaces, TArray<uint8>& ValidFaces)
{

	//Key - old index, Value - new index
	TMap<uint32, int64> ChangedFaces;
	uint32 delta = 0;
	for (int i = 0; i < ValidFaces.Num(); i++)
	{
		if (ValidFaces[i] == 0)
		{
#ifdef WITH_DEBUG
			//CheckFace(HalfFaces[i - delta]);
#endif
			//Remove invalid faces from original array
			HalfFaces.RemoveAt(i - delta);
			delta++;
		}
		else
		{
			/*if (delta > 0)
			{*/
			ChangedFaces.Add(i, i - delta);
			//}
		}
	}

	//CheckFaces(HalfFaces);
	for (int i = 0; i < HalfFaces.Num(); i++)
	{
		Face& face = HalfFaces[i];
#ifdef WITH_DEBUG
		//CheckFace(face);
#endif
		face.Number = i;
		for (int j = 0; j < face.Neighbours.Num(); j++)
		{
			long Neigh = face.Neighbours[j];
			/*if (Neigh >= 0)
			{*/
			long Ind = ChangedFaces[Neigh];
			if (Ind < 0)
			{
				Ind++;
				Ind--;
			}
			face.Neighbours[j] = Ind;
			//}
		}
	}
}

/**
Reorder Arrays of Faces and Vertices
**/
void ReorderArrays(TArray<Face>& FirstHalfFaces, TArray<FVector>& FirstHalfVertices, TArray<Face>&  SecondHalfFaces, TArray<FVector>& SecondHalfVertices)
{
	//Array index matches Face index
	//Value = 0 if face invalid, Value = 1 otherwise
	TArray<uint8> ValidFaces;

	//Array index matches Vertex index
	//Value = 0 if Vertex is excess, Value = 1 otherwise
	TArray<uint8> ValidVertices;

	//Fill valid afrrays of first half
	FillValidArrays(ValidFaces, ValidVertices, FirstHalfFaces, FirstHalfVertices);

	if (HasExcessVerts(ValidVertices))
		VerticesAndIndicesCorrection(FirstHalfVertices, FirstHalfFaces, ValidVertices);
	if (ValidFaces.Num() > 0)
	{
		FacesAndNeighboursCorrection(FirstHalfFaces, ValidFaces);

#ifdef WITH_DEBUG
		for (int i = 0; i < FirstHalfFaces.Num(); i++)
		{
			//CheckFace(FirstHalfFaces[i]);
		}

#endif
	}
	//CheckFaces(FirstHalfFaces);

	ValidFaces.Empty();
	ValidVertices.Empty();

	//Fill valid afrrays of second half
	FillValidArrays(ValidFaces, ValidVertices, SecondHalfFaces, SecondHalfVertices);

	if (HasExcessVerts(ValidVertices))
		VerticesAndIndicesCorrection(SecondHalfVertices, SecondHalfFaces, ValidVertices);
	if (ValidFaces.Num() > 0)
	{
		FacesAndNeighboursCorrection(SecondHalfFaces, ValidFaces);

#ifdef WITH_DEBUG
		for (int i = 0; i < SecondHalfFaces.Num(); i++)
		{
			//CheckFace(SecondHalfFaces[i]);
		}

#endif
	}
	//CheckFaces(SecondHalfFaces);
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
		const SlicedFace slicedFace = SlicedFaces[i];
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
			//CheckFace(FirstHalfFaces[slicedFace.Number]);	
			//CheckFaces(FirstHalfFaces);
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
			//CheckFace(SecondHalfFaces[Neighbour.Number]);
			//CheckFaces(SecondHalfFaces);
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
			if (i < SlicedFaces.Num() - 1 && !slicedFace.RightVertexIsOnSlicePlane)
			{
				First_R_Vert_Ind = FirstHalfVertices.Num();
				FirstHalfVertices.Add(slicedFace.RightSlicePoint);
				SecondHalfVertices.Add(slicedFace.RightSlicePoint);
			}
			else if (i < SlicedFaces.Num() - 1 && slicedFace.RightVertexIsOnSlicePlane)
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
			uint32 FinishEdgeIndex;

			//FinishEdge is previous edge before RightEdge
			FinishEdgeIndex = slicedFace.RightEdgeIndex == 0 ? slicedFace.Neighbours.Num() - 1 : slicedFace.RightEdgeIndex - 1;
			//If there are edges between LeftEdge and RightEdge
			//if(CurrentEdgeIndex!= slicedFace.RightEdgeIndex&&FinishEdgeIndex!= slicedFace.LeftEdgeIndex)
			if (!(CurrentEdgeIndex == slicedFace.RightEdgeIndex&&FinishEdgeIndex == slicedFace.LeftEdgeIndex))
				SetSlicedFaceAsUnfoundNeighbour(CurrentEdgeIndex, FinishEdgeIndex, FirstHalfFaces, slicedFace);
#ifdef WITH_DEBUG			
			//CheckFaces(FirstHalfFaces);
#endif
			//For SecondHalfFace:			
			//If RightVertexIsOnSlicePlane CurrentEdgeIndex = RightEdgeIndex at this moment			
			//Otherwise Set CurrentEdgeIndex as next index after slicedFace.RightEdgeIndex
			if (slicedFace.RightVertexIsOnSlicePlane)
				CurrentEdgeIndex = slicedFace.RightEdgeIndex;
			else
				CurrentEdgeIndex = slicedFace.RightEdgeIndex == slicedFace.Neighbours.Num() - 1 ? 0 : slicedFace.RightEdgeIndex + 1;


			//FinishEdgeIndex for setting slicedFace as notfound neighbour for faces from below of SlicePlane
			//If LeftVertexIsOnSlicePlane FinishEdgeIndex is LeftEdgeIndex
			//Otherwise FinishEdgeIndex is index of previous edge before LeftEdge

			if (slicedFace.LeftVertexIsOnSlicePlane)
				FinishEdgeIndex = slicedFace.LeftEdgeIndex;
			else
				FinishEdgeIndex = slicedFace.LeftEdgeIndex == 0 ? slicedFace.Neighbours.Num() - 1 : slicedFace.LeftEdgeIndex - 1;

			//Set for SlicedFace's neighbours at edges from below of SlicePlane slicedFace as unfound neighbour
			//If there are edges between LeftEdge and RightEdge
			//if (CurrentEdgeIndex != slicedFace.LeftEdgeIndex&&FinishEdgeIndex != slicedFace.RightEdgeIndex)
			if (!(CurrentEdgeIndex == slicedFace.LeftEdgeIndex&&FinishEdgeIndex == slicedFace.RightEdgeIndex))
				SetSlicedFaceAsUnfoundNeighbour(CurrentEdgeIndex, FinishEdgeIndex, SecondHalfFaces, slicedFace);
#ifdef WITH_DEBUG			
			//CheckFaces(SecondHalfFaces);
#endif
			//For FirstHalfFace:
			//Reorder Indices and neighbours for FirstHalfFace
			//First Index is First_L_Vert_Ind, second Index is First_R_Vert_Ind, first Neigbour is FirstSliceFace
			TArray<uint32> NewIndices;
			TMap<uint32, int64> NewNeighbours;
			NewIndices.Add(First_L_Vert_Ind);
			NewNeighbours.Add(NewNeighbours.Num(), FirstSliceFace.Number);

			NewIndices.Add(First_R_Vert_Ind);
			//Second Neighbour is neighbour at RightEdge
			//Set CurrentEdgeIndex as RightEdgeIndex
			CurrentEdgeIndex = slicedFace.RightEdgeIndex;
			NewNeighbours.Add(NewNeighbours.Num(), slicedFace.Neighbours[CurrentEdgeIndex]);

			//Add Indices and neighbours from below of SlicePlane
			ReorderOtherData(CurrentEdgeIndex, slicedFace.LeftEdgeIndex, NewIndices, NewNeighbours, slicedFace, FirstHalfFace);

#ifdef WITH_DEBUG
			//CheckFace(FirstHalfFace);
			//CheckFaces(FirstHalfFaces);
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
			//If RightVertexIsOnSlicePlane set FinishEdgeIndex as previous edge index before RightEdge
			//Othrewise FinishEdgeIndex is RightEdgeIndex
			FinishEdgeIndex =
				slicedFace.RightVertexIsOnSlicePlane ? (slicedFace.RightEdgeIndex == 0 ? slicedFace.Neighbours.Num() - 1 : slicedFace.RightEdgeIndex - 1) : slicedFace.RightEdgeIndex;

			//Add Indices and neighbours from above of SlicePlane
			ReorderOtherData(CurrentEdgeIndex, FinishEdgeIndex, NewIndices, NewNeighbours, slicedFace, SecondHalfFace);
#ifdef WITH_DEBUG
			//CheckFace(SecondHalfFace);
			//CheckFaces(SecondHalfFaces);
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
#ifdef WITH_DEBUG
	//CheckFace(FirstSliceFace);
	//CheckFace(SecondSliceFace);
#endif
	//Add SliceFaces to Arrays of Faces
	FirstHalfFaces.Add(FirstSliceFace);
	SecondHalfFaces.Add(SecondSliceFace);
#ifdef WITH_DEBUG
	//CheckFaces(FirstHalfFaces);
	//CheckFaces(SecondHalfFaces);
#endif
	//Reorder Arrays of Faces and Vertices
	ReorderArrays(FirstHalfFaces, FirstHalfVertices, SecondHalfFaces, SecondHalfVertices);
}


bool SliceConvexHull(const TArray<FVector>& OriginVertices, const TArray<Face>& OriginFaces, const FPlane& SlicePlane,
	TArray<FVector>& FirstHalfVertices, TArray<Face>& FirstHalfFaces,//First half is behind plane
	TArray<FVector>& SecondHalfVertices, TArray<Face>& SecondHalfFaces)
{
	


	TArray<SlicedFace> SlicedFaces;

	//Find first sliced face or face from behind of SlicePlane having edge on SlicePlane
	SlicedFace slicedFace;
	bool HullIsBehindPlane = false;
	bool FirstSlicedFaceFound = SearchFirstSlicedFace(OriginVertices, OriginFaces, SlicePlane, slicedFace, HullIsBehindPlane);
	if (!FirstSlicedFaceFound)
	{
		if (HullIsBehindPlane)
		{
			FirstHalfVertices = OriginVertices;
			FirstHalfFaces = OriginFaces;
		}
		else
		{
			SecondHalfVertices = OriginVertices;
			SecondHalfFaces = OriginFaces;
		}
		return false;
	}
	else
	{
		SlicedFaces.Add(slicedFace);
		SearchSlicedFaces(OriginVertices, OriginFaces, SlicePlane, SlicedFaces);
#ifdef WITH_DEBUG
		//CheckSlicedFaces(SlicedFaces);
#endif
		FirstHalfVertices = OriginVertices;
		FirstHalfFaces = OriginFaces;
		SetSlicedFaces(FirstHalfFaces, SlicedFaces);
#ifdef WITH_DEBUG
		//CheckFaces(FirstHalfFaces);
#endif
		SecondHalfVertices = OriginVertices;
		SecondHalfFaces = OriginFaces;
		SetSlicedFaces(SecondHalfFaces, SlicedFaces, false);
#ifdef WITH_DEBUG
		//CheckFaces(SecondHalfFaces);
#endif

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
	FVector BC = C - B;
	if (AB.IsZero() || BC.IsZero())
		return true;

	float cos = CosBetweenVerts(AB, BC);

	float delta = 1 - abs(cos);

	TryTruncateFloatToZero(delta);
	
	return delta == 0;
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
	qh_vertex_t* vertices = new qh_vertex_t[Vertices.Num()];
	for (int i = 0; i < Vertices.Num(); i++)
	{
		vertices[i] = Vertices[i];
	}
	return vertices;
}

/**
Write Vertices to txt file
**/
void WriteVertices(const TArray<FVector>& Vertices, char* FileName)
{

	stringstream iostr;
	char *s;
	s = new char[50];
	iostr << FileName;
	iostr >> s;
	ofstream fout;
	fout.open(iostr.str());

	FVector Temp;

	for (int i = 0; i < Vertices.Num(); i++)
	{

		fout << "ConvexVerts.Add(FVector(";
		Temp = Vertices[i];
		for (int j = 0; j < 3; j++)
		{
			stringstream iostr;
			char *s1;
			s1 = new char[50];
			iostr << Temp[j];
			iostr >> s1;

			fout << s1;
			if (j < 2)
				fout << ", ";
		}
		fout << "));";
		fout << "\r\n";
	}
	fout.close();
}




///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////QUICKHULL IMPLEMENTATION/////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void qh__find_second_duplicate(qh_index_t* eps, const int& UniqueIndex, int& SecondDuplicateIndex)
{
	bool FirstIndexFound = false;
	for (int i = 0; i < 6; i++)
	{
		if (eps[i] == UniqueIndex)
		{
			if (FirstIndexFound)
			{
				SecondDuplicateIndex = i;
				break;
			}
			else
				FirstIndexFound = true;
		}
	}
}
/**
Check if 6eps array contains only 2 unique indices
And search third unique index
**/
bool qh__6eps_only_2_uniques(qh_vertex_t* vertices, unsigned int nvertices, qh_index_t* eps)
{
	TArray<qh_index_t> UniqueIndices;
	for (int i = 0; i < 6; i++)
	{
		if (!UniqueIndices.Contains(eps[i]))
		{
			UniqueIndices.Add(eps[i]);
			if (UniqueIndices.Num() == 3)
				break;
		}
	}
	if (UniqueIndices.Num() == 2)
	{
		//search second duplicate of first unique index		
		int SecondDuplicateIndex = -1;
		qh__find_second_duplicate(eps, UniqueIndices[0], SecondDuplicateIndex);

		float min = +QH_FLT_MAX;
		float max = -QH_FLT_MAX;

		qh_vertex_t* ptr = vertices;
		if (SecondDuplicateIndex == 0)
		{
			for (unsigned int i = 0; i < nvertices; ++i) {
				if (i != UniqueIndices[0] && i != UniqueIndices[1])
				{
					if (ptr->Z < min) {
						eps[0] = i;
						min = ptr->Z;
					}
				}
				ptr++;
			}
		}
		else if (SecondDuplicateIndex == 1)
		{
			for (unsigned int i = 0; i < nvertices; ++i) {
				if (i != UniqueIndices[0] && i != UniqueIndices[1])
				{
					if (ptr->Y < min) {
						eps[1] = i;
						min = ptr->Y;
					}
				}
				ptr++;
			}
		}
		else if (SecondDuplicateIndex == 2)
		{
			for (unsigned int i = 0; i < nvertices; ++i) {
				if (i != UniqueIndices[0] && i != UniqueIndices[1])
				{
					if (ptr->X < min) {
						eps[2] = i;
						min = ptr->X;
					}
				}
				ptr++;
			}
		}
		else if (SecondDuplicateIndex == 3)
		{
			for (unsigned int i = 0; i < nvertices; ++i) {
				if (i != UniqueIndices[0] && i != UniqueIndices[1])
				{
					if (ptr->Z > max) {
						eps[3] = i;
						max = ptr->Z;
					}
				}
				ptr++;
			}
		}
		else if (SecondDuplicateIndex == 4)
		{
			for (unsigned int i = 0; i < nvertices; ++i) {
				if (i != UniqueIndices[0] && i != UniqueIndices[1])
				{
					if (ptr->Y > max) {
						eps[4] = i;
						max = ptr->Y;
					}
				}
				ptr++;
			}
		}
		else
		{
			for (unsigned int i = 0; i < nvertices; ++i) {
				if (i != UniqueIndices[0] && i != UniqueIndices[1])
				{
					if (ptr->X > max) {
						eps[5] = i;
						max = ptr->X;
					}
				}
				ptr++;
			}
		}

		return true;
	}
	else
		return false;
}

/**
Find indexes of vertices with min and max coordinates
**/
void qh__find_6eps(qh_vertex_t* vertices, unsigned int nvertices, qh_index_t* eps)
{
	qh_vertex_t* ptr = vertices;

	float minxy = +QH_FLT_MAX;
	float minxz = +QH_FLT_MAX;
	float minyz = +QH_FLT_MAX;

	float maxxy = -QH_FLT_MAX;
	float maxxz = -QH_FLT_MAX;
	float maxyz = -QH_FLT_MAX;

	unsigned int i = 0;
	for (i = 0; i < 6; ++i) {
		eps[i] = 0;
	}

	for (i = 0; i < nvertices; ++i) {
		if (ptr->Z < minxy) {
			eps[0] = i;
			minxy = ptr->Z;
		}
		if (ptr->Y < minxz) {
			eps[1] = i;
			minxz = ptr->Y;
		}
		if (ptr->X < minyz) {
			eps[2] = i;
			minyz = ptr->X;
		}
		if (ptr->Z > maxxy) {
			eps[3] = i;
			maxxy = ptr->Z;
		}
		if (ptr->Y > maxxz) {
			eps[4] = i;
			maxxz = ptr->Y;
		}
		if (ptr->X > maxyz) {
			eps[5] = i;
			maxyz = ptr->X;
		}
		ptr++;
	}
	qh__6eps_only_2_uniques(vertices, nvertices, eps);
}

float qh__vertex_segment_length2(qh_vertex_t* p, qh_vertex_t* a, qh_vertex_t* b)
{
	float dx = b->X - a->X;
	float dy = b->Y - a->Y;
	float dz = b->Z - a->Z;

	//ab length2
	float d = dx * dx + dy * dy + dz * dz;

	float x = a->X;
	float y = a->Y;
	float z = a->Z;

	if (d != 0) {
		float t = ((p->X - a->X) * dx +
			(p->Y - a->Y) * dy +
			(p->Z - a->Z) * dz) / d;

		if (t > 1) {
			x = b->X;
			y = b->Y;
			z = b->Z;
		}
		else if (t > 0) {
			x += dx * t;
			y += dy * t;
			z += dz * t;
		}
	}

	dx = p->X - x;
	dy = p->Y - y;
	dz = p->Z - z;

	return dx * dx + dy * dy + dz * dz;
}

void qh__vec3_sub(qh_vec3_t* a, qh_vec3_t* b)
{
	a->X -= b->X;
	a->Y -= b->Y;
	a->Z -= b->Z;
}

void qh__vec3_add(qh_vec3_t* a, qh_vec3_t* b)
{
	a->X += b->X;
	a->Y += b->Y;
	a->Z += b->Z;
}

void qh__vec3_multiply(qh_vec3_t* a, float v)
{
	a->X *= v;
	a->Y *= v;
	a->Z *= v;
}

int qh__vertex_equals_epsilon(qh_vertex_t* a, qh_vertex_t* b, float epsilon)
{
	return fabs(a->X - b->X) <= epsilon &&
		fabs(a->Y - b->Y) <= epsilon &&
		fabs(a->Z - b->Z) <= epsilon;
}

float qh__vec3_length2(qh_vec3_t* v)
{
	return v->X * v->X + v->Y * v->Y + v->Z * v->Z;
}

float qh__vec3_dot(qh_vec3_t* v1, qh_vec3_t* v2)
{
	return v1->X * v2->X + v1->Y * v2->Y + v1->Z * v2->Z;
}

void qh__vec3_normalize(qh_vec3_t* v)
{
	qh__vec3_multiply(v, 1.f / sqrt(qh__vec3_length2(v)));
}

/**
Find indices of furthest vertices from eps
**/
void qh__find_2dps_6eps(qh_vertex_t* vertices, qh_index_t* eps, int* ii, int* jj)
{
	int i, j;
	float max = -QH_FLT_MAX;

	for (i = 0; i < 6; ++i) {
		for (j = 0; j < 6; ++j) {
			qh_vertex_t d;
			float d2;

			if (i == j) {
				continue;
			}

			d = vertices[eps[i]];
			qh__vec3_sub(&d, &vertices[eps[j]]);
			d2 = qh__vec3_length2(&d);

			if (d2 > max) {
				*ii = i;
				*jj = j;
				max = d2;
			}
		}
	}
}

qh_vec3_t qh__vec3_cross(qh_vec3_t* v1, qh_vec3_t* v2)
{
	qh_vec3_t cross;

	cross.X = v1->Y * v2->Z - v1->Z * v2->Y;
	cross.Y = v1->Z * v2->X - v1->X * v2->Z;
	cross.Z = v1->X * v2->Y - v1->Y * v2->X;

	return cross;
}

qh_vertex_t qh__face_centroid(qh_index_t vertices[3], qh_context_t* context)
{
	qh_vertex_t centroid;
	int i;

	centroid.X = centroid.Y = centroid.Z = 0.0;
	for (i = 0; i < 3; ++i) {
		qh__vec3_add(&centroid, context->vertices + vertices[i]);
	}

	qh__vec3_multiply(&centroid, 1.0 / 3.0);

	return centroid;
}

float qh__dist_point_plane(qh_vertex_t* v, qh_vec3_t* normal, float sdist)
{
	return fabs(qh__vec3_dot(v, normal) - sdist);
}

float qh__dist_point_plane_not_abs(qh_vertex_t* v, qh_vec3_t* normal, float sdist)
{
	FPlane Plane = QH_Face_To_FPlane(normal, sdist);
	return Plane.PlaneDot(*v);
}

void qh__init_half_edge(qh_half_edge_t* half_edge) {
	half_edge->adjacent_face = -1;
	half_edge->he = -1;
	half_edge->next_he = -1;
	half_edge->opposite_he = -1;
	half_edge->to_vertex = -1;
	half_edge->previous_he = -1;
}

qh_half_edge_t* qh__next_edge(qh_context_t* context)
{
	qh_half_edge_t* edge = context->edges + context->nedges;

	qh__init_half_edge(edge);

	edge->he = context->nedges;
	context->nedges++;

	QH_ASSERT(context->nedges < context->maxedges);

	return edge;
}

qh_face_t* qh__next_face(qh_context_t* context)
{
	qh_face_t* face = context->faces + context->nfaces;

	face->is_coplanar = false;
	face->face = context->nfaces;
	face->iset.indices = NULL;
	context->valid[context->nfaces] = 1;
	context->nfaces++;

	QH_ASSERT(context->nfaces < context->maxfaces);

	return face;
}

qh_vec3_t qh__edge_vec3(qh_half_edge_t* edge, qh_context_t* context)
{
	qh_half_edge_t prevhe = context->edges[edge->previous_he];
	qh_vec3_t v0, v1;

	v0 = context->vertices[prevhe.to_vertex];
	v1 = context->vertices[edge->to_vertex];

	qh__vec3_sub(&v1, &v0);
	qh__vec3_normalize(&v1);

	return v1;
}

void qh__face_init(qh_face_t* face, qh_index_t vertices[3], qh_context_t* context)
{
	face->is_coplanar = false;

	qh_half_edge_t* e0 = qh__next_edge(context);
	qh_half_edge_t* e1 = qh__next_edge(context);
	qh_half_edge_t* e2 = qh__next_edge(context);
	qh_vec3_t v0, v1;
	qh_vertex_t centroid, normal;

	e2->to_vertex = vertices[0];
	e0->to_vertex = vertices[1];
	e1->to_vertex = vertices[2];

	e0->next_he = e1->he;
	e2->previous_he = e1->he;
	face->edges[1] = e1->he;

	e1->next_he = e2->he;
	e0->previous_he = e2->he;
	face->edges[2] = e2->he;
	v1 = qh__edge_vec3(e2, context);

	e2->next_he = e0->he;
	e1->previous_he = e0->he;
	face->edges[0] = e0->he;
	v0 = qh__edge_vec3(e0, context);

	e2->adjacent_face = face->face;
	e1->adjacent_face = face->face;
	e0->adjacent_face = face->face;

	qh__vec3_multiply(&v1, -1.f);
	normal = qh__vec3_cross(&v0, &v1);

	qh__vec3_normalize(&normal);
	centroid = qh__face_centroid(vertices, context);
	face->centroid = centroid;
	face->sdist = qh__vec3_dot(&normal, &centroid);
	face->normal = normal;
	face->iset.indices = QH_MALLOC(qh_index_t, QH_VERTEX_SET_SIZE);
	face->iset.capacity = QH_VERTEX_SET_SIZE;
	face->iset.size = 0;
	face->visitededges = 0;
}

/**

**/
void qh__tetrahedron_basis(qh_context_t* context, qh_index_t vertices[3])
{
	qh_index_t eps[6];
	int i, j, k, l;
	float max = -QH_FLT_MAX;

	qh__find_6eps(context->vertices, context->nvertices, eps);
	qh__find_2dps_6eps(context->vertices, eps, &j, &k);

	for (i = 0; i < 6; ++i) {
		float d2;

		if (i == j || i == k) {
			continue;
		}

		d2 = qh__vertex_segment_length2(context->vertices + eps[i],
			context->vertices + eps[j],
			context->vertices + eps[k]);

		if (d2 > max) {
			max = d2;
			l = i;
		}
	}

	vertices[0] = eps[j];
	vertices[1] = eps[k];
	vertices[2] = eps[l];
}

void qh__push_stack(qh_index_stack_t* stack, qh_index_t index)
{
	stack->begin[stack->size] = index;
	stack->size++;
}

qh_index_t qh__pop_stack(qh_index_stack_t* stack)
{
	qh_index_t top = -1;

	if (stack->size > 0) {
		top = stack->begin[stack->size - 1];
		stack->size--;
	}

	return top;
}

qh_index_t qh__furthest_point_from_plane(qh_context_t* context,
	qh_index_t* indices,
	int nindices,
	qh_vec3_t* normal,
	float sdist)
{
	int i, j;
	float max = -QH_FLT_MAX;

	for (i = 0; i < nindices; ++i) {
		qh_index_t index = indices ? *(indices + i) : i;
		float dist = qh__dist_point_plane(context->vertices + index, normal, sdist);

		if (dist > max) {
			j = i;
			max = dist;
		}
	}

	return j;
}

qh_index_t qh__furthest_point_from_plane_not_abs(qh_context_t* context,
	qh_index_t* indices,
	int nindices,
	qh_vec3_t* normal,
	float sdist)
{
	if (nindices == 1)
		return 0;

	int i, j;
	float max = -QH_FLT_MAX;

	for (i = 0; i < nindices; ++i) {
		qh_index_t index = indices ? *(indices + i) : i;
		float dist = qh__dist_point_plane_not_abs(context->vertices + index, normal, sdist);

		if (dist > max) {
			j = i;
			max = dist;
		}
	}

	return j;
}

int qh__face_can_see_vertex(qh_face_t* face, qh_vertex_t* v)
{
	FPlane Plane = QH_Face_To_FPlane(*face);
	float dist = Plane.PlaneDot(*v);
	TryTruncateFloatToZero(dist);
	return dist > 0;
}

int qh__face_can_see_vertex_epsilon(qh_context_t* context, qh_face_t* face, qh_vertex_t* v, float epsilon)
{
	FPlane Plane = QH_Face_To_FPlane(*face);
	float dist = Plane.PlaneDot(*v);
	TryTruncateFloatToZero(dist);
	if (dist > 0)
		return 1;
	return 0;
}

static inline void qh__assert_half_edge(qh_half_edge_t* edge, qh_context_t* context)
{
	QH_ASSERT(edge->opposite_he != -1);
	QH_ASSERT(edge->he != -1);
	QH_ASSERT(edge->adjacent_face != -1);
	QH_ASSERT(edge->next_he != -1);
	QH_ASSERT(edge->previous_he != -1);
	QH_ASSERT(edge->to_vertex != -1);
	QH_ASSERT(context->edges[edge->opposite_he].to_vertex != edge->to_vertex);
}

static inline void qh__assert_face(qh_face_t* face, qh_context_t* context)
{
	int i;

	for (i = 0; i < 3; ++i) {
		qh__assert_half_edge(context->edges + face->edges[i], context);
	}

	QH_ASSERT(context->valid[face->face]);
}

#ifdef QUICKHULL_DEBUG

void qh__log_face(qh_context_t* context, qh_face_t const* face) {
	QH_LOG("Face %ld:\n", face->face);
	for (int i = 0; i < 3; ++i) {
		qh_half_edge_t edge = context->edges[face->edges[i]];
		QH_LOG("\te%d %ld\n", i, edge.he);
		QH_LOG("\t\te%d.opposite_he %ld\n", i, edge.opposite_he);
		QH_LOG("\t\te%d.next_he %ld\n", i, edge.next_he);
		QH_LOG("\t\te%d.previous_he %ld\n", i, edge.previous_he);
		QH_LOG("\t\te%d.to_vertex %ld\n", i, edge.to_vertex);
		QH_LOG("\t\te%d.adjacent_face %ld\n", i, edge.adjacent_face);
	}
	QH_LOG("\tnormal %f %f %f\n", face->normal.x, face->normal.y, face->normal.z);
	QH_LOG("\tsdist %f\n", face->sdist);
	QH_LOG("\tcentroid %f %f %f\n", face->centroid.x, face->centroid.y, face->centroid.z);
}

#endif

int qh__test_hull(qh_context_t* context, float epsilon, int testiset)
{
	unsigned int i, j, k;

	for (i = 0; i < context->nvertices; ++i) {
		qh_index_t vindex = i;
		char valid = 1;

		for (j = 0; j < context->nfaces; ++j) {
			if (!context->valid[j]) {
				continue;
			}
			qh_face_t* face = context->faces + j;

			qh_half_edge_t* e0 = context->edges + face->edges[0];
			qh_half_edge_t* e1 = context->edges + face->edges[1];
			qh_half_edge_t* e2 = context->edges + face->edges[2];

			if (e0->to_vertex == vindex ||
				e1->to_vertex == vindex ||
				e2->to_vertex == vindex) {
				valid = 0;
				break;
			}

			if (testiset) {
				for (k = 0; k < face->iset.size; ++k) {
					if (vindex == face->iset.indices[k]) {
						valid = 0;
					}
				}
			}
		}

		if (!valid) {
			continue;
		}

		for (j = 0; j < context->nfaces; ++j) {
			if (!context->valid[j]) {
				continue;
			}
			qh_face_t* face = context->faces + j;

			qh_vertex_t vertex = context->vertices[vindex];
			qh__vec3_sub(&vertex, &face->centroid);
			if (qh__vec3_dot(&face->normal, &vertex) > epsilon) {
#ifdef QUICKHULL_DEBUG
				qh__log_face(context, face);
#endif
				return 0;
			}
		}
	}

	return 1;
}

void CheckHorizonEdges(qh_context_t* context)
{
	qh_index_t ToVert, PrevVert;
	for (unsigned int i = 0; i < context->horizonedges.size; i++)
	{
		qh_half_edge_t e = context->edges[context->horizonedges.begin[i]];
		PrevVert = context->edges[e.previous_he].to_vertex;
		ToVert = e.to_vertex;
	}
}

#ifdef QUICKHULL_DEBUG
void qh__build_hull(qh_context_t* context, float epsilon, unsigned int step, unsigned int* failurestep)
#else
void qh__build_hull(qh_context_t* context, float epsilon)
#endif
{
	qh_index_t topface = qh__pop_stack(&context->facestack);
	unsigned int i, j, k;

#ifdef QUICKHULL_DEBUG
	unsigned int iteration = 0;
#endif

	while (topface != -1) {

#ifdef WITH_DEBUG
		if (topface == 43)
		{
			topface++;
			topface--;
		}
#endif
		qh_face_t* face = context->faces + topface;
		qh_index_t fvi, apex;
		qh_vertex_t* fv;
		int reversed = 0;

#ifdef QUICKHULL_DEBUG
		if (!context->valid[topface] || face->iset.size == 0 || iteration == step)
#else
		if (!context->valid[topface] || face->iset.size == 0)
#endif
		{
			topface = qh__pop_stack(&context->facestack);
			continue;
		}

#ifdef QUICKHULL_DEBUG
		if (failurestep != NULL && !qh__test_hull(context, epsilon, 1)) {
			if (*failurestep == 0) {
				*failurestep = iteration;
				break;
			}
		}

		iteration++;
#endif

		fvi = qh__furthest_point_from_plane_not_abs(context, face->iset.indices,
			face->iset.size, &face->normal, face->sdist);
		fv = context->vertices + *(face->iset.indices + fvi);

		apex = face->iset.indices[fvi];

		qh__assert_face(face, context);

		// Reset visited flag for faces
		{
			for (i = 0; i < context->nfaces; ++i) {
				context->faces[i].visitededges = 0;
			}
		}

		// Find horizon edge
		{
			qh_index_t tovisit = topface;
			qh_face_t* facetovisit = context->faces + tovisit;

			// Release scratch
			context->scratch.size = 0;

			while (tovisit != -1) {
				if (facetovisit->visitededges >= 3) {
					context->valid[tovisit] = 0;
					tovisit = qh__pop_stack(&context->scratch);
					facetovisit = context->faces + tovisit;
				}
				else {
					qh_index_t edgeindex = facetovisit->edges[facetovisit->visitededges];
					qh_half_edge_t* edge;
					qh_half_edge_t* oppedge;
					qh_face_t* adjface;

					facetovisit->visitededges++;

					edge = context->edges + edgeindex;
					oppedge = context->edges + edge->opposite_he;
					adjface = context->faces + oppedge->adjacent_face;

					if (!context->valid[oppedge->adjacent_face]) { continue; }

					qh__assert_half_edge(oppedge, context);
					qh__assert_half_edge(edge, context);
					qh__assert_face(adjface, context);

					if (!qh__face_can_see_vertex(adjface, fv)) {
						qh__push_stack(&context->horizonedges, edge->he);
					}
					else {
						context->valid[tovisit] = 0;
						qh__push_stack(&context->scratch, adjface->face);
					}
				}
			}
		}
#ifdef WITH_DEBUG
		CheckHorizonEdges(context);
#endif

		// Sort horizon edges in CCW order
		{
			qh_vertex_t triangle[3];
			int vindex = 0;
			qh_vec3_t v0, v1, toapex;
			qh_vertex_t n;

			for (i = 0; i < context->horizonedges.size - 2; ++i) {
				qh_index_t he0 = context->horizonedges.begin[i];
				qh_index_t he0vert = context->edges[he0].to_vertex;
				qh_index_t phe0 = context->edges[he0].previous_he;
				qh_index_t phe0vert = context->edges[phe0].to_vertex;

				for (j = i + 2; j < context->horizonedges.size; ++j) {
					qh_index_t he1 = context->horizonedges.begin[j];
					qh_index_t he1vert = context->edges[he1].to_vertex;
					qh_index_t phe1 = context->edges[he1].previous_he;
					qh_index_t phe1vert = context->edges[phe1].to_vertex;

					if (phe1vert == he0vert/* || phe0vert == he1vert*/) {
						QH_SWAP(qh_index_t, context->horizonedges.begin[j],
							context->horizonedges.begin[i + 1]);
						break;
					}
				}
			}
#ifdef WITH_DEBUG
			CheckHorizonEdges(context);
#endif			
		}

		// Create new faces
		{

			qh_index_t top = qh__pop_stack(&context->horizonedges);
			qh_index_t last = qh__pop_stack(&context->horizonedges);
			qh_index_t first = top;
			int looped = 0;

			QH_ASSERT(context->newhorizonedges.size == 0);

			// Release scratch
			context->scratch.size = 0;

			while (!looped) {
#ifdef WITH_DEBUG

#endif		
				qh_half_edge_t* prevhe;
				qh_half_edge_t* nexthe;
				qh_half_edge_t* oppedge;
				qh_vec3_t normal;
				qh_vertex_t fcentroid;
				qh_index_t verts[3];
				qh_face_t* newface;

				if (last == -1) {
					looped = 1;
					last = first;
				}

				prevhe = context->edges + last;
				nexthe = context->edges + top;

				verts[0] = prevhe->to_vertex;
				verts[1] = nexthe->to_vertex;
				verts[2] = apex;

				context->valid[nexthe->adjacent_face] = 0;

				oppedge = context->edges + nexthe->opposite_he;
				newface = qh__next_face(context);

				qh__face_init(newface, verts, context);
#ifdef WITH_DEBUG
				Check_QH_Face(context, *newface);
#endif // WITH_DEBUG

				oppedge->opposite_he = context->edges[newface->edges[0]].he;
				context->edges[newface->edges[0]].opposite_he = oppedge->he;

				qh__push_stack(&context->scratch, newface->face);
				qh__push_stack(&context->newhorizonedges, newface->edges[0]);

				top = last;
				last = qh__pop_stack(&context->horizonedges);
			}
		}

		// Attach point sets to newly created faces
		{
			for (k = 0; k < context->nfaces; ++k) {
				qh_face_t* f = context->faces + k;

				if (context->valid[k] || f->iset.size == 0) {
					continue;
				}

				if (f->visitededges == 3) {
					context->valid[k] = 0;
				}

				for (i = 0; i < f->iset.size; ++i) {
					qh_index_t vertex = f->iset.indices[i];
					qh_vertex_t* v = context->vertices + vertex;
					qh_face_t* dface = NULL;

					for (j = 0; j < context->scratch.size; ++j) {
						qh_face_t* newface = context->faces + context->scratch.begin[j];
						qh_half_edge_t* e0 = context->edges + newface->edges[0];
						qh_half_edge_t* e1 = context->edges + newface->edges[1];
						qh_half_edge_t* e2 = context->edges + newface->edges[2];
						qh_vertex_t cv;

						if (e0->to_vertex == vertex ||
							e1->to_vertex == vertex ||
							e2->to_vertex == vertex) {
							continue;
						}

						if (qh__face_can_see_vertex_epsilon(context, newface, context->vertices + vertex, epsilon)) {
							dface = newface;
							break;
						}
					}

					if (dface) {
						if (dface->iset.size + 1 >= dface->iset.capacity) {
							dface->iset.capacity *= 2;
							dface->iset.indices = QH_REALLOC(qh_index_t,
								dface->iset.indices, dface->iset.capacity);
						}

						dface->iset.indices[dface->iset.size++] = vertex;
					}
				}

				f->iset.size = 0;
			}
		}

		// Link new faces together
		{
			for (i = 0; i < context->newhorizonedges.size; ++i) {
				qh_index_t phe0, nhe1;
				qh_half_edge_t* he0;
				qh_half_edge_t* he1;
				int ii;

				ii = (i + 1) % context->newhorizonedges.size;

				phe0 = context->edges[context->newhorizonedges.begin[i]].previous_he;
				nhe1 = context->edges[context->newhorizonedges.begin[ii]].next_he;

				he0 = context->edges + phe0;
				he1 = context->edges + nhe1;

				QH_ASSERT(he1->to_vertex == apex);
				QH_ASSERT(he0->opposite_he == -1);
				QH_ASSERT(he1->opposite_he == -1);

				he0->opposite_he = he1->he;
				he1->opposite_he = he0->he;
			}

			context->newhorizonedges.size = 0;
		}

		// Push new face to stack
		{
			for (i = 0; i < context->scratch.size; ++i) {
				qh_face_t* face = context->faces + context->scratch.begin[i];

				if (face->iset.size > 0) {
					qh__push_stack(&context->facestack, face->face);
				}
			}

			// Release scratch
			context->scratch.size = 0;
		}

		topface = qh__pop_stack(&context->facestack);

		// TODO: push all non-valid faces for reuse in face stack memory pool
	}
}

void qh_mesh_export(qh_mesh_t const* mesh, char const* filename)
{
	FILE* objfile = fopen(filename, "wt");
	fprintf(objfile, "o\n");

	for (unsigned int i = 0; i < mesh->nvertices; ++i) {
		qh_vertex_t v = mesh->vertices[i];
		fprintf(objfile, "v %f %f %f\n", v.X, v.Y, v.Z);
	}

	for (unsigned int i = 0; i < mesh->nnormals; ++i) {
		qh_vec3_t n = mesh->normals[i];
		fprintf(objfile, "vn %f %f %f\n", n.X, n.Y, n.Z);
	}

	for (unsigned int i = 0, j = 0; i < mesh->nindices; i += 3, j++) {
		fprintf(objfile, "f %u/%u %u/%u %u/%u\n",
			mesh->indices[i + 0] + 1, mesh->normalindices[j] + 1,
			mesh->indices[i + 1] + 1, mesh->normalindices[j] + 1,
			mesh->indices[i + 2] + 1, mesh->normalindices[j] + 1);
	}

	fclose(objfile);
}

qh_face_t* qh__build_tetrahedron(qh_context_t* context, float epsilon)
{
	unsigned int i, j;
	qh_index_t vertices[3];
	qh_index_t apex;
	qh_face_t* faces;
	qh_vertex_t normal, centroid, vapex, tcentroid;

	// Get the initial tetrahedron basis (first face)
	qh__tetrahedron_basis(context, &vertices[0]);

	// Find apex from the tetrahedron basis
	{
		float sdist;
		qh_vec3_t v0, v1;

		v0 = context->vertices[vertices[1]];
		v1 = context->vertices[vertices[2]];

		qh__vec3_sub(&v0, context->vertices + vertices[0]);
		qh__vec3_sub(&v1, context->vertices + vertices[0]);

		normal = qh__vec3_cross(&v0, &v1);
		qh__vec3_normalize(&normal);

		centroid = qh__face_centroid(vertices, context);
		sdist = qh__vec3_dot(&normal, &centroid);

		apex = qh__furthest_point_from_plane(context, NULL,
			context->nvertices, &normal, sdist);
		vapex = context->vertices[apex];

		qh__vec3_sub(&vapex, &centroid);

		// Whether the face is looking towards the apex
		if (qh__vec3_dot(&vapex, &normal) > 0) {
			QH_SWAP(qh_index_t, vertices[1], vertices[2]);
		}
	}

	faces = qh__next_face(context);
	qh__face_init(&faces[0], vertices, context);
#ifdef WITH_DEBUG
	//Check_QH_Face(context, faces[0]);
#endif // WITH_DEBUG


	// Build faces from the tetrahedron basis to the apex
	{
		qh_index_t facevertices[3];
		for (i = 0; i < 3; ++i) {
			qh_half_edge_t* edge = context->edges + faces[0].edges[i];
			qh_half_edge_t prevedge = context->edges[edge->previous_he];
			qh_face_t* face = faces + i + 1;
			qh_half_edge_t* e0;

			facevertices[0] = edge->to_vertex;
			facevertices[1] = prevedge.to_vertex;
			facevertices[2] = apex;

			qh__next_face(context);
			qh__face_init(face, facevertices, context);
#ifdef WITH_DEBUG
			//Check_QH_Face(context, *face);
#endif // WITH_DEBUG

			e0 = context->edges + faces[i + 1].edges[0];
			edge->opposite_he = e0->he;
			e0->opposite_he = edge->he;
		}
	}

	// Attach half edges to faces tied to the apex
	{
		for (i = 0; i < 3; ++i) {
			qh_face_t* face;
			qh_face_t* nextface;
			qh_half_edge_t* e1;
			qh_half_edge_t* e2;

			j = (i + 2) % 3;

			face = faces + i + 1;
			nextface = faces + j + 1;

			e1 = context->edges + face->edges[1];
			e2 = context->edges + nextface->edges[2];

			QH_ASSERT(e1->opposite_he == -1);
			QH_ASSERT(e2->opposite_he == -1);

			e1->opposite_he = e2->he;
			e2->opposite_he = e1->he;

			qh__assert_half_edge(e1, context);
			qh__assert_half_edge(e2, context);
		}
	}

	// Create initial point set; every point is
	// attached to the first face it can see
	{
		for (i = 0; i < context->nvertices; ++i) {
			//qh_vertex_t* v;
			qh_face_t* dface = NULL;

			if (vertices[0] == i || vertices[1] == i || vertices[2] == i || apex == i) {
				continue;
			}

			for (j = 0; j < 4; ++j) {
				if (qh__face_can_see_vertex_epsilon(context, context->faces + j, context->vertices + i, epsilon)) {
					dface = context->faces + j;
					break;
				}
			}

			if (dface) {
				int valid = 1;

				for (int j = 0; j < 3; ++j) {
					qh_half_edge_t* e = context->edges + dface->edges[j];
					if (i == e->to_vertex) {
						valid = 0;
						break;
					}
				}

				if (!valid) { continue; }

				if (dface->iset.size + 1 >= dface->iset.capacity) {
					dface->iset.capacity *= 2;
					dface->iset.indices = QH_REALLOC(qh_index_t,
						dface->iset.indices, dface->iset.capacity);
				}

				dface->iset.indices[dface->iset.size++] = i;
			}
		}
	}

	// Add initial tetrahedron faces to the face stack
	tcentroid.X = tcentroid.Y = tcentroid.Z = 0.0;
	for (i = 0; i < 4; ++i) {
		context->valid[i] = 1;
		qh__assert_face(context->faces + i, context);
		qh__push_stack(&context->facestack, i);
		qh__vec3_add(&tcentroid, &context->faces[i].centroid);
	}

	// Assign the tetrahedron centroid
	qh__vec3_multiply(&tcentroid, 0.25);
	context->centroid = tcentroid;

	QH_ASSERT(context->nedges == context->nfaces * 3);
	QH_ASSERT(context->nfaces == 4);
	QH_ASSERT(context->facestack.size == 4);

	return faces;
}

void qh__remove_vertex_duplicates(qh_context_t* context, float epsilon)
{
	unsigned int i, j, k;
	for (i = 0; i < context->nvertices; ++i) {
		qh_vertex_t* v = context->vertices + i;
		/*if (v->x == 0) v->x = 0;
		if (v->y == 0) v->y = 0;
		if (v->z == 0) v->z = 0;*/
		for (j = i + 1; j < context->nvertices; ++j) {
			if (qh__vertex_equals_epsilon(context->vertices + i,
				context->vertices + j, epsilon))
			{
				for (k = j; k < context->nvertices - 1; ++k) {
					context->vertices[k] = context->vertices[k + 1];
				}
				context->nvertices--;
			}
		}
	}
}

void qh__init_context(qh_context_t* context, qh_vertex_t const* vertices, unsigned int nvertices)
{
	// TODO:
	// size_t nedges = 3 * nvertices - 6;
	// size_t nfaces = 2 * nvertices - 4;
	unsigned int nfaces = nvertices * (nvertices - 1);
	unsigned int nedges = nfaces * 3;

	context->edges = QH_MALLOC(qh_half_edge_t, nedges);
	context->faces = QH_MALLOC(qh_face_t, nfaces);
	context->facestack.begin = QH_MALLOC(qh_index_t, nfaces);
	context->scratch.begin = QH_MALLOC(qh_index_t, nfaces);
	context->horizonedges.begin = QH_MALLOC(qh_index_t, nedges);
	context->newhorizonedges.begin = QH_MALLOC(qh_index_t, nedges);
	context->valid = QH_MALLOC(char, nfaces);

	context->vertices = QH_MALLOC(qh_vertex_t, nvertices);
	memcpy(context->vertices, vertices, sizeof(qh_vertex_t) * nvertices);
	delete[] vertices;

	context->nvertices = nvertices;
	context->nedges = 0;
	context->nfaces = 0;
	context->facestack.size = 0;
	context->scratch.size = 0;
	context->horizonedges.size = 0;
	context->newhorizonedges.size = 0;

#ifdef QUICKHULL_DEBUG
	context->maxfaces = nfaces;
	context->maxedges = nedges;
#endif
}

void qh__free_context(qh_context_t* context)
{
	unsigned int i;

	for (i = 0; i < context->nfaces; ++i) {
		QH_FREE(context->faces[i].iset.indices);
		context->faces[i].iset.size = 0;
	}

	context->nvertices = 0;
	context->nfaces = 0;

	QH_FREE(context->edges);

	QH_FREE(context->faces);
	QH_FREE(context->facestack.begin);
	QH_FREE(context->scratch.begin);
	QH_FREE(context->horizonedges.begin);
	QH_FREE(context->newhorizonedges.begin);
	QH_FREE(context->vertices);
	QH_FREE(context->valid);
}

void qh_free_mesh(qh_mesh_t mesh)
{
	QH_FREE(mesh.vertices);
	QH_FREE(mesh.indices);
	QH_FREE(mesh.normalindices);
	QH_FREE(mesh.normals);
}

float qh__compute_epsilon(qh_vertex_t const* vertices, unsigned int nvertices)
{
	float epsilon;
	unsigned int i;

	float maxxi = -QH_FLT_MAX;
	float maxyi = -QH_FLT_MAX;

	for (i = 0; i < nvertices; ++i) {
		float fxi = fabsf(vertices[i].X);
		float fyi = fabsf(vertices[i].Y);

		if (fxi > maxxi) {
			maxxi = fxi;
		}
		if (fyi > maxyi) {
			maxyi = fyi;
		}
	}

	epsilon = 2 * (maxxi + maxyi) * QH_FLT_EPS;

	return epsilon;
}

qh_mesh_t qh_quickhull3d(TArray<FVector>& Vertices, unsigned int nvertices)
{
	qh_vertex_t const* vertices = TArray_To_Pointer_Array(Vertices);

	qh_mesh_t m;
	qh_context_t context;
	//unsigned int* indices;
	unsigned int nfaces = 0, i, index, nindices;
	float epsilon = 0;

	epsilon = qh__compute_epsilon(vertices, nvertices);

	qh__init_context(&context, vertices, nvertices);

	qh__remove_vertex_duplicates(&context, epsilon);

	// Build the initial tetrahedron
	qh__build_tetrahedron(&context, epsilon);

	// Build the convex hull
#ifdef QUICKHULL_DEBUG
	qh__build_hull(&context, epsilon, -1, NULL);
#else
	qh__build_hull(&context, epsilon);
#endif

	QH_ASSERT(qh__test_hull(&context, epsilon));

	for (i = 0; i < context.nfaces; ++i) {
		if (context.valid[i]) { nfaces++; }
	}

	nindices = nfaces * 3;

	m.normals = QH_MALLOC(qh_vertex_t, nfaces);
	m.normalindices = QH_MALLOC(unsigned int, nfaces);
	m.vertices = QH_MALLOC(qh_vertex_t, nindices);
	m.indices = QH_MALLOC(unsigned int, nindices);
	m.nindices = nindices;
	m.nnormals = nfaces;
	m.nvertices = 0;

	{
		index = 0;
		for (i = 0; i < context.nfaces; ++i) {
			if (!context.valid[i]) { continue; }
			m.normals[index] = context.faces[i].normal;
			index++;
		}

		index = 0;
		for (i = 0; i < context.nfaces; ++i) {
			if (!context.valid[i]) { continue; }
			m.normalindices[index] = index;
			index++;
		}

		index = 0;
		for (i = 0; i < context.nfaces; ++i) {
			if (!context.valid[i]) { continue; }
			m.indices[index + 0] = index + 0;
			m.indices[index + 1] = index + 1;
			m.indices[index + 2] = index + 2;
			index += 3;
		}

		for (i = 0; i < context.nfaces; ++i) {
			if (!context.valid[i]) { continue; }
			qh_half_edge_t e0 = context.edges[context.faces[i].edges[0]];
			qh_half_edge_t e1 = context.edges[context.faces[i].edges[1]];
			qh_half_edge_t e2 = context.edges[context.faces[i].edges[2]];

			m.vertices[m.nvertices++] = context.vertices[e0.to_vertex];
			m.vertices[m.nvertices++] = context.vertices[e1.to_vertex];
			m.vertices[m.nvertices++] = context.vertices[e2.to_vertex];

#ifdef WITH_DEBUG			
			Check_QH_Face(&context, context.faces[i]);
#endif
		}
	}

	qh__free_context(&context);

	return m;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool EqualFaces(const qh_face_t& A, const qh_face_t& B)
{
	float dx = A.normal.X - B.normal.X;
	TryTruncateFloatToZero(dx);
	float dy = A.normal.Y - B.normal.Y;
	TryTruncateFloatToZero(dy);
	float dz = A.normal.Z - B.normal.Z;
	TryTruncateFloatToZero(dz);
	float ddist = A.sdist - B.sdist;
	TryTruncateFloatToZero(ddist);
	//return A.normal.X == B.normal.X && A.normal.Y == B.normal.Y && A.normal.Z == B.normal.Z && A.sdist == B.sdist;
	return dx == 0 && dy == 0 && dz == 0 && ddist == 0;
}

void FaceMult(qh_face_t& face, float Scale)
{
	qh__vec3_multiply(&face.normal, Scale);
	face.sdist *= Scale;
}

FVector QH_Vertex_To_FVector(const qh_vertex_t& Vertex)
{
	return FVector(Vertex.X, Vertex.Y, Vertex.Z);
}

FPlane QH_Face_To_FPlane(qh_context_t* context, const qh_face_t& face)
{
	qh_half_edge_t e0 = context->edges[face.edges[0]];
	qh_half_edge_t e1 = context->edges[face.edges[1]];
	qh_half_edge_t e2 = context->edges[face.edges[2]];

	FVector v0 = context->vertices[e0.to_vertex];
	FVector v1 = context->vertices[e1.to_vertex];
	FVector v2 = context->vertices[e2.to_vertex];

	return FPlane(v0, v1, v2);
}

FPlane QH_Face_To_FPlane(const qh_face_t& face)
{
	qh_vec3_t normal = face.normal;
	return FPlane(normal.X, normal.Y, normal.Z, face.sdist);
}

FPlane QH_Face_To_FPlane(const qh_vec3_t* normal, const float sdist)
{
	return FPlane(normal->X, normal->Y, normal->Z, sdist);
}

bool ComplanarFaces(const qh_face_t& A, const qh_face_t& B)
{
	if (EqualFaces(A, B))
		return true;
	else
	{
		qh_face_t C = B;
		FaceMult(C, -1);
		return EqualFaces(A, C);
	}
}

void GetValidFaces(qh_context_t* context, TArray<qh_face_t>& ValidFaces)
{
	for (unsigned int i = 0; i < context->nfaces; ++i) {
		if (context->valid[i])
		{			
			context->faces[i].validindex = ValidFaces.Num();
			context->faces[i].visitededges = 0;
			ValidFaces.Add(context->faces[i]);			
		}
	}
}

/**
Create NewFace from ValidFace, reorder its vertices and neighbours in CCW order and adding it to Faces array
**/
void AddTriangleFace(qh_context_t* context, TArray<Face>& Faces, const qh_face_t& ValidFace)
{
	Face NewFace;
	NewFace.Number = Faces.Num();
	//Add indices to NewFace in CCW order			
	qh_half_edge_t e0 = context->edges[ValidFace.edges[0]];
	qh_half_edge_t e1 = context->edges[ValidFace.edges[1]];
	qh_half_edge_t e2 = context->edges[ValidFace.edges[2]];
	NewFace.Indices.Add(e0.to_vertex);
	NewFace.Indices.Add(e2.to_vertex);
	NewFace.Indices.Add(e1.to_vertex);
	NewFace.Neighbours.Add(NewFace.Neighbours.Num(), context->faces[context->edges[e0.opposite_he].adjacent_face].validindex);
	NewFace.Neighbours.Add(NewFace.Neighbours.Num(), context->faces[context->edges[e2.opposite_he].adjacent_face].validindex);
	NewFace.Neighbours.Add(NewFace.Neighbours.Num(), context->faces[context->edges[e1.opposite_he].adjacent_face].validindex);
	NewFace.AllNeighboursFound = true;

	Faces.Add(NewFace);
}

/**
Recursive search of coplanar faces
**/
void SearchCoplanarFaces(qh_context_t* context, TArray<qh_face_t>& CoplanarFaces, TArray<qh_half_edge_t>& Edges, TArray<qh_face_t>& ValidFaces)
{
	qh_face_t face = CoplanarFaces.Last();
#ifdef WITH_DEBUG
	Check_QH_Face(context, face);
#endif
	//Check face's neighbours
	for (int i = 0; i < 3; i++)
	{
		qh_half_edge_t e = context->edges[face.edges[i]];
		qh_face_t& adjacentface = ValidFaces[context->faces[context->edges[e.opposite_he].adjacent_face].validindex];
#ifdef WITH_DEBUG
		Check_QH_Face(context, adjacentface);
#endif
		if (!adjacentface.is_coplanar)
		{
			if (!ComplanarFaces(face, adjacentface))
			{
				Edges.Add(e);
			}
			else
			{

				adjacentface.is_coplanar = true;
				adjacentface.visitededges = 1;
				CoplanarFaces.Add(adjacentface);
				SearchCoplanarFaces(context, CoplanarFaces, Edges, ValidFaces);
			}
		}
	}
}

/**
Set is_coplanar false for ValidFaces
**/
void ResetCoplanarFaces(const TArray<qh_face_t>& CoplanarFaces, TArray<qh_face_t>& ValidFaces)
{
	for (int i = 0; i < CoplanarFaces.Num(); i++)
	{
		ValidFaces[CoplanarFaces[i].validindex].is_coplanar = false;
	}
}

/**
Sort edges in CW order
**/
void SortEdges(qh_context_t* context, TArray<qh_half_edge_t>& Edges)
{
	for (int i = 0; i < Edges.Num() - 2; i++)
	{
		for (int j = i + 1; j < Edges.Num(); j++)
		{
			if (Edges[i].to_vertex == context->edges[Edges[j].previous_he].to_vertex)
			{
				if (j > i + 1)
				{
					Swap(Edges[i + 1], Edges[j]);
					break;
				}
				else
					break;
			}
		}
	}
}

void CheckEdges(const TArray<qh_half_edge_t>& Edges)
{
	qh_index_t VertexIndex;
	for (int i = 0; i < Edges.Num(); i++)
	{
		VertexIndex = Edges[i].to_vertex;
	}
}



void InitFace(Face& face, const TArray<qh_half_edge_t>& Edges)
{
	//At this moment Edges are sorted in CW order
	//Reorder them in CCW order
	//And add unfound neighbours 
	for (int i = Edges.Num() - 1; i >= 0; i--)
	{
		face.Indices.Add(Edges[i].to_vertex);
		face.Neighbours.Add(i, -1);
	}
}

bool AllNeighboursFound(Face& face)
{
	for (int i = 0; i < face.Neighbours.Num(); i++)
	{
		if (face.Neighbours[i] == -1)
			return false;
	}
	return true;
}

/**
Merge coplanar faces (if need) and reorder their points in CCW
If face is triangle (have not complanar faces) - reorder its points in CCW order
**/
void OrderAndOrMergeFaces(qh_context_t* context, TArray<qh_face_t>& ValidFaces, TArray<Face>& Faces, bool MergeTriangles)
{
	if (!MergeTriangles)
	{
		for (int i = 0; i < ValidFaces.Num(); i++)
		{
			AddTriangleFace(context, Faces, ValidFaces[i]);
		}
	}
	//Search coplanar faces, edges and neighbours
	else
	{
		TArray<qh_face_t> CoplanarFaces;
		TArray<qh_half_edge_t> Edges;
		TArray<TArray<qh_half_edge_t>> FacesAndTheirEdges;
		for (int i = 0; i < ValidFaces.Num(); i++)
		{
			qh_face_t& face = ValidFaces[i];
			if (face.visitededges == 1)
				continue;

			CoplanarFaces.Empty();
			Edges.Empty();

			face.is_coplanar = true;
			face.visitededges = 1;
			CoplanarFaces.Add(face);
			SearchCoplanarFaces(context, CoplanarFaces, Edges, ValidFaces);
			ResetCoplanarFaces(CoplanarFaces, ValidFaces);
			if (Edges.Num() > 3)
				SortEdges(context, Edges);
#ifdef WITH_DEBUG
			CheckEdges(Edges);
#endif
			FacesAndTheirEdges.Add(Edges);
			Face NewFace;
			NewFace.Number = Faces.Num();
			InitFace(NewFace, Edges);
			Faces.Add(NewFace);
		}
		CoplanarFaces.Empty();
		Edges.Empty();
		//Search neighbours
		for (int i = 0; i < Faces.Num(); i++)
		{
			//edges of Face in CCW order
			Face& OriginFace = Faces[i];
#ifdef WITH_DEBUG
			//CheckFace(OriginFace);
#endif

			int num = FacesAndTheirEdges[i].Num() - 1;
			for (int j = num; j >= 0; j--)
			{
				if (OriginFace.Neighbours[OriginFace.Neighbours.Num() - 1 - j] != -1)
					continue;

				qh_index_t he = FacesAndTheirEdges[i][j].he;

				//search neighbours among other faces

				for (int k = 0; k < FacesAndTheirEdges.Num(); k++)
				{
					Face& Neighbour = Faces[k];

					if (i == k || Neighbour.AllNeighboursFound)
						continue;
#ifdef WITH_DEBUG
					//CheckFace(Neighbour);
#endif
					bool NeighbourFound = false;
					int num1 = FacesAndTheirEdges[k].Num() - 1;
					for (int l = num1; l >= 0; l--)
					{
						if (FacesAndTheirEdges[k][l].opposite_he == he)
						{
							//Current Face's neighbour at edge (Neighbours.Num() - 1 - j) number is k
							OriginFace.Neighbours[OriginFace.Neighbours.Num() - 1 - j] = k;

							//Neighbour's neighbour is current face number (i) at edge (Neighbours.Num() - 1 - l) number is i							
							Neighbour.Neighbours[Neighbour.Neighbours.Num() - 1 - l] = i;
							NeighbourFound = true;
							break;
						}
					}
					if (NeighbourFound)
						break;
				}
			}
			Faces[i].AllNeighboursFound = true;
		}
	}
}

/**
Fill TMap
Key - vertex index in array of vertices
Value = 0 if excess vertex (inside face or on edge)
Value = 1 if hull vertex
**/
void FillExcessIndices(TArray<uint8>& ExcessIndices, const uint32& nvertices, const TArray<Face>& Faces)
{	
	for (uint32 i = 0; i < nvertices; i++)
	{
		ExcessIndices.Add(0);
	}
	for (int i = 0; i < Faces.Num(); i++)
	{
		for (int j = 0; j < Faces[i].Indices.Num(); j++)
		{
			ExcessIndices[Faces[i].Indices[j]] = 1;
		}
	}
}

void Check_QH_Face(qh_context_t* context, const qh_face_t& Face)
{
	uint32 VertexIndex;
	qh_half_edge_t e0 = context->edges[Face.edges[0]];
	qh_half_edge_t e1 = context->edges[Face.edges[1]];
	qh_half_edge_t e2 = context->edges[Face.edges[2]];

	VertexIndex = e0.to_vertex;
	VertexIndex = e1.to_vertex;
	VertexIndex = e2.to_vertex;
}

uint32 qh_quickhull3d(TArray<FVector>& Vertices, unsigned int nvertices, TArray<Face>& Faces, bool MergeTriangles, bool WithApproximation)
{
	Faces.Empty();


	qh_vertex_t const* vertices = TArray_To_Pointer_Array(Vertices);

	qh_context_t context;

	//unsigned int i;
	float epsilon = 0;

	if (WithApproximation)
		epsilon = qh__compute_epsilon(vertices, nvertices);

	qh__init_context(&context, vertices, nvertices);

	qh__remove_vertex_duplicates(&context, epsilon);

	// Build the initial tetrahedron
	qh__build_tetrahedron(&context, epsilon);

	// Build the convex hull
#ifdef QUICKHULL_DEBUG
	qh__build_hull(&context, epsilon, -1, NULL);
#else
	qh__build_hull(&context, epsilon);
#endif	

	QH_ASSERT(qh__test_hull(&context, epsilon));

	//get valid faces
	TArray<qh_face_t> ValidFaces;
	GetValidFaces(&context, ValidFaces);

#ifdef WITH_DEBUG
	for (int i = 0; i < ValidFaces.Num(); i++)
	{
		Check_QH_Face(&context, ValidFaces[i]);
	}
#endif

	//Fill Faces, merge complanar faces
	//And if face hasn't complanar faces (it is triangle) sort its points in CCW order
	OrderAndOrMergeFaces(&context, ValidFaces, Faces, MergeTriangles);

#ifdef WITH_DEBUG
	for (int i = 0; i < Faces.Num(); i++)
	{
		//CheckFace(Faces[i]);
	}
	CheckFaces(Faces);
#endif

	qh__free_context(&context);

	TArray<uint8> ExcessIndices;
	FillExcessIndices(ExcessIndices, nvertices, Faces);

	if (HasExcessVerts(ExcessIndices))
	{
		VerticesAndIndicesCorrection(Vertices, Faces, ExcessIndices);
	}

#ifdef WITH_DEBUG
	for (int i = 0; i < Faces.Num(); i++)
	{
		//CheckFace(Faces[i]);
	}
	CheckFaces(Faces);
#endif

	return Faces.Num();
}

//#endif //QUICKHULL_IMPLEMENTATION