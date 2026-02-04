// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Equipments/DS1Weapon.h"
#include "DS1FistWeapon.generated.h"

/**
 * 
 */
UCLASS()
class DS1_API ADS1FistWeapon : public ADS1Weapon
{
	GENERATED_BODY()

public:
	ADS1FistWeapon();

public:
	virtual void EquipItem() override;
	
};
