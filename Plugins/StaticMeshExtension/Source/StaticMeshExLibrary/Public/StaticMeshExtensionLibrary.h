// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "MaterialBakingStructures.h"
#include "StaticMeshExtensionLibrary.generated.h"


class UMaterialOptions;
class UMaterialInterface;
class UStaticMesh;
class UStaticMeshComponent;
class IMaterialBakingAdapter;
/**
 * 
 */
class FMyAsyncTask : public FNonAbandonableTask
{
public:
	FMyAsyncTask(AMyActor* InMyActor) : MyActor(InMyActor){}

	FORCEINLINE TStatId GetStatId()const
	{
		RETURN_QUICK_DECLARE_CYCLE_STAT(FMyAsyncTask, STATGROUP_ThreadPoolAsyncTasks);
	}
	
	void DoWork();
private:
	AMyActor* MyActor;
};

UCLASS()
class STATICMESHEXLIBRARY_API UStaticMeshExtensionLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
	
public:
	UFUNCTION(BlueprintCallable, Category = "StaticMeshExLibrary")
	static void BakeMaterialToVertexForStaticMesh(UStaticMesh* mesh);
	
	UFUNCTION(BlueprintCallable, Category = "StaticMeshExLibrary")
	static TArray<FColor> GetBaseColorForStaticMesh(UStaticMesh* mesh);
	
	UFUNCTION(BlueprintCallable, Category = "StaticMeshExLibrary")
	static TArray<FColor> GetBakeOutForStaticMeshComponent(UStaticMeshComponent* StaticMeshComponent);
	
	UFUNCTION(BlueprintCallable, Category = "StaticMeshExLibrary")
	static void Async_GetBakeOutForMyActor(AMyActor* MyActor);

	UFUNCTION(BlueprintCallable, Category = "StaticMeshExLibrary")
	static void ExportArrayColorsAsPNG(TArray<FColor> BaseColor, FString FileName);
	
	UFUNCTION(BlueprintCallable, Category = "StaticMeshExLibrary")
	static FColor GetColorFromArrayByUV(TArray<FColor> BaseColor, float U, float V);
	
private:
	static FBakeOutput BakeOutMaterialComponent(TArray<TWeakObjectPtr<UObject>>& OptionObjects, IMaterialBakingAdapter* Adapter);
	
	static void DetermineMaterialVertexDataUsage(TArray<bool>& InOutMaterialUsesVertexData, const TArray<UMaterialInterface*>& UniqueMaterials, const UMaterialOptions* MaterialOptions);

//#if WITH_EDITOR
	static void SetVertexColorsFromBakeOut(UStaticMesh* mesh, FBakeOutput BakeOut);
//#endif
	
};

