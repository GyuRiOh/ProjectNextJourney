// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/DS1PotionWidget.h"

#include "Components/TextBlock.h"
#include "Kismet/KismetTextLibrary.h"

void UDS1PotionWidget::SetPotionQuantity(const int InAmount) const
{
	PotionQuantityText->SetText(UKismetTextLibrary::Conv_IntToText(InAmount));
}
