// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "GameFramework/Actor.h"
#include "SamplePickup.generated.h"

/**
 * This UDataAsset let us configure our pickup from Blueprint.
 *
 * Create a new Blueprint inheriting from this class and assign
 * it to ASamplePickup.
 */
UCLASS(config = Game, Blueprintable, BlueprintType)
class SAMPLE_API USamplePickupConfig : public UDataAsset
{
	GENERATED_BODY()

public:
	USamplePickupConfig();

	/**
	 * Associated equipment gained on pickup.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Sample)
	class USampleEquipmentConfig* Equipment;

	/**
	 * Mesh displayed when on floor.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Sample)
	class UStaticMesh* Mesh;

	/**
	 * FX displayed when on floor.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Sample)
	class UNiagaraSystem* FX;

	/**
	 * Axis for rotating the mesh.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Rotation)
	FVector RotationAxis;

	/**
	 * Rotation speed along the RotationAxis.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Rotation)
	float RotationSpeed;
};

UCLASS(config = Game, BlueprintType)
class SAMPLE_API ASamplePickup : public AActor
{
	GENERATED_BODY()

public:
	ASamplePickup(const FObjectInitializer& ObjectInitializer);

	virtual void PostInitializeComponents() override;
#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif
	void NotifyActorBeginOverlap(class AActor* Other) override;
	virtual void Tick(float DeltaSeconds);

	FORCEINLINE class USamplePickupConfig* GetConfig() const { return Config; }

private:
	UPROPERTY()
	bool bIsActive;

	/**
	 * External pickup configuration.
	 */
	UPROPERTY(Category = Sample, EditAnywhere)
	class USamplePickupConfig* Config;

	UPROPERTY(Category=Sample, EditAnywhere, meta=(AllowPrivateAccess = "true"))
	class UCapsuleComponent* Capsule;

	UPROPERTY(Category=Sample, EditAnywhere, meta=(AllowPrivateAccess = "true"))
	class UStaticMeshComponent* StaticMesh;

	UPROPERTY(Category=Sample, EditAnywhere, meta=(AllowPrivateAccess = "true"))
	class UNiagaraComponent* Niagara;

	FTimerHandle TimerHandle_Respawn;

	void Respawn();
};

