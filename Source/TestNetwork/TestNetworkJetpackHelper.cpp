#include "TestNetwork.h"
#include "TestNetworkJetpackHelper.h"
#include "TestNetworkCharacterMovementComponent.h"
#include "TestNetworkCharacter.h"
#include "Runtime/Engine/Classes/Kismet/KismetMathLibrary.h"
#include "Runtime/Engine/Classes/Kismet/KismetSystemLibrary.h"

UTestNetworkJetpackHelper::UTestNetworkJetpackHelper()
    : Super(ETestNetworkMovementMode::MOVE_Jetpacking)
    , bPressedJetpack(false)
    , JetpackToggle(0)
{

}

UTestNetworkJetpackHelper::~UTestNetworkJetpackHelper()
{
}

void UTestNetworkJetpackHelper::RequestJetpack()
{
    bPressedJetpack = true;
}

void UTestNetworkJetpackHelper::RequestStopJetpacking()
{
    bPressedJetpack = false;
}

bool UTestNetworkJetpackHelper::CanJetpack()
{
    return !IsMovementMode() && !OwnerMovementComponent->IsFalling();
}

bool UTestNetworkJetpackHelper::GetFloorLocation(FVector& Location)
{
    FVector FromLocation = OwnerCharacter->GetActorLocation();
    FVector ToLocation = FromLocation - FVector::UpVector * 100000.0f;
    TArray<AActor*> ActorsToIgnore;
    FHitResult Hit;
    if (UKismetSystemLibrary::LineTraceSingle_NEW(OwnerCharacter, FromLocation, ToLocation, ETraceTypeQuery::TraceTypeQuery1, false, ActorsToIgnore, EDrawDebugTrace::None, Hit, true))
    {
        Location = Hit.Location;
        return true;
    }
    return false;
}

void UTestNetworkJetpackHelper::CheckInput(float DeltaTime)
{
    bool bJetpackToggled = false;
    if (bPressedJetpack)
        JetpackToggle = 1;
    else if (JetpackToggle == 1)
    {
        JetpackToggle = 0;
        bJetpackToggled = true;
    }
    if (bJetpackToggled && CanJetpack())
    {
        GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Blue, "Jetpacking");
        SetMovementMode();
    }
    else if (bJetpackToggled && IsMovementMode())
    {
        GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Blue, "Cancel Jetpacking");
        SetMovementMode(EMovementMode::MOVE_Falling);
    }
}

void UTestNetworkJetpackHelper::PhysCustom(float DeltaTime, int32 Iterations)
{
    FVector FloorLocation;
    if (!GetFloorLocation(FloorLocation))
    {
        GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Blue, "Cancel Jetpacking");
        SetMovementMode(EMovementMode::MOVE_Falling);
        OwnerMovementComponent->StartNewPhysics(DeltaTime, Iterations);
        return;
    }
    // Maintain vertical position.
    FVector Direction;
    float Distance;
    (OwnerCharacter->GetActorLocation() - FloorLocation).ToDirectionAndLength(Direction, Distance);
    if (Distance < JetpackDistance)
    {
        float DeltaSpeed = JetpackGotoDistanceSpeed * DeltaTime;
        FVector Velocity = FVector::UpVector * DeltaSpeed;
        OwnerCharacter->AddActorWorldOffset(Velocity);
    }
    else
    {
        float DeltaSpeed = -JetpackGotoDistanceSpeed * DeltaTime;
        FVector Velocity = FVector::UpVector * DeltaSpeed;
        OwnerCharacter->AddActorWorldOffset(Velocity / 2.0f);
    }
    // Go forward or backward depending on player input.
    if (OwnerMovementComponent->PlayerInput.Y != 0)
    {
        float DeltaSpeed = JetpackSpeed * DeltaTime;
        FVector Velocity = UKismetMathLibrary::GetForwardVector(OwnerCharacter->GetControlRotation()) * DeltaSpeed * OwnerMovementComponent->PlayerInput.Y;
        OwnerCharacter->AddActorWorldOffset(Velocity);
    }
}

void UTestNetworkJetpackHelper::OnMovementModeChanged(EMovementMode PreviousMovementMode, EMovementMode ActualMovementMode, ETestNetworkMovementMode PreviousCustomMode, ETestNetworkMovementMode ActualCustomMode)
{

}

void UTestNetworkJetpackHelper::ClearInput()
{
    bPressedJetpack = false;
    JetpackToggle = 0;
}