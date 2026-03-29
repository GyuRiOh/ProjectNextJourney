// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/DS1StaminaRingWidget.h"

#include "Rendering/DrawElements.h"

void UDS1StaminaRingWidget::SetStaminaRatio(float InRatio)
{
	StaminaRatio = FMath::Clamp(InRatio, 0.f, 1.f);
	// 스태미나 비율이 바뀔 때마다 위젯을 다시 그리도록 무효화
	Invalidate(EInvalidateWidgetReason::Paint);
}

int32 UDS1StaminaRingWidget::NativePaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry,
	const FSlateRect& MyCullingRect, FSlateWindowElementList& OutDrawElements,
	int32 LayerId, const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const
{
	LayerId = Super::NativePaint(Args, AllottedGeometry, MyCullingRect, OutDrawElements, LayerId, InWidgetStyle, bParentEnabled);

	const FVector2D Size = AllottedGeometry.GetLocalSize();
	// 링 중심 반지름 (두께 절반을 뺀 위치)
	const float Radius = FMath::Min(Size.X, Size.Y) * 0.5f - RingThickness * 0.5f;

	// 1. 배경 링 (360도 전체)
	DrawRingArc(OutDrawElements, LayerId, AllottedGeometry, 0.f, 360.f, Radius, BackgroundColor);
	++LayerId;

	// 2. 스태미나 아크 (위쪽(-90도)에서 시계방향)
	if (StaminaRatio > KINDA_SMALL_NUMBER)
	{
		DrawRingArc(OutDrawElements, LayerId, AllottedGeometry, -90.f, 360.f * StaminaRatio, Radius, StaminaColor);
		++LayerId;
	}

	return LayerId;
}

void UDS1StaminaRingWidget::DrawRingArc(FSlateWindowElementList& OutDrawElements, int32 LayerId,
	const FGeometry& AllottedGeometry, float StartAngleDeg, float ArcDeg,
	float Radius, const FLinearColor& Color) const
{
	constexpr int32 NumSegments = 64;
	const FVector2D Center = AllottedGeometry.GetLocalSize() * 0.5f;

	TArray<FVector2D> Points;
	Points.Reserve(NumSegments + 1);

	for (int32 i = 0; i <= NumSegments; i++)
	{
		const float AngleDeg = StartAngleDeg + ArcDeg * static_cast<float>(i) / static_cast<float>(NumSegments);
		const float AngleRad = FMath::DegreesToRadians(AngleDeg);
		Points.Add(Center + FVector2D(FMath::Cos(AngleRad), FMath::Sin(AngleRad)) * Radius);
	}

	FSlateDrawElement::MakeLines(
		OutDrawElements,
		LayerId,
		AllottedGeometry.ToPaintGeometry(),
		Points,
		ESlateDrawEffect::None,
		Color,
		true,         // bAntialias
		RingThickness
	);
}
