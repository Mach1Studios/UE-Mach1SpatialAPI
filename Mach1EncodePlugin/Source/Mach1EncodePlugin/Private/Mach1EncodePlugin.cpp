// Copyright 1998-2016 Epic Games, Inc. All Rights Reserved.

#include "Mach1EncodePluginPrivatePCH.h"

class FMach1EncodePlugin : public IMach1EncodePlugin
{
	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
};

IMPLEMENT_MODULE( FMach1EncodePlugin, Mach1EncodePlugin )

void FMach1EncodePlugin::StartupModule()
{
	// This code will execute after your module is loaded into memory (but after global variables are initialized, of course.)
}

void FMach1EncodePlugin::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.
} 