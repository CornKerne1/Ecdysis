// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "EcdysisCharacter.h"
#include "MagazineBase.h"
#include "WeaponBase.generated.h"

UCLASS()
class ECDYSIS_API AWeaponBase : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AWeaponBase();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	void Interact(AEcdysisCharacter player);

	virtual void OnFire();

	virtual void FireWeapon();

	virtual void ChamberNextRound();

	virtual void AddMag(AMagazineBase* mag);

public:

	UPROPERTY(VisibleDefaultsOnly, Category = Weapon_Mesh)
	class USkeletalMeshComponent* WeaponFpp;

	UPROPERTY(VisibleDefaultsOnly, Category = Weapon_Mesh)
	class USceneComponent* MuzzleLocation;

	UPROPERTY(VisibleDefaultsOnly, Category = Weapon_Mesh)
	FVector GunOffset;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Weapon_Gameplay)
	float timeBetweenShots;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Weapon_Gameplay)
	float timeToReload;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Weapon_Gameplay)
	class AMagazineBase*  currentMag = nullptr;

	UPROPERTY()
	AEcdysisCharacter* playerRef;

protected:
	
	virtual void AddAndEquip();

	ABulletBase roundChambered;

};
