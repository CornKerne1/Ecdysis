// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "EcdysisCharacter.h"
#include "GameFramework/Actor.h"
#include "BaseInteractable.h"
#include "BaseWeapon.generated.h"

UCLASS()
class ECDYSIS_API ABaseWeapon : public ABaseInteractable
{
	GENERATED_BODY()

		/** Gun mesh: 1st person view (seen only by self) */
	UPROPERTY(VisibleDefaultsOnly, Category = Mesh)
	USkeletalMeshComponent* FP_Gun;
	/** Location on gun mesh where projectiles should spawn. */
	UPROPERTY(VisibleDefaultsOnly, Category = Mesh)
	USceneComponent* FP_MuzzleLocation;

	
public:	
	// Sets default values for this actor's properties
	ABaseWeapon();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Weapon_AmmoReserve)
	int currentAmmo;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Weapon_AmmoReserve)
	int maxAmmo;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Weapon_Ammo)
	int currentMagAmount;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Weapon_Ammo)
	int maxMagCapacity;	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Weapon_Reload)
	float reloadTime;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Weapon_Firing)
	UAnimMontage* FireAnimation;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Weapon_Firing)
	USoundBase* FireSound;
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	void ChamberRound();

	UPROPERTY()
	bool roundChambered;
	UPROPERTY(VisibleAnywhere, Category = Player)
	class AEcdysisCharacter* player;
public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	void FireWeapon();
	void ReloadWeapon();
	
	UFUNCTION(BlueprintCallable)
	void AddAndEquip(AActor* caller);
};
