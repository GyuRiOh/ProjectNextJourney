// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "DS1StaminaRingWidget.generated.h"

/**
 * 캐릭터 발 아래 지면에 표시되는 원형 스태미나 링 위젯.
 * NativePaint로 배경 링(어두운 원)과 스태미나 아크(초록)를 직접 그린다.
 */
UCLASS()
class DS1_API UDS1StaminaRingWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	/** 링 선 두께 (월드 위젯 드로우 사이즈 기준) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "StaminaRing")
	float RingThickness = 10.f;

	/** 배경 링 색상 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "StaminaRing")
	FLinearColor BackgroundColor = FLinearColor(0.05f, 0.05f, 0.05f, 0.7f);

	/** 스태미나 아크 색상 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "StaminaRing")
	FLinearColor StaminaColor = FLinearColor(0.f, 1.f, 0.2f, 1.f);

private:
	float StaminaRatio = 1.f;

public:
	/** AttributeComponent 델리게이트에서 호출 */
	void SetStaminaRatio(float InRatio);

protected:
	virtual int32 NativePaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry,
		const FSlateRect& MyCullingRect, FSlateWindowElementList& OutDrawElements,
		int32 LayerId, const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const override;

private:
	/** StartAngleDeg에서 ArcDeg만큼 호를 그린다 */
	void DrawRingArc(FSlateWindowElementList& OutDrawElements, int32 LayerId,
		const FGeometry& AllottedGeometry, float StartAngleDeg, float ArcDeg,
		float Radius, const FLinearColor& Color) const;
};
