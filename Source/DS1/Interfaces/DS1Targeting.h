// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "DS1Targeting.generated.h"

// This class does not need to be modified.
UINTERFACE()
class UDS1Targeting : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class DS1_API IDS1Targeting
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:
	virtual void OnTargeted(bool bTargeted) = 0;

	virtual bool CanBeTargeted() = 0;
};
