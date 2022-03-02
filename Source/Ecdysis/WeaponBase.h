// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "EcdysisCharacter.h"
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

public:

	UPROPERTY(VisibleDefaultsOnly, Category = Mesh)
		class USkeletalMeshComponent* WeaponFpp;

	UPROPERTY(VisibleDefaultsOnly, Category = Gameplay)
		class USceneComponent* MuzzleLocation;

	UPROPERTY(VisibleDefaultsOnly, Category = Gameplay)
		FVector GunOffset;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Gameplay)
		float timeBetweenShots;

		AEcdysisCharacter* playerRef;

protected:
	
	virtual void AddAndEquip();

};
