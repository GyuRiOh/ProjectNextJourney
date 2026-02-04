// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "DS1Define.h"
#include "UObject/Interface.h"
#include "DS1CombatInterface.generated.h"

struct FGameplayTag;

// This class does not need to be modified.
UINTERFACE()
class UDS1CombatInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class DS1_API IDS1CombatInterface
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:
	virtual void ActivateWeaponCollision(EWeaponCollisionType WeaponCollisionType) = 0;

	virtual void DeactivateWeaponCollision(EWeaponCollisionType WeaponCollisionType) = 0;

	virtual void PerformAttack(FGameplayTag& AttackTypeTag, FOnMontageEnded& MontageEndedDelegate) {}

	virtual void Parried() {}

	virtual void ToggleIFrames(const bool bEnabled) {}
};
