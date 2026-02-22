// Fill out your copyright notice in the Description page of Project Settings.

#include "UI/DS1InventorySlotWidget.h"

#include "Components/Border.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "Components/DS1InventoryComponent.h"
#include "Data/DS1ItemData.h"
#include "UI/DS1InventoryDragDropOp.h"
#include "UI/DS1ItemTooltipWidget.h"
#include "Blueprint/WidgetBlueprintLibrary.h"

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
		// 우클릭: 소비 아이템 사용 또는 장비 장착
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

	// 드래그 비주얼로 아이콘 표시
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
		// 장비 슬롯에서 그리드로 — 장착 해제
		if (InventoryComponent->UnequipToInventory(DragOp->EquipSlotType))
		{
			return true;
		}
		return false;
	}

	// 그리드 → 그리드 이동/교환
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

	// 드래그가 어떤 위젯에도 드롭되지 않음 — 월드에 아이템 버리기
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
