// Copyright 2016-2018 Chris Conway (Koderz). All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "FractureChunkActor.generated.h"

class UFractureChunkComponent;
//class UPhysicsConstraintComponent;

class AIsleActor;

UCLASS()
class RUNTIMEMESHCOMPONENT_API AFractureChunkActor : public AActor
{
	GENERATED_UCLASS_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fracture Chunk Component", DisplayName = "Fracture Chunk Component")
	UFractureChunkComponent* FractureChunkComponent;
	
	//UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics Constraint Component", DisplayName = "Physics Constraint Component")
	//UPhysicsConstraintComponent* ConstraintComponent = nullptr;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	//void SetupConstraintComponent(AIsleActor* RootIsleActor);
	
};
