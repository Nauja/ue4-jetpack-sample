// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "GameFramework/Actor.h"
#include "SampleEquipment.generated.h"

/**
 * This UDataAsset let us configure our jetpack from Blueprint.
 *
 * Create a new Blueprint inheriting from this class and assign
 * it to ASampleEquipment.
 */
UCLASS(config = Game, Blueprintable, BlueprintType)
class SAMPLE_API USampleEquipmentConfig : public UDataAsset
{
	GENERATED_BODY()

public:
	/**
	 * Mesh to display on actor.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Sample)
	class UStaticMesh* Mesh;

	/**
	 * Class to spawn.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Sample)
	TSubclassOf<class ASampleEquipment> Class;

	/**
	 * Which socket to attach from actor.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Sample)
	FName ActorSocket;
};

UCLASS(config = Game, BlueprintType)
class SAMPLE_API ASampleEquipment : public AActor
{
	GENERATED_BODY()

public:
	static FName MeshComponentName;

public:
	ASampleEquipment(const FObjectInitializer& ObjectInitializer);

	virtual void PostInitializeComponents() override;
#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif

	void UpdateConfig(class USampleEquipmentConfig* Config);

	FORCEINLINE class USampleEquipmentConfig* GetConfig() const { return Config; }
	FORCEINLINE class UStaticMeshComponent* GetMeshComponent() const { return Mesh; }

private:
	/**
	 * External equipment configuration.
	 */
	UPROPERTY(Category = Sample, EditAnywhere)
	class USampleEquipmentConfig* Config;

	UPROPERTY(Category = Sample, EditAnywhere, meta = (AllowPrivateAccess = "true"))
	class UStaticMeshComponent* Mesh;

	void RefreshConfig();
};