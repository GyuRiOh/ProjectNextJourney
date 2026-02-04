// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "DS1WeaponWidget.generated.h"

class UImage;
/**
 * 
 */
UCLASS()
class DS1_API UDS1WeaponWidget : public UUserWidget
{
	GENERATED_BODY()

protected:
	UPROPERTY(meta=(BindWidget), BlueprintReadWrite)
	UImage* WeaponImage;

public:
	void SetWeaponImage(UTexture2D* InTexture) const;
};
