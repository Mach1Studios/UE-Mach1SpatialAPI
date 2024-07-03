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
            bPrecompile = true;
            //bUsePrecompiled = true;

            if (!bUsePrecompiled) {
                PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

                PublicIncludePaths.AddRange(
                    new string[] {
                        "Developer/TargetPlatform/Public"
                        // ... add public include paths required here ...
                    }
                    );

                PrivateIncludePaths.AddRange(
                    new string[] {
                       "Mach1DecodePlugin/Private"
                        // ... add other private include paths required here ...
                    }
                    );

                 PublicDependencyModuleNames.AddRange(
                    new string[]
                    {
                        // in UE 4.18 needed HeadMountedDisplay
                        "Core", "CoreUObject", "Engine", "InputCore", "HeadMountedDisplay", "XRBase"
                        // ... add other public dependencies that you statically link with here ...
                    }
                    );

                PrivateDependencyModuleNames.AddRange(
                    new string[]
                    {
                        // ... add private dependencies that you statically link with here ...
                    }
                    );

                DynamicallyLoadedModuleNames.AddRange(
                    new string[]
                    {
                        // ... add any modules that your module loads dynamically here ...
                    }
                    );

                // add Mach1 library
                string Mach1BaseDirectory = Path.GetFullPath(Path.Combine(ModuleDirectory, "..", "..", "ThirdParty", "Mach1"));
                string Mach1BinDirectory = Path.Combine(Mach1BaseDirectory, "bin", Target.Platform.ToString());
                PublicIncludePaths.Add(Path.Combine(Mach1BaseDirectory, "include"));

                PublicDefinitions.Add("M1_STATIC");
                if (Target.Platform == UnrealTargetPlatform.Android)
                {
                    //Mach1BinDirectory = Path.Combine(Mach1BinDirectory, Target.Architecture);
                    var archs =  new string[] { "armeabi-v7a", "x86" };
                    foreach(var arch in archs)
                    {
                        PublicAdditionalLibraries.Add(Path.Combine(Mach1BinDirectory, arch, "libMach1DecodeCAPI.a"));
                        PublicAdditionalLibraries.Add(Path.Combine(Mach1BinDirectory, arch, "libMach1DecodePositionalCAPI.a"));
                    }
                }
                else if (Target.Platform == UnrealTargetPlatform.IOS)
                {
                    PublicAdditionalLibraries.Add(Path.Combine(Mach1BinDirectory, "libMach1DecodeCAPI.a"));
                    PublicAdditionalLibraries.Add(Path.Combine(Mach1BinDirectory, "libMach1DecodePositionalCAPI.a"));
                }
                else if (Target.Platform == UnrealTargetPlatform.Mac)
                {
                    PublicAdditionalLibraries.Add(Path.Combine(Mach1BinDirectory, "libMach1DecodeCAPI.a"));
                    PublicAdditionalLibraries.Add(Path.Combine(Mach1BinDirectory, "libMach1DecodePositionalCAPI.a"));
                }
                else if (Target.Platform == UnrealTargetPlatform.Win64)
                { 
                    PublicAdditionalLibraries.Add(Path.Combine(Mach1BinDirectory, "Mach1DecodeCAPI.lib"));
                    PublicAdditionalLibraries.Add(Path.Combine(Mach1BinDirectory, "Mach1DecodePositionalCAPI.lib"));
                }
            }
        }
    }
}
