// Copyright Epic Games, Inc. All Rights Reserved.

#include "StaticMeshExtensionStyle.h"
#include "StaticMeshExtension.h"
#include "Framework/Application/SlateApplication.h"
#include "Styling/SlateStyleRegistry.h"
#include "Slate/SlateGameResources.h"
#include "Interfaces/IPluginManager.h"
#include "Styling/SlateStyleMacros.h"

#define RootToContentDir Style->RootToContentDir

TSharedPtr<FSlateStyleSet> FStaticMeshExtensionStyle::StyleInstance = nullptr;

void FStaticMeshExtensionStyle::Initialize()
{
	if (!StyleInstance.IsValid())
	{
		StyleInstance = Create();
		FSlateStyleRegistry::RegisterSlateStyle(*StyleInstance);
	}
}

void FStaticMeshExtensionStyle::Shutdown()
{
	FSlateStyleRegistry::UnRegisterSlateStyle(*StyleInstance);
	ensure(StyleInstance.IsUnique());
	StyleInstance.Reset();
}

FName FStaticMeshExtensionStyle::GetStyleSetName()
{
	static FName StyleSetName(TEXT("StaticMeshExtensionStyle"));
	return StyleSetName;
}


const FVector2D Icon16x16(16.0f, 16.0f);
const FVector2D Icon20x20(20.0f, 20.0f);

TSharedRef< FSlateStyleSet > FStaticMeshExtensionStyle::Create()
{
	TSharedRef< FSlateStyleSet > Style = MakeShareable(new FSlateStyleSet("StaticMeshExtensionStyle"));
	Style->SetContentRoot(IPluginManager::Get().FindPlugin("StaticMeshExtension")->GetBaseDir() / TEXT("Resources"));

	Style->Set("StaticMeshExtension.PluginAction", new IMAGE_BRUSH_SVG(TEXT("PlaceholderButtonIcon"), Icon20x20));
	return Style;
}

void FStaticMeshExtensionStyle::ReloadTextures()
{
	if (FSlateApplication::IsInitialized())
	{
		FSlateApplication::Get().GetRenderer()->ReloadTextureResources();
	}
}

const ISlateStyle& FStaticMeshExtensionStyle::Get()
{
	return *StyleInstance;
}
