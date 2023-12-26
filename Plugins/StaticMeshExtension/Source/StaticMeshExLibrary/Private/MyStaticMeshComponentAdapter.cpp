// Copyright Epic Games, Inc. All Rights Reserved.

#include "MyStaticMeshComponentAdapter.h"
#include "MaterialBakingStructures.h"

#include "Engine/MapBuildDataRegistry.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
//#include "MeshMergeUtilities.h"
#include "MyMeshMergeHelpers.h"
#include "StaticMeshComponentLODInfo.h"
#include "UObject/Package.h"

FMyStaticMeshComponentAdapter::FMyStaticMeshComponentAdapter(UStaticMeshComponent* InStaticMeshComponent)
	: StaticMeshComponent(InStaticMeshComponent), StaticMesh(InStaticMeshComponent->GetStaticMesh())
{
	checkf(StaticMesh != nullptr, TEXT("Invalid static mesh in adapter"));
	NumLODs = StaticMesh->GetNumLODs();
}

int32 FMyStaticMeshComponentAdapter::GetNumberOfLODs() const
{
	return NumLODs;
}

void FMyStaticMeshComponentAdapter::RetrieveRawMeshData(int32 LODIndex, FMeshDescription& InOutRawMesh, bool bPropogateMeshData) const
{
	FMyMeshMergeHelpers::RetrieveMesh(StaticMeshComponent, LODIndex, InOutRawMesh, bPropogateMeshData);
}

void FMyStaticMeshComponentAdapter::RetrieveMeshSections(int32 LODIndex, TArray<FSectionInfo>& InOutSectionInfo) const
{
	FMyMeshMergeHelpers::ExtractSections(StaticMeshComponent, LODIndex, InOutSectionInfo);
}

int32 FMyStaticMeshComponentAdapter::GetMaterialIndex(int32 LODIndex, int32 SectionIndex) const
{
	return StaticMesh->GetSectionInfoMap().Get(LODIndex, SectionIndex).MaterialIndex;
}

void FMyStaticMeshComponentAdapter::ApplySettings(int32 LODIndex, FMeshData& InOutMeshData) const
{
	if (StaticMeshComponent->LODData.IsValidIndex(LODIndex))
	{
		// Retrieve lightmap reference from the static mesh component (if it exists)
		const FStaticMeshComponentLODInfo& ComponentLODInfo = StaticMeshComponent->LODData[LODIndex];
		const FMeshMapBuildData* MeshMapBuildData = StaticMeshComponent->GetMeshMapBuildData(ComponentLODInfo);
		if (MeshMapBuildData)
		{
			InOutMeshData.LightMap = MeshMapBuildData->LightMap;
			InOutMeshData.LightMapIndex = StaticMeshComponent->GetStaticMesh()->GetLightMapCoordinateIndex();
			InOutMeshData.LightmapResourceCluster = MeshMapBuildData->ResourceCluster;
		}
	}
}

UPackage* FMyStaticMeshComponentAdapter::GetOuter() const
{
	return nullptr;
}

FString FMyStaticMeshComponentAdapter::GetBaseName() const
{
	return StaticMesh->GetOutermost()->GetName();
}

FName FMyStaticMeshComponentAdapter::GetMaterialSlotName(int32 MaterialIndex) const
{
	return StaticMeshComponent->GetMaterialSlotNames()[MaterialIndex];
}

FName FMyStaticMeshComponentAdapter::GetImportedMaterialSlotName(int32 MaterialIndex) const
{
	return FName();
}

void FMyStaticMeshComponentAdapter::SetMaterial(int32 MaterialIndex, UMaterialInterface* Material)
{
	StaticMeshComponent->SetMaterial(MaterialIndex, Material);
}

void FMyStaticMeshComponentAdapter::RemapMaterialIndex(int32 LODIndex, int32 SectionIndex, int32 NewMaterialIndex)
{
}

int32 FMyStaticMeshComponentAdapter::AddMaterial(UMaterialInterface* Material)
{
	return INDEX_NONE;
}

int32 FMyStaticMeshComponentAdapter::AddMaterial(UMaterialInterface* Material, const FName& SlotName, const FName& ImportedSlotName)
{
	return INDEX_NONE;
}

void FMyStaticMeshComponentAdapter::UpdateUVChannelData()
{
	StaticMesh->UpdateUVChannelData(false);
}

bool FMyStaticMeshComponentAdapter::IsAsset() const
{
	return false;
}

int32 FMyStaticMeshComponentAdapter::LightmapUVIndex() const
{
	return StaticMesh->GetLightMapCoordinateIndex();
}

FBoxSphereBounds FMyStaticMeshComponentAdapter::GetBounds() const
{
	return StaticMeshComponent->Bounds;
}
