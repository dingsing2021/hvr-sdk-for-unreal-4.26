// Copyright 1998-2017 Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;
using System.IO;
public class HuaweiVRController : ModuleRules
{
	public HuaweiVRController(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;
		
		//PublicIncludePaths.AddRange(
		//	new string[] {
		//		"HuaweiVRController/Public"
		//		// ... add public include paths required here ...
		//	}
		//	);
				
		
		PrivateIncludePaths.AddRange(
			new string[] {
				"HuaweiVRController/Private",
                Path.Combine(Path.GetFullPath(Target.RelativeEnginePath), "Source/Runtime/InputDevice/Public"),
                Path.Combine(Path.GetFullPath(Target.RelativeEnginePath), "Source/Runtime/Launch/Public"),
				// ... add other private include paths required here ...
			}
			);
			
		
		//PublicDependencyModuleNames.AddRange(
		//	new string[]
		//	{
		//		"Core",
		//		// ... add other public dependencies that you statically link with here ...
		//	}
		//	);
			
		
		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
                "Core",
                "CoreUObject",
                "ApplicationCore",
                "Engine",
                "InputCore",
                "InputDevice",
                "HeadMountedDisplay",
                "UMG",
                "Slate",
                "SlateCore"
				// ... add private dependencies that you statically link with here ...	
			}
			);

        if (Target.Platform == UnrealTargetPlatform.Android)
        {
            PrivateDependencyModuleNames.Add("Launch");


        }

        if (Target.bBuildEditor == true)
        {
            PrivateDependencyModuleNames.Add("UnrealEd");
            if (Target.Platform == UnrealTargetPlatform.Win32 || Target.Platform == UnrealTargetPlatform.Win64)
            {
                PublicIncludePaths.Add("Developer/Android/AndroidDeviceDetection/Public");
                PublicIncludePaths.Add("Developer/Android/AndroidDeviceDetection/Public/Interfaces");
            }
        }

        if (Target.Platform == UnrealTargetPlatform.Android)
        {

            PublicLibraryPaths.Add(Path.Combine(ModuleDirectory, "libs", "android_arm"));

            //PublicAdditionalLibraries.Add("vrunityplugin");
            PublicAdditionalLibraries.Add("PluginProxy");
        }
        DynamicallyLoadedModuleNames.AddRange(
			new string[]
			{
				// ... add any modules that your module loads dynamically here ...
			}
			);
	}
}
