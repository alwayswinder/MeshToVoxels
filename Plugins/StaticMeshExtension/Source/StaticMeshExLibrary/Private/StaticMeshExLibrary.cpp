// Copyright Epic Games, Inc. All Rights Reserved.


#include "StaticMeshExLibrary.h"

IMPLEMENT_MODULE(FStaticMeshExLibraryModule, StaticMeshExLibrary)

#define LOCTEXT_NAMESPACE "FStaticMeshExLibraryModule"

void FStaticMeshExLibraryModule::StartupModule()
{
	// This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module

} 

void FStaticMeshExLibraryModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.

}

#undef LOCTEXT_NAMESPACE
	
