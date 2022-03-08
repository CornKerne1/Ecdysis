// Fill out your copyright notice in the Description page of Project Settings.


#include "BaseWeapon.h"
#include "Kismet/GameplayStatics.h"
#include "XProjectile/Public/XProjectileMBC.h"
#include "XProjectile/Public/XProjectileMBOT.h"
#include "XProjectile/Public/XProjectileSBC.h"
#include "XProjectile/Public/XProjectileSBOT.h"

// Sets default values
ABaseWeapon::ABaseWeapon()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;
	
	SetReplicates(true);

	Mesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("Mesh"));

	Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	//RootComponent = Root;
	player = nullptr;
}

// Called when the game starts or when spawned
void ABaseWeapon::BeginPlay()
{
	Super::BeginPlay();

	if(!CurrentOwner)
	{
		Mesh->SetVisibility(false);
	}
	
}

void ABaseWeapon::ChamberRound()
{
	if (!roundChambered && currentMagAmount > 0)
	{
		currentMagAmount -= 1;
		roundChambered = true;
	}
}


void ABaseWeapon::FireWeapon()
{
	if (auto p = player)
	{
		if (roundChambered)
		{
			//Spawn Projectile
			//UXProjectileSBOT::SpawnProjectileSingleByObjectType()
			roundChambered = false;
			if (FireSound != nullptr)
			{
				UGameplayStatics::PlaySoundAtLocation(this, FireSound, GetActorLocation());
			}
			//player->PlayAnimationMontage(FireAnimation);
			ChamberRound();
		}
		else
		{
			ChamberRound();
		}
	
	}

}

void ABaseWeapon::ReloadWeapon()
{
	if (currentMagAmount != maxMagCapacity)
	{
		if (currentAmmo - (maxMagCapacity - currentMagAmount) >= 0)
		{
			currentAmmo -= (maxMagCapacity - currentMagAmount);
			currentMagAmount = maxMagCapacity;
		}
		else
		{
			currentMagAmount += currentAmmo;
			currentAmmo = 0;
		}
	}
}


