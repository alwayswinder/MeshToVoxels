// Fill out your copyright notice in the Description page of Project Settings.


#include "StaticMeshExtensionLibrary.h"
#include "ToolMenus.h"
#include "EditorUtilityLibrary.h"
#include "ImageUtils.h"
#include "Toolkits/AssetEditorToolkitMenuContext.h"
#include "Toolkits/AssetEditorToolkit.h"
#include "AssetRegistry/AssetRegistryModule.h"


#include "MaterialBakingStructures.h"
#include "IMaterialBakingModule.h"
#include "MaterialOptions.h"
#include "StaticMeshAttributes.h"
#include "IMaterialBakingAdapter.h"
#include "MyActor.h"
#include "StaticMeshOperations.h"
#include "MyStaticMeshAdapter.h"
#include "Engine/StaticMesh.h"
#include "MyMeshMergeHelpers.h"
#include "MyStaticMeshComponentAdapter.h"
#include "Async/Async.h"
#include "Misc/FileHelper.h"

#define SIZE_X 512
#define SIZE_Y 512

void UStaticMeshExtensionLibrary::Async_GetBakeOutForMyActor(AMyActor* MyActor)
{
	AsyncTask(ENamedThreads::GameThread, [MyActor]()
	{
		auto MyTask = new FAsyncTask<FMyAsyncTask>(MyActor);
		MyTask->StartBackgroundTask();
		MyTask->EnsureCompletion();
		delete MyTask;
		UE_LOG(LogTemp, Log, TEXT("[MyLog] Stop : AsyncTask!"));
	});
}

void FMyAsyncTask::DoWork()
{
	if(MyActor)
	{
		if(MyActor->BaseColor.Num() <= 0)
		{
			//MyActor->BaseColor.Add(FColor::Blue);
			MyActor->BaseColor = UStaticMeshExtensionLibrary::GetBakeOutForStaticMeshComponent(MyActor->StaticMeshComponent);
		}
	}
	UE_LOG(LogTemp, Log, TEXT("[MyLog] Completion : AsyncTask!"));
}

void UStaticMeshExtensionLibrary::BakeMaterialToVertexForStaticMesh(UStaticMesh* mesh)
{
	UMaterialOptions* MaterialOptionsInit = DuplicateObject(GetMutableDefault<UMaterialOptions>(), GetTransientPackage());
	UAssetBakeOptions* AssetOptionsInit = GetMutableDefault<UAssetBakeOptions>();
	UMaterialMergeOptions* MergeOptionsInit = GetMutableDefault<UMaterialMergeOptions>();
	MaterialOptionsInit->bUseMeshData = true;
	MaterialOptionsInit->bUseSpecificUVIndex = true;
	MaterialOptionsInit->TextureCoordinateIndex = 0;
	MaterialOptionsInit->TextureSize = FIntPoint(SIZE_X,SIZE_Y);

	TArray<TWeakObjectPtr<UObject>> OptionObjects{ MergeOptionsInit, AssetOptionsInit, MaterialOptionsInit };
	
	FMyStaticMeshAdapter AdapterInit(mesh);

	FBakeOutput BakeOut = BakeOutMaterialComponent(OptionObjects, &AdapterInit);

	TArray<FColor>& BaseColor = BakeOut.PropertyData.FindChecked(EMaterialProperty::MP_BaseColor);
	if(BaseColor.Num() > 0)
	{
		SetVertexColorsFromBakeOut(mesh, BakeOut);
	}
}

TArray<FColor> UStaticMeshExtensionLibrary::GetBaseColorForStaticMesh(UStaticMesh* mesh)
{
	UMaterialOptions* MaterialOptionsInit = DuplicateObject(GetMutableDefault<UMaterialOptions>(), GetTransientPackage());
	UAssetBakeOptions* AssetOptionsInit = GetMutableDefault<UAssetBakeOptions>();
	UMaterialMergeOptions* MergeOptionsInit = GetMutableDefault<UMaterialMergeOptions>();
	//MaterialOptionsInit->bUseMeshData = true;
	//MaterialOptionsInit->bUseSpecificUVIndex = true;
	//MaterialOptionsInit->TextureCoordinateIndex = 0;
	MaterialOptionsInit->TextureSize = FIntPoint(SIZE_X,SIZE_Y);

	TArray<TWeakObjectPtr<UObject>> OptionObjects{ MergeOptionsInit, AssetOptionsInit, MaterialOptionsInit };
	
	FMyStaticMeshAdapter AdapterInit(mesh);

	FBakeOutput BakeOut = BakeOutMaterialComponent(OptionObjects, &AdapterInit);

	TArray<FColor>& BaseColor = BakeOut.PropertyData.FindChecked(EMaterialProperty::MP_BaseColor);
	return  BaseColor;
}

TArray<FColor> UStaticMeshExtensionLibrary::GetBakeOutForStaticMeshComponent(UStaticMeshComponent* StaticMeshComponent)
{
	UMaterialOptions* MaterialOptionsInit = DuplicateObject(GetMutableDefault<UMaterialOptions>(), GetTransientPackage());
	UAssetBakeOptions* AssetOptionsInit = GetMutableDefault<UAssetBakeOptions>();
	UMaterialMergeOptions* MergeOptionsInit = GetMutableDefault<UMaterialMergeOptions>();
	MaterialOptionsInit->bUseMeshData = false;
	MaterialOptionsInit->bUseSpecificUVIndex = false;
	MaterialOptionsInit->TextureCoordinateIndex = 0;
	MaterialOptionsInit->TextureSize = FIntPoint(SIZE_X,SIZE_Y);
	TArray<TWeakObjectPtr<UObject>> OptionObjects{ MergeOptionsInit, AssetOptionsInit, MaterialOptionsInit };
	
	FMyStaticMeshComponentAdapter AdapterInit(StaticMeshComponent);
	
	FBakeOutput BakeOut = BakeOutMaterialComponent(OptionObjects, &AdapterInit);
	TArray<FColor>& BaseColor = BakeOut.PropertyData.FindChecked(EMaterialProperty::MP_BaseColor);
	
	FIntPoint Size = BakeOut.PropertySizes.FindChecked(EMaterialProperty::MP_BaseColor);

	return BaseColor;
}

FBakeOutput UStaticMeshExtensionLibrary::BakeOutMaterialComponent(TArray<TWeakObjectPtr<UObject>>& OptionObjects, IMaterialBakingAdapter* Adapter)
{
	
	// Try and find material (merge) options from provided set of objects
	TWeakObjectPtr<UObject>* MaterialOptionsObject = OptionObjects.FindByPredicate([](TWeakObjectPtr<UObject> Object)
	{
		return Cast<UMaterialOptions>(Object.Get()) != nullptr;
	});

	TWeakObjectPtr<UObject>* MaterialMergeOptionsObject = OptionObjects.FindByPredicate([](TWeakObjectPtr<UObject> Object)
	{
		return Cast<UMaterialMergeOptions>(Object.Get()) != nullptr;
	});

	UMaterialOptions* MaterialOptions = MaterialOptionsObject ? Cast<UMaterialOptions>(MaterialOptionsObject->Get()) : nullptr;
	checkf(MaterialOptions, TEXT("No valid material options found"));


	UMaterialMergeOptions* MaterialMergeOptions  = MaterialMergeOptionsObject ? Cast<UMaterialMergeOptions>(MaterialMergeOptionsObject->Get()) : nullptr;

	// Mesh / LOD index	
	TMap<uint32, FMeshDescription> RawMeshLODs;

	// Unique set of sections in mesh
	TArray<FSectionInfo> UniqueSections;

	TArray<FSectionInfo> Sections;

	int32 NumLODs = Adapter->GetNumberOfLODs();

	// LOD index, <original section index, unique section index>
	TArray<TMap<int32, int32>> UniqueSectionIndexPerLOD;
	UniqueSectionIndexPerLOD.AddDefaulted(NumLODs);

	// Retrieve raw mesh data and unique sections
	for (int32 LODIndex = 0; LODIndex < NumLODs; ++LODIndex)
	{
		// Reset section for reuse
		Sections.SetNum(0, false);

		// Extract raw mesh data 
		const bool bProcessedLOD = MaterialOptions->LODIndices.Contains(LODIndex);
		if (bProcessedLOD)
		{
			FMeshDescription& RawMesh = RawMeshLODs.Add(LODIndex);
			FStaticMeshAttributes(RawMesh).Register();
			Adapter->RetrieveRawMeshData(LODIndex, RawMesh, MaterialOptions->bUseMeshData);
		}

		// Extract sections for given LOD index from the mesh 
		Adapter->RetrieveMeshSections(LODIndex, Sections);

		for (int32 SectionIndex = 0; SectionIndex < Sections.Num(); ++SectionIndex)
		{
			FSectionInfo& Section = Sections[SectionIndex];
			Section.bProcessed = bProcessedLOD;
			const int32 UniqueIndex = UniqueSections.AddUnique(Section);
			UniqueSectionIndexPerLOD[LODIndex].Emplace(SectionIndex, UniqueIndex);
		}
	}

	TArray<UMaterialInterface*> UniqueMaterials;
	TMap<UMaterialInterface*, int32> MaterialIndices;
	TMultiMap<uint32, uint32> UniqueMaterialToUniqueSectionMap;
	// Populate list of unique materials and store section mappings
	for (int32 SectionIndex = 0; SectionIndex < UniqueSections.Num(); ++SectionIndex)
	{
		FSectionInfo& Section = UniqueSections[SectionIndex];
		const int32 UniqueIndex = UniqueMaterials.AddUnique(Section.Material);
		UniqueMaterialToUniqueSectionMap.Add(UniqueIndex, SectionIndex);
	}

	TArray<bool> bMaterialUsesVertexData;
	UStaticMeshExtensionLibrary::DetermineMaterialVertexDataUsage(bMaterialUsesVertexData, UniqueMaterials, MaterialOptions);

	TArray<FMeshData> GlobalMeshSettings;
	TArray<FMaterialData> GlobalMaterialSettings;
	TArray<TMap<uint32, uint32>> OutputMaterialsMap;
	OutputMaterialsMap.AddDefaulted(NumLODs);

	for (int32 MaterialIndex = 0; MaterialIndex < UniqueMaterials.Num(); ++MaterialIndex)
	{
		UMaterialInterface* Material = UniqueMaterials[MaterialIndex];
		const bool bDoesMaterialUseVertexData = bMaterialUsesVertexData[MaterialIndex];
		// Retrieve all sections using this material 
		TArray<uint32> SectionIndices;
		UniqueMaterialToUniqueSectionMap.MultiFind(MaterialIndex, SectionIndices);

		if (MaterialOptions->bUseMeshData)
		{
			for (const int32 LODIndex : MaterialOptions->LODIndices)
			{
				FMeshData MeshSettings;
				MeshSettings.MeshDescription = nullptr;

				// Add material indices used for rendering out material
				for (const auto& Pair : UniqueSectionIndexPerLOD[LODIndex])
				{
					if (SectionIndices.Contains(Pair.Value))
					{
						MeshSettings.MaterialIndices.Add(Pair.Key);
					}
				}

				if (MeshSettings.MaterialIndices.Num())
				{
					// Retrieve raw mesh
					MeshSettings.MeshDescription = RawMeshLODs.Find(LODIndex);
					
					//Should not be using mesh data if there is no mesh
					check(MeshSettings.MeshDescription);

					MeshSettings.TextureCoordinateBox = FBox2D(FVector2D(0.0f, 0.0f), FVector2D(1.0f, 1.0f));
					const bool bUseVertexColor = FStaticMeshOperations::HasVertexColor(*(MeshSettings.MeshDescription));
					if (MaterialOptions->bUseSpecificUVIndex)
					{
						MeshSettings.TextureCoordinateIndex = MaterialOptions->TextureCoordinateIndex;
					}
					// if you use vertex color, we can't rely on overlapping UV channel, so use light map UV to unwrap UVs
					else if (bUseVertexColor)
					{
						MeshSettings.TextureCoordinateIndex = Adapter->LightmapUVIndex();
					}
					else
					{
						MeshSettings.TextureCoordinateIndex = 0;
					}
					
					Adapter->ApplySettings(LODIndex, MeshSettings);
					
					// In case part of the UVs is not within the 0-1 range try to use the lightmap UVs
					const bool bNeedsUniqueUVs = FMyMeshMergeHelpers::CheckWrappingUVs(*(MeshSettings.MeshDescription), MeshSettings.TextureCoordinateIndex);
					const int32 LightMapUVIndex = Adapter->LightmapUVIndex();
					
					TVertexInstanceAttributesConstRef<FVector2f> VertexInstanceUVs = FStaticMeshConstAttributes(*MeshSettings.MeshDescription).GetVertexInstanceUVs();
					if (bNeedsUniqueUVs && MeshSettings.TextureCoordinateIndex != LightMapUVIndex && VertexInstanceUVs.GetNumElements() > 0 && VertexInstanceUVs.GetNumChannels() > LightMapUVIndex)
					{
						MeshSettings.TextureCoordinateIndex = LightMapUVIndex;
					}

					FMaterialData MaterialSettings;
					MaterialSettings.Material = Material;					

					// Add all user defined properties for baking out
					for (const FPropertyEntry& Entry : MaterialOptions->Properties)
					{
						if (!Entry.bUseConstantValue && Entry.Property != MP_MAX)
						{
							int32 NumTextureCoordinates;
							bool bUsesVertexData;
							Material->AnalyzeMaterialProperty(Entry.Property, NumTextureCoordinates, bUsesVertexData);

							MaterialSettings.PropertySizes.Add(Entry.Property, Entry.bUseCustomSize ? Entry.CustomSize : MaterialOptions->TextureSize);
						}
					}

					// For each original material index add an entry to the corresponding LOD and bake output index 
					for (int32 Index : MeshSettings.MaterialIndices)
					{
						OutputMaterialsMap[LODIndex].Emplace(Index, GlobalMeshSettings.Num());
					}

					GlobalMeshSettings.Add(MeshSettings);
					GlobalMaterialSettings.Add(MaterialSettings);
				}
			}
		}
		else
		{
			// If we are not using the mesh data we aren't doing anything special, just bake out uv range
			FMeshData MeshSettings;
			for (int32 LODIndex : MaterialOptions->LODIndices)
			{
				for (const auto& Pair : UniqueSectionIndexPerLOD[LODIndex])
				{
					if (SectionIndices.Contains(Pair.Value))
					{
						MeshSettings.MaterialIndices.Add(Pair.Key);
					}
				}
			}

			if (MeshSettings.MaterialIndices.Num())
			{
				MeshSettings.MeshDescription = nullptr;
				MeshSettings.TextureCoordinateBox = FBox2D(FVector2D(0.0f, 0.0f), FVector2D(1.0f, 1.0f));
				MeshSettings.TextureCoordinateIndex = 0;

				FMaterialData MaterialSettings;
				MaterialSettings.Material = Material;

				// Add all user defined properties for baking out
				for (const FPropertyEntry& Entry : MaterialOptions->Properties)
				{
					if (!Entry.bUseConstantValue && Material->IsPropertyActive(Entry.Property) && Entry.Property != MP_MAX)
					{
						MaterialSettings.PropertySizes.Add(Entry.Property, Entry.bUseCustomSize ? Entry.CustomSize : MaterialOptions->TextureSize);
					}
				}

				for (int32 LODIndex : MaterialOptions->LODIndices)
				{
					for (const auto& Pair : UniqueSectionIndexPerLOD[LODIndex])
					{
						if (SectionIndices.Contains(Pair.Value))
						{
							/// For each original material index add an entry to the corresponding LOD and bake output index 
							OutputMaterialsMap[LODIndex].Emplace(Pair.Key, GlobalMeshSettings.Num());
						}
					}
				}

				GlobalMeshSettings.Add(MeshSettings);
				GlobalMaterialSettings.Add(MaterialSettings);
			}
		}
	}
	
	TArray<FMeshData*> MeshSettingPtrs;
	for (int32 SettingsIndex = 0; SettingsIndex < GlobalMeshSettings.Num(); ++SettingsIndex)
	{
		MeshSettingPtrs.Add(&GlobalMeshSettings[SettingsIndex]);
	}

	TArray<FMaterialData*> MaterialSettingPtrs;
	for (int32 SettingsIndex = 0; SettingsIndex < GlobalMaterialSettings.Num(); ++SettingsIndex)
	{
		MaterialSettingPtrs.Add(&GlobalMaterialSettings[SettingsIndex]);
	}
	
	TArray<FBakeOutput> BakeOutputs;
	IMaterialBakingModule& Module = FModuleManager::Get().LoadModuleChecked<IMaterialBakingModule>("MaterialBaking");
	Module.BakeMaterials(MaterialSettingPtrs, MeshSettingPtrs, BakeOutputs);

	if(BakeOutputs.Num() > 0)
	{
		FBakeOutput BakeOut = BakeOutputs[0];
		return BakeOut;
	}
	return FBakeOutput();
}

void UStaticMeshExtensionLibrary::DetermineMaterialVertexDataUsage(TArray<bool>& InOutMaterialUsesVertexData, const TArray<UMaterialInterface*>& UniqueMaterials, const UMaterialOptions* MaterialOptions)
{
	//TRACE_CPUPROFILER_EVENT_SCOPE(DetermineMaterialVertexDataUsage);
	
	InOutMaterialUsesVertexData.SetNum(UniqueMaterials.Num());
	for (int32 MaterialIndex = 0; MaterialIndex < UniqueMaterials.Num(); ++MaterialIndex)
	{
		UMaterialInterface* Material = UniqueMaterials[MaterialIndex];
		for (const FPropertyEntry& Entry : MaterialOptions->Properties)
		{
			// Don't have to check a property if the result is going to be constant anyway
			if (!Entry.bUseConstantValue && Entry.Property != MP_MAX)
			{
				int32 NumTextureCoordinates;
				bool bUsesVertexData;
				Material->AnalyzeMaterialProperty(Entry.Property, NumTextureCoordinates, bUsesVertexData);

				if (bUsesVertexData || NumTextureCoordinates > 1)
				{
					InOutMaterialUsesVertexData[MaterialIndex] = true;
					break;
				}
			}
		}
	}
}

void UStaticMeshExtensionLibrary::SetVertexColorsFromBakeOut(UStaticMesh* mesh, FBakeOutput BakeOut)
{
//#if WITH_EDITOR

	TArray<FColor>& BaseColor = BakeOut.PropertyData.FindChecked(EMaterialProperty::MP_BaseColor);
	FIntPoint Size = BakeOut.PropertySizes.FindChecked(EMaterialProperty::MP_BaseColor);
	
	bool bResetVertexColors = false;

	for (int32 LodIndex = 0; LodIndex < mesh->GetNumSourceModels(); LodIndex++)
	{
		FStaticMeshSourceModel& SourceModel = mesh->GetSourceModel(LodIndex);
		if (!SourceModel.IsRawMeshEmpty())
		{
			FMeshDescription* MeshDescription = mesh->GetMeshDescription(LodIndex);
			TVertexInstanceAttributesRef<FVector4f> Colors = FStaticMeshAttributes(*MeshDescription).GetVertexInstanceColors();
			TVertexInstanceAttributesRef<FVector2f> UVs = FStaticMeshAttributes(*MeshDescription).GetVertexInstanceUVs();
			
			for (FVertexInstanceID VertexInstanceID : MeshDescription->VertexInstances().GetElementIDs())
			{
				//UE_LOG(LogTemp, Error, TEXT("VertexInstanceID=%d"), VertexInstanceID.GetValue());

				FVector2f UV = UVs[VertexInstanceID];
				
				int32 X = Size.X * UV.X;
				int32 Y = Size.Y * UV.Y;

				FColor PixelColor = FColor::White;
				if(BaseColor.IsValidIndex(Y * Size.X + X))
				{
					PixelColor = BaseColor[Y * Size.X + X];
				}
				
				Colors[VertexInstanceID] = FVector4f(PixelColor);
			}
			mesh->CommitMeshDescription(LodIndex);
			bResetVertexColors = true;
		}
	}

	if (bResetVertexColors)
	{
		mesh->Build();
		mesh->MarkPackageDirty();
		mesh->OnMeshChanged.Broadcast();
	}
//#endif
}

void UStaticMeshExtensionLibrary::ExportArrayColorsAsPNG(TArray<FColor> BaseColor, FString FileName)
{
	TArray<uint8> ImageData;
	FImageUtils::CompressImageArray(SIZE_X, SIZE_Y, BaseColor, ImageData);
	FString PicturePath = FPaths::ProjectSavedDir() + FileName + ".TGA";
	FFileHelper::SaveArrayToFile(ImageData, *PicturePath);
}
FColor UStaticMeshExtensionLibrary::GetColorFromArrayByUV(TArray<FColor> BaseColor, float U, float V)
{
	int32 X = SIZE_X * U;
	int32 Y = SIZE_Y * V;
	
	FColor PixelColor = FColor::White;
	if(BaseColor.IsValidIndex(Y * SIZE_X + X))
	{
		PixelColor = BaseColor[Y * SIZE_X + X];
	}
	return PixelColor;
}
