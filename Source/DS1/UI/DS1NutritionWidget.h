// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "DS1Define.h"
#include "Blueprint/UserWidget.h"
#include "DS1NutritionWidget.generated.h"

class UImage;
class UDS1AttributeComponent;

/**
 * 덕코프 스타일 원형 배고픔/갈증 표시 위젯.
 * NativePaint로 배경 링 + 잔량 아크를 그린다.
 * AttributeComponent에 직접 구독해 WBP 이름 바인딩에 의존하지 않는다.
 */
UCLASS()
class DS1_API UDS1NutritionWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	/** 감시할 속성 (에디터에서 Hunger 또는 Thirst 선택) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Nutrition")
	EDS1AttributeType WatchedAttribute = EDS1AttributeType::Hunger;

	/** 링 선 두께 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Nutrition")
	float RingThickness = 10.f;

	/** 배경 링 색상 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Nutrition")
	FLinearColor BackgroundColor = FLinearColor(0.05f, 0.05f, 0.05f, 0.8f);

	/** 잔량 아크 색상 (배고픔: 주황, 갈증: 하늘) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Nutrition")
	FLinearColor FillColor = FLinearColor(1.f, 0.45f, 0.f, 1.f);

	/** 링 하단 갭(도). 0이면 갭 없음, 30이면 하단 30도를 비운다 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Nutrition", meta = (ClampMin = "0", ClampMax = "180"))
	float GapDegrees = 30.f;

	/** 중앙 아이콘 이미지 (WBP에서 선택적으로 배치) */
	UPROPERTY(meta = (BindWidgetOptional), BlueprintReadWrite)
	UImage* IconImage;

private:
	float Ratio = 1.f;

public:
	void SetRatio(float InRatio);

protected:
	virtual void NativeConstruct() override;

	virtual int32 NativePaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry,
		const FSlateRect& MyCullingRect, FSlateWindowElementList& OutDrawElements,
		int32 LayerId, const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const override;

private:
	UFUNCTION()
	void OnAttributeChanged(EDS1AttributeType InType, float InValue);

	void DrawArc(FSlateWindowElementList& OutDrawElements, int32 LayerId,
		const FGeometry& AllottedGeometry, float StartAngleDeg, float ArcDeg,
		float Radius, const FLinearColor& Color) const;
};
