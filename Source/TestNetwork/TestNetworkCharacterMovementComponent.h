#pragma once

#include "GameFramework/CharacterMovementComponent.h"
#include "TestNetworkCharacterMovementComponent.generated.h"

enum class ETestNetworkMovementMode : uint8
{
    MOVE_None,
    MOVE_Hooking,
    MOVE_Jetpacking
};

USTRUCT()
struct TESTNETWORK_API FSavedMove_CustomData
{
    GENERATED_USTRUCT_BODY()

public:
    UPROPERTY()
    FVector2D PlayerInput;
};

UCLASS(ClassGroup = "TestNetwork")
class TESTNETWORK_API UTestNetworkCharacterMovementComponent : public UCharacterMovementComponent
{
    GENERATED_BODY()

public:
    UTestNetworkCharacterMovementComponent(const FObjectInitializer& ObjectInitializer);

    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
    virtual void PhysCustom(float deltaTime, int32 Iterations) override;
    virtual void OnMovementModeChanged(EMovementMode PreviousMovementMode, uint8 PreviousCustomMode);

    UPROPERTY()
    class UTestNetworkHookHelper* HookHelper;

    UPROPERTY()
    class UTestNetworkJetpackHelper* JetpackHelper;

    UPROPERTY(EditDefaultsOnly, Category = "Test Network | Hook")
    float HookSpeed;

    UPROPERTY(EditDefaultsOnly, Category = "Test Network | Hook")
    float HookOkDistance;

    UPROPERTY(EditDefaultsOnly, Category = "Test Network | Jetpack")
    float JetpackDistance;

    UPROPERTY(EditDefaultsOnly, Category = "Test Network | Jetpack")
    float JetpackGotoDistanceSpeed;

    UPROPERTY(EditDefaultsOnly, Category = "Test Network | Jetpack")
    float JetpackSpeed;

    UPROPERTY()
    FVector2D PlayerInput;

private:
    UPROPERTY()
    TArray<class UTestNetworkMovementHelper*> Helpers;

public:
	//=========================================
	// Networking

	// Replace unreal FNetworkPredictionData_Client by the custom one.
	virtual class FNetworkPredictionData_Client* GetPredictionData_Client() const override;

	/** If bUpdatePosition is true, then replay any unacked moves. Returns whether any moves were actually replayed. */
	virtual bool ClientUpdatePositionAfterServerUpdate() override;

	// Set character custom inputs based on the given flags.
	virtual void UpdateFromCompressedFlags(uint8 Flags) override;
	virtual void UpdateFromCustomData(const FSavedMove_CustomData& CustomData);
	
	virtual void TestNetworkMoveAutonomous(float ClientTimeStamp, float DeltaTime, uint8 CompressedFlags, const FSavedMove_CustomData& CustomData, const FVector& NewAccel);

	/** Call the appropriate replicated servermove() function to send a client player move to the server. */
	virtual void CallServerMove(const class FSavedMove_Character* NewMove, const class FSavedMove_Character* OldMove) override;

    void EngineTickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction);

	////////////////////////////////////
	// Network RPCs for movement
	////////////////////////////////////

	/** Replicated function sent by client to server - contains client movement and view info. */
	UFUNCTION(unreliable, server, WithValidation)
	virtual void TestNetworkServerMove(float TimeStamp, FVector_NetQuantize10 InAccel, FVector_NetQuantize100 ClientLoc, uint8 CompressedMoveFlags, FSavedMove_CustomData ClientCustomData, uint8 ClientRoll, uint32 View, UPrimitiveComponent* ClientMovementBase, FName ClientBaseBoneName, uint8 ClientMovementMode);
	virtual void TestNetworkServerMove_Implementation(float TimeStamp, FVector_NetQuantize10 InAccel, FVector_NetQuantize100 ClientLoc, uint8 CompressedMoveFlags, FSavedMove_CustomData ClientCustomData, uint8 ClientRoll, uint32 View, UPrimitiveComponent* ClientMovementBase, FName ClientBaseBoneName, uint8 ClientMovementMode);
	virtual bool TestNetworkServerMove_Validate(float TimeStamp, FVector_NetQuantize10 InAccel, FVector_NetQuantize100 ClientLoc, uint8 CompressedMoveFlags, FSavedMove_CustomData ClientCustomData, uint8 ClientRoll, uint32 View, UPrimitiveComponent* ClientMovementBase, FName ClientBaseBoneName, uint8 ClientMovementMode);

	/** Replicated function sent by client to server - contains client movement and view info for two moves. */
	UFUNCTION(unreliable, server, WithValidation)
	virtual void TestNetworkServerMoveDual(float TimeStamp0, FVector_NetQuantize10 InAccel0, uint8 PendingFlags, FSavedMove_CustomData PendingCustomData,uint32 View0, float TimeStamp, FVector_NetQuantize10 InAccel, FVector_NetQuantize100 ClientLoc, uint8 NewFlags, FSavedMove_CustomData NewCustomData, uint8 ClientRoll, uint32 View, UPrimitiveComponent* ClientMovementBase, FName ClientBaseBoneName, uint8 ClientMovementMode);
	virtual void TestNetworkServerMoveDual_Implementation(float TimeStamp0, FVector_NetQuantize10 InAccel0, uint8 PendingFlags, FSavedMove_CustomData PendingCustomData, uint32 View0, float TimeStamp, FVector_NetQuantize10 InAccel, FVector_NetQuantize100 ClientLoc, uint8 NewFlags, FSavedMove_CustomData NewCustomData, uint8 ClientRoll, uint32 View, UPrimitiveComponent* ClientMovementBase, FName ClientBaseBoneName, uint8 ClientMovementMode);
	virtual bool TestNetworkServerMoveDual_Validate(float TimeStamp0, FVector_NetQuantize10 InAccel0, uint8 PendingFlags, FSavedMove_CustomData PendingCustomData, uint32 View0, float TimeStamp, FVector_NetQuantize10 InAccel, FVector_NetQuantize100 ClientLoc, uint8 NewFlags, FSavedMove_CustomData NewCustomData, uint8 ClientRoll, uint32 View, UPrimitiveComponent* ClientMovementBase, FName ClientBaseBoneName, uint8 ClientMovementMode);

	/** Replicated function sent by client to server - contains client movement and view info for two moves. First move is non root motion, second is root motion. */
	UFUNCTION(unreliable, server, WithValidation)
	virtual void TestNetworkServerMoveDualHybridRootMotion(float TimeStamp0, FVector_NetQuantize10 InAccel0, uint8 PendingFlags, FSavedMove_CustomData PendingCustomData, uint32 View0, float TimeStamp, FVector_NetQuantize10 InAccel, FVector_NetQuantize100 ClientLoc, uint8 NewFlags, FSavedMove_CustomData NewCustomData, uint8 ClientRoll, uint32 View, UPrimitiveComponent* ClientMovementBase, FName ClientBaseBoneName, uint8 ClientMovementMode);
	virtual void TestNetworkServerMoveDualHybridRootMotion_Implementation(float TimeStamp0, FVector_NetQuantize10 InAccel0, uint8 PendingFlags, FSavedMove_CustomData PendingCustomData, uint32 View0, float TimeStamp, FVector_NetQuantize10 InAccel, FVector_NetQuantize100 ClientLoc, uint8 NewFlags, FSavedMove_CustomData NewCustomData, uint8 ClientRoll, uint32 View, UPrimitiveComponent* ClientMovementBase, FName ClientBaseBoneName, uint8 ClientMovementMode);
	virtual bool TestNetworkServerMoveDualHybridRootMotion_Validate(float TimeStamp0, FVector_NetQuantize10 InAccel0, uint8 PendingFlags, FSavedMove_CustomData PendingCustomData, uint32 View0, float TimeStamp, FVector_NetQuantize10 InAccel, FVector_NetQuantize100 ClientLoc, uint8 NewFlags, FSavedMove_CustomData NewCustomData, uint8 ClientRoll, uint32 View, UPrimitiveComponent* ClientMovementBase, FName ClientBaseBoneName, uint8 ClientMovementMode);

	/* Resending an (important) old move. Process it if not already processed. */
	UFUNCTION(unreliable, server, WithValidation)
	virtual void TestNetworkServerMoveOld(float OldTimeStamp, FVector_NetQuantize10 OldAccel, uint8 OldMoveFlags, FSavedMove_CustomData OldCustomData);
	virtual void TestNetworkServerMoveOld_Implementation(float OldTimeStamp, FVector_NetQuantize10 OldAccel, uint8 OldMoveFlags, FSavedMove_CustomData OldCustomData);
	virtual bool TestNetworkServerMoveOld_Validate(float OldTimeStamp, FVector_NetQuantize10 OldAccel, uint8 OldMoveFlags, FSavedMove_CustomData OldCustomData);
};

// Custom FSavedMove_Character used to save custom inputs.
class TESTNETWORK_API FSavedMove_TestNetworkCharacter : public FSavedMove_Character
{
    typedef FSavedMove_Character Super;

public:
    FSavedMove_TestNetworkCharacter();
    virtual ~FSavedMove_TestNetworkCharacter();

    FSavedMove_CustomData CustomData;

    bool bPressedHook;

    bool bPressedJetpack;

    // Clear custom inputs.
    virtual void Clear() override;

    // Set custom inputs based on the given character.
    virtual void SetMoveFor(ACharacter* Character, float InDeltaTime, FVector const& NewAccel, class FNetworkPredictionData_Client_Character & ClientData) override;

    virtual void PrepMoveFor(ACharacter* Character) override;

    virtual void PostUpdate(ACharacter* Character, EPostUpdateMode PostUpdateMode) override;

    // Compress custom inputs in a uint8.
    virtual uint8 GetCompressedFlags() const override;

    // Indicate if this saved move can be combined with the given one based on the custom inputs.
    virtual bool CanCombineWith(const FSavedMovePtr& NewMove, ACharacter* Character, float MaxDelta) const override;

    // Indicate if this saved move is important based on the custom inputs.
    virtual bool IsImportantMove(const FSavedMovePtr& LastAckedMove) const override;

    enum CustomCompressedFlags : uint8
    {
        FLAG_HookPressed = 0x10,
        FLAG_JetpackPressed = 0x20
    };
};

// Custom FNetworkPredictionData_Client_Character used to replace FSavedMove_Character by FSavedMove_TestNetworkCharacter.
class TESTNETWORK_API FNetworkPredictionData_Client_TestNetworkCharacter : public FNetworkPredictionData_Client_Character
{
public:
    FNetworkPredictionData_Client_TestNetworkCharacter(const UCharacterMovementComponent& ClientMovement);
    virtual ~FNetworkPredictionData_Client_TestNetworkCharacter();

    virtual FSavedMovePtr AllocateNewMove() override;
};