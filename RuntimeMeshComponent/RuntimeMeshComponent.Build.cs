// Copyright 2016-2018 Chris Conway (Koderz). All Rights Reserved.

using UnrealBuildTool;

public class RuntimeMeshComponent : ModuleRules
{
    public RuntimeMeshComponent(ReadOnlyTargetRules rules) : base(rules)
    {
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

        PublicIncludePaths.AddRange(
            new string[] {
                "RuntimeMeshComponent/Public",
				// ... add public include paths required here ...
                //"RuntimeMeshComponent/Public/RuntimeDestructibleActor"
            }
            );


        PrivateIncludePaths.AddRange(
            new string[] {
                "RuntimeMeshComponent/Private",
				// ... add other private include paths required here ...
                //"RuntimeMeshComponent/Private/RuntimeDestructibleActor"
            }
            );


        PublicDependencyModuleNames.AddRange(
            new string[]
            {
                "Core",
                // ... add other public dependencies that you statically link with here ...    
                //"PhysX", "APEX","ApexDestruction"
            }
            );

        string ApexVersion = "APEX_1.4";
        string APEXDir = rules.UEThirdPartySourceDirectory + "PhysX3/" + ApexVersion + "/";
        string APEXLibDir = rules.UEThirdPartySourceDirectory + "PhysX3/Lib";

        string PxSharedVersion = "PxShared";
        string PxSharedDir = rules.UEThirdPartySourceDirectory + "PhysX3/" + PxSharedVersion + "/";
        string PxSharedIncludeDir = PxSharedDir + "include/";

        //string PxSourceVersion = "Source/PhysX";
        //string PxSourceDir = rules.UEThirdPartySourceDirectory + "PhysX3/" + PxSourceVersion + "/";

        PublicSystemIncludePaths.AddRange(
            new string[] {               
                PxSharedDir + "src/foundation/include", //PsArray.h
                APEXDir + "nvparameterized/include",//"NvParametersTypes.h"
                //APEXDir + "include/destructible",//DestructibleActorJoint.h
                APEXDir + "module/destructible/include/",//DestructibleScene.h, DestructibleActorImpl.h
                //APEXDir + "module/destructible/src/",// DestructibleActorImpl.cpp
                //PxSourceDir + "src",
                //PxSourceDir + "src/buffering"
            }
            );

        //To fix issues with Atomic.h
        PublicDependencyModuleNames.AddRange(new string[] { "ApplicationCore" });
        PrivateDependencyModuleNames.AddRange(new string[] { "PhysX", "APEX"  });//PhysX   
        PrivateDependencyModuleNames.AddRange(new string[] { "RMC_ApexDestruction" });

        if (rules.Type==TargetRules.TargetType.Editor)
            PublicDependencyModuleNames.AddRange(new string[] { "MeshBuilder" });

        PrivateDependencyModuleNames.AddRange(
            new string[]
            {
                "CoreUObject",
                "Engine",
				// ... add private dependencies that you statically link with here ...	
                "RenderCore",
                "ShaderCore",
                "RHI",
                //"MeshBuilder",//BuildSkeletalAdjacencyIndexBuffer()
                
            }
            );

       

        DynamicallyLoadedModuleNames.AddRange(
            new string[]
            {
				// ... add any modules that your module loads dynamically here ...
			}
            );
    }
}
