// Copyright Epic Games, Inc. All Rights Reserved.

#include "StaticMeshExtensionCommands.h"

#define LOCTEXT_NAMESPACE "FStaticMeshExtensionModule"

void FStaticMeshExtensionCommands::RegisterCommands()
{
	UI_COMMAND(BakeVertColors, "BakeVertColors", "Execute BakeVertColors action", EUserInterfaceActionType::Button, FInputChord());
}

#undef LOCTEXT_NAMESPACE
