// Fill out your copyright notice in the Description page of Project Settings.

#pragma once


#include "CoreMinimal.h"
#include "GeomTools.h"

#define FLT_ZERO 2e-5f

//#define WITH_DEBUG



/**
**/
struct Face
{
	/**
	* Index of current Face in array of Faces
	**/
	uint32 Number;

	/**
	* Array of indices of vertices of current Face from array of all vertices
	**/
	TArray<uint32> Indices;

	/**
	* Key - Local Edge Number (matches with Indices number)
	* Value - Neighbour Face Index in array of Faces
	* If Neighbour not found yet Value = -1
	**/
	TMap<uint32, int64> Neighbours;

	bool AllNeighboursFound;

	bool IsSlicedFace;


	Face()
	{
		AllNeighboursFound = false;
		IsSlicedFace = false;
	}

	friend FArchive& operator <<(FArchive& Ar, Face& face)
	{
		Ar << face.Number;
		Ar << face.Indices;		
		Ar << face.Neighbours;
		Ar << face.AllNeighboursFound;
		Ar << face.IsSlicedFace;
		
		return Ar;
	}
};

struct SlicedFace :Face
{

	//bool LeftEdgeFound;
	/**
	Index of Left Sliced Edge relatively to SlicePlane 2D proection turned down
	If SlicePlane intersects face in vertex at left - LeftEdgeIndex is index of previous edge relatively to vertex
	**/
	uint32 LeftEdgeIndex;
	/**
	Slice Point of left Sliced Edge
	**/
	FVector LeftSlicePoint;
	bool LeftVertexIsOnSlicePlane;

	//bool RightEdgeFound;
	/**
	Index of Right Sliced Edge relatively to SlicePlane 2D proection turned down
	If SlicePlane intersects face in vertex at right - LeftEdgeIndex is index of current edge relatively to vertex
	**/
	uint32 RightEdgeIndex;
	/**
	Slice Point of right Sliced Edge
	**/
	FVector RightSlicePoint;
	bool RightVertexIsOnSlicePlane;

	/**
	Edge of face is on SlcePlane
	**/
	bool EdgeOnSlicePlane;
	uint32 EdgeOnSlicePlaneIndex;

	SlicedFace() :Face()
	{
		//LeftEdgeFound = false;
		LeftVertexIsOnSlicePlane = false;
		//RightEdgeFound = false;
		RightVertexIsOnSlicePlane = false;
		EdgeOnSlicePlane = false;
	}

	SlicedFace(const Face& face)
	{
		this->Number = face.Number;
		this->Indices = face.Indices;
		this->Neighbours = face.Neighbours;
		this->AllNeighboursFound = face.AllNeighboursFound;
		//this->LeftEdgeFound = false;
		this->LeftVertexIsOnSlicePlane = false;
		//this->RightEdgeFound = false;
		this->RightVertexIsOnSlicePlane = false;
		this->EdgeOnSlicePlane = false;
	}

};

struct EdgeIndices
{
	int32 Index0, Index1;

	EdgeIndices SwapIndices()
	{
		Swap(this->Index0, this->Index1);
		return *this;
	}

	bool operator==(const EdgeIndices& EI) const
	{
		return (this->Index0 == EI.Index0&&this->Index1 == EI.Index1) || (this->Index0 == EI.Index1&&this->Index1 == EI.Index0);
	}

};

struct SlicedEdge:EdgeIndices
{
	int32 FirstHalfIndex, OtherHalfIndex;
	
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////
//#ifndef QUICKHULL_IMPLEMENTATION
//#define QUICKHULL_IMPLEMENTATION
//#endif

//#ifndef QUICKHULL_H
//#define QUICKHULL_H

// ------------------------------------------------------------------------------------------------
// QUICKHULL PUBLIC API
//

typedef FVector qh_vertex_t;

typedef FVector qh_vec3_t;

typedef struct qh_mesh {
	qh_vertex_t* vertices;
	qh_vec3_t* normals;
	unsigned int* indices;
	unsigned int* normalindices;
	unsigned int nindices;
	unsigned int nvertices;
	unsigned int nnormals;
} qh_mesh_t;

qh_mesh_t qh_quickhull3d(TArray<FVector>& Vertices, unsigned int nvertices);

uint32 qh_quickhull3d(TArray<FVector>& Vertices, unsigned int nvertices, TArray<Face>& Faces, bool MergeTriangles = true, bool WithApproximation = false);

void qh_mesh_export(qh_mesh_t const* mesh, char const* filename);

void qh_free_mesh(qh_mesh_t mesh);


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool PointsAreOnOneLine(FVector A, FVector B, FVector C);

float CosBetweenPlanes(const FPlane& A, const FPlane& B);

float CosBetweenVerts(const FVector& A, const FVector& B);

template <typename T>
int GetIndex(const TArray<T> Array, const T& Elem);

template <typename T>
bool Contains3DArray(const TArray<TArray<T>> Array, const T& Elem);

void CheckNeighbours(Face& F);

void FillTempVertices(TArray<BYTE>& TempIndices, const TArray<FVector>& HullVertices);

void RemoveTempVertices(TArray<BYTE>& TempVertIndices, const TArray<BYTE>& PlaneIndices, const TArray<BYTE>&  ExcessIndices);

bool ComplanarPlanes(const FPlane& A, const FPlane& B);

qh_vertex_t const* TArray_To_Pointer_Array(const TArray<FVector>& Vertices);

void CheckVertices(const Face& face, const TArray<FVector>& OriginVertices);

void CheckFace(const Face& face);

void CheckFaces(const TArray<Face>& Faces);

void CheckSlicedFace(const SlicedFace& face);

void CheckSlicedFaces(const TArray<SlicedFace>& Faces);

//void CheckEdges(const TArray<qh_half_edge_t>& Edges);

void CheckEdges(const TArray<EdgeIndices>& Edges);

void TryTruncateFloatToZero(float& Value);

void VerticesAndIndicesCorrection(TArray<FVector>& Vertices, TArray<Face>& Faces, TArray<uint8>& ExcessIndices);

bool HasExcessVerts(const TArray <uint8>& ExcessIndices);

void WriteVertices(const TArray<FVector>& Vertices, char* FileName = "Vertices.txt");
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////



//
// END QUICKHULL PUBLIC API
// ------------------------------------------------------------------------------------------------

//#endif // QUICKHULL_H

//#ifdef QUICKHULL_IMPLEMENTATION

#include <math.h>   // sqrt & fabs
#include <stdio.h>  // FILE
#include <string.h> // memcpy

// Quickhull helpers, define your own if needed
//#ifndef QUICKHULL_HELPERS
#include <stdlib.h> // malloc, free, realloc
//#define QUICKHULL_HELPERS 1
#define QH_MALLOC(T, N) ((T*) malloc(N * sizeof(T)))
#define QH_REALLOC(T, P, N) ((T*)realloc(P, sizeof(T) * N))
#define QH_FREE(T) free(T)
#define QH_SWAP(T, A, B) { T tmp = B; B = A; A = tmp; }
//#ifdef QUICKHULL_DEBUG
//#define QH_ASSERT(STMT) if (!(STMT)) { *(int *)0 = 0; }
//#define QH_LOG(FMT, ...) printf(FMT, ## __VA_ARGS__)
//#else
#define QH_ASSERT(STMT)
#define QH_LOG(FMT, ...)
//#endif // QUICKHULL_DEBUG
//#endif // QUICKHULL_HELPERS

//#ifndef QH_FLT_MAX
#define QH_FLT_MAX 1e+37F
//#endif

//#ifndef QH_FLT_EPS
#define QH_FLT_EPS 1E-5F
//#endif

//#ifndef QH_VERTEX_SET_SIZE
#define QH_VERTEX_SET_SIZE 128
//#endif


typedef long qh_index_t;

typedef struct qh_half_edge {
	qh_index_t opposite_he;     // index of the opposite half edge
	qh_index_t next_he;         // index of the next half edge
	qh_index_t previous_he;     // index of the previous half edge
	qh_index_t he;              // index of the current half edge
	qh_index_t to_vertex;       // index of the next vertex
	qh_index_t adjacent_face;   // index of the ajacent face
} qh_half_edge_t;

typedef struct qh_index_set {
	qh_index_t* indices;
	unsigned int size;
	unsigned int capacity;
} qh_index_set_t;

typedef struct qh_face {
	qh_index_set_t iset;
	qh_vec3_t normal;
	qh_vertex_t centroid;
	qh_index_t edges[3];
	qh_index_t face;
	float sdist;
	int visitededges;
	/**
	Curent face's index in ValidFaces array
	**/
	qh_index_t validindex;
	bool is_coplanar;
} qh_face_t;

typedef struct qh_index_stack {
	qh_index_t* begin;
	unsigned int size;
} qh_index_stack_t;

typedef struct qh_context {
	qh_face_t* faces;
	qh_half_edge_t* edges;
	qh_vertex_t* vertices;
	qh_vertex_t centroid;
	qh_index_stack_t facestack;
	qh_index_stack_t scratch;
	qh_index_stack_t horizonedges;
	qh_index_stack_t newhorizonedges;
	char* valid;
	unsigned int nedges;
	unsigned int nvertices;
	unsigned int nfaces;

#ifdef QUICKHULL_DEBUG
	unsigned int maxfaces;
	unsigned int maxedges;
#endif
} qh_context_t;


void Check_QH_Face(qh_context_t* context, const qh_face_t& Face);
FPlane QH_Face_To_FPlane(const qh_face_t& face);
FPlane QH_Face_To_FPlane(const qh_vec3_t* normal, const float sdist);




//#endif // QUICKHULL_IMPLEMENTATION


bool SliceConvexHull(const TArray<FVector>& OriginVertices, const TArray<Face>& OriginFaces, const FPlane& SlicePlane,
	TArray<FVector>& FirstHalfVertices, TArray<Face>& FirstHalfFaces,
	TArray<FVector>& SecondHalfVertices, TArray<Face>& SecondHalfFaces);


bool SliceConvexHull(const TArray<FVector>& OriginVertices, const TArray<int32>& OriginIndices, const FPlane& SlicePlane,
	TArray<FVector>& FirstHalfVertices, TArray<FVector>& OtherHalfVertices,
	TArray<int32>& FirstHalfIndices, TArray<int32>& OtherHalfIndices,
	bool bCreateOtherHalf);