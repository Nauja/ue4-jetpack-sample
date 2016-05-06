#include "TestNetwork.h"
#include "TestNetworkCharacterMovementComponent.h"
#include "TestNetworkHookHelper.h"
#include "TestNetworkJetpackHelper.h"

UTestNetworkCharacterMovementComponent::UTestNetworkCharacterMovementComponent(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
{
    bAutoRegister = true;
    bAutoActivate = true;
    bWantsInitializeComponent = true;
    SetIsReplicated(true);

    NetworkSmoothingMode = ENetworkSmoothingMode::Exponential;
}

void UTestNetworkCharacterMovementComponent::BeginPlay()
{
    Super::BeginPlay();

    Helpers.Empty();

    HookHelper = NewObject<UTestNetworkHookHelper>(this);
    HookHelper->Initialize(Cast<ATestNetworkCharacter>(PawnOwner), this);
    HookHelper->HookSpeed = HookSpeed;
    HookHelper->HookOkDistance = HookOkDistance;
    Helpers.Add(HookHelper);

    JetpackHelper = NewObject<UTestNetworkJetpackHelper>(this);
    JetpackHelper->Initialize(Cast<ATestNetworkCharacter>(PawnOwner), this);
    JetpackHelper->JetpackDistance = JetpackDistance;
    JetpackHelper->JetpackGotoDistanceSpeed = JetpackGotoDistanceSpeed;
    JetpackHelper->JetpackSpeed = JetpackSpeed;
    Helpers.Add(JetpackHelper);
}

void UTestNetworkCharacterMovementComponent::PhysCustom(float deltaTime, int32 Iterations)
{
    Super::PhysCustom(deltaTime, Iterations);
    for (auto& Helper : Helpers)
    {
        if (static_cast<uint8>(Helper->GetAssociatedMovementMode()) == CustomMovementMode)
        {
            Helper->PhysCustom(deltaTime, Iterations);
            break;
        }
    }
}

void UTestNetworkCharacterMovementComponent::OnMovementModeChanged(EMovementMode PreviousMovementMode, uint8 PreviousCustomMode)
{
    Super::OnMovementModeChanged(PreviousMovementMode, PreviousCustomMode);
    for (auto& Helper : Helpers)
    {
        Helper->OnMovementModeChanged(PreviousMovementMode, MovementMode, (ETestNetworkMovementMode)PreviousCustomMode, (ETestNetworkMovementMode)CustomMovementMode);
    }
}