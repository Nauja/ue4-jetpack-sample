#pragma once

#include "TestNetworkMovementHelper.h"
#include "TestNetworkJetpackHelper.generated.h"

UCLASS(ClassGroup = "TestNetwork")
class TESTNETWORK_API UTestNetworkJetpackHelper : public UTestNetworkMovementHelper
{
    GENERATED_BODY()

public:
    UTestNetworkJetpackHelper();
    virtual ~UTestNetworkJetpackHelper();

    virtual void CheckInput(float DeltaTime) override;
    virtual void PhysCustom(float DeltaTime, int32 Iterations) override;
    virtual void ClearInput() override;

    virtual void OnMovementModeChanged(EMovementMode PreviousMovementMode, EMovementMode ActualMovementMode, ETestNetworkMovementMode PreviousCustomMode, ETestNetworkMovementMode ActualCustomMode) override;

    void RequestJetpack();
    void RequestStopJetpacking();

    bool bPressedJetpack;
    int JetpackToggle;

    float JetpackDistance;

    float JetpackGotoDistanceSpeed;

    float JetpackSpeed;

private:
    bool CanJetpack();

    bool GetFloorLocation(FVector& Location);
};

