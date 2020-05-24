// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#include "SamplePickup.h"
#include "SampleCharacter.h"
#include "Components/StaticMeshComponent.h"
#include "Components/CapsuleComponent.h"
#include "NiagaraComponent.h"

USamplePickupConfig::USamplePickupConfig()
    : RotationAxis(FVector::UpVector)
    , RotationSpeed(0.0f)
{}

ASamplePickup::ASamplePickup(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
    , bIsActive(true)
    , Config(nullptr)
    , StaticMesh(nullptr)
    , Niagara(nullptr)
{
    PrimaryActorTick.bCanEverTick = true;

    RootComponent = ObjectInitializer.CreateDefaultSubobject<USceneComponent>(this, TEXT("RootComponent"));

    Capsule = ObjectInitializer.CreateDefaultSubobject<UCapsuleComponent>(this, TEXT("Capsule"));
    Capsule->SetupAttachment(RootComponent);

    StaticMesh = ObjectInitializer.CreateDefaultSubobject<UStaticMeshComponent>(this, TEXT("StaticMesh"));
    StaticMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    StaticMesh->SetupAttachment(RootComponent);

    Niagara = ObjectInitializer.CreateDefaultSubobject<UNiagaraComponent>(this, TEXT("Niagara"));
    Niagara->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    Niagara->SetupAttachment(RootComponent);
}

void ASamplePickup::PostInitializeComponents()
{
    if (Config)
    {
        StaticMesh->SetStaticMesh(Config->Mesh);
        Niagara->SetAsset(Config->FX);
    }
    else
    {
        StaticMesh->SetStaticMesh(nullptr);
        Niagara->SetAsset(nullptr);
    }

    Super::PostInitializeComponents();
}

#if WITH_EDITOR
/**
 * Update the AActor when assigned USamplePickupConfig changes.
 */
void ASamplePickup::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
    UProperty* PropertyThatChanged = PropertyChangedEvent.Property;
    if (PropertyThatChanged)
    {
        if (PropertyThatChanged->GetFName() == TEXT("Config"))
        {
            if (Config)
            {
                StaticMesh->SetStaticMesh(Config->Mesh);
                Niagara->SetAsset(Config->FX);
            }
            else
            {
                StaticMesh->SetStaticMesh(nullptr);
                Niagara->SetAsset(nullptr);
            }
        }
    }

    Super::PostEditChangeProperty(PropertyChangedEvent);
}
#endif

void ASamplePickup::NotifyActorBeginOverlap(class AActor* Other)
{
    Super::NotifyActorBeginOverlap(Other);

    if (IsValid(Other) && !IsPendingKill() && bIsActive)
    {
        bIsActive = false;
        StaticMesh->SetVisibility(false);
        Niagara->Deactivate();

        if (Config)
        {
            static_cast<ASampleCharacter*>(Other)->AddEquipment(Config->Equipment);
        }

        GetWorldTimerManager().SetTimer(TimerHandle_Respawn, this, &ASamplePickup::Respawn, 5.0f, false);
    }
}

void ASamplePickup::Respawn()
{
    bIsActive = true;
    StaticMesh->SetVisibility(true);
    Niagara->Activate();
}

void ASamplePickup::Tick(float DeltaSeconds)
{
    if (Config)
    {
        StaticMesh->AddLocalRotation(FRotator::MakeFromEuler(Config->RotationAxis) * Config->RotationSpeed);
    }
}