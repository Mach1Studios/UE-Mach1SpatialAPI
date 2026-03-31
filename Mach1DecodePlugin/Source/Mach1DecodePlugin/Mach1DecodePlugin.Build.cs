// Copyright 1998-2016 Epic Games, Inc. All Rights Reserved.
using System.IO;
using System.Diagnostics;

namespace UnrealBuildTool.Rules
{
    public class Mach1DecodePlugin : ModuleRules
    {
        public string GetUProjectPath()
        {
            return Directory.GetParent(ModuleDirectory).Parent.Parent.Parent.FullName;
        } 

        public Mach1DecodePlugin(ReadOnlyTargetRules Target) : base(Target)
        {
            PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;
            
            PublicIncludePaths.Add("Developer/TargetPlatform/Public");

            // Add GLM include paths from ThirdParty
            string ThirdPartyDir = Path.GetFullPath(Path.Combine(ModuleDirectory, "../ThirdParty"));
            PublicSystemIncludePaths.Add(Path.Combine(ThirdPartyDir, "glm"));
            PublicSystemIncludePaths.Add(Path.Combine(ThirdPartyDir, "glm/glm"));

            // Mach1 C API headers use dllimport/dllexport heuristics which don't apply when
            // compiled as part of a UE module. Force static linkage semantics.
            PublicDefinitions.Add("M1_STATIC=1");

            PublicDependencyModuleNames.AddRange(
                new string[]
                {
                    "Core", "CoreUObject", "Engine", "InputCore"
                }
            );

            // UE4.27 does not have the XRBase module; UE5+ does.
            if (Target.Version.MajorVersion >= 5)
            {
                PublicDependencyModuleNames.Add("XRBase");
            }
            else
            {
                PublicDependencyModuleNames.Add("HeadMountedDisplay");
            }
        }
    }
}
