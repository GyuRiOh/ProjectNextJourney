// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "DS1Define.h"
#include "Blueprint/UserWidget.h"
#include "DS1InventoryWidget.generated.h"

class UDS1InventoryComponent;
class UDS1InventorySlotWidget;
class UDS1ItemTooltipWidget;
class UUniformGridPanel;
class UTextBlock;

/**
 * 메인 인벤토리 위젯 — 그리드 + 장비 슬롯 + 무게 표시
 */
UCLASS()
class DS1_API UDS1InventoryWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	/** 인벤토리 컴포넌트 바인딩 */
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	void InitInventory(UDS1InventoryComponent* InInventoryComponent);

protected:
	virtual void NativeConstruct() override;
	virtual bool NativeOnDrop(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation) override;

	// ── 그리드 ──
protected:
	/** 그리드 패널 (5열) */
	UPROPERTY(meta = (BindWidgetOptional))
	UUniformGridPanel* InventoryGrid;

	/** 그리드 슬롯 위젯 클래스 */
	UPROPERTY(EditAnywhere, Category = "Inventory")
	TSubclassOf<UDS1InventorySlotWidget> SlotWidgetClass;

	// ── 장비 슬롯 ──
protected:
	UPROPERTY(meta = (BindWidgetOptional))
	UDS1InventorySlotWidget* WeaponSlot;

	UPROPERTY(meta = (BindWidgetOptional))
	UDS1InventorySlotWidget* ShieldSlot;

	UPROPERTY(meta = (BindWidgetOptional))
	UDS1InventorySlotWidget* ChestSlot;

	UPROPERTY(meta = (BindWidgetOptional))
	UDS1InventorySlotWidget* PantsSlot;

	UPROPERTY(meta = (BindWidgetOptional))
	UDS1InventorySlotWidget* BootsSlot;

	UPROPERTY(meta = (BindWidgetOptional))
	UDS1InventorySlotWidget* GlovesSlot;

	// ── 무게 표시 ──
protected:
	UPROPERTY(meta = (BindWidgetOptional))
	UTextBlock* WeightText;

private:
	void BuildGrid();
	void RefreshAllSlots();
	void RefreshWeight();

	void OnInventoryChanged();
	void OnSlotChanged(int32 SlotIndex);

	UPROPERTY()
	UDS1InventoryComponent* InventoryComponent = nullptr;

	UPROPERTY()
	TArray<UDS1InventorySlotWidget*> GridSlotWidgets;

	static constexpr int32 GridColumns = 5;
};
