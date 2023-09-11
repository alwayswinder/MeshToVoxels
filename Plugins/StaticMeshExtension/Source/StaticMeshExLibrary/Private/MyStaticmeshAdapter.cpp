// Copyright Epic Games, Inc. All Rights Reserved.

#include "MyStaticMeshAdapter.h"

#include "Engine/StaticMesh.h"
#include "MaterialBakingStructures.h"
#include "MyMeshMergeHelpers.h"
#include "MeshUtilities.h"
#include "Modules/ModuleManager.h"
#include "UObject/Package.h"

FMyStaticMeshAdapter::FMyStaticMeshAdapter(UStaticMesh* InStaticMesh)
	: StaticMesh(InStaticMesh)
{
	checkf(StaticMesh != nullptr, TEXT("Invalid static mesh in adapter"));
	NumLODs = StaticMesh->GetNumLODs();
}

int32 FMyStaticMeshAdapter::GetNumberOfLODs() const
{
	return NumLODs;
}

void FMyStaticMeshAdapter::RetrieveRawMeshData(int32 LODIndex, FMeshDescription& InOutRawMesh, bool bPropogateMeshData) const
{
	FMyMeshMergeHelpers::RetrieveMesh(StaticMesh, LODIndex, InOutRawMesh);
}

void FMyStaticMeshAdapter::RetrieveMeshSections(int32 LODIndex, TArray<FSectionInfo>& InOutSectionInfo) const
{
	FMyMeshMergeHelpers::ExtractSections(StaticMesh, LODIndex, InOutSectionInfo);
}

int32 FMyStaticMeshAdapter::GetMaterialIndex(int32 LODIndex, int32 SectionIndex) const
{
	return StaticMesh->GetSectionInfoMap().Get(LODIndex, SectionIndex).MaterialIndex;
}

void FMyStaticMeshAdapter::ApplySettings(int32 LODIndex, FMeshData& InOutMeshData) const
{
	InOutMeshData.LightMapIndex = StaticMesh->GetLightMapCoordinateIndex();
}

UPackage* FMyStaticMeshAdapter::GetOuter() const
{
	return nullptr;
}

FString FMyStaticMeshAdapter::GetBaseName() const
{
	return StaticMesh->GetOutermost()->GetName();
}

FName FMyStaticMeshAdapter::GetMaterialSlotName(int32 MaterialIndex) const
{
	return StaticMesh->GetStaticMaterials()[MaterialIndex].MaterialSlotName;
}

FName FMyStaticMeshAdapter::GetImportedMaterialSlotName(int32 MaterialIndex) const
{
	return StaticMesh->GetStaticMaterials()[MaterialIndex].ImportedMaterialSlotName;
}

void FMyStaticMeshAdapter::SetMaterial(int32 MaterialIndex, UMaterialInterface* Material)
{
	const FStaticMaterial& OriginalMaterialSlot = StaticMesh->GetStaticMaterials()[MaterialIndex];
	StaticMesh->GetStaticMaterials()[MaterialIndex] = FStaticMaterial(Material, OriginalMaterialSlot.MaterialSlotName, OriginalMaterialSlot.ImportedMaterialSlotName);
}

void FMyStaticMeshAdapter::RemapMaterialIndex(int32 LODIndex, int32 SectionIndex, int32 NewMaterialIndex)
{
	FMeshSectionInfo SectionInfo = StaticMesh->GetSectionInfoMap().Get(LODIndex, SectionIndex);
	SectionInfo.MaterialIndex = NewMaterialIndex;
	StaticMesh->GetSectionInfoMap().Set(LODIndex, SectionIndex, SectionInfo);
}

int32 FMyStaticMeshAdapter::AddMaterial(UMaterialInterface* Material)
{
	int32 Index = StaticMesh->GetStaticMaterials().Emplace(Material);
	IMeshUtilities& MeshUtilities = FModuleManager::Get().LoadModuleChecked<IMeshUtilities>("MeshUtilities");
	MeshUtilities.FixupMaterialSlotNames(StaticMesh);
	return Index;
}

int32 FMyStaticMeshAdapter::AddMaterial(UMaterialInterface* Material, const FName& SlotName, const FName& ImportedSlotName)
{
	int32 Index = StaticMesh->GetStaticMaterials().Emplace(Material, SlotName, ImportedSlotName);
	IMeshUtilities& MeshUtilities = FModuleManager::Get().LoadModuleChecked<IMeshUtilities>("MeshUtilities");
	MeshUtilities.FixupMaterialSlotNames(StaticMesh);
	return Index;
}

void FMyStaticMeshAdapter::UpdateUVChannelData()
{
	StaticMesh->UpdateUVChannelData(false);
}

bool FMyStaticMeshAdapter::IsAsset() const
{
	return true;
}

int32 FMyStaticMeshAdapter::LightmapUVIndex() const
{
	return StaticMesh->GetLightMapCoordinateIndex();
}

FBoxSphereBounds FMyStaticMeshAdapter::GetBounds() const
{
	return StaticMesh->GetBounds();
}
