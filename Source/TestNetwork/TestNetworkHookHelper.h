#pragma once

#include "TestNetworkMovementHelper.h"
#include "TestNetworkHookHelper.generated.h"

UCLASS(ClassGroup = "TestNetwork")
class TESTNETWORK_API UTestNetworkHookHelper : public UTestNetworkMovementHelper
{
    GENERATED_BODY()

public:
    UTestNetworkHookHelper();
    virtual ~UTestNetworkHookHelper();

    virtual void CheckInput(float DeltaTime) override;
    virtual void PhysCustom(float DeltaTime, int32 Iterations);
    virtual void ClearInput() override;
    
    void RequestHook();
    void RequestStopHooking();

    bool bPressedHook;

    FVector HookLocation;

    float HookSpeed;

    float HookOkDistance;
    
private:
    bool CanHook();

    bool GetPointedLocation(FVector& Location);
};

