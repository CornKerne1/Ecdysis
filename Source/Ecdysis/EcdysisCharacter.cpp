// Copyright Epic Games, Inc. All Rights Reserved.

#include "EcdysisCharacter.h"
#include "EcdysisProjectile.h"
#include "Animation/AnimInstance.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/InputComponent.h"
#include "GameFramework/InputSettings.h"
#include "HeadMountedDisplayFunctionLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "MotionControllerComponent.h"
#include "XRMotionControllerBase.h" // for FXRMotionControllerBase::RightHandSourceId
#include "Kismet/KismetMathLibrary.h"
#include "Misc/App.h"
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

	// Create a gun mesh component
	FP_Gun = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("FP_Gun"));
	FP_Gun->SetOnlyOwnerSee(false);			// otherwise won't be visible in the multiplayer
	FP_Gun->bCastDynamicShadow = false;
	FP_Gun->CastShadow = false;
	// FP_Gun->SetupAttachment(Mesh1P, TEXT("GripPoint"));
	FP_Gun->SetupAttachment(RootComponent);

	FP_MuzzleLocation = CreateDefaultSubobject<USceneComponent>(TEXT("MuzzleLocation"));
	FP_MuzzleLocation->SetupAttachment(FP_Gun);
	FP_MuzzleLocation->SetRelativeLocation(FVector(0.2f, 48.4f, -10.6f));

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

	// Create a gun and attach it to the right-hand VR controller.
	// Create a gun mesh component
	VR_Gun = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("VR_Gun"));
	VR_Gun->SetOnlyOwnerSee(false);			// otherwise won't be visible in the multiplayer
	VR_Gun->bCastDynamicShadow = false;
	VR_Gun->CastShadow = false;
	VR_Gun->SetupAttachment(R_MotionController);
	VR_Gun->SetRelativeRotation(FRotator(0.0f, -90.0f, 0.0f));

	VR_MuzzleLocation = CreateDefaultSubobject<USceneComponent>(TEXT("VR_MuzzleLocation"));
	VR_MuzzleLocation->SetupAttachment(VR_Gun);
	VR_MuzzleLocation->SetRelativeLocation(FVector(0.000004, 53.999992, 10.000000));
	VR_MuzzleLocation->SetRelativeRotation(FRotator(0.0f, 90.0f, 0.0f));		// Counteract the rotation of the VR gun model.

	// Uncomment the following line to turn motion controllers on by default:
	//bUsingMotionControllers = true;
}

void AEcdysisCharacter::BeginPlay()
{
	// Call the base class  
	Super::BeginPlay();

	//Attach gun mesh component to Skeleton, doing it here because the skeleton is not yet created in the constructor
	FP_Gun->AttachToComponent(ArmsFpp, FAttachmentTransformRules(EAttachmentRule::SnapToTarget, true), TEXT("GripPoint"));

	// Show or hide the two versions of the gun based on whether or not we're using motion controllers.
	if (bUsingMotionControllers)
	{
		VR_Gun->SetHiddenInGame(false, true);
		ArmsFpp->SetHiddenInGame(true, true);
	}
	else
	{
		VR_Gun->SetHiddenInGame(true, true);
		ArmsFpp->SetHiddenInGame(false, true);
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

	// Bind movement events
	PlayerInputComponent->BindAxis("MoveForward", this, &AEcdysisCharacter::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &AEcdysisCharacter::MoveRight);

	PlayerInputComponent->BindAction("Sprint", IE_Pressed, this, &AEcdysisCharacter::Sprint);
	PlayerInputComponent->BindAction("Sprint", IE_Released, this, &AEcdysisCharacter::StopSprint);

	PlayerInputComponent->BindAction("ADS", IE_Pressed, this, &AEcdysisCharacter::OnPressADS);
	PlayerInputComponent->BindAction("ADS", IE_Released, this, &AEcdysisCharacter::OnReleaseADS);
	
	// We have 2 versions of the rotation bindings to handle different kinds of devices differently
	// "turn" handles devices that provide an absolute delta, such as a mouse.
	// "turnrate" is for devices that we choose to treat as a rate of change, such as an analog joystick
	PlayerInputComponent->BindAxis("Turn", this, &APawn::AddControllerYawInput);
	PlayerInputComponent->BindAxis("TurnRate", this, &AEcdysisCharacter::TurnAtRate);
	PlayerInputComponent->BindAxis("LookUp", this, &APawn::AddControllerPitchInput);
	PlayerInputComponent->BindAxis("LookUpRate", this, &AEcdysisCharacter::LookUpAtRate);
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

void AEcdysisCharacter::StopFire()
{
	/*GetWorldTimerManager().ClearTimer(TimerHandle_HandleRefire);*/
}

void AEcdysisCharacter::Sprint()
{
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
			if (CheckStamina() && currentStamina > 0.75f * maxStamina)
			{
				GetCharacterMovement()->MaxWalkSpeed = supersprintSpeed;
				staminaReduceRate = staminaReduceRateSupersprint;
				GetWorldTimerManager().ClearTimer(TimerHandle_HandleStaminaIncrease);
				GetWorldTimerManager().ClearTimer(TimerHandle_HandleStaminaDecrease);
				GetWorldTimerManager().SetTimer(TimerHandle_HandleStaminaDecrease, this, &AEcdysisCharacter::ReduceStamina, 1, true);
			}
			else
				Sprint();
			break;
		case 2://Sprint
			if (CheckStamina())
			{
				GetCharacterMovement()->MaxWalkSpeed = sprintSpeed;
				staminaReduceRate = staminaReduceRateSprint;
				GetWorldTimerManager().ClearTimer(TimerHandle_HandleStaminaIncrease);
				GetWorldTimerManager().ClearTimer(TimerHandle_HandleStaminaDecrease);
				GetWorldTimerManager().SetTimer(TimerHandle_HandleStaminaDecrease, this, &AEcdysisCharacter::ReduceStamina, 1, true);
			}
			else
				Sprint();
			break;
		}
	}
}

void AEcdysisCharacter::StopSprint()
{
	//GetCharacterMovement()->MaxWalkSpeed = walkSpeed;
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
	if (adsToggle)
	{
		if (isADS)
		{
			CancelADS();
		}
		else
		{
			PerformADS();
		}
	}
	PerformADS();
}

void AEcdysisCharacter::PerformADS()
{
	if (!noADS)
	{
		GetWorldTimerManager().SetTimer(TimerHandle_HandleAdsZoomIn, this, &AEcdysisCharacter::HandleZoomIn, .02f, true);
	}
}

void AEcdysisCharacter::OnReleaseADS()
{
	if (!adsToggle)
	{
		//CancelADS();
	}
}

void AEcdysisCharacter::CancelADS()
{
		if(isADS)
		{
			GetWorldTimerManager().SetTimer(TimerHandle_HandleAdsZoomOut, this, &AEcdysisCharacter::HandleZoomOut, .02f, true);
		}
}

void AEcdysisCharacter::HandleZoomIn()
{	
	if(!isADS)
	{
		if (auto fpCam = GetFirstPersonCameraComponent())
		{
			if (fpCam->FieldOfView == fovADS)
			{
				GetWorldTimerManager().ClearTimer(TimerHandle_HandleStaminaDecrease);
				isADS = true;
			}
			else
			{
				fpCam->SetFieldOfView(UKismetMathLibrary::Lerp(fpCam->FieldOfView, fovADS, adsZoomSpeed));
			}
		}
	}
}

void AEcdysisCharacter::HandleZoomOut()
{

	if (isADS)
	{
		if (auto fpCam = GetFirstPersonCameraComponent())
		{
			if (fpCam->FieldOfView == playerFOV)
			{
				GetWorldTimerManager().ClearTimer(TimerHandle_HandleStaminaDecrease);
				isADS = false;
			}
			else
			{
				fpCam->SetFieldOfView(UKismetMathLibrary::Lerp(fpCam->FieldOfView, playerFOV, adsZoomSpeed));
			}
		}
	}
	else
	{
		if (auto fpCam = GetFirstPersonCameraComponent())
		{
			if (fpCam->FieldOfView == fovADS)
			{
				GetWorldTimerManager().ClearTimer(TimerHandle_HandleStaminaDecrease);
				isADS = true;
			}
			else
			{
				fpCam->SetFieldOfView(UKismetMathLibrary::Lerp(fpCam->FieldOfView, fovADS, adsZoomSpeed));
			}
		}
	}
}

void AEcdysisCharacter::OnFire()
{
		// try and play the sound if specified
	if (FireSound != nullptr)
	{
		UGameplayStatics::PlaySoundAtLocation(this, FireSound, GetActorLocation());
	}

	// try and play a firing animation if specified
	if (FireAnimation != nullptr)
	{
		// Get the animation object for the arms mesh
		UAnimInstance* AnimInstance = ArmsFpp->GetAnimInstance();
		if (AnimInstance != nullptr)
		{
			AnimInstance->Montage_Play(FireAnimation, 1.f);
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

//Commenting this section out to be consistent with FPS BP template.
//This allows the user to turn without using the right virtual joystick

//void AEcdysisCharacter::TouchUpdate(const ETouchIndex::Type FingerIndex, const FVector Location)
//{
//	if ((TouchItem.bIsPressed == true) && (TouchItem.FingerIndex == FingerIndex))
//	{
//		if (TouchItem.bIsPressed)
//		{
//			if (GetWorld() != nullptr)
//			{
//				UGameViewportClient* ViewportClient = GetWorld()->GetGameViewport();
//				if (ViewportClient != nullptr)
//				{
//					FVector MoveDelta = Location - TouchItem.Location;
//					FVector2D ScreenSize;
//					ViewportClient->GetViewportSize(ScreenSize);
//					FVector2D ScaledDelta = FVector2D(MoveDelta.X, MoveDelta.Y) / ScreenSize;
//					if (FMath::Abs(ScaledDelta.X) >= 4.0 / ScreenSize.X)
//					{
//						TouchItem.bMoved = true;
//						float Value = ScaledDelta.X * BaseTurnRate;
//						AddControllerYawInput(Value);
//					}
//					if (FMath::Abs(ScaledDelta.Y) >= 4.0 / ScreenSize.Y)
//					{
//						TouchItem.bMoved = true;
//						float Value = ScaledDelta.Y * BaseTurnRate;
//						AddControllerPitchInput(Value);
//					}
//					TouchItem.Location = Location;
//				}
//				TouchItem.Location = Location;
//			}
//		}
//	}
//}

void AEcdysisCharacter::MoveForward(float Value)
{
	if (Value != 0.0f)
	{
		// add movement in that direction
		AddMovementInput(GetActorForwardVector(), Value);
	}
}

void AEcdysisCharacter::MoveRight(float Value)
{
	if (Value != 0.0f)
	{
		// add movement in that direction
		AddMovementInput(GetActorRightVector(), Value);
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
