// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BaseWeapon.h"
#include "UObject/Object.h"
#include "TrueFPSAnimInstance.generated.h"

/**
 * 
 */
UCLASS()
class ECDYSIS_API UTrueFPSAnimInstance : public UAnimInstance
{
	GENERATED_BODY()

public:
	UTrueFPSAnimInstance();

protected:

	virtual void NativeBeginPlay() override;
	virtual void NativeUpdateAnimation(float DeltaTime) override;

	UFUNCTION()
	virtual void CurrentWeaponChanged(class ABaseWeapon* NewWeapon, const  class ABaseWeapon* OldWeapon);
	virtual void SetVars(const float DeltaTime);
	virtual void CalculateWeaponSway(const float DeltaTime);

	virtual void SetIKTransforms();

public:
	UPROPERTY(BlueprintReadWrite, Category = "Anim")
	class ATrueFPCharacter* Character;
	
	UPROPERTY(BlueprintReadWrite, Category = "Anim")
	class USkeletalMeshComponent* Mesh;

	UPROPERTY(BlueprintReadWrite, Category = "Anim")
	class ABaseWeapon* CurrentWeapon;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Anim")
	FIKProperties IkProperties;

	//State
	UPROPERTY(BlueprintReadOnly, Category="Anim")
	FRotator LastRotation;
	
	
	//IK Variables
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Anim")
	FTransform CameraTransform;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Anim")
	FTransform RelativeCameraTransform;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Anim")
	FTransform RHandToSightsTransform;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Anim")
	FTransform OffsetTransform;
	UPROPERTY(EditAnywhere, BlueprintReadWrite,Category="Anim")
	float AdsWeight = 0.f;

	//Accumulative Offsets
	UPROPERTY(BlueprintReadWrite, Category= "Anim")
	FRotator AccumulativeRotation;
	UPROPERTY(BlueprintReadWrite, Category= "Anim")
	FRotator AccumulativeRotationInterp;
	
	
};
