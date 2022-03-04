// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "BulletBase.h"
#include "MagazineBase.generated.h"

UCLASS()
class ECDYSIS_API AMagazineBase : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AMagazineBase();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Magazine_Ammo)
	TArray<ABulletBase*> magContents;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Magazine_Ammo)
	int maxCapacity;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	void virtual AddRound(ABulletBase* roundToAdd, int amount);

	void virtual RemoveRound();


};
