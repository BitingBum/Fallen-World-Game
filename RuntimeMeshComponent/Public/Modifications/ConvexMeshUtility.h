// Copyright 2016-2018 Chris Conway (Koderz). All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

/**
 * 
 */
void AddVertexIfNotPresent(TArray<FVector> &Vertices, const FVector& NewVertex);

void RemoveDuplicateVerts(TArray<FVector>& InVerts);

float DistanceToLine(const FVector& LineStart, const FVector& LineEnd, const FVector& Point);

bool EnsureHullIsValid(TArray<FVector>& InVerts);

bool IsNewCombination(TArray<int> Combination, TArray<TArray<int>>& CheckedCombinations);

bool PointsOfPlane(TArray<TArray<FVector>>PlanesAndTheirPoints, FVector A, FVector B, FVector C);

bool PointsOnOneLine(FVector A, FVector B, FVector C);

bool PlaneIsValid(FVector A, FVector B, FVector C, TArray<FVector> HullVertices, TArray<TArray<FVector>>& PlanesAndTheirPoints, TArray<FPlane>& Planes, bool& AllPointsAreOnPlane);

bool GetPlanesFromHull(TArray<FVector> HullVertices, TArray<FPlane>& Planes, TArray<TArray<FVector>>& PlanesAndTheirPoints);

void FindSlicePoints(const TArray<FVector>&FrontPoints, const TArray<FVector>&BehindPoints, TArray<FVector>&PointsOfPlane, const FPlane& SlicePlane);

bool SliceHull(const TArray<TArray<FVector>>& PlanesAndTheirPoints, const FPlane& SlicePlane, TArray<FVector>& OneHalfVertices, TArray<FVector>& OtherHalfVertices, bool CreateOtherHalf);