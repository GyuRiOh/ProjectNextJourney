// Fill out your copyright notice in the Description page of Project Settings.

#include "UI/DS1ItemTooltipWidget.h"

#include "Data/DS1ItemData.h"
#include "Components/TextBlock.h"

void UDS1ItemTooltipWidget::SetItemData(UDS1ItemData* InItemData)
{
	if (!InItemData)
	{
		return;
	}

	if (ItemNameText)
	{
		ItemNameText->SetText(InItemData->DisplayName);
	}

	if (ItemDescriptionText)
	{
		ItemDescriptionText->SetText(InItemData->Description);
	}

	if (ItemTypeText)
	{
		FText TypeText;
		switch (InItemData->ItemType)
		{
		case EDS1ItemType::Equipment:
			TypeText = FText::FromString(TEXT("장비"));
			break;
		case EDS1ItemType::Consumable:
			TypeText = FText::FromString(TEXT("소비"));
			break;
		case EDS1ItemType::Misc:
			TypeText = FText::FromString(TEXT("기타"));
			break;
		}
		ItemTypeText->SetText(TypeText);
	}

	if (ItemWeightText)
	{
		ItemWeightText->SetText(FText::FromString(FString::Printf(TEXT("무게: %.1f"), InItemData->Weight)));
	}

	if (ItemStatText)
	{
		FString StatString;
		if (InItemData->ItemType == EDS1ItemType::Consumable)
		{
			StatString = FString::Printf(TEXT("효과: +%.0f"), InItemData->EffectValue);
		}
		ItemStatText->SetText(FText::FromString(StatString));
	}
}
