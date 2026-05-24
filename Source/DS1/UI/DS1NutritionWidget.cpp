// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/DS1NutritionWidget.h"

#include "Components/DS1AttributeComponent.h"
#include "Rendering/DrawElements.h"

void UDS1NutritionWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (APawn* Pawn = GetOwningPlayerPawn())
	{
		if (UDS1AttributeComponent* Attr = Pawn->GetComponentByClass<UDS1AttributeComponent>())
		{
			Attr->OnAttributeChanged.AddUObject(this, &ThisClass::OnAttributeChanged);
			Attr->BroadcastAttributeChanged(WatchedAttribute);
		}
	}
}

void UDS1NutritionWidget::SetRatio(float InRatio)
{
	Ratio = FMath::Clamp(InRatio, 0.f, 1.f);
	Invalidate(EInvalidateWidgetReason::Paint);
}

void UDS1NutritionWidget::OnAttributeChanged(EDS1AttributeType InType, float InValue)
{
	if (InType == WatchedAttribute)
	{
		Ratio = FMath::Clamp(InValue, 0.f, 1.f);
		Invalidate(EInvalidateWidgetReason::Paint);
	}
}

int32 UDS1NutritionWidget::NativePaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry,
	const FSlateRect& MyCullingRect, FSlateWindowElementList& OutDrawElements,
	int32 LayerId, const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const
{
	LayerId = Super::NativePaint(Args, AllottedGeometry, MyCullingRect, OutDrawElements, LayerId, InWidgetStyle, bParentEnabled);

	const FVector2D Size = AllottedGeometry.GetLocalSize();
	const float Radius = FMath::Min(Size.X, Size.Y) * 0.5f - RingThickness * 0.5f;

	const float TotalArc = 360.f - GapDegrees;
	const float StartAngle = 90.f + GapDegrees * 0.5f;

	// 1. 배경 링
	DrawArc(OutDrawElements, LayerId, AllottedGeometry, StartAngle, TotalArc, Radius, BackgroundColor);
	++LayerId;

	// 2. 잔량 아크 (시계방향)
	if (Ratio > KINDA_SMALL_NUMBER)
	{
		DrawArc(OutDrawElements, LayerId, AllottedGeometry, StartAngle, TotalArc * Ratio, Radius, FillColor);
		++LayerId;
	}

	return LayerId;
}

void UDS1NutritionWidget::DrawArc(FSlateWindowElementList& OutDrawElements, int32 LayerId,
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
		true,
		RingThickness
	);
}
