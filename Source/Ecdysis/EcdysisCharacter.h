// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/TimelineComponent.h"
#include "GameFramework/Character.h"
#include "BaseWeapon.h"
#include "EcdysisCharacter.generated.h"


class UCurveFloat;
class UInputComponent;
class USkeletalMeshComponent;
class USceneComponent;
class UCameraComponent;
class UMotionControllerComponent;
class UAnimMontage;
class USoundBase;

UCLASS(config=Game)
class AEcdysisCharacter : public ACharacter
{
	GENERATED_BODY()

	/** Pawn mesh: 1st person view (arms; seen only by self) */
	UPROPERTY(VisibleDefaultsOnly, Category=Mesh)
	USkeletalMeshComponent* ArmsFpp;
	/** First person camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	UCameraComponent* FirstPersonCameraComponent;

	/** Motion controller (right hand) */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	UMotionControllerComponent* R_MotionController;
	/** Motion controller (left hand) */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	UMotionControllerComponent* L_MotionController;

	UPROPERTY()
	float deltaTime;
	
	UPROPERTY(VisibleAnywhere, Category = Gameplay_Movement)
	int movementType;

	UPROPERTY(VisibleAnywhere, Category = Gameplay_Crouching)
	bool isCrouching;
	UPROPERTY(VisibleAnywhere, Category = Gameplay_Crouching)
	bool crouchKeyDown;
	
	UPROPERTY(VisibleAnywhere, Category = Gameplay_Reloading)
	bool isReloading;
	
	UPROPERTY(VisibleAnywhere, Category = Gameplay_ADS)
	float currentStrafeSpeedModifier;
	UPROPERTY(VisibleAnywhere, Category = Gameplay_ADS)
	bool isADS;
	UPROPERTY(VisibleAnywhere, Category = Gameplay_ADS)
	bool adsKeyDown;
	UPROPERTY(VisibleAnywhere, Category = Gameplay_ADS)
	bool stopADS;
	UPROPERTY()
	float adsTimer;
	UPROPERTY()
	float adsZoom;
	
	UPROPERTY()
	float isCrouched;
	UPROPERTY()
	float crouchZoom;

	UPROPERTY()
	float leanState;
	UPROPERTY()
	bool leanZoom;
	UPROPERTY()
	bool canLean;


public:
	AEcdysisCharacter();

	bool IsReloading();

	void GunToHands();

protected:
	virtual void BeginPlay();

public:
	
	virtual void Tick(float DeltaTime) override;


	void PlayAnimationMontage(UAnimMontage* anim);
	/** Base turn rate, in deg/sec. Other scaling may affect final turn rate. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category=Camera)
	float BaseTurnRate;

	/** Base look up/down rate, in deg/sec. Other scaling may affect final rate. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category=Camera)
	float BaseLookUpRate;

	/** Gun muzzle's offset from the characters location */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Gameplay)
	FVector GunOffset;

	/** Projectile class to spawn */
	UPROPERTY(EditDefaultsOnly, Category=Projectile)
	TSubclassOf<class AEcdysisProjectile> ProjectileClass;

	/** Whether to use motion controller location for aiming. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Gameplay)
	uint8 bUsingMotionControllers : 1;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Gameplay_Interaction)
	float interactRange;
	
	UPROPERTY(BlueprintReadOnly, Category = Gameplay_Interaction)
	float rayCastRange = 100000.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Gameplay_Camera)
	float playerFOV = 90.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Gameplay_Stamina)
	int currentStamina;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Gameplay_Stamina)
	int maxStamina;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Gameplay_Stamina)
	int staminaReduceRate;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Gameplay_Stamina)
	int staminaReduceRateSprint;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Gameplay_Stamina)
	int staminaReduceRateSupersprint;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Gameplay_Stamina)
	int staminaIncrease;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Gameplay_Movement)
	float strafeSpeedModifier = .75f;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Gameplay_Movement)
	float crouchSpeed = 300.0f;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Gameplay_Movement)
	float walkSpeed = 600.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Gameplay_Movement)
	float sprintSpeed = 1100.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Gameplay_Movement)
	float supersprintSpeed = 1300.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Gameplay_Weapons)
	float fovADS = 70.0f;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Controls)
	bool adsToggle = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Gameplay_Weapons)
	float adsZoomSpeed = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Gameplay_Crouch)
	float standHeight;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Gameplay_Crouch)
	float crouchHeight;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Gameplay_Crouch)
	float crouchZoomSpeed;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = Gameplay_Weapons)
	class AActor* currentInteractable;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = Gameplay_Weapons)
	class ABaseWeapon* equippedWeapon;

protected:
	
	void Initialize();
	void StartInteractionSystem();
	void MainRayCast();
	/** Fires a projectile. */
	void OnFire();
	
	void OnInteract();
	
	void OnCrouch();
	void OnUnCrouch();
	void PerformCrouch();
	void HandleCrouchZoom();
	void CrouchHoldCheck();
	
	void StopFire();
	void Sprint();
	void StopSprint();
	
	void ReduceStamina();
	void IncreaseStamina();
	void ResetAndStartTimer(FTimerHandle handle, float loopTime, bool looping);
	bool CheckStamina();

	void HandleGroundMovementType();

	void OnPressADS();
	void OnReleaseADS();	
	void PerformADS();	
	void CancelADS();
	void HandleZoomIn();
	void AdsHoldCheck();

	void OnReload();

	FTimeline CurveFTimeline;
	UPROPERTY(EditAnywhere, Category = "Timeline")
	UCurveFloat* CurveFloat;

	UPROPERTY()
	FTimerHandle TimerHandle_HandleStaminaIncrease;
	UPROPERTY()
	FTimerHandle TimerHandle_HandleStaminaDecrease;
	UPROPERTY()
	FTimerHandle TimerHandle_AdsZoom;
	UPROPERTY()
	FTimerHandle TimerHandle_Crouch;
	UPROPERTY()
	FTimerHandle TimerHandle_AdsHoldCheck;
	UPROPERTY()
	FTimerHandle TimerHandle_CrouchHoldCheck;
	UPROPERTY()
	FTimerHandle TimerHandle_Interaction;

	/** Resets HMD orientation and position in VR. */
	void OnResetVR();

	/** Handles moving forward/backward */
	void MoveForward(float Val);

	/** Handles stafing movement, left and right */
	void MoveRight(float Val);

	/** Handles lean, left and right */
	void OnLean(float Val);
	
	/**
	 * Called via input to turn at a given rate.
	 * @param Rate	This is a normalized rate, i.e. 1.0 means 100% of desired turn rate
	 */
	void TurnAtRate(float Rate);

	/**
	 * Called via input to turn look up/down at a given rate.
	 * @param Rate	This is a normalized rate, i.e. 1.0 means 100% of desired turn rate
	 */
	void LookUpAtRate(float Rate);

	struct TouchData
	{
		TouchData() { bIsPressed = false;Location=FVector::ZeroVector;}
		bool bIsPressed;
		ETouchIndex::Type FingerIndex;
		FVector Location;
		bool bMoved;
	};
	void BeginTouch(const ETouchIndex::Type FingerIndex, const FVector Location);
	void EndTouch(const ETouchIndex::Type FingerIndex, const FVector Location);
	void TouchUpdate(const ETouchIndex::Type FingerIndex, const FVector Location);
	TouchData	TouchItem;
	
protected:
	// APawn interface
	virtual void SetupPlayerInputComponent(UInputComponent* InputComponent) override;
	// End of APawn interface

	/* 
	 * Configures input for touchscreen devices if there is a valid touch interface for doing so 
	 *
	 * @param	InputComponent	The input component pointer to bind controls to
	 * @returns true if touch controls were enabled.
	 */
	bool EnableTouchscreenMovement(UInputComponent* InputComponent);

public:
	/** Returns Mesh1P subobject **/
	USkeletalMeshComponent* GetMesh1P() const { return ArmsFpp; }
	/** Returns FirstPersonCameraComponent subobject **/
	UCameraComponent* GetFirstPersonCameraComponent() const { return FirstPersonCameraComponent; }

};

