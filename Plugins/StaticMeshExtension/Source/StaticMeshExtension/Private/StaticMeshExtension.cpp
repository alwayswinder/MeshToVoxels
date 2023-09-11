// Copyright Epic Games, Inc. All Rights Reserved.

#include "StaticMeshExtension.h"
#include "StaticMeshExtensionStyle.h"
#include "StaticMeshExtensionCommands.h"
#include "Misc/MessageDialog.h"
#include "ToolMenus.h"
#include "EditorUtilityLibrary.h"
#include "Toolkits/AssetEditorToolkitMenuContext.h"
#include "Toolkits/AssetEditorToolkit.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "StaticMeshExtensionLibrary.h"



IMPLEMENT_MODULE(FStaticMeshExtensionModule, StaticMeshExtension)

static const FName StaticMeshExtensionTabName("StaticMeshExtension");

#define LOCTEXT_NAMESPACE "FStaticMeshExtensionModule"

void FStaticMeshExtensionModule::StartupModule()
{
	// This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module
	FStaticMeshExtensionStyle::Initialize();
	FStaticMeshExtensionStyle::ReloadTextures();

	FStaticMeshExtensionCommands::Register();
	
	TArray<FAssetEditorExtender>& AssetEditorMenuExtenderDelegates = FAssetEditorToolkit::GetSharedMenuExtensibilityManager()->GetExtenderDelegates();
	AssetEditorMenuExtenderDelegates.Add(FAssetEditorExtender::CreateRaw(this, &FStaticMeshExtensionModule::OnExtendAssetEditor));
	AssetEditorExtenderDelegateHandle = AssetEditorMenuExtenderDelegates.Last().GetHandle();

	UToolMenus::RegisterStartupCallback(FSimpleMulticastDelegate::FDelegate::CreateRaw(this, &FStaticMeshExtensionModule::RegisterMenus));
	
	FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>(TEXT("AssetRegistry"));
	AssetRegistry = &AssetRegistryModule.Get();
}

void FStaticMeshExtensionModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.

	UToolMenus::UnRegisterStartupCallback(this);

	UToolMenus::UnregisterOwner(this);

	FStaticMeshExtensionStyle::Shutdown();

	FStaticMeshExtensionCommands::Unregister();

	TArray<FAssetEditorExtender>& AssetEditorMenuExtenderDelegates = FAssetEditorToolkit::GetSharedMenuExtensibilityManager()->GetExtenderDelegates();
	AssetEditorMenuExtenderDelegates.RemoveAll([this](const FAssetEditorExtender& Delegate) { return Delegate.GetHandle() == AssetEditorExtenderDelegateHandle; });

}

void FStaticMeshExtensionModule::PluginButtonClicked(TArray<FString> SelectedPackages)
{
	for (FString package : SelectedPackages)
	{
		FAssetData AssetToLoad = AssetRegistry->GetAssetByObjectPath(package);
		UObject * obj = AssetToLoad.GetAsset();
		if(obj != nullptr)
		{
			UStaticMesh* StaticMesh = Cast<UStaticMesh>(obj);
            // Bake out materials for static mesh asset
            StaticMesh->Modify();
            //BakeMaterialToVertex(StaticMesh);
			UStaticMeshExtensionLibrary::BakeMaterialToVertexForStaticMesh(StaticMesh);
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("Asset %s not found"), *package);
		}
	}
}

void FStaticMeshExtensionModule::RegisterMenus()
{
	// Owner will be used for cleanup in call to UToolMenus::UnregisterOwner
	FToolMenuOwnerScoped OwnerScoped(this);

	{
		UToolMenu* Menu = UToolMenus::Get()->ExtendMenu("AssetEditor.StaticMeshEditor.MainMenu.Asset");
		{
			FToolMenuSection& Section = Menu->FindOrAddSection("AssetEditorActions");
			Section.AddMenuEntryWithCommandList(FStaticMeshExtensionCommands::Get().BakeVertColors, BakeVertColorsCommands);
		}
	}
}

TSharedRef<FExtender> FStaticMeshExtensionModule::OnExtendAssetEditor(const TSharedRef<FUICommandList> CommandList,
	const TArray<UObject*> ContextSensitiveObjects)
{
	TArray<FString> PackageNames;
	for (UObject* EditedAsset : ContextSensitiveObjects)
	{
		if (IsValid(EditedAsset) && EditedAsset->IsAsset())
		{
			//PackageNames.AddUnique(EditedAsset->GetOutermost()->GetFName());
			PackageNames.AddUnique(EditedAsset->GetPathName());
		}
	}

	TSharedRef<FExtender> Extender(new FExtender());

	if (PackageNames.Num() > 0)
	{
		CommandList->MapAction(
			FStaticMeshExtensionCommands::Get().BakeVertColors,
			FExecuteAction::CreateRaw(this, &FStaticMeshExtensionModule::PluginButtonClicked, PackageNames),
			FCanExecuteAction());
	}

	return Extender;
}
#undef LOCTEXT_NAMESPACE
	
