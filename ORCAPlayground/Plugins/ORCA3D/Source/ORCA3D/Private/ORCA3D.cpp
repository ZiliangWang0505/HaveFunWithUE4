// Copyright Epic Games, Inc. All Rights Reserved.

#include "ORCA3D.h"

#define LOCTEXT_NAMESPACE "FORCA3DModule"

void FORCA3DModule::StartupModule()
{
	// This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module
}

void FORCA3DModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FORCA3DModule, ORCA3D)