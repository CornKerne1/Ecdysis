//// Fill out your copyright notice in the Description page of Project Settings.
//
//
//#include "WeaponBase.h"
//#include "BulletBase.h"
//#include "MagazineBase.h"
//#include "Runtime/Engine/Classes/Kismet/GameplayStatics.h"
//
//// Sets default values
//AWeaponBase::AWeaponBase()
//{
// 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
//	PrimaryActorTick.bCanEverTick = true;
//	WeaponFpp = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("gun"));
//	WeaponFpp->SetOnlyOwnerSee(true);
//	WeaponFpp -> bCastDynamicShadow = false;
//	WeaponFpp->CastShadow = false;
//
//	MuzzleLocation = CreateDefaultSubobject<USceneComponent>(TEXT("Muzzle Location"));
//	MuzzleLocation->SetupAttachment(WeaponFpp);
//	MuzzleLocation->SetRelativeLocation(FVector(0.2f, 48.4f, -10.6));
//
//	GunOffset = FVector(100.0f, 0.0f, 10.0f);
//
//
//}
//
//void AWeaponBase::OnFire()
//{
//	/*if (auto rc = roundChambered)
//	{
//		FireWeapon();
//		ChamberNextRound();
//	}*/
//	/*else
//	{
//
//	}*/
//}
//
//void AWeaponBase::FireWeapon()
//{
//}
//
//void AWeaponBase::ChamberNextRound()
//{
//	if (!playerRef->IsReloading())
//	{
//		if (auto cM = currentMag)
//		{
//			/*roundChambered = currentMag->magContents.GetData();*/
//		}
//	}
//}
//
//void AWeaponBase::AddMag(AMagazineBase* mag)
//{
//	currentMag = mag;
//}
//
//// Called when the game starts or when spawned
//void AWeaponBase::BeginPlay()
//{
//	Super::BeginPlay();
//	
//	playerRef = Cast<AEcdysisCharacter>(UGameplayStatics::GetPlayerCharacter(GetWorld(),0));
//}
//
//void AWeaponBase::AddAndEquip()
//{
//	//WeaponFpp->AttachToComponent(playerRef->__PPO__ArmsFpp);
//}
//
//// Called every frame
//void AWeaponBase::Tick(float DeltaTime)
//{
//	Super::Tick(DeltaTime);
//
//}
//
//void AWeaponBase::Interact(AEcdysisCharacter player)
//{
//	
//}
