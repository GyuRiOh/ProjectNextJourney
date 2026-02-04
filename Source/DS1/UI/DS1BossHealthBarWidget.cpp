// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/DS1BossHealthBarWidget.h"

#include "DS1StatBarWidget.h"

void UDS1BossHealthBarWidget::Init(UDS1AttributeComponent* AttributeComponent)
{
	if (AttributeComponent)
	{
		// 델리게이트
		AttributeComponent->OnAttributeChanged.AddUObject(this, &ThisClass::OnAttributeChanged);
		AttributeComponent->BroadcastAttributeChanged(EDS1AttributeType::Health);
	}
}

void UDS1BossHealthBarWidget::OnAttributeChanged(EDS1AttributeType AttributeType, float InValue)
{
	if (AttributeType == EDS1AttributeType::Health)
	{
		HealthBarWidget->SetRatio(InValue);
	}
}
