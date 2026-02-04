// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/DS1WeaponWidget.h"

#include "Components/Image.h"

void UDS1WeaponWidget::SetWeaponImage(UTexture2D* InTexture) const
{
	WeaponImage->SetBrushFromTexture(InTexture);
}
