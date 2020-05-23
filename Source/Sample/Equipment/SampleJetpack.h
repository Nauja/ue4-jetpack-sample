// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "GameFramework/Actor.h"
#include "SampleJetpack.generated.h"

/**
 * This UDataAsset let us configure our jetpack from Blueprint.
 *
 * Create a new Blueprint inheriting from this class and assign
 * it to ASampleJetpack.
 */
UCLASS(config = Game, Blueprintable, BlueprintType)
class SAMPLE_API USampleJetpackConfig : public UDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Sample)
	class UStaticMesh* Mesh;
};

UCLASS(config = Game, BlueprintType)
class SAMPLE_API ASampleJetpack : public AActor
{
	GENERATED_BODY()

public:
	ASampleJetpack(const FObjectInitializer& ObjectInitializer);

	virtual void PostInitializeComponents() override;
#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif

	FORCEINLINE class USampleJetpackConfig* GetConfig() const { return Config; }

private:
	/**
	 * External jetpack configuration.
	 */
	UPROPERTY(Category = Sample, EditAnywhere)
	class USampleJetpackConfig* Config;

	UPROPERTY(Category=Sample, EditAnywhere, meta=(AllowPrivateAccess = "true"))
	class UStaticMeshComponent* Mesh;
};

