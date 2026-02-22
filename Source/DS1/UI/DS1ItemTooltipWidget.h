// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "DS1ItemTooltipWidget.generated.h"

class UDS1ItemData;
class UTextBlock;

/**
 * 아이템 툴팁 위젯 — 이름, 설명, 타입, 무게, 스탯 표시
 */
UCLASS()
class DS1_API UDS1ItemTooltipWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	/** 툴팁에 표시할 아이템 데이터 설정 */
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	void SetItemData(UDS1ItemData* InItemData);

protected:
	UPROPERTY(meta = (BindWidgetOptional))
	UTextBlock* ItemNameText;

	UPROPERTY(meta = (BindWidgetOptional))
	UTextBlock* ItemDescriptionText;

	UPROPERTY(meta = (BindWidgetOptional))
	UTextBlock* ItemTypeText;

	UPROPERTY(meta = (BindWidgetOptional))
	UTextBlock* ItemWeightText;

	UPROPERTY(meta = (BindWidgetOptional))
	UTextBlock* ItemStatText;
};
