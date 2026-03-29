// Fill out your copyright notice in the Description page of Project Settings.

#include "UI/DS1InventoryWidget.h"

#include "Blueprint/WidgetTree.h"
#include "Components/Border.h"
#include "Components/BorderSlot.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/SizeBox.h"
#include "Components/TextBlock.h"
#include "Components/UniformGridPanel.h"
#include "Components/VerticalBox.h"
#include "Components/VerticalBoxSlot.h"
#include "Components/DS1InventoryComponent.h"
#include "GameFramework/Pawn.h"
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

	if (WidgetTree && !InventoryGrid)
	{
		UCanvasPanel* RootCanvas = Cast<UCanvasPanel>(GetRootWidget());
		if (!RootCanvas)
		{
			RootCanvas = WidgetTree->ConstructWidget<UCanvasPanel>(UCanvasPanel::StaticClass(), TEXT("InventoryRootCanvas"));
			WidgetTree->RootWidget = RootCanvas;
		}

		USizeBox* PanelSizeBox = WidgetTree->ConstructWidget<USizeBox>(USizeBox::StaticClass(), TEXT("InventoryPanelSize"));
		PanelSizeBox->SetWidthOverride(620.f);
		PanelSizeBox->SetHeightOverride(560.f);

		UBorder* PanelBorder = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass(), TEXT("InventoryPanelBorder"));
		PanelBorder->SetBrushColor(FLinearColor(0.05f, 0.04f, 0.03f, 0.94f));
		PanelBorder->SetPadding(FMargin(16.f));

		UVerticalBox* RootBox = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), TEXT("InventoryRoot"));
		PanelBorder->SetContent(RootBox);
		PanelSizeBox->AddChild(PanelBorder);

		if (UCanvasPanelSlot* PanelSlot = RootCanvas->AddChildToCanvas(PanelSizeBox))
		{
			PanelSlot->SetAnchors(FAnchors(0.5f, 0.5f));
			PanelSlot->SetAlignment(FVector2D(0.5f, 0.5f));
			PanelSlot->SetAutoSize(true);
		}

		UTextBlock* TitleText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("InventoryTitle"));
		TitleText->SetText(FText::FromString(TEXT("Inventory")));
		TitleText->SetColorAndOpacity(FSlateColor(FLinearColor(0.93f, 0.83f, 0.62f, 1.f)));
		if (UVerticalBoxSlot* TitleSlot = RootBox->AddChildToVerticalBox(TitleText))
		{
			TitleSlot->SetPadding(FMargin(8.f, 4.f, 8.f, 10.f));
		}

		if (!WeightText)
		{
			WeightText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("WeightText"));
			WeightText->SetText(FText::FromString(TEXT("0.0 / 0.0")));
			WeightText->SetColorAndOpacity(FSlateColor(FLinearColor(0.75f, 0.73f, 0.67f, 1.f)));
			if (UVerticalBoxSlot* WeightSlot = RootBox->AddChildToVerticalBox(WeightText))
			{
				WeightSlot->SetPadding(FMargin(8.f, 0.f, 8.f, 12.f));
			}
		}

		InventoryGrid = WidgetTree->ConstructWidget<UUniformGridPanel>(UUniformGridPanel::StaticClass(), TEXT("InventoryGrid"));
		if (UVerticalBoxSlot* GridSlot = RootBox->AddChildToVerticalBox(InventoryGrid))
		{
			GridSlot->SetPadding(FMargin(8.f));
		}
	}

	if (!InventoryComponent)
	{
		if (APlayerController* PC = GetOwningPlayer())
		{
			if (APawn* ControlledPawn = PC->GetPawn())
			{
				if (UDS1InventoryComponent* InvComp = ControlledPawn->FindComponentByClass<UDS1InventoryComponent>())
				{
					InitInventory(InvComp);
				}
			}
		}
	}
}

bool UDS1InventoryWidget::NativeOnDrop(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation)
{
	return true;
}

void UDS1InventoryWidget::BuildGrid()
{
	if (!InventoryGrid || !InventoryComponent)
	{
		return;
	}

	InventoryGrid->ClearChildren();
	GridSlotWidgets.Empty();

	TSubclassOf<UDS1InventorySlotWidget> EffectiveSlotClass = SlotWidgetClass;
	if (!EffectiveSlotClass)
	{
		EffectiveSlotClass = UDS1InventorySlotWidget::StaticClass();
	}

	const int32 NumSlots = InventoryComponent->GetMaxSlots();
	for (int32 i = 0; i < NumSlots; ++i)
	{
		UDS1InventorySlotWidget* SlotWidget = CreateWidget<UDS1InventorySlotWidget>(GetOwningPlayer(), EffectiveSlotClass);
		if (!SlotWidget)
		{
			continue;
		}

		SlotWidget->InitSlot(InventoryComponent, i);

		const int32 Row = i / GridColumns;
		const int32 Col = i % GridColumns;
		InventoryGrid->AddChildToUniformGrid(SlotWidget, Row, Col);
		GridSlotWidgets.Add(SlotWidget);
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
			FString::Printf(TEXT("Weight  %.1f / %.1f"),
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
