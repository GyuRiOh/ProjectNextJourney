// Fill out your copyright notice in the Description page of Project Settings.

#include "UI/DS1InventorySlotWidget.h"

#include "Blueprint/WidgetBlueprintLibrary.h"
#include "Blueprint/WidgetTree.h"
#include "Components/Border.h"
#include "Components/Image.h"
#include "Components/Overlay.h"
#include "Components/OverlaySlot.h"
#include "Components/TextBlock.h"
#include "Components/DS1InventoryComponent.h"
#include "Components/DS1QuickSlotComponent.h"
#include "Data/DS1ItemData.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"
#include "UI/DS1InventoryDragDropOp.h"
#include "UI/DS1ItemTooltipWidget.h"

void UDS1InventorySlotWidget::InitSlot(UDS1InventoryComponent* InInventoryComponent, int32 InSlotIndex)
{
	InventoryComponent = InInventoryComponent;
	SlotIndex = InSlotIndex;

	RefreshSlot();
}

void UDS1InventorySlotWidget::RefreshSlot()
{
	if (!InventoryComponent || !InventoryComponent->GetInventorySlots().IsValidIndex(SlotIndex))
	{
		return;
	}

	const FDS1ItemInstance& SlotItem = InventoryComponent->GetSlot(SlotIndex);

	if (ItemIcon)
	{
		if (SlotItem.IsValid() && SlotItem.ItemData && SlotItem.ItemData->Icon)
		{
			ItemIcon->SetBrushFromTexture(SlotItem.ItemData->Icon);
			ItemIcon->SetVisibility(ESlateVisibility::Visible);
			if (SlotBorder)
			{
				SlotBorder->SetBrushColor(FLinearColor(0.16f, 0.14f, 0.11f, 0.96f));
			}
		}
		else
		{
			ItemIcon->SetVisibility(ESlateVisibility::Hidden);
			if (SlotBorder)
			{
				SlotBorder->SetBrushColor(FLinearColor(0.10f, 0.09f, 0.07f, 0.92f));
			}
		}
	}

	if (StackCountText)
	{
		if (SlotItem.IsValid() && SlotItem.StackCount > 1)
		{
			StackCountText->SetText(FText::AsNumber(SlotItem.StackCount));
			StackCountText->SetVisibility(ESlateVisibility::Visible);
		}
		else
		{
			StackCountText->SetVisibility(ESlateVisibility::Hidden);
		}
	}
}

void UDS1InventorySlotWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (!WidgetTree || SlotBorder)
	{
		return;
	}

	SlotBorder = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass(), TEXT("SlotBorder"));
	SlotBorder->SetPadding(FMargin(4.f));
	SlotBorder->SetBrushColor(FLinearColor(0.10f, 0.09f, 0.07f, 0.92f));
	WidgetTree->RootWidget = SlotBorder;

	UOverlay* Overlay = WidgetTree->ConstructWidget<UOverlay>(UOverlay::StaticClass(), TEXT("SlotOverlay"));
	SlotBorder->SetContent(Overlay);

	ItemIcon = WidgetTree->ConstructWidget<UImage>(UImage::StaticClass(), TEXT("ItemIcon"));
	ItemIcon->SetVisibility(ESlateVisibility::Hidden);
	if (UOverlaySlot* IconSlot = Overlay->AddChildToOverlay(ItemIcon))
	{
		IconSlot->SetHorizontalAlignment(HAlign_Fill);
		IconSlot->SetVerticalAlignment(VAlign_Fill);
		IconSlot->SetPadding(FMargin(8.f));
	}

	StackCountText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("StackCountText"));
	StackCountText->SetVisibility(ESlateVisibility::Hidden);
	StackCountText->SetColorAndOpacity(FSlateColor(FLinearColor(0.96f, 0.90f, 0.74f, 1.f)));
	if (UOverlaySlot* CountSlot = Overlay->AddChildToOverlay(StackCountText))
	{
		CountSlot->SetHorizontalAlignment(HAlign_Right);
		CountSlot->SetVerticalAlignment(VAlign_Bottom);
		CountSlot->SetPadding(FMargin(0.f, 0.f, 4.f, 2.f));
	}
}

FReply UDS1InventorySlotWidget::NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	if (InMouseEvent.GetEffectingButton() == EKeys::LeftMouseButton)
	{
		if (!InventoryComponent || !InventoryComponent->GetInventorySlots().IsValidIndex(SlotIndex))
		{
			return FReply::Unhandled();
		}

		const FDS1ItemInstance& SlotItem = InventoryComponent->GetSlot(SlotIndex);
		if (SlotItem.IsValid())
		{
			return UWidgetBlueprintLibrary::DetectDragIfPressed(InMouseEvent, this, EKeys::LeftMouseButton).NativeReply;
		}
	}
	else if (InMouseEvent.GetEffectingButton() == EKeys::RightMouseButton)
	{
		if (InMouseEvent.IsControlDown())
		{
			if (APlayerController* PC = GetOwningPlayer())
			{
				if (APawn* ControlledPawn = PC->GetPawn())
				{
					if (UDS1QuickSlotComponent* QuickSlotComp = ControlledPawn->FindComponentByClass<UDS1QuickSlotComponent>())
					{
						QuickSlotComp->RegisterToFirstEmptySlot(SlotIndex);
						return FReply::Handled();
					}
				}
			}
		}

		if (InventoryComponent && InventoryComponent->GetInventorySlots().IsValidIndex(SlotIndex))
		{
			const FDS1ItemInstance& SlotItem = InventoryComponent->GetSlot(SlotIndex);
			if (SlotItem.IsValid() && SlotItem.ItemData)
			{
				if (SlotItem.ItemData->ItemType == EDS1ItemType::Consumable)
				{
					InventoryComponent->UseConsumableFromSlot(SlotIndex);
				}
				else if (SlotItem.ItemData->ItemType == EDS1ItemType::Equipment)
				{
					InventoryComponent->EquipFromSlot(SlotIndex);
				}
			}
		}
	}

	return FReply::Handled();
}

void UDS1InventorySlotWidget::NativeOnDragDetected(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent, UDragDropOperation*& OutOperation)
{
	if (!InventoryComponent || !InventoryComponent->GetInventorySlots().IsValidIndex(SlotIndex))
	{
		return;
	}

	const FDS1ItemInstance& SlotItem = InventoryComponent->GetSlot(SlotIndex);
	if (!SlotItem.IsValid())
	{
		return;
	}

	UDS1InventoryDragDropOp* DragOp = NewObject<UDS1InventoryDragDropOp>();
	DragOp->SourceSlotIndex = SlotIndex;
	DragOp->bFromEquipSlot = false;
	DragOp->DraggedItem = SlotItem;

	if (SlotItem.ItemData && SlotItem.ItemData->Icon)
	{
		UImage* DragVisual = NewObject<UImage>();
		DragVisual->SetBrushFromTexture(SlotItem.ItemData->Icon);
		DragOp->DefaultDragVisual = DragVisual;
		DragOp->Pivot = EDragPivot::CenterCenter;
	}

	OutOperation = DragOp;
}

bool UDS1InventorySlotWidget::NativeOnDrop(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation)
{
	UDS1InventoryDragDropOp* DragOp = Cast<UDS1InventoryDragDropOp>(InOperation);
	if (!DragOp || !InventoryComponent)
	{
		return false;
	}

	if (DragOp->bFromEquipSlot)
	{
		return InventoryComponent->UnequipToInventory(DragOp->EquipSlotType);
	}

	if (DragOp->SourceSlotIndex != SlotIndex)
	{
		InventoryComponent->MoveItem(DragOp->SourceSlotIndex, SlotIndex);
		return true;
	}

	return false;
}

void UDS1InventorySlotWidget::NativeOnDragCancelled(const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation)
{
	Super::NativeOnDragCancelled(InDragDropEvent, InOperation);

	UDS1InventoryDragDropOp* DragOp = Cast<UDS1InventoryDragDropOp>(InOperation);
	if (DragOp && InventoryComponent && !DragOp->bFromEquipSlot)
	{
		InventoryComponent->DropItemFromSlot(DragOp->SourceSlotIndex);
	}
}

void UDS1InventorySlotWidget::NativeOnMouseEnter(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	Super::NativeOnMouseEnter(InGeometry, InMouseEvent);

	if (!InventoryComponent || !InventoryComponent->GetInventorySlots().IsValidIndex(SlotIndex))
	{
		return;
	}

	const FDS1ItemInstance& SlotItem = InventoryComponent->GetSlot(SlotIndex);
	TSubclassOf<UDS1ItemTooltipWidget> EffectiveTooltipClass = TooltipWidgetClass;
	if (!EffectiveTooltipClass)
	{
		EffectiveTooltipClass = UDS1ItemTooltipWidget::StaticClass();
	}

	if (SlotItem.IsValid() && SlotItem.ItemData && EffectiveTooltipClass)
	{
		UDS1ItemTooltipWidget* Tooltip = CreateWidget<UDS1ItemTooltipWidget>(GetOwningPlayer(), EffectiveTooltipClass);
		if (Tooltip)
		{
			Tooltip->SetItemData(SlotItem.ItemData);
			SetToolTip(Tooltip);
		}
	}
}

void UDS1InventorySlotWidget::NativeOnMouseLeave(const FPointerEvent& InMouseEvent)
{
	Super::NativeOnMouseLeave(InMouseEvent);
	SetToolTip(nullptr);
}
