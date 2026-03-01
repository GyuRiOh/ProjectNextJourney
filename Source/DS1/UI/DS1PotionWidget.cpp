// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/DS1PotionWidget.h"

#include "Components/Image.h"
#include "Components/DS1QuickSlotComponent.h"
#include "Components/TextBlock.h"
#include "Data/DS1ItemData.h"
#include "UI/DS1InventoryDragDropOp.h"
#include "Blueprint/DragDropOperation.h"
#include "Kismet/KismetTextLibrary.h"

void UDS1PotionWidget::SetPotionQuantity(const int InAmount) const
{
	if (PotionQuantityText)
	{
		PotionQuantityText->SetText(UKismetTextLibrary::Conv_IntToText(InAmount));
	}
}

void UDS1PotionWidget::BindQuickSlot(UDS1QuickSlotComponent* InQuickSlotComponent)
{
	QuickSlotComponent = InQuickSlotComponent;
	if (!QuickSlotComponent)
	{
		return;
	}

	QuickSlotComponent->OnQuickSlotChanged.AddUObject(this, &ThisClass::OnQuickSlotChanged);
	RefreshQuickSlots();
}

void UDS1PotionWidget::RefreshQuickSlots()
{
	SetSlotVisual(0, QuickSlotIcon1, QuickSlotCount1);
	SetSlotVisual(1, QuickSlotIcon2, QuickSlotCount2);
	SetSlotVisual(2, QuickSlotIcon3, QuickSlotCount3);
	SetSlotVisual(3, QuickSlotIcon4, QuickSlotCount4);
	SetSlotVisual(4, QuickSlotIcon5, QuickSlotCount5);
	SetSlotVisual(5, QuickSlotIcon6, QuickSlotCount6);

	// Keep legacy potion text as the first slot count if the old HUD still uses it.
	if (PotionQuantityText)
	{
		const int32 FirstSlotCount = QuickSlotComponent ? QuickSlotComponent->GetItemCountAt(0) : 0;
		PotionQuantityText->SetText(UKismetTextLibrary::Conv_IntToText(FirstSlotCount));
	}
}

void UDS1PotionWidget::SetSlotVisual(int32 SlotIndex, UImage* SlotIconWidget, UTextBlock* SlotCountWidget) const
{
	if (!SlotIconWidget || !SlotCountWidget)
	{
		return;
	}

	if (!QuickSlotComponent)
	{
		SlotIconWidget->SetVisibility(ESlateVisibility::Hidden);
		SlotCountWidget->SetVisibility(ESlateVisibility::Hidden);
		return;
	}

	UDS1ItemData* ItemData = QuickSlotComponent->GetItemDataAt(SlotIndex);
	const int32 ItemCount = QuickSlotComponent->GetItemCountAt(SlotIndex);

	if (!ItemData || ItemCount <= 0)
	{
		if (EmptySlotIcon)
		{
			SlotIconWidget->SetBrushFromTexture(EmptySlotIcon);
			SlotIconWidget->SetVisibility(ESlateVisibility::Visible);
		}
		else
		{
			SlotIconWidget->SetVisibility(ESlateVisibility::Hidden);
		}
		SlotCountWidget->SetVisibility(ESlateVisibility::Hidden);
		return;
	}

	SlotIconWidget->SetBrushFromTexture(ItemData->Icon);
	SlotIconWidget->SetVisibility(ESlateVisibility::Visible);
	SlotCountWidget->SetText(UKismetTextLibrary::Conv_IntToText(ItemCount));
	SlotCountWidget->SetVisibility(ESlateVisibility::Visible);
}

void UDS1PotionWidget::OnQuickSlotChanged()
{
	RefreshQuickSlots();
}

bool UDS1PotionWidget::NativeOnDragOver(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation)
{
	if (Cast<UDS1InventoryDragDropOp>(InOperation))
	{
		return true;
	}

	return Super::NativeOnDragOver(InGeometry, InDragDropEvent, InOperation);
}

bool UDS1PotionWidget::NativeOnDrop(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation)
{
	UDS1InventoryDragDropOp* DragOp = Cast<UDS1InventoryDragDropOp>(InOperation);
	if (!DragOp || !QuickSlotComponent || !DragOp->DraggedItem.IsValid())
	{
		return Super::NativeOnDrop(InGeometry, InDragDropEvent, InOperation);
	}

	const int32 QuickSlotIndex = GetSlotIndexFromGeometry(InGeometry, InDragDropEvent);
	if (QuickSlotIndex < 0 || QuickSlotIndex >= 6)
	{
		return false;
	}

	const bool bRegistered = QuickSlotComponent->RegisterSlot(QuickSlotIndex, DragOp->DraggedItem.ItemData);
	if (bRegistered)
	{
		RefreshQuickSlots();
	}
	return bRegistered;
}

int32 UDS1PotionWidget::GetSlotIndexFromGeometry(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent) const
{
	const FVector2D LocalPosition = InGeometry.AbsoluteToLocal(InDragDropEvent.GetScreenSpacePosition());
	const FVector2D Size = InGeometry.GetLocalSize();
	if (Size.X <= KINDA_SMALL_NUMBER)
	{
		return -1;
	}

	const float SlotWidth = Size.X / 6.f;
	const int32 SlotIndex = FMath::FloorToInt(LocalPosition.X / SlotWidth);
	return FMath::Clamp(SlotIndex, 0, 5);
}
