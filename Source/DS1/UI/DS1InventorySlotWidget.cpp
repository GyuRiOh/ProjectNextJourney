// Fill out your copyright notice in the Description page of Project Settings.

#include "UI/DS1InventorySlotWidget.h"

#include "Components/Border.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "Components/DS1InventoryComponent.h"
#include "Components/DS1QuickSlotComponent.h"
#include "Data/DS1ItemData.h"
#include "UI/DS1InventoryDragDropOp.h"
#include "UI/DS1ItemTooltipWidget.h"
#include "Blueprint/WidgetBlueprintLibrary.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"

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
		if (SlotItem.IsValid() && SlotItem.ItemData->Icon)
		{
			ItemIcon->SetBrushFromTexture(SlotItem.ItemData->Icon);
			ItemIcon->SetVisibility(ESlateVisibility::Visible);
		}
		else
		{
			ItemIcon->SetVisibility(ESlateVisibility::Hidden);
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
		// Ctrl + RightClick: register to the first empty quick slot.
		if (InMouseEvent.IsControlDown())
		{
			if (APlayerController* PC = GetOwningPlayer())
			{
				if (APawn* Pawn = PC->GetPawn())
				{
					if (UDS1QuickSlotComponent* QuickSlotComp = Pawn->FindComponentByClass<UDS1QuickSlotComponent>())
					{
						QuickSlotComp->RegisterToFirstEmptySlot(SlotIndex);
						return FReply::Handled();
					}
				}
			}
		}

		// ?고겢由? ?뚮퉬 ?꾩씠???ъ슜 ?먮뒗 ?λ퉬 ?μ갑
		if (InventoryComponent && InventoryComponent->GetInventorySlots().IsValidIndex(SlotIndex))
		{
			const FDS1ItemInstance& SlotItem = InventoryComponent->GetSlot(SlotIndex);
			if (SlotItem.IsValid())
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

	// ?쒕옒洹?鍮꾩＜?쇰줈 ?꾩씠肄??쒖떆
	if (SlotItem.ItemData->Icon)
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
		// ?λ퉬 ?щ’?먯꽌 洹몃━?쒕줈 ???μ갑 ?댁젣
		if (InventoryComponent->UnequipToInventory(DragOp->EquipSlotType))
		{
			return true;
		}
		return false;
	}

	// 洹몃━????洹몃━???대룞/援먰솚
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

	// ?쒕옒洹멸? ?대뼡 ?꾩젽?먮룄 ?쒕∼?섏? ?딆쓬 ???붾뱶???꾩씠??踰꾨━湲?
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
	if (SlotItem.IsValid() && TooltipWidgetClass)
	{
		UDS1ItemTooltipWidget* Tooltip = CreateWidget<UDS1ItemTooltipWidget>(GetOwningPlayer(), TooltipWidgetClass);
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



