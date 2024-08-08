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

            PublicDependencyModuleNames.AddRange(
                new string[]
                {
                    // in UE 4.18 needed HeadMountedDisplay
                    "Core", "CoreUObject", "Engine", "InputCore", "Mach1DecodeAPI", "HeadMountedDisplay", "XRBase"
                }
            );
        }
    }
}
