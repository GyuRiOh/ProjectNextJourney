// Fill out your copyright notice in the Description page of Project Settings.

#include "UI/DS1InventoryWidget.h"

#include "Components/UniformGridPanel.h"
#include "Components/TextBlock.h"
#include "Components/DS1InventoryComponent.h"
#include "UI/DS1InventorySlotWidget.h"

void UDS1InventoryWidget::InitInventory(UDS1InventoryComponent* InInventoryComponent)
{
	InventoryComponent = InInventoryComponent;

	if (InventoryComponent)
	{
		InventoryComponent->OnInventoryChanged.AddUObject(this, &ThisClass::OnInventoryChanged);
		InventoryComponent->OnSlotChanged.AddUObject(this, &ThisClass::OnSlotChanged);
	}

	BuildGrid();
	RefreshAllSlots();
	RefreshWeight();
}

void UDS1InventoryWidget::NativeConstruct()
{
	Super::NativeConstruct();

	// 자동으로 소유 Pawn에서 인벤토리 컴포넌트를 찾아 바인딩
	if (!InventoryComponent)
	{
		if (APlayerController* PC = GetOwningPlayer())
		{
			if (APawn* Pawn = PC->GetPawn())
			{
				if (UDS1InventoryComponent* InvComp = Pawn->FindComponentByClass<UDS1InventoryComponent>())
				{
					InitInventory(InvComp);
				}
			}
		}
	}
}

bool UDS1InventoryWidget::NativeOnDrop(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation)
{
	// 인벤토리 영역 내에서 슬롯 밖에 드롭 — 무시 (소비되지 않도록)
	// 월드 버리기는 NativeOnDragCancelled에서 처리
	return true;
}

void UDS1InventoryWidget::BuildGrid()
{
	if (!InventoryGrid || !SlotWidgetClass || !InventoryComponent)
	{
		return;
	}

	InventoryGrid->ClearChildren();
	GridSlotWidgets.Empty();

	const int32 NumSlots = InventoryComponent->GetMaxSlots();
	for (int32 i = 0; i < NumSlots; ++i)
	{
		UDS1InventorySlotWidget* SlotWidget = CreateWidget<UDS1InventorySlotWidget>(GetOwningPlayer(), SlotWidgetClass);
		if (SlotWidget)
		{
			SlotWidget->InitSlot(InventoryComponent, i);

			const int32 Row = i / GridColumns;
			const int32 Col = i % GridColumns;
			InventoryGrid->AddChildToUniformGrid(SlotWidget, Row, Col);

			GridSlotWidgets.Add(SlotWidget);
		}
	}
}

void UDS1InventoryWidget::RefreshAllSlots()
{
	for (UDS1InventorySlotWidget* SlotWidget : GridSlotWidgets)
	{
		if (SlotWidget)
		{
			SlotWidget->RefreshSlot();
		}
	}
	RefreshWeight();
}

void UDS1InventoryWidget::RefreshWeight()
{
	if (WeightText && InventoryComponent)
	{
		WeightText->SetText(FText::FromString(
			FString::Printf(TEXT("%.1f / %.1f"),
				InventoryComponent->GetCurrentWeight(),
				InventoryComponent->GetMaxCarryWeight())));
	}
}

void UDS1InventoryWidget::OnInventoryChanged()
{
	RefreshAllSlots();
}

void UDS1InventoryWidget::OnSlotChanged(int32 SlotIndex)
{
	if (GridSlotWidgets.IsValidIndex(SlotIndex))
	{
		GridSlotWidgets[SlotIndex]->RefreshSlot();
	}
	RefreshWeight();
}
