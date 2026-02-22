// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Data/DS1ItemInstance.h"
#include "DS1InventorySlotWidget.generated.h"

class UImage;
class UTextBlock;
class UBorder;
class UDS1ItemTooltipWidget;
class UDS1InventoryComponent;

/**
 * 인벤토리 그리드 슬롯 하나를 담당하는 위젯
 */
UCLASS()
class DS1_API UDS1InventorySlotWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	/** 슬롯 초기화 */
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	void InitSlot(UDS1InventoryComponent* InInventoryComponent, int32 InSlotIndex);

	/** 슬롯 UI 갱신 */
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	void RefreshSlot();

	FORCEINLINE int32 GetSlotIndex() const { return SlotIndex; }

protected:
	virtual void NativeConstruct() override;
	virtual FReply NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
	virtual void NativeOnDragDetected(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent, UDragDropOperation*& OutOperation) override;
	virtual bool NativeOnDrop(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation) override;
	virtual void NativeOnDragCancelled(const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation) override;
	virtual void NativeOnMouseEnter(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
	virtual void NativeOnMouseLeave(const FPointerEvent& InMouseEvent) override;

protected:
	UPROPERTY(meta = (BindWidgetOptional))
	UImage* ItemIcon;

	UPROPERTY(meta = (BindWidgetOptional))
	UTextBlock* StackCountText;

	UPROPERTY(meta = (BindWidgetOptional))
	UBorder* SlotBorder;

	/** 툴팁 위젯 클래스 (블루프린트에서 할당) */
	UPROPERTY(EditAnywhere, Category = "Inventory")
	TSubclassOf<UDS1ItemTooltipWidget> TooltipWidgetClass;

private:
	UPROPERTY()
	UDS1InventoryComponent* InventoryComponent = nullptr;

	int32 SlotIndex = -1;
};
