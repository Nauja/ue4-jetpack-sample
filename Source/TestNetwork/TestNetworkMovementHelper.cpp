#include "TestNetwork.h"
#include "TestNetworkMovementHelper.h"

UTestNetworkMovementHelper::UTestNetworkMovementHelper()
    : OwnerCharacter(NULL)
    , OwnerMovementComponent(NULL)
    , AssociatedMovementMode(ETestNetworkMovementMode::MOVE_None)
{
}

UTestNetworkMovementHelper::UTestNetworkMovementHelper(ETestNetworkMovementMode MovementMode)
    : OwnerCharacter(NULL)
    , OwnerMovementComponent(NULL)
    , AssociatedMovementMode(MovementMode)
{
}

UTestNetworkMovementHelper::~UTestNetworkMovementHelper()
{
    OwnerCharacter = NULL;
    OwnerMovementComponent = NULL;
}

void UTestNetworkMovementHelper::Initialize(ATestNetworkCharacter* Character, UTestNetworkCharacterMovementComponent* MovementComponent)
{
    check(Character && MovementComponent);
    OwnerCharacter = Character;
    OwnerMovementComponent = MovementComponent;
}

void UTestNetworkMovementHelper::CheckInput(float DeltaTime)
{
}

void UTestNetworkMovementHelper::Tick(float DeltaTime)
{
}

void UTestNetworkMovementHelper::PhysCustom(float DeltaTime, int32 Iterations)
{
}

void UTestNetworkMovementHelper::ClearInput()
{
}

ETestNetworkMovementMode UTestNetworkMovementHelper::GetAssociatedMovementMode() const
{
    return AssociatedMovementMode;
}

void UTestNetworkMovementHelper::OnMovementModeChanged(EMovementMode PreviousMovementMode, EMovementMode ActualMovementMode, ETestNetworkMovementMode PreviousCustomMode, ETestNetworkMovementMode ActualCustomMode)
{
}

void UTestNetworkMovementHelper::SetDefaultMovementMode()
{
    OwnerMovementComponent->SetDefaultMovementMode();
}

void UTestNetworkMovementHelper::SetMovementMode()
{
    OwnerMovementComponent->SetMovementMode(MOVE_Custom, static_cast<uint8>(AssociatedMovementMode));
}

void UTestNetworkMovementHelper::SetMovementMode(EMovementMode MovementMode, ETestNetworkMovementMode CustomMovementMode)
{
    OwnerMovementComponent->SetMovementMode(MovementMode, static_cast<uint8>(CustomMovementMode));
}

bool UTestNetworkMovementHelper::IsMovementMode() const
{
    return OwnerMovementComponent->MovementMode == EMovementMode::MOVE_Custom && OwnerMovementComponent->CustomMovementMode == static_cast<uint8>(AssociatedMovementMode);
}