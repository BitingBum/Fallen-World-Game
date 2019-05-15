// Copyright 2016-2018 Chris Conway (Koderz). All Rights Reserved.

#include "FractureChunkComponent.h"

UFractureChunkComponent::UFractureChunkComponent()
{
}

UFractureChunkComponent::UFractureChunkComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}


///* Serializes this component */
//void UFractureChunkComponent::Serialize(FArchive& Ar)
//{
//	//I made URuntimeMeshComponent::Serialize() public
//	Super::Serialize(Ar);
//
//	Ar << ChunkIndex;
//	Ar << ParentIndex;
//	Ar << DepthLevel;
//	Ar << IsFractured;
//	Ar << ChildrenIndices;
//	
//}


