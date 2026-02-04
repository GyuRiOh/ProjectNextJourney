// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "DS1PotionInventoryComponent.generated.h"


class ADS1Potion;

DECLARE_MULTICAST_DELEGATE_OneParam(FDelegateOnUpdatePotionAmount, int32);

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class DS1_API UDS1PotionInventoryComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	FDelegateOnUpdatePotionAmount OnUpdatePotionAmount;

protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 PotionQuantity = 10;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float PotionHealAmount = 15.f;

	UPROPERTY(EditAnywhere)
	FName PotionSocketName;

	UPROPERTY(EditAnywhere)
	TSubclassOf<ADS1Potion> PotionClass;

	UPROPERTY()
	ADS1Potion* PotionActor;
	

public:	
	UDS1PotionInventoryComponent();

	void DrinkPotion();

	void SpawnPotion();

	void DespawnPotion();

	void SetPointQuantity(int32 InQuantity);
	FORCEINLINE int GetPotionQuantity() const { return PotionQuantity; }

	void BroadcastPotionUpdate() const;
};
