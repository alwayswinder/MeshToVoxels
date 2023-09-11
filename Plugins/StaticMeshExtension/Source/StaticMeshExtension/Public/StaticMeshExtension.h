// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"

class FToolBarBuilder;
class FMenuBuilder;
class FExtender;
class FUICommandList;
class IAssetRegistry;
class IMaterialBakingAdapter;


class FStaticMeshExtensionModule : public IModuleInterface
{
public:

	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
	
	/** This function will be bound to Command. */
	void PluginButtonClicked(TArray<FString> SelectedPackages);
	
private:

	void RegisterMenus();
	
	TSharedRef<FExtender> OnExtendAssetEditor(const TSharedRef<FUICommandList> CommandList, const TArray<UObject*> ContextSensitiveObjects);

private:
	TSharedPtr<class FUICommandList> BakeVertColorsCommands;

	FDelegateHandle AssetEditorExtenderDelegateHandle;

	IAssetRegistry* AssetRegistry;
};
