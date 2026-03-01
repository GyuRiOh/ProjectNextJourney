// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "DS1PotionWidget.generated.h"

class UImage;
class UTextBlock;
class UDS1QuickSlotComponent;
class UTexture2D;
class UDragDropOperation;
class FDragDropEvent;
struct FGeometry;
/**
 * 
 */
UCLASS()
class DS1_API UDS1PotionWidget : public UUserWidget
{
	GENERATED_BODY()

protected:
	UPROPERTY(meta = (BindWidgetOptional), BlueprintReadWrite)
	UTextBlock* PotionQuantityText;

	UPROPERTY(meta = (BindWidgetOptional), BlueprintReadWrite)
	UImage* QuickSlotIcon1;
	UPROPERTY(meta = (BindWidgetOptional), BlueprintReadWrite)
	UImage* QuickSlotIcon2;
	UPROPERTY(meta = (BindWidgetOptional), BlueprintReadWrite)
	UImage* QuickSlotIcon3;
	UPROPERTY(meta = (BindWidgetOptional), BlueprintReadWrite)
	UImage* QuickSlotIcon4;
	UPROPERTY(meta = (BindWidgetOptional), BlueprintReadWrite)
	UImage* QuickSlotIcon5;
	UPROPERTY(meta = (BindWidgetOptional), BlueprintReadWrite)
	UImage* QuickSlotIcon6;

	UPROPERTY(meta = (BindWidgetOptional), BlueprintReadWrite)
	UTextBlock* QuickSlotCount1;
	UPROPERTY(meta = (BindWidgetOptional), BlueprintReadWrite)
	UTextBlock* QuickSlotCount2;
	UPROPERTY(meta = (BindWidgetOptional), BlueprintReadWrite)
	UTextBlock* QuickSlotCount3;
	UPROPERTY(meta = (BindWidgetOptional), BlueprintReadWrite)
	UTextBlock* QuickSlotCount4;
	UPROPERTY(meta = (BindWidgetOptional), BlueprintReadWrite)
	UTextBlock* QuickSlotCount5;
	UPROPERTY(meta = (BindWidgetOptional), BlueprintReadWrite)
	UTextBlock* QuickSlotCount6;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QuickSlot")
	UTexture2D* EmptySlotIcon = nullptr;

	UPROPERTY()
	UDS1QuickSlotComponent* QuickSlotComponent = nullptr;

public:
	void BindQuickSlot(UDS1QuickSlotComponent* InQuickSlotComponent);

	void RefreshQuickSlots();

	void SetPotionQuantity(const int InAmount) const;

protected:
	virtual bool NativeOnDragOver(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation) override;
	virtual bool NativeOnDrop(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation) override;

private:
	void SetSlotVisual(int32 SlotIndex, UImage* SlotIconWidget, UTextBlock* SlotCountWidget) const;
	int32 GetSlotIndexFromGeometry(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent) const;

	UFUNCTION()
	void OnQuickSlotChanged();
};
