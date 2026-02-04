// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/DS1StatBarWidget.h"

#include "Components/ProgressBar.h"

UDS1StatBarWidget::UDS1StatBarWidget(const FObjectInitializer& ObjectInitializer)
	:Super(ObjectInitializer)
{
}

void UDS1StatBarWidget::NativePreConstruct()
{
	Super::NativePreConstruct();

	if (StatBar)
	{
		StatBar->SetFillColorAndOpacity(FillColorAndOpacity);
	}
}

void UDS1StatBarWidget::SetRatio(float Ratio) const
{
	if (StatBar)
	{
		StatBar->SetPercent(Ratio);
	}
}