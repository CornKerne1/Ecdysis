// Fill out your copyright notice in the Description page of Project Settings.


#include "MagazineBase.h"
#include "BulletBase.h"
#include "Containers/Array.h"
#include "EngineUtils.h"
// Sets default values
AMagazineBase::AMagazineBase()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void AMagazineBase::BeginPlay()
{
	Super::BeginPlay();

	//if (auto world = GetWorld())
	//{
	//	for (TActorIterator<ABulletBase> It(world, ABulletBase::StaticClass()); It; ++It)
	//	{
	//		ABulletBase* actor = *It;
	//		if(actor != NULL)
	//			magContents.Add(actor);
	//	}
	//}
}

// Called every frame
void AMagazineBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AMagazineBase::AddRound(ABulletBase* roundToAdd, int amount)
{
	if (magContents.Num() <= maxCapacity)
	{
		int i;
		for (i = 0; i < amount; i++)
		{
			if (roundToAdd != NULL)
			{
				if (magContents.Num() <= maxCapacity)
				{
					magContents.Add(roundToAdd);

				}
			}
		}
	}
}

void AMagazineBase::RemoveRound()
{
	magContents.RemoveAt(magContents.Num() + 1);
}




