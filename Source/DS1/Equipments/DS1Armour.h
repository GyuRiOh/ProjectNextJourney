// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "DS1Define.h"
#include "Characters/DS1Character.h"
#include "Equipments/DS1Equipment.h"
#include "DS1Armour.generated.h"

/**
 * 
 */
UCLASS()
class DS1_API ADS1Armour : public ADS1Equipment
{
	GENERATED_BODY()

protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Equipment | Armour")
	EDS1ArmourType ArmourType = EDS1ArmourType::Chest;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Equipment | Armour")
	float DefenseAmount = 0.f;


public:
	ADS1Armour();

	virtual void EquipItem() override;

	virtual void UnequipItem() override;

	virtual void AttachToOwner(FName SocketName) override;

	FORCEINLINE EDS1ArmourType GetArmourType() const { return ArmourType; }

	void SetBodyPartActive(const bool bActive) const;
};

