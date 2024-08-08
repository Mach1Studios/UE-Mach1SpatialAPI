// Copyright 1998-2016 Epic Games, Inc. All Rights Reserved.
using System.IO;
using System.Diagnostics;

namespace UnrealBuildTool.Rules
{
    public class Mach1DecodeAPI : ModuleRules
    {
        public Mach1DecodeAPI(ReadOnlyTargetRules Target) : base(Target)
        {
            string SDKDIR = Path.GetFullPath(Path.Combine(ModuleDirectory, "../ThirdParty"));

            // Replace with PCHUsageMode.UseExplicitOrSharedPCHs when this plugin can compile with cpp20
            PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

            PublicSystemIncludePaths.Add(Path.Combine(SDKDIR, "glm"));

            PublicDependencyModuleNames.AddRange(
                new string[]
                {
                    "Core", "Engine"
                }
            );
        }
    }
}
