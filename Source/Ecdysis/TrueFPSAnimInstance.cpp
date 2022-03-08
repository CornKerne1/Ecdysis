// Fill out your copyright notice in the Description page of Project Settings.


#include "TrueFPSAnimInstance.h"
#include "Camera/CameraComponent.h"
#include "TrueFPCharacter.h"
#include "Kismet/KismetMathLibrary.h"

UTrueFPSAnimInstance::UTrueFPSAnimInstance()
{
}

void UTrueFPSAnimInstance::NativeBeginPlay()
{
	Super::NativeBeginPlay();

	Character = Cast<ATrueFPCharacter>(TryGetPawnOwner());
	/*if(Character)
	{
		Mesh = Character->GetMesh();
		Character->CurrentWeaponChangedDelegate.AddDynamic(this, &UTrueFPSAnimInstance::CurrentWeaponChanged);
		CurrentWeaponChanged(Character->CurrentWeapon, nullptr);
	}*/
}

void UTrueFPSAnimInstance::NativeUpdateAnimation(float DeltaTime)
{
	Super::NativeUpdateAnimation(DeltaTime);

	if(!Character)
	{
		Character = Cast<ATrueFPCharacter>(TryGetPawnOwner());
		if(Character)
		{
			Mesh = Character->GetMesh();
			Character->CurrentWeaponChangedDelegate.AddDynamic(this, &UTrueFPSAnimInstance::CurrentWeaponChanged);
			CurrentWeaponChanged(Character->CurrentWeapon, nullptr);
		}
		else return;
	}
	SetVars(DeltaTime);
	CalculateWeaponSway(DeltaTime);

	LastRotation = CameraTransform.Rotator();

}

void UTrueFPSAnimInstance::CurrentWeaponChanged(ABaseWeapon* NewWeapon, const ABaseWeapon* OldWeapon)
{
	CurrentWeapon = NewWeapon;
	if(CurrentWeapon)
	{
		IkProperties = CurrentWeapon->IkProperties;
		GetWorld()->GetTimerManager().SetTimerForNextTick(this, &UTrueFPSAnimInstance::SetIKTransforms);
	}
	else
	{
		
	}
}

void UTrueFPSAnimInstance::SetVars(const float DeltaTime)
{
	CameraTransform = FTransform(Character->GetBaseAimRotation(), Character->Camera->GetComponentLocation());

	const FTransform RootOffset = Mesh->GetSocketTransform(FName("root"), RTS_Component).Inverse() * Mesh->GetSocketTransform(FName("ik_hand_root"));
	RelativeCameraTransform = CameraTransform.GetRelativeTransform(RootOffset);

	AdsWeight = Character->ADSWeight;

	//Offsets(WeaponSway)

	//Accumulative Rotation
	constexpr float AngleClamp =6.f;
	const FRotator& AddRotation = CameraTransform.Rotator() - LastRotation;
	FRotator AddRotationClamped = FRotator(FMath::ClampAngle(AddRotation.Pitch, - AngleClamp, AngleClamp) * 1.5f,
		FMath::ClampAngle(AddRotation.Yaw, -AngleClamp, AngleClamp), 0.f);
	AddRotationClamped.Roll = AddRotationClamped.Yaw * 0.7f;

	AccumulativeRotation += AddRotationClamped;
	AccumulativeRotation = UKismetMathLibrary::RInterpTo(AccumulativeRotation, FRotator::ZeroRotator, DeltaTime, 30.f);//Expose for weapon sway
	AccumulativeRotationInterp = UKismetMathLibrary::RInterpTo(AccumulativeRotationInterp, AccumulativeRotation, DeltaTime, 5.f);//Expose for weapon sway
}

void UTrueFPSAnimInstance::CalculateWeaponSway(const float DeltaTime)
{
	FVector LocationOffset = FVector::ZeroVector;
	FRotator RotationOffset = FRotator::ZeroRotator;

	const FRotator& AccumulativeRotationInterpInverse = AccumulativeRotationInterp.GetInverse();
	RotationOffset += AccumulativeRotationInterpInverse;

	LocationOffset += FVector(0.f, AccumulativeRotationInterpInverse.Yaw, AccumulativeRotationInterpInverse.Pitch) / 6.f;

	OffsetTransform = FTransform(RotationOffset, LocationOffset);
}

void UTrueFPSAnimInstance::SetIKTransforms()
{
	RHandToSightsTransform = CurrentWeapon->GetSightsWorldTransform().GetRelativeTransform(Mesh->GetSocketTransform(FName("hand_r")));
}
