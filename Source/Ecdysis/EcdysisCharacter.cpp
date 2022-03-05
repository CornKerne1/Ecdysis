// Copyright Epic Games, Inc. All Rights Reserved.

#include "EcdysisCharacter.h"
#include "Engine.h"
#include "EcdysisProjectile.h"
#include "Animation/AnimInstance.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/InputComponent.h"
#include "GameFramework/InputSettings.h"
#include "HeadMountedDisplayFunctionLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetStringLibrary.h"
#include "MotionControllerComponent.h"
#include "XRMotionControllerBase.h" // for FXRMotionControllerBase::RightHandSourceId
#include "Kismet/KismetMathLibrary.h"
#include "Misc/App.h"
#include "Engine/World.h"
#include "Interaction.h"
#include "GameFramework/CharacterMovementComponent.h"

DEFINE_LOG_CATEGORY_STATIC(LogFPChar, Warning, All);

//////////////////////////////////////////////////////////////////////////
// AEcdysisCharacter

AEcdysisCharacter::AEcdysisCharacter()
{
	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(55.f, 96.0f);

	// set our turn rates for input
	BaseTurnRate = 45.f;
	BaseLookUpRate = 45.f;

	// Create a CameraComponent	
	FirstPersonCameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("FirstPersonCamera"));
	FirstPersonCameraComponent->SetupAttachment(GetCapsuleComponent());
	FirstPersonCameraComponent->SetRelativeLocation(FVector(-39.56f, 1.75f, 64.f)); // Position the camera
	FirstPersonCameraComponent->bUsePawnControlRotation = true;

	// Create a mesh component that will be used when being viewed from a '1st person' view (when controlling this pawn)
	ArmsFpp = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("CharacterMesh1P"));
	ArmsFpp->SetOnlyOwnerSee(true);
	ArmsFpp->SetupAttachment(FirstPersonCameraComponent);
	ArmsFpp->bCastDynamicShadow = false;
	ArmsFpp->CastShadow = false;
	ArmsFpp->SetRelativeRotation(FRotator(1.9f, -19.19f, 5.2f));
	ArmsFpp->SetRelativeLocation(FVector(-0.5f, -4.4f, -155.7f));
	// Default offset from the character location for projectiles to spawn
	GunOffset = FVector(100.0f, 0.0f, 10.0f);

	// Note: The ProjectileClass and the skeletal mesh/anim blueprints for Mesh1P, FP_Gun, and VR_Gun 
	// are set in the derived blueprint asset named MyCharacter to avoid direct content references in C++.

	// Create VR Controllers.
	R_MotionController = CreateDefaultSubobject<UMotionControllerComponent>(TEXT("R_MotionController"));
	R_MotionController->MotionSource = FXRMotionControllerBase::RightHandSourceId;
	R_MotionController->SetupAttachment(RootComponent);
	L_MotionController = CreateDefaultSubobject<UMotionControllerComponent>(TEXT("L_MotionController"));
	L_MotionController->SetupAttachment(RootComponent);

	equippedWeapon = nullptr;
}

bool AEcdysisCharacter::IsReloading()
{
	return isReloading;
}

void AEcdysisCharacter::GunToHands()
{
	equippedWeapon->AttachToComponent(ArmsFpp, FAttachmentTransformRules(EAttachmentRule::SnapToTarget, true), TEXT("GripPoint"));
}

void AEcdysisCharacter::BeginPlay()
{
	// Call the base class  
	Super::BeginPlay();

	Initialize();
	ArmsFpp->SetHiddenInGame(false, true);
}

void AEcdysisCharacter::Initialize()
{
	currentStamina = maxStamina;
	currentStrafeSpeedModifier = strafeSpeedModifier;
	if (auto cam = GetFirstPersonCameraComponent())
	{
		cam->SetFieldOfView(playerFOV);
	}
	if (auto cM = GetCharacterMovement())
	{
		cM->MaxWalkSpeed = walkSpeed;
	}
	if (auto cC = GetCapsuleComponent())
	{
		cC->SetCapsuleHalfHeight(standHeight);
	}
	StartInteractionSystem();
}
void AEcdysisCharacter::StartInteractionSystem()
{
	GetWorldTimerManager().SetTimer(TimerHandle_Interaction, this, &AEcdysisCharacter::MainRayCast, .01f, true);
}
void AEcdysisCharacter::MainRayCast()
{
	FVector Loc;
	FRotator Rot;
	GetController()->GetPlayerViewPoint(Loc, Rot);
	FVector Start = Loc;
	FVector End = Start + (Rot.Vector() * rayCastRange);
	FCollisionQueryParams TraceParams;
	//DrawDebugLine(GetWorld(), FirstPersonCameraComponent->GetComponentLocation(), FirstPersonCameraComponent->GetComponentLocation() + FirstPersonCameraComponent->GetForwardVector() * rayCastRange, FColor::Blue, false, 1, 0, 1);
	FHitResult outHit;
	if (GetWorld()->LineTraceSingleByChannel(outHit, Start, End, ECC_Visibility, TraceParams))
	{
		if (auto Interactable = outHit.GetActor())
		{
			if (AActor::GetDistanceTo(Interactable) <= interactRange)
			{
				if (Interactable != currentInteractable)
				{
					if (currentInteractable)
					{
						if (auto* interface = Cast<IInteraction>(currentInteractable))
						{
							interface->Execute_EndFocus(currentInteractable, this);
						}

					}
					if (auto* interface = Cast<IInteraction>(Interactable))
					{
						interface->Execute_OnFocus(Interactable, this);
					}
					currentInteractable = Interactable;
				}
			}
		}
		else
			if (currentInteractable)
			{
				if (auto* interface = Cast<IInteraction>(currentInteractable))
				{
					interface->Execute_EndFocus(currentInteractable, this);
				}
			}
		currentInteractable = nullptr;
	}
}
//////////////////////////////////////////////////////////////////////////
// Input

void AEcdysisCharacter::SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent)
{
	// set up gameplay key bindings
	check(PlayerInputComponent);

	// Bind jump events
	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &ACharacter::Jump);
	PlayerInputComponent->BindAction("Jump", IE_Released, this, &ACharacter::StopJumping);

	// Bind fire event
	PlayerInputComponent->BindAction("Fire", IE_Pressed, this, &AEcdysisCharacter::OnFire);
	PlayerInputComponent->BindAction("Fire", IE_Released, this, &AEcdysisCharacter::StopFire);

	// Enable touchscreen input
	EnableTouchscreenMovement(PlayerInputComponent);

	PlayerInputComponent->BindAction("ResetVR", IE_Pressed, this, &AEcdysisCharacter::OnResetVR);

	// Bind crouch events
	PlayerInputComponent->BindAction("Crouch", IE_Pressed, this, &AEcdysisCharacter::OnCrouch);
	PlayerInputComponent->BindAction("Crouch", IE_Released, this, &AEcdysisCharacter::OnUnCrouch);

	// Bind movement events
	PlayerInputComponent->BindAxis("MoveForward", this, &AEcdysisCharacter::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &AEcdysisCharacter::MoveRight);

	PlayerInputComponent->BindAction("Sprint", IE_Pressed, this, &AEcdysisCharacter::Sprint);
	//PlayerInputComponent->BindAction("Sprint", IE_Released, this, &AEcdysisCharacter::StopSprint);

	PlayerInputComponent->BindAction("ADS", IE_Pressed, this, &AEcdysisCharacter::OnPressADS);
	PlayerInputComponent->BindAction("ADS", IE_Released, this, &AEcdysisCharacter::OnReleaseADS);

	PlayerInputComponent->BindAction("Reload", IE_Pressed, this, &AEcdysisCharacter::OnReload);

	PlayerInputComponent->BindAction("Interact", IE_Pressed, this, &AEcdysisCharacter::OnInteract);
	
	// We have 2 versions of the rotation bindings to handle different kinds of devices differently
	// "turn" handles devices that provide an absolute delta, such as a mouse.
	// "turnrate" is for devices that we choose to treat as a rate of change, such as an analog joystick
	PlayerInputComponent->BindAxis("Turn", this, &APawn::AddControllerYawInput);
	PlayerInputComponent->BindAxis("TurnRate", this, &AEcdysisCharacter::TurnAtRate);
	PlayerInputComponent->BindAxis("LookUp", this, &APawn::AddControllerPitchInput);
	PlayerInputComponent->BindAxis("LookUpRate", this, &AEcdysisCharacter::LookUpAtRate);
	PlayerInputComponent->BindAxis("Lean", this, &AEcdysisCharacter::OnLean);
}


//	// try and play the sound if specified
//	if (FireSound != nullptr)
//	{
//		UGameplayStatics::PlaySoundAtLocation(this, FireSound, GetActorLocation());
//	}
//
//	// try and play a firing animation if specified
//	if (FireAnimation != nullptr)
//	{
//		// Get the animation object for the arms mesh
//		UAnimInstance* AnimInstance = Mesh1P->GetAnimInstance();
//		if (AnimInstance != nullptr)
//		{
//			AnimInstance->Montage_Play(FireAnimation, 1.f);
//		}
//	}
//}

//void AEcdysisCharacter::StartFire()
//{
//	FireShot();
//	GetWorldTimerManager().SetTimer(TimerHandle_HandleRefire, this, &AEcdysisCharacter::FireShot, timeBetweenShots, true);
//}

void AEcdysisCharacter::Tick(float DeltaTime)
{
	deltaTime = DeltaTime;
}


void AEcdysisCharacter::OnInteract()
{
	if (currentInteractable)
	{
		if (auto interface = Cast<IInteraction>(currentInteractable))
		{
			if (AActor::GetDistanceTo(currentInteractable) <= interactRange)
			{
				interface->Execute_OnInteraction(currentInteractable, this);
			}
			else
			{
				currentInteractable = nullptr;
			}
		}
	}
}

void AEcdysisCharacter::PlayAnimationMontage(UAnimMontage* anim)
{
	if (anim != nullptr)
	{
		// Get the animation object for the arms mesh
		UAnimInstance* AnimInstance = ArmsFpp->GetAnimInstance();
		if (AnimInstance != nullptr)
		{
			AnimInstance->Montage_Play(anim, 1.f);
		}
	}
}

void AEcdysisCharacter::StopFire()
{
	/*GetWorldTimerManager().ClearTimer(TimerHandle_HandleRefire);*/
}

void AEcdysisCharacter::Sprint()
{
	if (isCrouched)
	{
		PerformCrouch();
		movementType = 0;
	}
	if (auto playerMovement = GetCharacterMovement())
	{
		movementType = movementType + 1;
		HandleGroundMovementType();
		switch (movementType)
		{
		case 0://Walk
			GetCharacterMovement()->MaxWalkSpeed = walkSpeed;
			GetWorldTimerManager().ClearTimer(TimerHandle_HandleStaminaDecrease);
			if (!CheckStamina() || currentStamina < maxStamina)
			{
				GetWorldTimerManager().SetTimer(TimerHandle_HandleStaminaIncrease, this, &AEcdysisCharacter::IncreaseStamina, 1, true);
			}
			break;
		case 1://SuperSprint
			if (isADS)
			{
				PerformADS();
			}
			if (CheckStamina() && currentStamina > 0.75f * maxStamina)
			{
				GetCharacterMovement()->MaxWalkSpeed = supersprintSpeed;
				staminaReduceRate = staminaReduceRateSupersprint;
				ResetAndStartTimer(TimerHandle_HandleStaminaDecrease, 1, true);
			}
			else
				Sprint();
			break;
		case 2://Sprint
			if (isADS)
			{
				PerformADS();
			}
			if (CheckStamina())
			{
				GetCharacterMovement()->MaxWalkSpeed = sprintSpeed;
				staminaReduceRate = staminaReduceRateSprint;
				ResetAndStartTimer(TimerHandle_HandleStaminaDecrease, 1, true);
			}
			else
				Sprint();
			break;
		}
	}
}

void AEcdysisCharacter::StopSprint()
{
	if (movementType != 0)
	{
		movementType = 2;
		Sprint();
	}
}

void AEcdysisCharacter::ReduceStamina()
{
	currentStamina = currentStamina - 1 * staminaReduceRate;
	if (currentStamina < (maxStamina * .75f) && movementType == 1)
	{
		Sprint();
	}
	if (!CheckStamina())
	{
		Sprint();
	}
}

void AEcdysisCharacter::IncreaseStamina()
{
	currentStamina = currentStamina + staminaIncrease;
}

void AEcdysisCharacter::ResetAndStartTimer(FTimerHandle handle, float loopTime, bool looping)
{
	GetWorldTimerManager().ClearTimer(TimerHandle_HandleStaminaIncrease);
	GetWorldTimerManager().ClearTimer(TimerHandle_HandleStaminaDecrease);
	GetWorldTimerManager().SetTimer(TimerHandle_HandleStaminaDecrease, this, &AEcdysisCharacter::ReduceStamina, 1, true);
}

bool AEcdysisCharacter::CheckStamina()
{
	if (currentStamina <= 0) { currentStamina = 0; return false; }

	else
		if (currentStamina >= maxStamina) { currentStamina = maxStamina; }
		return true;


}

void AEcdysisCharacter::HandleGroundMovementType()
{
	if (movementType > 2)
	{
		movementType = 0;
	}
	
}

void AEcdysisCharacter::OnPressADS()
{
	adsKeyDown = true;
	if (isADS)
	{
		GetWorldTimerManager().SetTimer(TimerHandle_AdsHoldCheck, this, &AEcdysisCharacter::AdsHoldCheck, .05, false);
	}
	else
	{
		StopSprint();
		PerformADS();
	}
}

void AEcdysisCharacter::PerformADS()
{
	if(!adsZoom)
	{
		adsZoom = true;
		GetWorldTimerManager().SetTimer(TimerHandle_AdsZoom, this, &AEcdysisCharacter::HandleZoomIn, .02f, true);
	}
}

void AEcdysisCharacter::OnReleaseADS()
{
	adsKeyDown = false;
	PerformADS();


}

void AEcdysisCharacter::AdsHoldCheck()
{
	if (!adsKeyDown)
	{		
		StopSprint();
		PerformADS();
	}
	GetWorldTimerManager().ClearTimer(TimerHandle_AdsHoldCheck);
}

void AEcdysisCharacter::OnReload()
{
	if (equippedWeapon)
	{
		equippedWeapon->ReloadWeapon();
	}
}

void AEcdysisCharacter::CancelADS()
{
	if (!adsZoom)
	{
		adsZoom = true;
	}
}


void AEcdysisCharacter::HandleZoomIn()
{
	if (adsZoom)
	{
		if (isADS)
		{
			if (auto fpCam = GetFirstPersonCameraComponent())
			{
				if (FMath::IsNearlyEqual(fpCam->FieldOfView, playerFOV, .6f))
				{
					fpCam->SetFieldOfView(playerFOV);
					GetWorldTimerManager().ClearTimer(TimerHandle_AdsZoom);
					isADS = false;
					adsZoom = false;
				}
				else
				{
					fpCam->SetFieldOfView(UKismetMathLibrary::FInterpTo(fpCam->FieldOfView, playerFOV, deltaTime, adsZoomSpeed));
				}
			}
		}
		else if (auto fpCam = GetFirstPersonCameraComponent())
		{
			if (FMath::IsNearlyEqual(fpCam->FieldOfView, fovADS, .6f))
			{
				fpCam->SetFieldOfView(fovADS);
				GetWorldTimerManager().ClearTimer(TimerHandle_AdsZoom);
				isADS = true;
				adsZoom = false;
			}
			else
			{
				fpCam->SetFieldOfView(UKismetMathLibrary::FInterpTo(fpCam->FieldOfView, fovADS, deltaTime, adsZoomSpeed));
			}
		}
	}
}
void AEcdysisCharacter::OnLean(float Val)
{
	leanState = Val;
	float leanValue = GetControlRotation().Roll;
	if (leanState != 0)
	{
		if (leanValue + leanState <= 20.0f || leanValue + leanState >= 340.0f)
		{
			AddControllerRollInput(leanState / 2.0f);
			if (leanState > 0.0f)
			{
				AddActorLocalOffset(FVector(0.0f, 2.0f, 0.0f));//Right
			}
			else
			{
				AddActorLocalOffset(FVector(0.0f, -2.0f, 0.0f));//Left
			}
		}
	}
	else
	{
		if (leanValue != 0.0f)
		{
			if (leanValue >= 340.0f)
			{
				if (leanValue <= 359.0f)
				{
					AddControllerRollInput(1.0f / 2.0f);
					AddActorLocalOffset(FVector(0.0f, 2.0f, 0.0f));
				}
				else
				{
					AddControllerRollInput(360.0f - leanValue);
				}
			}
			else if (leanValue <= 20.0f)
			{
				if (leanValue >= 1.0f)
				{
					AddControllerRollInput(-1.0f / 2.0f);
					AddActorLocalOffset(FVector(0.0f, -2.0f, 0.0f));
				}
				else
				{
					AddControllerRollInput(0.0f - leanValue);
				}
			}
		}
	}

}

void AEcdysisCharacter::OnFire()
{
	if (equippedWeapon)
	{
		equippedWeapon->FireWeapon();
	}
}

void AEcdysisCharacter::OnCrouch()
{
	crouchKeyDown = true;
	if (isCrouched)
	{
		GetWorldTimerManager().SetTimer(TimerHandle_CrouchHoldCheck, this, &AEcdysisCharacter::CrouchHoldCheck, .05, false);
	}
	else
	{
		StopSprint();
		PerformCrouch();
	}
}

void AEcdysisCharacter::OnUnCrouch()
{
	crouchKeyDown = false;
	PerformCrouch();
}

void AEcdysisCharacter::PerformCrouch()
{
	if (!crouchZoom)
	{
		crouchZoom = true;
		GetWorldTimerManager().SetTimer(TimerHandle_Crouch, this, &AEcdysisCharacter::HandleCrouchZoom, .01f, true);
	}
}

void AEcdysisCharacter::CrouchHoldCheck()
{
	if (!crouchKeyDown)
	{
		StopSprint();
		PerformCrouch();
	}
	GetWorldTimerManager().ClearTimer(TimerHandle_CrouchHoldCheck);
}

void AEcdysisCharacter::HandleCrouchZoom()
{
	if (crouchZoom)
	{
		if (isCrouched)
		{
			if (auto fpCap = GetCapsuleComponent())
			{
				if (fpCap->GetUnscaledCapsuleHalfHeight() >= standHeight)
				{
					fpCap->SetCapsuleHalfHeight(standHeight);
					GetWorldTimerManager().ClearTimer(TimerHandle_Crouch);
					isCrouched = false;
					crouchZoom = false;
					GetCharacterMovement()->MaxWalkSpeed = walkSpeed;
					movementType = 0;
				}
				else
				{
					fpCap->SetCapsuleHalfHeight(UKismetMathLibrary::FInterpTo_Constant(fpCap->GetUnscaledCapsuleHalfHeight(), standHeight, deltaTime, crouchZoomSpeed));
				}
			}
		}
		else if (auto fpCap = GetCapsuleComponent())
		{
			if (fpCap->GetUnscaledCapsuleHalfHeight() <= crouchHeight)
			{
				fpCap->SetCapsuleHalfHeight(crouchHeight);
				GetWorldTimerManager().ClearTimer(TimerHandle_Crouch);
				isCrouched = true;
				crouchZoom = false;
				GetCharacterMovement()->MaxWalkSpeed = crouchSpeed;
			}
			else
			{
				fpCap->SetCapsuleHalfHeight(UKismetMathLibrary::FInterpTo_Constant(fpCap->GetUnscaledCapsuleHalfHeight(), crouchHeight, deltaTime, crouchZoomSpeed));
			}
		}

	}
}

void AEcdysisCharacter::OnResetVR()
{
	UHeadMountedDisplayFunctionLibrary::ResetOrientationAndPosition();
}

void AEcdysisCharacter::BeginTouch(const ETouchIndex::Type FingerIndex, const FVector Location)
{
	if (TouchItem.bIsPressed == true)
	{
		return;
	}
	if ((FingerIndex == TouchItem.FingerIndex) && (TouchItem.bMoved == false))
	{
		OnFire();
	}
	TouchItem.bIsPressed = true;
	TouchItem.FingerIndex = FingerIndex;
	TouchItem.Location = Location;
	TouchItem.bMoved = false;
}

void AEcdysisCharacter::EndTouch(const ETouchIndex::Type FingerIndex, const FVector Location)
{
	if (TouchItem.bIsPressed == false)
	{
		return;
	}
	TouchItem.bIsPressed = false;
}

void AEcdysisCharacter::MoveForward(float Value)
{
	if (Value != 0.0f)
	{
		// add movement in that direction
		AddMovementInput(GetActorForwardVector(), Value);
	}
	else
	{
		StopSprint();
	}
}


void AEcdysisCharacter::MoveRight(float Value)
{
	if (Value != 0.0f)
	{
		// add movement in that direction
		AddMovementInput(GetActorRightVector(), Value * currentStrafeSpeedModifier);
	}
}

void AEcdysisCharacter::TurnAtRate(float Rate)
{
	// calculate delta for this frame from the rate information
	AddControllerYawInput(Rate * BaseTurnRate * GetWorld()->GetDeltaSeconds());
}

void AEcdysisCharacter::LookUpAtRate(float Rate)
{
	// calculate delta for this frame from the rate information
	AddControllerPitchInput(Rate * BaseLookUpRate * GetWorld()->GetDeltaSeconds());
}

bool AEcdysisCharacter::EnableTouchscreenMovement(class UInputComponent* PlayerInputComponent)
{
	if (FPlatformMisc::SupportsTouchInput() || GetDefault<UInputSettings>()->bUseMouseForTouch)
	{
		PlayerInputComponent->BindTouch(EInputEvent::IE_Pressed, this, &AEcdysisCharacter::BeginTouch);
		PlayerInputComponent->BindTouch(EInputEvent::IE_Released, this, &AEcdysisCharacter::EndTouch);

		//Commenting this out to be more consistent with FPS BP template.
		//PlayerInputComponent->BindTouch(EInputEvent::IE_Repeat, this, &AEcdysisCharacter::TouchUpdate);
		return true;
	}
	
	return false;
}
