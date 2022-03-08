// Fill out your copyright notice in the Description page of Project Settings.


#include "TrueFPCharacter.h"
#include "BaseWeapon.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/GameModeBase.h"
#include "Net/UnrealNetwork.h"
#include "Engine.h"
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

// Sets default values
ATrueFPCharacter::ATrueFPCharacter()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	GetMesh()->SetTickGroup(ETickingGroup::TG_PostUpdateWork);
	GetMesh()->bVisibleInReflectionCaptures = true;
	GetMesh()->bCastHiddenShadow = true;

	ClientMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("ClientMesh"));
	ClientMesh->SetCastShadow(false);
	ClientMesh->bCastHiddenShadow = false;
	ClientMesh->bVisibleInReflectionCaptures = false;
	ClientMesh->SetTickGroup(ETickingGroup::TG_PostUpdateWork);
	ClientMesh->SetupAttachment(GetMesh());
	
	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	Camera->bUsePawnControlRotation = true;
	Camera->SetupAttachment(GetMesh(), FName("head"));

}

// Called when the game starts or when spawned
void ATrueFPCharacter::BeginPlay()
{
	Super::BeginPlay();
	
	Initialize();
}
void ATrueFPCharacter::Initialize()
{
	currentStamina = maxStamina;
	currentStrafeSpeedModifier = strafeSpeedModifier;
	canLean = true;
	if (auto cam = this->Camera)
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
	//StartInteractionSystem();

	//ADS Timeline
	if(AimingCurve)
	{
		FOnTimelineFloat TimelineFloat;
		TimelineFloat.BindDynamic(this, &ATrueFPCharacter::TimeLineProgress);

		AimingTimeline.AddInterpFloat(AimingCurve, TimelineFloat);
	}

	//Client Mesh Logic
	if(IsLocallyControlled())
	{
		ClientMesh->HideBoneByName(FName("neck_01"), EPhysBodyOp::PBO_None);
		GetMesh()->SetVisibility(false);
	}
	//Spawning Weapons
	for(const TSubclassOf<ABaseWeapon>& WeaponClass : DefaultWeapons)
	{
		if(!WeaponClass) continue;
		FActorSpawnParameters Params;
		Params.Owner = this;
		ABaseWeapon* SpawnedWeapon = GetWorld()->SpawnActor<ABaseWeapon>(WeaponClass, Params);
		const int32 Index = Weapons.Add(SpawnedWeapon);
		if(Index == CurrentIndex)
		{
			CurrentWeapon = SpawnedWeapon;
			OnRep_CurrentWeapon(nullptr);
		}
	}
}
void ATrueFPCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION(ATrueFPCharacter, Weapons, COND_None);
	DOREPLIFETIME_CONDITION(ATrueFPCharacter, CurrentWeapon, COND_None);
	DOREPLIFETIME_CONDITION(ATrueFPCharacter, ADSWeight, COND_None);
}

void ATrueFPCharacter::PreReplication(IRepChangedPropertyTracker& ChangedPropertyTracker)
{
	Super::PreReplication(ChangedPropertyTracker);

	DOREPLIFETIME_ACTIVE_OVERRIDE(ATrueFPCharacter, ADSWeight, ADSWeight >= 1 || ADSWeight <= 0.f);
}

//Sprintt
void ATrueFPCharacter::ResetAndStartTimer(FTimerHandle handle, float loopTime, bool looping)
{
	GetWorldTimerManager().ClearTimer(TimerHandle_HandleStaminaIncrease);
	GetWorldTimerManager().ClearTimer(TimerHandle_HandleStaminaDecrease);
	GetWorldTimerManager().SetTimer(TimerHandle_HandleStaminaDecrease, this, &ATrueFPCharacter::ReduceStamina, 1, true);
}
void ATrueFPCharacter::HandleGroundMovementType()
{
	if (movementType > 2)
	{
		movementType = 0;
	}
	
}
void ATrueFPCharacter::Sprint()
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
			currentStrafeSpeedModifier = strafeSpeedModifier;
			canLean = true;
			if (!CheckStamina() || currentStamina < maxStamina)
			{
				GetWorldTimerManager().SetTimer(TimerHandle_HandleStaminaIncrease, this, &ATrueFPCharacter::IncreaseStamina, 1, true);
			}
			break;
		case 1://SuperSprint
			if (isADS)
			{
				PerformADS();
			}
			if (CheckStamina() && currentStamina > 0.75f * maxStamina)
			{
				canLean = false;
				currentStrafeSpeedModifier = 0.3f;
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
				canLean = false;
				currentStrafeSpeedModifier = 0.4f;
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

void ATrueFPCharacter::StopSprint()
{
	if (movementType != 0)
	{
		movementType = 2;
		Sprint();
	}
}

void ATrueFPCharacter::OnLean(float Val)
{
	leanState = Val;
	float leanValue = GetControlRotation().Roll;
	if (!canLean)
	{
		leanState = 0;
	}

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

bool ATrueFPCharacter::CheckStamina()
{
	if (currentStamina <= 0) { currentStamina = 0; return false; }

	else
		if (currentStamina >= maxStamina) { currentStamina = maxStamina; }
	return true;


}

void ATrueFPCharacter::ReduceStamina()
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

void ATrueFPCharacter::IncreaseStamina()
{
	currentStamina = currentStamina + staminaIncrease;
}

//CROUCH
void ATrueFPCharacter::OnCrouch()
{
	crouchKeyDown = true;
	if (isCrouched)
	{
		GetWorldTimerManager().SetTimer(TimerHandle_CrouchHoldCheck, this, &ATrueFPCharacter::CrouchHoldCheck, .05, false);
	}
	else
	{
		StopSprint();
		PerformCrouch();
	}
}

void ATrueFPCharacter::OnUnCrouch()
{
	crouchKeyDown = false;
	PerformCrouch();
}

void ATrueFPCharacter::PerformCrouch()
{
	if (!crouchZoom)
	{
		crouchZoom = true;
		GetWorldTimerManager().SetTimer(TimerHandle_Crouch, this, &ATrueFPCharacter::HandleCrouchZoom, .01f, true);
	}
}

void ATrueFPCharacter::CrouchHoldCheck()
{
	if (!crouchKeyDown)
	{
		StopSprint();
		PerformCrouch();
	}
	GetWorldTimerManager().ClearTimer(TimerHandle_CrouchHoldCheck);
}

void ATrueFPCharacter::HandleCrouchZoom()
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


//ADS
void ATrueFPCharacter::Multi_Aim_Implementation(const bool bForward)
{
	if(bForward)
	{
		AimingTimeline.Play();
	}
	else
	{
		AimingTimeline.Reverse();
	}
}

void ATrueFPCharacter::TimeLineProgress(const float Value)
{
	ADSWeight = Value;
}
void ATrueFPCharacter::OnPressADS()
{
	adsKeyDown = true;
	if (isADS)
	{
		GetWorldTimerManager().SetTimer(TimerHandle_AdsHoldCheck, this, &ATrueFPCharacter::AdsHoldCheck, .05, false);
	}
	else
	{
		StopSprint();
		PerformADS();
	}
}
void ATrueFPCharacter::PerformADS()
{
	if(IsLocallyControlled() || HasAuthority())
	{
		if(!adsZoom)
		{
			adsZoom = true;
			GetWorldTimerManager().SetTimer(TimerHandle_AdsZoom, this, &ATrueFPCharacter::HandleZoomIn, .02f, true);
			if(!isADS)Multi_Aim_Implementation(true);
			else Multi_Aim_Implementation(false);
		}
	}
	if(!HasAuthority())
	{
		if(!isADS)Server_StartAim(true);
		else Server_StartAim(false);
	}
}
void ATrueFPCharacter::OnReleaseADS()
{
	adsKeyDown = false;
	PerformADS();


}
void ATrueFPCharacter::AdsHoldCheck()
{
	if (!adsKeyDown)
	{		
		StopSprint();
		PerformADS();
	}
	GetWorldTimerManager().ClearTimer(TimerHandle_AdsHoldCheck);
}
void ATrueFPCharacter::CancelADS()
{
	if (!adsZoom)
	{
		adsZoom = true;
	}
}
void ATrueFPCharacter::HandleZoomIn()
{
	if (adsZoom)
	{
		if (isADS)
		{
			if (auto fpCam = this->Camera)
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
		else if (auto fpCam = this->Camera)
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

void ATrueFPCharacter::MoveForward(float Value)
{
	if (Value != 0.0f)
	{
		// add movement in that direction
		const FVector& Direction = FRotationMatrix(FRotator(0.f, GetControlRotation().Yaw, 0.f)).GetUnitAxis(EAxis::X);
		AddMovementInput(Direction, Value);
	}
	else
	{
		StopSprint();
	}
}


void ATrueFPCharacter::MoveRight(float Value)
{
	if (Value != 0.0f)
	{
		const FVector& Direction = FRotationMatrix(FRotator(0.f, GetControlRotation().Yaw, 0.f)).GetUnitAxis(EAxis::Y);
		// add movement in that direction
		AddMovementInput(Direction, (Value * currentStrafeSpeedModifier));
	}
}

void ATrueFPCharacter::TurnAtRate(float Rate)
{
	// calculate delta for this frame from the rate information
	AddControllerYawInput(Rate * BaseTurnRate * GetWorld()->GetDeltaSeconds());
}

void ATrueFPCharacter::LookUpAtRate(float Rate)
{
	// calculate delta for this frame from the rate information
	AddControllerPitchInput(Rate * BaseLookUpRate * GetWorld()->GetDeltaSeconds());
}

void ATrueFPCharacter::EquipWeapon(const int32 Index)
{
	if(!Weapons.IsValidIndex(Index) || CurrentWeapon == Weapons[Index]) return;

	if(IsLocallyControlled() || HasAuthority())
	{
		CurrentIndex = Index;
		GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Yellow, TEXT("Some debug message!"));  
		const ABaseWeapon* OldWeapon = CurrentWeapon;
		CurrentWeapon = Weapons[Index];
		OnRep_CurrentWeapon(OldWeapon);
	}
	if(!HasAuthority())
	{
		Server_SetCurrentWeapon(Weapons[Index]);
	}
	
}


void ATrueFPCharacter::OnRep_CurrentWeapon(const ABaseWeapon* OldWeapon)
{
	if(CurrentWeapon)
	{
		if(!CurrentWeapon->CurrentOwner)
		{
			const FTransform& PlacementTransform = CurrentWeapon->PlacementTransform * GetMesh()->GetSocketTransform(FName("GripPoint_R"));
			CurrentWeapon->SetActorTransform(PlacementTransform, false, nullptr, ETeleportType::TeleportPhysics);
			CurrentWeapon->AttachToComponent(GetMesh(), FAttachmentTransformRules::KeepWorldTransform, FName("GripPoint_R"));
			//CurrentWeapon->GetRootComponent()->AttachToComponent(GetMesh(), FAttachmentTransformRules::KeepWorldTransform, FName("GripPoint_R"));
			//CurrentWeapon->Mesh->AttachToComponent(GetMesh(), FAttachmentTransformRules::KeepWorldTransform, FName("GripPoint_R"));
			
			CurrentWeapon->CurrentOwner = this;
		}
		CurrentWeapon->Mesh->SetVisibility(true);
	}

	if(OldWeapon)
	{
		OldWeapon->Mesh->SetVisibility(false);
	}
	CurrentWeaponChangedDelegate.Broadcast(CurrentWeapon, OldWeapon);
}

void ATrueFPCharacter::Server_SetCurrentWeapon_Implementation(ABaseWeapon* NewWeapon)
{
	const ABaseWeapon* OldWeapon = CurrentWeapon;
	CurrentWeapon = NewWeapon;
	OnRep_CurrentWeapon(OldWeapon);
}

void ATrueFPCharacter::NextWeapon()
{
	const int32 Index = Weapons.IsValidIndex(CurrentIndex + 1) ? CurrentIndex + 1 : 0;
	EquipWeapon(Index);
}

void ATrueFPCharacter::LastWeapon()
{
	
	const int32 Index = Weapons.IsValidIndex(CurrentIndex - 1) ? CurrentIndex - 1 : Weapons.Num() - 1;
	EquipWeapon(Index);
	
}

// Called every frame
void ATrueFPCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	deltaTime = DeltaTime;

	AimingTimeline.TickTimeline(DeltaTime);

}

// Called to bind functionality to input
void ATrueFPCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	// set up gameplay key bindings
	check(PlayerInputComponent);

	// Bind jump events
	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &ACharacter::Jump);
	PlayerInputComponent->BindAction("Jump", IE_Released, this, &ACharacter::StopJumping);

	PlayerInputComponent->BindAction("NextWeapon", IE_Pressed, this, &ATrueFPCharacter::NextWeapon);
	PlayerInputComponent->BindAction("LastWeapon", IE_Released, this, &ATrueFPCharacter::LastWeapon);
	
	// Bind fire event
	//PlayerInputComponent->BindAction("Fire", IE_Pressed, this, &ATrueFPCharacter::OnFire);
	//PlayerInputComponent->BindAction("Fire", IE_Released, this, &ATrueFPCharacter::StopFire);

	// Bind crouch events
	PlayerInputComponent->BindAction("Crouch", IE_Pressed, this, &ATrueFPCharacter::OnCrouch);
	PlayerInputComponent->BindAction("Crouch", IE_Released, this, &ATrueFPCharacter::OnUnCrouch);

	// Bind movement events
	PlayerInputComponent->BindAxis("MoveForward", this, &ATrueFPCharacter::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &ATrueFPCharacter::MoveRight);

	PlayerInputComponent->BindAction("Sprint", IE_Pressed, this, &ATrueFPCharacter::Sprint);
	PlayerInputComponent->BindAction("Sprint", IE_Released, this, &ATrueFPCharacter::StopSprint);

	PlayerInputComponent->BindAction("ADS", IE_Pressed, this, &ATrueFPCharacter::OnPressADS);
	PlayerInputComponent->BindAction("ADS", IE_Released, this, &ATrueFPCharacter::OnReleaseADS);

	//PlayerInputComponent->BindAction("Reload", IE_Pressed, this, &ATrueFPCharacter::OnReload);

	//PlayerInputComponent->BindAction("Interact", IE_Pressed, this, &ATrueFPCharacter::OnInteract);
	
	// We have 2 versions of the rotation bindings to handle different kinds of devices differently
	// "turn" handles devices that provide an absolute delta, such as a mouse.
	// "turnrate" is for devices that we choose to treat as a rate of change, such as an analog joystick
	PlayerInputComponent->BindAxis("Turn", this, &APawn::AddControllerYawInput);
	PlayerInputComponent->BindAxis("TurnRate", this, &ATrueFPCharacter::TurnAtRate);
	PlayerInputComponent->BindAxis("LookUp", this, &APawn::AddControllerPitchInput);
	PlayerInputComponent->BindAxis("LookUpRate", this, &ATrueFPCharacter::LookUpAtRate);
	PlayerInputComponent->BindAxis("Lean", this, &ATrueFPCharacter::OnLean);

}

