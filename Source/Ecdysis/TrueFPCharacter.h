// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/TimelineComponent.h"
#include "GameFramework/Character.h"
#include "TrueFPCharacter.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FCurrentWeaponChangedDelegate, class ABaseWeapon*, CurrentWeapon, const class ABaseWeapon*, OldWeapon);

UCLASS()
class ECDYSIS_API ATrueFPCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	ATrueFPCharacter();

	UPROPERTY(EditAnywhere,BlueprintReadWrite, Category= "Components")
	float camBaseRotationX;
	UPROPERTY(EditAnywhere,BlueprintReadWrite, Category= "Components")
	float camBaseRotationY;
	UPROPERTY(EditAnywhere,BlueprintReadWrite, Category= "Components")
	float camBaseRotationZ;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Replicated, Category= "Anim")
	float ADSWeight = 0.f;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void PreReplication(IRepChangedPropertyTracker& ChangedPropertyTracker) override;

	virtual void Initialize();

	/** Handles moving forward/backward */
	void MoveForward(float Val);

	/** Handles stafing movement, left and right */
	void MoveRight(float Val);

	void TurnAtRate(float Rate);
	void LookUpAtRate(float Rate);

	void OnCrouch();
	void OnUnCrouch();
	void PerformCrouch();
	void HandleCrouchZoom();
	void CrouchHoldCheck();
	
	void Sprint();
	void StopSprint();

	void OnLean(float Val);
	
	void OnPressADS();
	void OnReleaseADS();	
	void PerformADS();	
	void CancelADS();
	void HandleZoomIn();
	void AdsHoldCheck();
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Configurations|Anim")
	class UCurveFloat* AimingCurve;
	FTimeline AimingTimeline;

	UFUNCTION(Server, Reliable)
	void Server_StartAim(const bool bForward = true);
	virtual FORCEINLINE void Server_StartAim_Implementation(const bool bForward)
	{
		Multi_Aim(bForward);
		Multi_Aim_Implementation(bForward);
	}

	UFUNCTION(NetMulticast, Reliable)
	void Multi_Aim(const bool bForward);
	virtual void Multi_Aim_Implementation(const bool bForward);

	UFUNCTION()
	virtual void TimeLineProgress(const float Value);

	

	
	void ResetAndStartTimer(FTimerHandle handle, float loopTime, bool looping);
	void HandleGroundMovementType();
	bool CheckStamina();
	void ReduceStamina();
	void IncreaseStamina();

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

public:	
	virtual void Tick(float DeltaTime) override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	UPROPERTY(VisibleAnywhere,BlueprintReadWrite, Category= "Components")
	class UCameraComponent* Camera;

	UPROPERTY(VisibleAnywhere,BlueprintReadWrite, Category= "Components")
	class USkeletalMeshComponent* ClientMesh;

	/** Base turn rate, in deg/sec. Other scaling may affect final turn rate. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category=Camera)
	float BaseTurnRate;

	/** Base look up/down rate, in deg/sec. Other scaling may affect final rate. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category=Camera)
	float BaseLookUpRate;

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

	

protected:
	UPROPERTY(EditDefaultsOnly, Category="Configurations")
	TArray<TSubclassOf<class ABaseWeapon>> DefaultWeapons;

public:
	UPROPERTY(VisibleInstanceOnly, BlueprintReadWrite, Replicated, Category = "State")
	TArray<class ABaseWeapon*> Weapons;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadWrite, ReplicatedUsing = OnRep_CurrentWeapon, Category = "State")
	class ABaseWeapon* CurrentWeapon;

	UPROPERTY(BlueprintAssignable, Category = "Delegates")
	FCurrentWeaponChangedDelegate CurrentWeaponChangedDelegate;
	
	UPROPERTY(VisibleInstanceOnly, BlueprintReadWrite, Replicated, Category = "State")
	int32 CurrentIndex = 0;

	UFUNCTION(BlueprintCallable, Category = "Character")
	virtual void EquipWeapon(const int32 Index);
protected:
	UFUNCTION()
	virtual void OnRep_CurrentWeapon(const class ABaseWeapon* OldWeapon);
	UFUNCTION(Server, Reliable)
	void Server_SetCurrentWeapon(class ABaseWeapon* NewWeapon);
	virtual void Server_SetCurrentWeapon_Implementation(class ABaseWeapon* NewWeapon);


protected:
	virtual void NextWeapon();
	virtual void LastWeapon();

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



};
