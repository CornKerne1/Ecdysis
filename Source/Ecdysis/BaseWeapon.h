// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "BaseInteractable.h"
#include "BaseWeapon.generated.h"

USTRUCT(BlueprintType)
struct FIKProperties
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere,BlueprintReadWrite)
	class UAnimSequence* AnimPose;

	UPROPERTY(EditAnywhere,BlueprintReadWrite)
	float AnimOffset = 15.f;

	UPROPERTY(EditAnywhere,BlueprintReadWrite)
	FTransform CustomOffsetTransform;

	UPROPERTY(EditAnywhere,BlueprintReadWrite)
	FVector RElbowModifier;

	UPROPERTY(EditAnywhere,BlueprintReadWrite)
	FVector LElbowModifier;
};

UCLASS(Abstract)
class ECDYSIS_API ABaseWeapon : public ABaseInteractable
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ABaseWeapon();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category ="Weapon_AmmoReserve")
	int currentAmmo;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon_AmmoReserve")
	int maxAmmo;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon_Ammo")
	int currentMagAmount;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon_Ammo")
	int maxMagCapacity;	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon_Reload")
	float reloadTime;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon_Firing")
	UAnimMontage* FireAnimation;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon_Firing")
	USoundBase* FireSound;
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	void ChamberRound();

	UPROPERTY()
	bool roundChambered;
	UPROPERTY(VisibleAnywhere, Category = Player)
	class ATrueFPCharacter* player;
public:	

	UPROPERTY(EditAnywhere,BlueprintReadWrite, Category="Configurations")
	FIKProperties IkProperties;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadWrite, Category = "State")
	class ATrueFPCharacter* CurrentOwner;
	
	UPROPERTY(VisibleAnywhere, Category = "Component")
	class USceneComponent* Root;
	UPROPERTY(VisibleAnywhere, Category = "Component")
	USkeletalMeshComponent* Mesh;

	UPROPERTY(EditAnywhere,BlueprintReadWrite, Category ="Configurations")
	FTransform PlacementTransform;
	
	void FireWeapon();
	void ReloadWeapon();

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="IK")
	FTransform GetSightsWorldTransform() const;
	virtual FTransform GetSightsWorldTransform_Implementation() const { return Mesh->GetSocketTransform(FName("Sights"));}
};
