// Copyright 2016-2018 Chris Conway (Koderz). All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "IsleActor.generated.h"

class UIsleComponent;

UCLASS()
class RUNTIMEMESHCOMPONENT_API AIsleActor : public AActor
{
	GENERATED_UCLASS_BODY()
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Isle Component", DisplayName = "Isle Component")
	UIsleComponent* IsleComponent;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	
	
};
