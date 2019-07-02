// Copyright 2016-2018 Chris Conway (Koderz). All Rights Reserved.


#include "FractureChunkActor.h"
#include "FractureChunkComponent.h"
//#include "IsleActor.h"
//#include "PhysicsEngine/PhysicsConstraintComponent.h"

AFractureChunkActor::AFractureChunkActor(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;		

	FractureChunkComponent = CreateDefaultSubobject<UFractureChunkComponent>(TEXT("Fracture Chunk Component"));
	//ConstraintComponent = CreateDefaultSubobject<UPhysicsConstraintComponent>(TEXT("Constraint Component"));
	//SetupConstraintComponent();

	RootComponent = FractureChunkComponent;
	
	
	//FractureChunkComponent->SetupAttachment(RootComponent);
}

// Called when the game starts or when spawned
void AFractureChunkActor::BeginPlay()
{
	Super::BeginPlay();

	/*if (GetAttachParentActor())
	{
		UPrimitiveComponent* BodyRoot = Cast<UPrimitiveComponent>(GetAttachParentActor()->GetRootComponent());
		if (BodyRoot)
			ConstraintComponent->SetConstrainedComponents(BodyRoot, NAME_None, FractureChunkComponent, NAME_None);
	}*/
	
}

// Called every frame
void AFractureChunkActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

//void AFractureChunkActor::SetupConstraintComponent()
//{
//	//set up the constraint instance with all the desired values
//	FConstraintInstance ConstraintInstance;
//
//	ConstraintInstance.ProfileInstance.LinearLimit.bSoftConstraint = false;
//	ConstraintInstance.SetDisableCollision(true);
//
//
//	ConstraintInstance.SetLinearXMotion(ELinearConstraintMotion::LCM_Locked);
//	ConstraintInstance.SetLinearYMotion(ELinearConstraintMotion::LCM_Locked);
//	ConstraintInstance.SetLinearZMotion(ELinearConstraintMotion::LCM_Locked);
//
//	ConstraintInstance.SetLinearLimitSize(0);
//	ConstraintInstance.SetLinearXLimit(ELinearConstraintMotion::LCM_Locked, 0);
//	ConstraintInstance.SetLinearYLimit(ELinearConstraintMotion::LCM_Locked, 0);
//	ConstraintInstance.SetLinearZLimit(ELinearConstraintMotion::LCM_Locked, 0);
//	
//	ConstraintInstance.ProfileInstance.LinearLimit.bSoftConstraint = false;
//	ConstraintInstance.ProfileInstance.LinearLimit.Stiffness = 0;
//	ConstraintInstance.ProfileInstance.LinearLimit.Damping = 0;
//
//
//
//	ConstraintInstance.AngularRotationOffset = FRotator::ZeroRotator;
//
//	ConstraintInstance.SetAngularSwing1Motion(EAngularConstraintMotion::ACM_Locked);
//	ConstraintInstance.SetAngularSwing2Motion(EAngularConstraintMotion::ACM_Locked);
//	ConstraintInstance.SetAngularTwistMotion(EAngularConstraintMotion::ACM_Locked);
//
//	ConstraintInstance.SetAngularDOFLimitScale(0, 0, 0);
//	ConstraintInstance.SetAngularSwing1Limit(EAngularConstraintMotion::ACM_Locked, 0);
//	ConstraintInstance.SetAngularSwing2Limit(EAngularConstraintMotion::ACM_Locked, 0);
//	ConstraintInstance.SetAngularTwistLimit(EAngularConstraintMotion::ACM_Locked, 0);
//
//	ConstraintInstance.ProfileInstance.ConeLimit.bSoftConstraint = false;
//
//	ConstraintInstance.ProfileInstance.TwistLimit.bSoftConstraint = false;	
//
//	ConstraintInstance.ProfileInstance.ConeLimit.Stiffness = 0;
//	ConstraintInstance.ProfileInstance.ConeLimit.Damping = 0;
//	ConstraintInstance.ProfileInstance.TwistLimit.Stiffness = 0;
//	ConstraintInstance.ProfileInstance.TwistLimit.Damping = 0;
//
//	
//	ConstraintComponent->ConstraintInstance = ConstraintInstance;
//	
//	/*ConstraintComponent->ConstraintActor1 = this;
//
//	ConstraintComponent->ConstraintActor2 = RootIsleActor;	
//	
//	ConstraintComponent->SetConstrainedComponents(FractureChunkComponent, NAME_None, Cast<UPrimitiveComponent>(RootIsleActor->IsleComponent), NAME_None);*/
//}