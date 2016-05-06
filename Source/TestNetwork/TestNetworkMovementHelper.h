#pragma once

#include "TestNetworkCharacter.h"
#include "TestNetworkCharacterMovementComponent.h"
#include "TestNetworkMovementHelper.generated.h"

UCLASS(Abstract, NotBlueprintable, NotPlaceable, ClassGroup = "TestNetwork")
class TESTNETWORK_API UTestNetworkMovementHelper : public UObject
{
    GENERATED_BODY()

public:
    UTestNetworkMovementHelper(ETestNetworkMovementMode MovementMode);
    virtual ~UTestNetworkMovementHelper();

    virtual void Initialize(ATestNetworkCharacter* Character, UTestNetworkCharacterMovementComponent* MovementComponent);

    virtual void CheckInput(float DeltaTime);
    virtual void Tick(float DeltaTime);
    virtual void PhysCustom(float DeltaTime, int32 Iterations);
    virtual void ClearInput();

    virtual void OnMovementModeChanged(EMovementMode PreviousMovementMode, EMovementMode ActualMovementMode, ETestNetworkMovementMode PreviousCustomMode, ETestNetworkMovementMode ActualCustomMode);

    ETestNetworkMovementMode GetAssociatedMovementMode() const;

protected:
    void SetDefaultMovementMode();
    void SetMovementMode();
    void SetMovementMode(EMovementMode MovementMode, ETestNetworkMovementMode CustomMovementMode = ETestNetworkMovementMode::MOVE_None);
    bool IsMovementMode() const;

    ATestNetworkCharacter* OwnerCharacter;
    UTestNetworkCharacterMovementComponent* OwnerMovementComponent;
    ETestNetworkMovementMode AssociatedMovementMode;

private:
    UTestNetworkMovementHelper();
};

