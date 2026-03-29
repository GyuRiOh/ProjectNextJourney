// Fill out your copyright notice in the Description page of Project Settings.

#include "UI/DS1ItemTooltipWidget.h"

#include "Blueprint/WidgetTree.h"
#include "Components/Border.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"
#include "Components/VerticalBoxSlot.h"
#include "Data/DS1ItemData.h"

void UDS1ItemTooltipWidget::SetItemData(UDS1ItemData* InItemData)
{
	if (!InItemData)
	{
		return;
	}

	if (WidgetTree && !ItemNameText)
	{
		UBorder* RootBorder = Cast<UBorder>(GetRootWidget());
		UVerticalBox* RootBox = RootBorder ? Cast<UVerticalBox>(RootBorder->GetContent()) : nullptr;
		if (!RootBox)
		{
			RootBorder = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass(), TEXT("TooltipBorder"));
			RootBorder->SetPadding(FMargin(8.f));
			RootBorder->SetBrushColor(FLinearColor(0.04f, 0.04f, 0.03f, 0.96f));

			RootBox = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), TEXT("TooltipRoot"));
			RootBorder->SetContent(RootBox);
			WidgetTree->RootWidget = RootBorder;
		}

		auto AddText = [this, RootBox](const TCHAR* Name) -> UTextBlock*
		{
			UTextBlock* Text = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), Name);
			if (UVerticalBoxSlot* Slot = RootBox->AddChildToVerticalBox(Text))
			{
				Slot->SetPadding(FMargin(4.f, 2.f));
			}
			return Text;
		};

		ItemNameText = AddText(TEXT("ItemNameText"));
		ItemDescriptionText = AddText(TEXT("ItemDescriptionText"));
		ItemTypeText = AddText(TEXT("ItemTypeText"));
		ItemWeightText = AddText(TEXT("ItemWeightText"));
		ItemStatText = AddText(TEXT("ItemStatText"));
	}

	if (ItemNameText)
	{
		ItemNameText->SetText(InItemData->DisplayName);
		ItemNameText->SetColorAndOpacity(FSlateColor(FLinearColor(0.95f, 0.86f, 0.65f, 1.f)));
	}

	if (ItemDescriptionText)
	{
		ItemDescriptionText->SetText(InItemData->Description);
		ItemDescriptionText->SetColorAndOpacity(FSlateColor(FLinearColor(0.78f, 0.76f, 0.71f, 1.f)));
	}

	if (ItemTypeText)
	{
		FText TypeText;
		switch (InItemData->ItemType)
		{
		case EDS1ItemType::Equipment:
			TypeText = FText::FromString(TEXT("Equipment"));
			break;
		case EDS1ItemType::Consumable:
			TypeText = FText::FromString(TEXT("Consumable"));
			break;
		case EDS1ItemType::Misc:
		default:
			TypeText = FText::FromString(TEXT("Misc"));
			break;
		}
		ItemTypeText->SetText(TypeText);
		ItemTypeText->SetColorAndOpacity(FSlateColor(FLinearColor(0.70f, 0.68f, 0.62f, 1.f)));
	}

	if (ItemWeightText)
	{
		ItemWeightText->SetText(FText::FromString(FString::Printf(TEXT("Weight: %.1f"), InItemData->Weight)));
		ItemWeightText->SetColorAndOpacity(FSlateColor(FLinearColor(0.70f, 0.68f, 0.62f, 1.f)));
	}

	if (ItemStatText)
	{
		FString StatString;
		if (InItemData->ItemType == EDS1ItemType::Consumable)
		{
			StatString = FString::Printf(TEXT("Effect: +%.0f"), InItemData->EffectValue);
		}
		ItemStatText->SetText(FText::FromString(StatString));
	}
}
