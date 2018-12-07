// Fill out your copyright notice in the Description page of Project Settings.
// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/StaticMesh.h"//UStaticMesh Class
#include "Components/StaticMeshComponent.h"
#include "GameFramework/Actor.h"
#include "Runtime/CoreUObject/Public/UObject/UObjectGlobals.h"

#include "CustomDestructibleActor.generated.h"

UCLASS()
class MYPROJECT_API ACustomDestructibleActor : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	ACustomDestructibleActor();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Base Static Mesh Component")
		UStaticMeshComponent* BaseStaticMeshComponent;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sub Static Mesh Components")
		TArray <UStaticMeshComponent*> SubStaticMeshComponents;

	/*TMap<int32, FKConvexElem> ConvexCollisionShapes;
	TMap<int32, FRuntimeMeshCollisionConvexMesh> ConvexCollisionSections;*/


	UFUNCTION(BlueprintCallable)
		void TestHalfSlice();

	void AddSubStaticMeshComponent(UStaticMeshComponent* SubStaticMeshComponent);


	template<typename T> int GetIndex(const TArray<T>& Array, T& Elem);

	void AddVertexIfNotPresent(TArray<FVector> &Vertices, const FVector& NewVertex);

	void RemoveDuplicateVerts(TArray<FVector>& InVerts);
	
	float DistanceToLine(const FVector& LineStart, const FVector& LineEnd, const FVector& Point);

	bool EnsureHullIsValid(TArray<FVector>& InVerts);

	bool IsNewCombination(TArray<int> Combination, TArray<TArray<int>>& CheckedCombinations);

	bool PointsOfPlane(TArray<TArray<FVector>>PlanesAndTheirPoints, FVector A, FVector B, FVector C);

	bool PointsAreOnOneLine(FVector A, FVector B, FVector C);

	bool PlaneIsValid(FVector A, FVector B, FVector C, TArray<FVector> HullVertices, TArray<TArray<FVector>>& PlanesAndTheirPoints, TArray<FPlane>& Planes, bool& AllPointsAreOnPlane);

	float DegreesAngleBetweenVerts(const FVector& A, const FVector& B);

	float CosAngleBetweenVerts(const FVector& A, const FVector& B);

	float SinAngleBetweenVerts(const FVector& A, const FVector& B);

	//float MixedProduct3(const FVector& A, const FVector& B, const FVector& C);// == FVector Triple(A,B,C)

	void FindEdgesOfLine(const FVector& A, const FVector& B, const TArray<FVector>& LinePoints, TArray<FVector>& NewVertices, TArray<FVector>& ExcessPoints, int& NewAIndex, int& NewBIndex);

	bool IsLastEdge(TArray<FVector>& NewVertices, TArray<FVector>& ExcessVertices, const TArray<FVector>& PlaneVertices, const FPlane& Plane);

	void FindEdges(TArray<FVector>& NewVertices, TArray<FVector>& ExcessVertices, const TArray<FVector>& PlaneVertices, const FPlane& Plane);

	void SortPointsOfPlane(TArray<FVector>& PlaneVertices, const FPlane& Plane);

	bool GetPlanesFromHull(TArray<FVector> HullVertices, TArray<FPlane>& Planes, TArray<TArray<FVector>>& PlanesAndTheirPoints);

	void FindEdgeIndices(int& I, int& J, const TArray<FVector>& Points);

	void FindSlicePoints(const TArray<FVector>&FrontPoints, const TArray<FVector>&BehindPoints, TArray<FVector>&PointsOfPlane, const FPlane& SlicePlane);

	void AddUniqueVector(const TArray<FVector>& InVertices, TArray<FVector>& OutVertices);

	bool SliceHull(const TArray<TArray<FVector>>& PlanesAndTheirPoints, const FPlane& SlicePlane, TArray<FVector>& OneHalfVertices, TArray<FVector>& OtherHalfVertices, bool CreateOtherHalf);

	/*int32 AddConvexCollisionSection(FKConvexElem ConvexShape);
	
	void CopyCollisionFromStaticMesh(UStaticMesh* StaticMesh, URuntimeMesh* RuntimeMesh);

	int32 AddConvexCollisionSection(FKConvexElem ConvexShape);
	

	void SetCollisionConvexMeshes(const TArray<FKConvexElem>& ConvexMeshes);


	int32 CompareBoxPlane(const FBox& InBox, const FPlane& InPlane);

	void SliceConvexElem(FKConvexElem* ConvexSection, const FPlane& SlicePlane, TArray<FKConvexElem>& OutConvexShapes);

	void SliceRuntimeMeshConvexCollision(URuntimeMesh* InRuntimeMesh, URuntimeMesh* OutOtherHalf, FVector PlanePosition, FVector PlaneNormal);

	FRuntimeMeshCollisionConvexMesh* GetConvexCollisionSection(int32 Index);*/
};
