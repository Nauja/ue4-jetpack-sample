// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#include "SampleEquipment.h"

FName ASampleEquipment::MeshComponentName(TEXT("StaticMesh"));

/// <summary>
/// 
/// </summary>
/// <param name="ObjectInitializer"></param>
/// <returns></returns>
ASampleEquipment::ASampleEquipment(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
    , Config(nullptr)
    , Mesh(nullptr)
{
    RootComponent = ObjectInitializer.CreateDefaultSubobject<USceneComponent>(this, TEXT("RootComponent"));

    Mesh = ObjectInitializer.CreateDefaultSubobject<UStaticMeshComponent>(this, MeshComponentName);
    Mesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    Mesh->SetupAttachment(RootComponent);
}

void ASampleEquipment::PostInitializeComponents()
{
    RefreshConfig();

    Super::PostInitializeComponents();
}

#if WITH_EDITOR
/**
 * Update the AActor when assigned USampleEquipmentConfig changes.
 */
void ASampleEquipment::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
    UProperty* PropertyThatChanged = PropertyChangedEvent.Property;
    if (PropertyThatChanged)
    {
        if (PropertyThatChanged->GetFName() == TEXT("Config"))
        {
            RefreshConfig();
        }
    }

    Super::PostEditChangeProperty(PropertyChangedEvent);
}
#endif

void ASampleEquipment::UpdateConfig(class USampleEquipmentConfig* Config)
{
    this->Config = Config;
    RefreshConfig();
}

void ASampleEquipment::RefreshConfig()
{
    if (Config != nullptr)
    {
        Mesh->SetStaticMesh(Config->Mesh);
    }
    else
    {
        Mesh->SetStaticMesh(nullptr);
    }
}