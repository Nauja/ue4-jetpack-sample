// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#include "SampleJetpack.h"
#include "Components/StaticMeshComponent.h"

ASampleJetpack::ASampleJetpack(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
    , Config(nullptr)
    , Mesh(nullptr)
{
    RootComponent = ObjectInitializer.CreateDefaultSubobject<USceneComponent>(this, TEXT("RootComponent"));

    Mesh = ObjectInitializer.CreateDefaultSubobject<UStaticMeshComponent>(this, TEXT("Mesh"));
    Mesh->SetupAttachment(RootComponent);
}

void ASampleJetpack::PostInitializeComponents()
{
    if (Config != nullptr)
    {
        Mesh->SetStaticMesh(Config->Mesh);
    }
    else
    {
        Mesh->SetStaticMesh(nullptr);
    }

    Super::PostInitializeComponents();
}

#if WITH_EDITOR
/**
 * Update the AActor when assigned USampleJetpackConfig changes.
 */
void ASampleJetpack::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
    UProperty* PropertyThatChanged = PropertyChangedEvent.Property;
    if (PropertyThatChanged)
    {
        if (PropertyThatChanged->GetFName() == TEXT("Config"))
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
    }

    Super::PostEditChangeProperty(PropertyChangedEvent);
}
#endif