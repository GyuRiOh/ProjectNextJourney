// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Equipments/DS1Equipment.h"
#include "DS1Shield.generated.h"

/**
 * 
 */
UCLASS()
class DS1_API ADS1Shield : public ADS1Equipment
{
	GENERATED_BODY()

protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName EquipSocketName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName UnequipSocketName;

public:
	virtual void EquipItem() override;

	FORCEINLINE FName GetEquipSocketName() const { return EquipSocketName; }
	FORCEINLINE FName GetUnequipSocketName() const { return UnequipSocketName; }

};
