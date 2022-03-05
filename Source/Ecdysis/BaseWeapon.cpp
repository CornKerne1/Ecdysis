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
	PrimaryActorTick.bCanEverTick = true;

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

	player = nullptr;
}

// Called when the game starts or when spawned
void ABaseWeapon::BeginPlay()
{
	Super::BeginPlay();
	
}

void ABaseWeapon::ChamberRound()
{
	if (!roundChambered && currentMagAmount > 0)
	{
		currentMagAmount -= 1;
		roundChambered = true;
	}
}

// Called every frame
void ABaseWeapon::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

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
			player->PlayAnimationMontage(FireAnimation);
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

void ABaseWeapon::AddAndEquip(AActor* caller)
{
	if (auto p = Cast<AEcdysisCharacter>(caller))
	{
		player = p;
		player->equippedWeapon = this;
	}
}


