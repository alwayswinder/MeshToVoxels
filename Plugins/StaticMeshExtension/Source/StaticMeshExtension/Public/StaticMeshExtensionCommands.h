// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Framework/Commands/Commands.h"
#include "StaticMeshExtensionStyle.h"

class FStaticMeshExtensionCommands : public TCommands<FStaticMeshExtensionCommands>
{
public:

	FStaticMeshExtensionCommands()
		: TCommands<FStaticMeshExtensionCommands>(TEXT("StaticMeshExtension"), NSLOCTEXT("Contexts", "StaticMeshExtension", "StaticMeshExtension Plugin"), NAME_None, FStaticMeshExtensionStyle::GetStyleSetName())
	{
	}

	// TCommands<> interface
	virtual void RegisterCommands() override;

public:
	TSharedPtr< FUICommandInfo > BakeVertColors;
};
