#include "TestNetwork.h"
#include "TestNetworkCharacter.h"
#include "TestNetworkCharacterMovementComponent.h"
#include "TestNetworkMovementHelper.h"
#include "TestNetworkHookHelper.h"
#include "TestNetworkJetpackHelper.h"
#include "GameFramework/GameNetworkManager.h"
#include "GameFramework/WorldSettings.h"

//======================================================
// Networking Support

bool UTestNetworkCharacterMovementComponent::ClientUpdatePositionAfterServerUpdate()
{
    if (!HasValidData())
    {
        return false;
    }

    FNetworkPredictionData_Client_Character* ClientData = GetPredictionData_Client_Character();
    check(ClientData);

    if (!ClientData->bUpdatePosition)
    {
        return false;
    }

    if (bIgnoreClientMovementErrorChecksAndCorrection)
    {
        return false;
    }

    ClientData->bUpdatePosition = false;

    // Don't do any network position updates on things running PHYS_RigidBody
    if (CharacterOwner->GetRootComponent() && CharacterOwner->GetRootComponent()->IsSimulatingPhysics())
    {
        return false;
    }

    if (ClientData->SavedMoves.Num() == 0)
    {
        UE_LOG(LogNetPlayerMovement, VeryVerbose, TEXT("ClientUpdatePositionAfterServerUpdate No saved moves to replay"), ClientData->SavedMoves.Num());
        return false;
    }

    // Save important values that might get affected by the replay.
    const float SavedAnalogInputModifier = AnalogInputModifier;
    const FRootMotionMovementParams BackupRootMotionParams = RootMotionParams; // For animation root motion
    const FRootMotionSourceGroup BackupRootMotion = CurrentRootMotion;
    const bool bRealJump = CharacterOwner->bPressedJump;
    const bool bRealCrouch = bWantsToCrouch;
    const bool bRealForceMaxAccel = bForceMaxAccel;
    CharacterOwner->bClientWasFalling = (MovementMode == MOVE_Falling);
    CharacterOwner->bClientUpdating = true;
    bForceNextFloorCheck = true;

    const bool bRealHook = HookHelper->bPressedHook;
    const bool bRealJetpack = JetpackHelper->bPressedJetpack;
    const FVector2D bSavedPlayerInput = PlayerInput;

    // Replay moves that have not yet been acked.
    UE_LOG(LogNetPlayerMovement, VeryVerbose, TEXT("ClientUpdatePositionAfterServerUpdate Replaying %d Moves, starting at Timestamp %f"), ClientData->SavedMoves.Num(), ClientData->SavedMoves[0]->TimeStamp);
    for (int32 i = 0; i<ClientData->SavedMoves.Num(); i++)
    {
        const FSavedMovePtr& CurrentMove = ClientData->SavedMoves[i];
        CurrentMove->PrepMoveFor(CharacterOwner);
        TestNetworkMoveAutonomous(CurrentMove->TimeStamp, CurrentMove->DeltaTime, CurrentMove->GetCompressedFlags(), ((const FSavedMove_TestNetworkCharacter&)*CurrentMove).CustomData, CurrentMove->Acceleration);
        CurrentMove->PostUpdate(CharacterOwner, FSavedMove_Character::PostUpdate_Replay);
    }

    if (ClientData->PendingMove.IsValid())
    {
        ClientData->PendingMove->bForceNoCombine = true;
    }

    // Restore saved values.
    AnalogInputModifier = SavedAnalogInputModifier;
    RootMotionParams = BackupRootMotionParams;
    CurrentRootMotion = BackupRootMotion;
    if (CharacterOwner->bClientResimulateRootMotionSources)
    {
        // If we were resimulating root motion sources, it's because we had mismatched state
        // with the server - we just resimulated our SavedMoves and now need to restore
        // CurrentRootMotion with the latest "good state"
        UE_LOG(LogRootMotion, VeryVerbose, TEXT("CurrentRootMotion getting updated after ServerUpdate replays: %s"), *CharacterOwner->GetName());
        CurrentRootMotion.UpdateStateFrom(CharacterOwner->SavedRootMotion);
        CharacterOwner->bClientResimulateRootMotionSources = false;
    }
    CharacterOwner->SavedRootMotion.Clear();
    CharacterOwner->bClientResimulateRootMotion = false;
    CharacterOwner->bClientUpdating = false;
    CharacterOwner->bPressedJump = bRealJump;
    bWantsToCrouch = bRealCrouch;
    bForceMaxAccel = bRealForceMaxAccel;
    bForceNextFloorCheck = true;

    HookHelper->bPressedHook = bRealHook;
    JetpackHelper->bPressedJetpack = bRealJetpack;
    PlayerInput = bSavedPlayerInput;

    return (ClientData->SavedMoves.Num() > 0);
}

void UTestNetworkCharacterMovementComponent::UpdateFromCompressedFlags(uint8 Flags)
{
    Super::UpdateFromCompressedFlags(Flags);
    
    HookHelper->bPressedHook = ((Flags & FSavedMove_TestNetworkCharacter::FLAG_HookPressed) != 0);
    JetpackHelper->bPressedJetpack = ((Flags & FSavedMove_TestNetworkCharacter::FLAG_JetpackPressed) != 0);
}

void UTestNetworkCharacterMovementComponent::UpdateFromCustomData(const FSavedMove_CustomData& CustomData)
{
    // CustomData -> Character
    PlayerInput = CustomData.PlayerInput;
}

/////////////// region FSavedMove_TestNetworkCharacter ///////////////

FSavedMove_TestNetworkCharacter::FSavedMove_TestNetworkCharacter()
{
}

FSavedMove_TestNetworkCharacter::~FSavedMove_TestNetworkCharacter()
{
}

void FSavedMove_TestNetworkCharacter::Clear()
{
    Super::Clear();

    bPressedHook = false;
    bPressedJetpack = false;
    CustomData.PlayerInput = FVector2D::ZeroVector;
}

void FSavedMove_TestNetworkCharacter::SetMoveFor(ACharacter* Character, float InDeltaTime, FVector const& NewAccel, class FNetworkPredictionData_Client_Character & ClientData)
{
    Super::SetMoveFor(Character, InDeltaTime, NewAccel, ClientData);

    // Character -> Save
    UTestNetworkCharacterMovementComponent* MoveComponent = Cast<UTestNetworkCharacterMovementComponent>(Character->GetMovementComponent());

    bPressedHook = MoveComponent->HookHelper->bPressedHook;
    bPressedJetpack = MoveComponent->JetpackHelper->bPressedJetpack;
    CustomData.PlayerInput = MoveComponent->PlayerInput;
}

void FSavedMove_TestNetworkCharacter::PrepMoveFor(ACharacter* Character)
{
    Super::PrepMoveFor(Character);

    // Save -> Character
}

void FSavedMove_TestNetworkCharacter::PostUpdate(ACharacter* Character, FSavedMove_Character::EPostUpdateMode PostUpdateMode)
{
    Super::PostUpdate(Character, PostUpdateMode);
}

uint8 FSavedMove_TestNetworkCharacter::GetCompressedFlags() const
{
    uint8 Result = Super::GetCompressedFlags();

    if (bPressedHook)
    {
        Result |= FLAG_HookPressed;
    }

    if (bPressedJetpack)
    {
        Result |= FLAG_JetpackPressed;
    }

    return Result;
}

bool FSavedMove_TestNetworkCharacter::CanCombineWith(const FSavedMovePtr& NewMove, ACharacter* Character, float MaxDelta) const
{
    const FSavedMove_TestNetworkCharacter* TestNetworkNewMove = (FSavedMove_TestNetworkCharacter*)&NewMove;

    if (bPressedHook != TestNetworkNewMove->bPressedHook)
    {
        return false;
    }

    if (bPressedJetpack != TestNetworkNewMove->bPressedJetpack)
    {
        return false;
    }

    if (CustomData.PlayerInput != TestNetworkNewMove->CustomData.PlayerInput)
    {
        return false;
    }

    return Super::CanCombineWith(NewMove, Character, MaxDelta);
}

bool FSavedMove_TestNetworkCharacter::IsImportantMove(const FSavedMovePtr& LastAckedMove) const
{
    const FSavedMove_TestNetworkCharacter* TestNetworkLastAckedMove = (FSavedMove_TestNetworkCharacter*)&LastAckedMove;

    if (bPressedHook != TestNetworkLastAckedMove->bPressedHook)
    {
        return true;
    }

    if (bPressedJetpack != TestNetworkLastAckedMove->bPressedJetpack)
    {
        return true;
    }

    if (CustomData.PlayerInput != TestNetworkLastAckedMove->CustomData.PlayerInput)
    {
        return true;
    }

    return Super::IsImportantMove(LastAckedMove);
}

/////////////// region FNetworkPredictionData_Client_TestNetworkCharacter ///////////////

FNetworkPredictionData_Client_TestNetworkCharacter::FNetworkPredictionData_Client_TestNetworkCharacter(const UCharacterMovementComponent& ClientMovement)
    : FNetworkPredictionData_Client_Character(ClientMovement)
{
}

FNetworkPredictionData_Client_TestNetworkCharacter::~FNetworkPredictionData_Client_TestNetworkCharacter()
{
}

FSavedMovePtr FNetworkPredictionData_Client_TestNetworkCharacter::AllocateNewMove()
{
    return FSavedMovePtr(new FSavedMove_TestNetworkCharacter());
}

FNetworkPredictionData_Client* UTestNetworkCharacterMovementComponent::GetPredictionData_Client() const
{
    // Should only be called on client in network games
    check(CharacterOwner != NULL);
    check(CharacterOwner->Role < ROLE_Authority);
    checkSlow(GetNetMode() == NM_Client);

    if (!ClientPredictionData)
    {
        UTestNetworkCharacterMovementComponent* MutableThis = const_cast<UTestNetworkCharacterMovementComponent*>(this);
        MutableThis->ClientPredictionData = new FNetworkPredictionData_Client_TestNetworkCharacter(*this);
    }

    return ClientPredictionData;
}

//

void UTestNetworkCharacterMovementComponent::TestNetworkMoveAutonomous
(
    float ClientTimeStamp,
    float DeltaTime,
    uint8 CompressedFlags,
    const FSavedMove_CustomData& CustomData,
    const FVector& NewAccel
    )
{
    if (!HasValidData())
    {
        return;
    }

    UpdateFromCompressedFlags(CompressedFlags);
    UpdateFromCustomData(CustomData);
    CharacterOwner->CheckJumpInput(DeltaTime);

    for (auto& Helper : Helpers)
    {
        Helper->CheckInput(DeltaTime);
    }

    Acceleration = ConstrainInputAcceleration(NewAccel);
    Acceleration = Acceleration.GetClampedToMaxSize(GetMaxAcceleration());
    AnalogInputModifier = ComputeAnalogInputModifier();

    PerformMovement(DeltaTime);

    // Check if data is valid as PerformMovement can mark character for pending kill
    if (!HasValidData())
    {
        return;
    }

    // If not playing root motion, tick animations after physics. We do this here to keep events, notifies, states and transitions in sync with client updates.
    if (!CharacterOwner->bClientUpdating && !CharacterOwner->IsPlayingRootMotion() && CharacterOwner->GetMesh())
    {
        TickCharacterPose(DeltaTime);
        // TODO: SaveBaseLocation() in case tick moves us?
    }
}

void UTestNetworkCharacterMovementComponent::CallServerMove(const FSavedMove_Character* NewMove, const FSavedMove_Character* OldMove)
{
    check(NewMove != NULL);

    // Compress rotation down to 5 bytes
    const uint32 ClientYawPitchINT = PackYawAndPitchTo32(NewMove->SavedControlRotation.Yaw, NewMove->SavedControlRotation.Pitch);
    const uint8 ClientRollBYTE = FRotator::CompressAxisToByte(NewMove->SavedControlRotation.Roll);

    // Determine if we send absolute or relative location
    UPrimitiveComponent* ClientMovementBase = NewMove->EndBase.Get();
    const FName ClientBaseBone = NewMove->EndBoneName;
    const FVector SendLocation = MovementBaseUtility::UseRelativeLocation(ClientMovementBase) ? NewMove->SavedRelativeLocation : NewMove->SavedLocation;

    // send old move if it exists
    if (OldMove)
    {
        TestNetworkServerMoveOld(OldMove->TimeStamp, OldMove->Acceleration, OldMove->GetCompressedFlags(), ((const FSavedMove_TestNetworkCharacter*)OldMove)->CustomData);
    }

    FNetworkPredictionData_Client_Character* ClientData = GetPredictionData_Client_Character();
    if (ClientData->PendingMove.IsValid())
    {
        const uint32 OldClientYawPitchINT = PackYawAndPitchTo32(ClientData->PendingMove->SavedControlRotation.Yaw, ClientData->PendingMove->SavedControlRotation.Pitch);

        // If we delayed a move without root motion, and our new move has root motion, send these through a special function, so the server knows how to process them.
        if ((ClientData->PendingMove->RootMotionMontage == NULL) && (NewMove->RootMotionMontage != NULL))
        {
            // send two moves simultaneously
            TestNetworkServerMoveDualHybridRootMotion
                (
                    ClientData->PendingMove->TimeStamp,
                    ClientData->PendingMove->Acceleration,
                    ClientData->PendingMove->GetCompressedFlags(),
                    ((FSavedMove_TestNetworkCharacter&)*ClientData->PendingMove).CustomData,
                    OldClientYawPitchINT,
                    NewMove->TimeStamp,
                    NewMove->Acceleration,
                    SendLocation,
                    NewMove->GetCompressedFlags(),
                    ((const FSavedMove_TestNetworkCharacter*)NewMove)->CustomData,
                    ClientRollBYTE,
                    ClientYawPitchINT,
                    ClientMovementBase,
                    ClientBaseBone,
                    NewMove->MovementMode
                    );
        }
        else
        {
            // send two moves simultaneously
            TestNetworkServerMoveDual
                (
                    ClientData->PendingMove->TimeStamp,
                    ClientData->PendingMove->Acceleration,
                    ClientData->PendingMove->GetCompressedFlags(),
                    ((FSavedMove_TestNetworkCharacter&)*ClientData->PendingMove).CustomData,
                    OldClientYawPitchINT,
                    NewMove->TimeStamp,
                    NewMove->Acceleration,
                    SendLocation,
                    NewMove->GetCompressedFlags(),
                    ((const FSavedMove_TestNetworkCharacter*)NewMove)->CustomData,
                    ClientRollBYTE,
                    ClientYawPitchINT,
                    ClientMovementBase,
                    ClientBaseBone,
                    NewMove->MovementMode
                    );
        }
    }
    else
    {
        TestNetworkServerMove
            (
                NewMove->TimeStamp,
                NewMove->Acceleration,
                SendLocation,
                NewMove->GetCompressedFlags(),
                ((const FSavedMove_TestNetworkCharacter*)NewMove)->CustomData,
                ClientRollBYTE,
                ClientYawPitchINT,
                ClientMovementBase,
                ClientBaseBone,
                NewMove->MovementMode
                );
    }


    APlayerController* PC = Cast<APlayerController>(CharacterOwner->GetController());
    APlayerCameraManager* PlayerCameraManager = (PC ? PC->PlayerCameraManager : NULL);
    if (PlayerCameraManager != NULL && PlayerCameraManager->bUseClientSideCameraUpdates)
    {
        PlayerCameraManager->bShouldSendClientSideCameraUpdate = true;
    }
}

void UTestNetworkCharacterMovementComponent::TestNetworkServerMove_Implementation(
    float TimeStamp,
    FVector_NetQuantize10 InAccel,
    FVector_NetQuantize100 ClientLoc,
    uint8 MoveFlags,
    FSavedMove_CustomData ClientCustomData,
    uint8 ClientRoll,
    uint32 View,
    UPrimitiveComponent* ClientMovementBase,
    FName ClientBaseBoneName,
    uint8 ClientMovementMode)
{
    if (!HasValidData() || !IsComponentTickEnabled())
    {
        return;
    }

    FNetworkPredictionData_Server_Character* ServerData = GetPredictionData_Server_Character();
    check(ServerData);

    if (!VerifyClientTimeStamp(TimeStamp, *ServerData))
    {
        return;
    }

    bool bServerReadyForClient = true;
    APlayerController* PC = Cast<APlayerController>(CharacterOwner->GetController());
    if (PC)
    {
        bServerReadyForClient = PC->NotifyServerReceivedClientData(CharacterOwner, TimeStamp);
        if (!bServerReadyForClient)
        {
            InAccel = FVector::ZeroVector;
        }
    }

    // View components
    const uint16 ViewPitch = (View & 65535);
    const uint16 ViewYaw = (View >> 16);

    const FVector Accel = InAccel;
    // Save move parameters.
    const float DeltaTime = ServerData->GetServerMoveDeltaTime(TimeStamp) * CharacterOwner->CustomTimeDilation;

    ServerData->CurrentClientTimeStamp = TimeStamp;
    ServerData->ServerTimeStamp = GetWorld()->TimeSeconds;
    FRotator ViewRot;
    ViewRot.Pitch = FRotator::DecompressAxisFromShort(ViewPitch);
    ViewRot.Yaw = FRotator::DecompressAxisFromShort(ViewYaw);
    ViewRot.Roll = FRotator::DecompressAxisFromByte(ClientRoll);

    if (PC)
    {
        PC->SetControlRotation(ViewRot);
    }

    if (!bServerReadyForClient)
    {
        return;
    }

    // Perform actual movement
    if ((CharacterOwner->GetWorldSettings()->Pauser == NULL) && (DeltaTime > 0.f))
    {
        if (PC)
        {
            PC->UpdateRotation(DeltaTime);
        }

        TestNetworkMoveAutonomous(TimeStamp, DeltaTime, MoveFlags, ClientCustomData, Accel);
    }

    UE_LOG(LogNetPlayerMovement, Verbose, TEXT("ServerMove Time %f Acceleration %s Position %s DeltaTime %f"),
        TimeStamp, *Accel.ToString(), *UpdatedComponent->GetComponentLocation().ToString(), DeltaTime);

    ServerMoveHandleClientError(TimeStamp, DeltaTime, Accel, ClientLoc, ClientMovementBase, ClientBaseBoneName, ClientMovementMode);
}

void UTestNetworkCharacterMovementComponent::TestNetworkServerMoveOld_Implementation(
    float OldTimeStamp,
    FVector_NetQuantize10 OldAccel,
    uint8 OldMoveFlags,
    FSavedMove_CustomData OldCustomData)
{
    if (!HasValidData() || !IsComponentTickEnabled())
    {
        return;
    }

    FNetworkPredictionData_Server_Character* ServerData = GetPredictionData_Server_Character();
    check(ServerData);

    if (!VerifyClientTimeStamp(OldTimeStamp, *ServerData))
    {
        return;
    }

    UE_LOG(LogNetPlayerMovement, Log, TEXT("Recovered move from OldTimeStamp %f, DeltaTime: %f"), OldTimeStamp, OldTimeStamp - ServerData->CurrentClientTimeStamp);
    const float MaxResponseTime = ServerData->MaxResponseTime * CharacterOwner->GetWorldSettings()->GetEffectiveTimeDilation();

    TestNetworkMoveAutonomous(OldTimeStamp, FMath::Min(OldTimeStamp - ServerData->CurrentClientTimeStamp, MaxResponseTime), OldMoveFlags, OldCustomData, OldAccel);

    ServerData->CurrentClientTimeStamp = OldTimeStamp;
}


void UTestNetworkCharacterMovementComponent::TestNetworkServerMoveDual_Implementation(
    float TimeStamp0,
    FVector_NetQuantize10 InAccel0,
    uint8 PendingFlags,
    FSavedMove_CustomData PendingCustomData,
    uint32 View0,
    float TimeStamp,
    FVector_NetQuantize10 InAccel,
    FVector_NetQuantize100 ClientLoc,
    uint8 NewFlags,
    FSavedMove_CustomData NewCustomData,
    uint8 ClientRoll,
    uint32 View,
    UPrimitiveComponent* ClientMovementBase,
    FName ClientBaseBone,
    uint8 ClientMovementMode)
{
    TestNetworkServerMove_Implementation(TimeStamp0, InAccel0, FVector(1.f, 2.f, 3.f), PendingFlags, PendingCustomData, ClientRoll, View0, ClientMovementBase, ClientBaseBone, ClientMovementMode);
    TestNetworkServerMove_Implementation(TimeStamp, InAccel, ClientLoc, NewFlags, NewCustomData, ClientRoll, View, ClientMovementBase, ClientBaseBone, ClientMovementMode);
}

void UTestNetworkCharacterMovementComponent::TestNetworkServerMoveDualHybridRootMotion_Implementation(
    float TimeStamp0,
    FVector_NetQuantize10 InAccel0,
    uint8 PendingFlags,
    FSavedMove_CustomData PendingCustomData,
    uint32 View0,
    float TimeStamp,
    FVector_NetQuantize10 InAccel,
    FVector_NetQuantize100 ClientLoc,
    uint8 NewFlags,
    FSavedMove_CustomData NewCustomData,
    uint8 ClientRoll,
    uint32 View,
    UPrimitiveComponent* ClientMovementBase,
    FName ClientBaseBone,
    uint8 ClientMovementMode)
{
    // First move received didn't use root motion, process it as such.
    CharacterOwner->bServerMoveIgnoreRootMotion = CharacterOwner->IsPlayingNetworkedRootMotionMontage();
    TestNetworkServerMove_Implementation(TimeStamp0, InAccel0, FVector(1.f, 2.f, 3.f), PendingFlags, PendingCustomData, ClientRoll, View0, ClientMovementBase, ClientBaseBone, ClientMovementMode);
    CharacterOwner->bServerMoveIgnoreRootMotion = false;

    TestNetworkServerMove_Implementation(TimeStamp, InAccel, ClientLoc, NewFlags, NewCustomData, ClientRoll, View, ClientMovementBase, ClientBaseBone, ClientMovementMode);
}

bool UTestNetworkCharacterMovementComponent::TestNetworkServerMove_Validate(float TimeStamp, FVector_NetQuantize10 InAccel, FVector_NetQuantize100 ClientLoc, uint8 MoveFlags, FSavedMove_CustomData ClientCustomData, uint8 ClientRoll, uint32 View, UPrimitiveComponent* ClientMovementBase, FName ClientBaseBoneName, uint8 ClientMovementMode)
{
    return true;
}

bool UTestNetworkCharacterMovementComponent::TestNetworkServerMoveDual_Validate(float TimeStamp0, FVector_NetQuantize10 InAccel0, uint8 PendingFlags, FSavedMove_CustomData PendingCustomData, uint32 View0, float TimeStamp, FVector_NetQuantize10 InAccel, FVector_NetQuantize100 ClientLoc, uint8 NewFlags, FSavedMove_CustomData NewCustomData, uint8 ClientRoll, uint32 View, UPrimitiveComponent* ClientMovementBase, FName ClientBaseBoneName, uint8 ClientMovementMode)
{
    return true;
}

bool UTestNetworkCharacterMovementComponent::TestNetworkServerMoveDualHybridRootMotion_Validate(float TimeStamp0, FVector_NetQuantize10 InAccel0, uint8 PendingFlags, FSavedMove_CustomData PendingCustomData, uint32 View0, float TimeStamp, FVector_NetQuantize10 InAccel, FVector_NetQuantize100 ClientLoc, uint8 NewFlags, FSavedMove_CustomData NewCustomData, uint8 ClientRoll, uint32 View, UPrimitiveComponent* ClientMovementBase, FName ClientBaseBoneName, uint8 ClientMovementMode)
{
    return true;
}

bool UTestNetworkCharacterMovementComponent::TestNetworkServerMoveOld_Validate(float OldTimeStamp, FVector_NetQuantize10 OldAccel, uint8 OldMoveFlags, FSavedMove_CustomData OldCustomData)
{
    return true;
}

void UTestNetworkCharacterMovementComponent::TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    EngineTickComponent(DeltaTime, TickType, ThisTickFunction);
}

void UTestNetworkCharacterMovementComponent::EngineTickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    const FVector InputVector = ConsumeInputVector();
    if (!HasValidData() || ShouldSkipUpdate(DeltaTime))
    {
        return;
    }

    UMovementComponent::TickComponent(DeltaTime, TickType, ThisTickFunction);

    // Super tick may destroy/invalidate CharacterOwner or UpdatedComponent, so we need to re-check.
    if (!HasValidData())
    {
        return;
    }

    // See if we fell out of the world.
    const bool bIsSimulatingPhysics = UpdatedComponent->IsSimulatingPhysics();
    if (CharacterOwner->Role == ROLE_Authority && (!bCheatFlying || bIsSimulatingPhysics) && !CharacterOwner->CheckStillInWorld())
    {
        return;
    }

    // We don't update if simulating physics (eg ragdolls).
    if (bIsSimulatingPhysics)
    {
        return;
    }

    AvoidanceLockTimer -= DeltaTime;

    if (CharacterOwner->Role > ROLE_SimulatedProxy)
    {
        // If we are a client we might have received an update from the server.
        const bool bIsClient = (CharacterOwner->Role == ROLE_AutonomousProxy && GetNetMode() == NM_Client);
        if (bIsClient)
        {
            ClientUpdatePositionAfterServerUpdate();
        }

        // Allow root motion to move characters that have no controller.
        if (CharacterOwner->IsLocallyControlled() || (!CharacterOwner->Controller && bRunPhysicsWithNoController) || (!CharacterOwner->Controller && CharacterOwner->IsPlayingRootMotion()))
        {
            {
                // We need to check the jump state before adjusting input acceleration, to minimize latency
                // and to make sure acceleration respects our potentially new falling state.
                CharacterOwner->CheckJumpInput(DeltaTime);

                for (auto& Helper : Helpers)
                {
                    Helper->CheckInput(DeltaTime);
                }

                // apply input to acceleration
                Acceleration = ScaleInputAcceleration(ConstrainInputAcceleration(InputVector));
                AnalogInputModifier = ComputeAnalogInputModifier();
            }

            if (CharacterOwner->Role == ROLE_Authority)
            {
                PerformMovement(DeltaTime);
            }
            else if (bIsClient)
            {
                ReplicateMoveToServer(DeltaTime, Acceleration);
            }
        }
        else if (CharacterOwner->GetRemoteRole() == ROLE_AutonomousProxy)
        {
            // Server ticking for remote client.
            // Between net updates from the client we need to update position if based on another object,
            // otherwise the object will move on intermediate frames and we won't follow it.
            MaybeUpdateBasedMovement(DeltaTime);
            MaybeSaveBaseLocation();
        }
    }
    else if (CharacterOwner->Role == ROLE_SimulatedProxy)
    {
        if (bShrinkProxyCapsule)
        {
            AdjustProxyCapsuleSize();
        }
        SimulatedTick(DeltaTime);
    }

    if (bUseRVOAvoidance)
    {
        UpdateDefaultAvoidance();
    }

    if (bEnablePhysicsInteraction)
    {
        ApplyDownwardForce(DeltaTime);
        ApplyRepulsionForce(DeltaTime);
    }
}
