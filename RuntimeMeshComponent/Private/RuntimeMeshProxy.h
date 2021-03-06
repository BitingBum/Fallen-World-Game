// Copyright 2016-2018 Chris Conway (Koderz). All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "RuntimeMeshSectionProxy.h"
#include "RuntimeMeshUpdateCommands.h"


template<typename Type>
struct FRuntimeMeshRenderThreadDeleter
{
	void operator()(Type* Object) const
	{
		// This is a custom deleter to make sure the runtime mesh proxy is only ever deleted on the rendering thread.
		if (IsInRenderingThread())
		{
			delete Object;
		}
		else
		{
			ENQUEUE_UNIQUE_RENDER_COMMAND_ONEPARAMETER(
				FRuntimeMeshProxyDeleterCommand,
				void*, Object, Object,
				{
					delete static_cast<Type*>(Object);
				}
			);
		}
	}
};

/**
 *
 */
class FRuntimeMeshProxy
{
	ERHIFeatureLevel::Type FeatureLevel;

	TMap<int32, FRuntimeMeshSectionProxyPtr> Sections;

public:
	FRuntimeMeshProxy(ERHIFeatureLevel::Type InFeatureLevel);
	~FRuntimeMeshProxy();

	ERHIFeatureLevel::Type GetFeatureLevel() const { return FeatureLevel; }


	void CreateSection_GameThread(int32 SectionId, const FRuntimeMeshSectionCreationParamsPtr& SectionData);
	void CreateSection_RenderThread(int32 SectionId, const FRuntimeMeshSectionCreationParamsPtr& SectionData);
	void UpdateSection_GameThread(int32 SectionId, const FRuntimeMeshSectionUpdateParamsPtr& SectionData);
	void UpdateSection_RenderThread(int32 SectionId, const FRuntimeMeshSectionUpdateParamsPtr& SectionData);

	//////////////////////////////////////////////////////////////////////////////////////////My Modifications///////////////////////////////////////////////////////////////////////////////////////
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void UpdateAllSectionsProperties_GameThread(const TArray<FRuntimeMeshSectionPropertyUpdateParamsPtr>& SectionsData);

	void UpdateAllSectionsProperties_RenderThread(const TArray<FRuntimeMeshSectionPropertyUpdateParamsPtr>& SectionsData);

	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////////////////////My Modifications///////////////////////////////////////////////////////////////////////////////////////

	void UpdateSectionProperties_GameThread(int32 SectionId, const FRuntimeMeshSectionPropertyUpdateParamsPtr& SectionData);
	void UpdateSectionProperties_RenderThread(int32 SectionId, const FRuntimeMeshSectionPropertyUpdateParamsPtr& SectionData);
	void DeleteSection_GameThread(int32 SectionId);
	void DeleteSection_RenderThread(int32 SectionId);



	TMap<int32, FRuntimeMeshSectionProxyPtr>& GetSections() { return Sections; }

	void CalculateViewRelevance(bool& bHasStaticSections, bool& bHasDynamicSections, bool& bHasShadowableSections)
	{
		check(IsInRenderingThread());
		bHasStaticSections = false;
		bHasDynamicSections = false;
		bHasShadowableSections = false;
		for (const auto& SectionEntry : Sections)
		{
			if (SectionEntry.Value.IsValid())
			{
				bool bWantsStaticPath = SectionEntry.Value->WantsToRenderInStaticPath();
				bHasStaticSections |= bWantsStaticPath;
				bHasDynamicSections |= !bWantsStaticPath;
				bHasShadowableSections = SectionEntry.Value->CastsShadow();
			}
		}
	}
private:
	void UpdateCachedValues();

};

