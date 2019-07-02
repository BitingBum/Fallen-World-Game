// Copyright 2016-2018 Chris Conway (Koderz). All Rights Reserved.

#include "IsleActor.h"
#include "IsleComponent.h"

AIsleActor::AIsleActor(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	IsleComponent = CreateDefaultSubobject<UIsleComponent>(TEXT("Isle Component"));

	RootComponent = IsleComponent;
}

// Called when the game starts or when spawned
void AIsleActor::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AIsleActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

