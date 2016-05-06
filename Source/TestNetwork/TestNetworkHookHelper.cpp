#include "TestNetwork.h"
#include "TestNetworkHookHelper.h"
#include "TestNetworkCharacterMovementComponent.h"
#include "TestNetworkCharacter.h"
#include "Runtime/Engine/Classes/Kismet/KismetMathLibrary.h"
#include "Runtime/Engine/Classes/Kismet/KismetSystemLibrary.h"

UTestNetworkHookHelper::UTestNetworkHookHelper()
    : Super(ETestNetworkMovementMode::MOVE_Hooking)
    , bPressedHook(false)
{

}

UTestNetworkHookHelper::~UTestNetworkHookHelper()
{
}

void UTestNetworkHookHelper::RequestHook()
{
    bPressedHook = true;
}

void UTestNetworkHookHelper::RequestStopHooking()
{
    bPressedHook = false;
}

bool UTestNetworkHookHelper::CanHook()
{
    if (IsMovementMode() || OwnerMovementComponent->IsFalling())
        return false;
    return GetPointedLocation(HookLocation); // Check and save hook location.
}

bool UTestNetworkHookHelper::GetPointedLocation(FVector& Location)
{
    FVector FromLocation = OwnerCharacter->GetActorLocation();
    FVector ToLocation = FromLocation + UKismetMathLibrary::GetForwardVector(OwnerCharacter->GetControlRotation()) * 100000.0f;
    TArray<AActor*> ActorsToIgnore;
    FHitResult Hit;
    if (UKismetSystemLibrary::LineTraceSingle_NEW(OwnerCharacter, FromLocation, ToLocation, ETraceTypeQuery::TraceTypeQuery1, false, ActorsToIgnore, EDrawDebugTrace::ForOneFrame, Hit, true))
    {
        Location = Hit.Location;
        return true;
    }
    return false;
}

void UTestNetworkHookHelper::CheckInput(float DeltaTime)
{
    if (bPressedHook && CanHook())
    {
        GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Blue, "Hooking");
        SetMovementMode();
    }
    else if (!bPressedHook && IsMovementMode())
    {
        GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Blue, "Cancel Hooking");
        SetDefaultMovementMode();
    }
}

void UTestNetworkHookHelper::PhysCustom(float DeltaTime, int32 Iterations)
{
    DrawDebugLine(OwnerCharacter->GetWorld(), OwnerCharacter->GetActorLocation(), HookLocation, FColor::Red, false);
    DrawDebugSphere(OwnerCharacter->GetWorld(), HookLocation, 50.0f, 10, FColor::Red, false);
    FVector Direction;
    float Distance;
    (HookLocation - OwnerCharacter->GetActorLocation()).ToDirectionAndLength(Direction, Distance);
    Distance = FMath::Max(Distance - HookOkDistance, 0.0f);
    float DeltaSpeed = HookSpeed * DeltaTime;
    if (Distance <= DeltaSpeed) // Reached hook location, start falling mode.
    {
        OwnerCharacter->SetActorLocation(OwnerCharacter->GetActorLocation() + Direction * Distance);
        ClearInput();
        SetMovementMode(EMovementMode::MOVE_Falling);
        OwnerMovementComponent->StartNewPhysics(DeltaTime, Iterations);
    }
    else
    {
        FVector Velocity = Direction * FMath::Min(Distance, DeltaSpeed);
		OwnerCharacter->AddActorWorldOffset(Velocity);
    }
}

void UTestNetworkHookHelper::ClearInput()
{
    bPressedHook = false;
}