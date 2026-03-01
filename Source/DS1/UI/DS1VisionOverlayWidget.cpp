// Fill out your copyright notice in the Description page of Project Settings.

#include "UI/DS1VisionOverlayWidget.h"

#include "Camera/PlayerCameraManager.h"
#include "Blueprint/WidgetLayoutLibrary.h"
#include "Components/DS1VisibilityComponent.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"
#include "Rendering/DrawElementTypes.h"
#include "Styling/CoreStyle.h"

// ?????????????????????????????????????????????????????????????????????????????
// Lifecycle
// ?????????????????????????????????????????????????????????????????????????????

void UDS1VisionOverlayWidget::NativeConstruct()
{
	Super::NativeConstruct();
}

void UDS1VisionOverlayWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	bDataValid = false;

	APlayerController* PC = GetOwningPlayer();
	if (!PC) return;

	APawn* Pawn = PC->GetPawn();
	if (!Pawn) return;

	if (!CachedVisComp.IsValid())
	{
		CachedVisComp = Pawn->FindComponentByClass<UDS1VisibilityComponent>();
	}
	if (!CachedVisComp.IsValid()) return;

	const FVector2D LocalSize = MyGeometry.GetLocalSize();
	if (LocalSize.X <= 0.f || LocalSize.Y <= 0.f) return;
	CachedAbsViewportSize = LocalSize;

	const FVector ApexWorld = Pawn->GetActorLocation() + FVector(0.f, 0.f, ConeApexOffsetZ);

	FVector2D ApexScreenLocal;
	if (!UWidgetLayoutLibrary::ProjectWorldLocationToWidgetPosition(PC, ApexWorld, ApexScreenLocal, true)) return;
	CachedPlayerAbsPos = ApexScreenLocal;

	// ?? ?꾨갑 諛⑺뼢: XY ?섑룊 ?깅텇留??ъ슜 (VisibilityComponent ? ?숈씪 湲곗?)
	const FVector Fwd2D = CachedVisComp->GetVisionForward2D();

	// ?? 移대찓???됰젹 湲곕컲 諛⑺뼢 怨꾩궛
	//    ProjectWorldLocationToScreen ???먭렐 ?쒓끝 ?뚮Ц??罹먮┃?곌? ?붾㈃ 以묒떖??
	//    踰쀬뼱?좎닔濡?媛곷룄媛 ??댁쭊?? 移대찓??Right/Up 異뺤뿉 吏곸젒 dot product ?섎㈃
	//    ?쒓끝 ?놁씠 ?뺥솗???ㅽ겕由?怨듦컙 媛곷룄瑜??살쓣 ???덈떎.
	if (!PC->PlayerCameraManager) return;
	const FRotator CamRot    = PC->PlayerCameraManager->GetCameraRotation();
	const FMatrix  CamMatrix = FRotationMatrix(CamRot);
	const FVector  CamRight  = CamMatrix.GetUnitAxis(EAxis::Y); // ?ㅽ겕由?+X
	const FVector  CamUp     = CamMatrix.GetUnitAxis(EAxis::Z); // ?ㅽ겕由?-Y (?꾩そ)

	const float ScreenFwdX =  FVector::DotProduct(Fwd2D, CamRight);
	const float ScreenFwdY = -FVector::DotProduct(Fwd2D, CamUp);  // ?ㅽ겕由?Y 異뺤? ?꾨옒媛 ?묒닔

	if (FMath::Abs(ScreenFwdX) < KINDA_SMALL_NUMBER && FMath::Abs(ScreenFwdY) < KINDA_SMALL_NUMBER) return;

	CachedForwardAngleDeg = FMath::RadiansToDegrees(FMath::Atan2(ScreenFwdY, ScreenFwdX))
	                        + ForwardAngleCorrectionDeg;
	CachedHalfConeDeg     = CachedVisComp->GetConeHalfAngleDegrees();

	// ?? 洹쇱젒 ???ㅽ겕由?諛섏?由?怨꾩궛
	//    ?붾뱶 NearDetectionRadius 留뚰겮 ?⑥뼱吏??먯쓣 ?ъ쁺???쎌? 嫄곕━瑜?援ы븿
	const float NearWorldRadius = CachedVisComp->GetNearDetectionRadius();
	if (NearWorldRadius > 0.f)
	{
		FVector2D NearTestLocal;
		// ?섑룊 +X 諛⑺뼢?쇰줈 NearRadius 留뚰겮 ?⑥뼱吏??먯쓣 湲곗??쇰줈 ?쇰뒗??
		if (UWidgetLayoutLibrary::ProjectWorldLocationToWidgetPosition(PC, ApexWorld + FVector(NearWorldRadius, 0.f, 0.f), NearTestLocal, true))
		{
			CachedNearRadiusAbs = FVector2D::Distance(NearTestLocal, CachedPlayerAbsPos);
		}
	}
	else
	{
		CachedNearRadiusAbs = 0.f;
	}

	bDataValid = true;
}

// ?????????????????????????????????????????????????????????????????????????????
// Paint
// ?????????????????????????????????????????????????????????????????????????????

int32 UDS1VisionOverlayWidget::NativePaint(
	const FPaintArgs& Args,
	const FGeometry& AllottedGeometry,
	const FSlateRect& MyCullingRect,
	FSlateWindowElementList& OutDrawElements,
	int32 LayerId,
	const FWidgetStyle& InWidgetStyle,
	bool bParentEnabled) const
{
	LayerId = Super::NativePaint(Args, AllottedGeometry, MyCullingRect, OutDrawElements,
	                             LayerId, InWidgetStyle, bParentEnabled);
	if (!bDataValid) return LayerId;

	const FVector2D AbsSize    = AllottedGeometry.GetLocalSize();
	const float     ConeArcR   = FMath::Min(AbsSize.X, AbsSize.Y) * FMath::Clamp(ConeArcRadiusPct, 0.05f, 0.8f);
	const float     NearR      = CachedNearRadiusAbs;
	const float     StartAngle = CachedForwardAngleDeg + CachedHalfConeDeg;
	const float     TotalSweep = 360.f - (2.f * CachedHalfConeDeg);

	// ???대몢???꾨꽋 ?뱁꽣 (?먮퓭 諛?+ 洹쇱젒 ??諛?
	DrawDarkAnnularSector(AllottedGeometry, OutDrawElements, LayerId,
		CachedPlayerAbsPos, StartAngle, TotalSweep, NearR, DarkColor.ToFColorSRGB());
	++LayerId;

	// ??FOV ?먮퓭 寃쎄퀎??+ ??
	if (bDrawConeBoundary)
	{
		DrawConeBoundary(AllottedGeometry, OutDrawElements, LayerId,
			CachedPlayerAbsPos, CachedForwardAngleDeg, CachedHalfConeDeg, ConeArcR);
		++LayerId;
	}

	// ??洹쇱젒 ???꾩썐?쇱씤
	if (bDrawNearCircleBoundary && NearR > 1.f)
	{
		DrawNearCircle(AllottedGeometry, OutDrawElements, LayerId,
			CachedPlayerAbsPos, NearR);
		++LayerId;
	}

	return LayerId;
}

// ?????????????????????????????????????????????????????????????????????????????
// RayToScreenEdge
// ?????????????????????????????????????????????????????????????????????????????

FVector2D UDS1VisionOverlayWidget::RayToScreenEdge(
	const FVector2D& Apex, float AngleDeg, const FVector2D& AbsSize) const
{
	const float    Rad = FMath::DegreesToRadians(AngleDeg);
	const FVector2D Dir(FMath::Cos(Rad), FMath::Sin(Rad));
	const float W = AbsSize.X, H = AbsSize.Y;
	float tMin = TNumericLimits<float>::Max();

	auto TryEdge = [&](float t, float perp, float perpMin, float perpMax)
	{
		if (t > 0.f && perp >= perpMin && perp <= perpMax)
			tMin = FMath::Min(tMin, t);
	};

	if (Dir.X >  KINDA_SMALL_NUMBER) TryEdge((W - Apex.X) / Dir.X, Apex.Y + (W - Apex.X) / Dir.X * Dir.Y, 0.f, H);
	if (Dir.X < -KINDA_SMALL_NUMBER) TryEdge(-Apex.X      / Dir.X, Apex.Y + (-Apex.X)      / Dir.X * Dir.Y, 0.f, H);
	if (Dir.Y >  KINDA_SMALL_NUMBER) TryEdge((H - Apex.Y) / Dir.Y, Apex.X + (H - Apex.Y) / Dir.Y * Dir.X, 0.f, W);
	if (Dir.Y < -KINDA_SMALL_NUMBER) TryEdge(-Apex.Y      / Dir.Y, Apex.X + (-Apex.Y)      / Dir.Y * Dir.X, 0.f, W);

	return (tMin == TNumericLimits<float>::Max()) ? Apex : Apex + Dir * tMin;
}

// ?????????????????????????????????????????????????????????????????????????????
// DrawDarkAnnularSector
// ?????????????????????????????????????????????????????????????????????????????
// ?먮퓭 諛?媛곷룄 踰붿쐞?????洹쇱젒 ??NearRadius) ~ ?붾㈃ ???ъ씠瑜??대몼寃?梨꾩슫??
// ?덉そ ?ｌ? = Apex + Dir * NearRadius (?먰삎 ??
// 諛붽묑 ?ｌ? = ?붾㈃ ?ш컖??援먯젏 (?붾㈃ 紐⑥꽌由??ы븿)
// ?????????????????????????????????????????????????????????????????????????????

void UDS1VisionOverlayWidget::DrawDarkAnnularSector(
	const FGeometry& AllottedGeometry,
	FSlateWindowElementList& OutDrawElements,
	int32 LayerId,
	const FVector2D& Apex,
	float StartAngleDeg,
	float TotalSweepDeg,
	float NearRadiusAbs,
	const FColor& Color) const
{
	if (TotalSweepDeg <= 0.f) return;

	const FSlateBrush* WhiteBrush = FCoreStyle::Get().GetBrush("WhiteBrush");
	if (!WhiteBrush) return;

	const FVector2D AbsSize  = AllottedGeometry.GetLocalSize();
	const float     SafeStep = FMath::Max(ArcStepDeg, 1.f);

	// ?? ?섑뵆 媛곷룄 紐⑸줉 援ъ꽦 (?쇱젙 媛꾧꺽 + ?붾㈃ 紐⑥꽌由?
	struct FSample { float Angle; FVector2D Outer; };
	TArray<FSample> Samples;

	// ?쇱젙 媛꾧꺽 ?섑뵆
	const int32 NumSteps = FMath::CeilToInt(TotalSweepDeg / SafeStep);
	for (int32 i = 0; i <= NumSteps; ++i)
	{
		const float RelDeg = FMath::Min(SafeStep * i, TotalSweepDeg);
		const float AbsDeg = StartAngleDeg + RelDeg;
		Samples.Add({ AbsDeg, RayToScreenEdge(Apex, AbsDeg, AbsSize) });
	}

	// ?붾㈃ 紐⑥꽌由щ? 媛곷룄??留욌뒗 ?꾩튂???쎌엯
	const FVector2D Corners[4] = {
		{0.f,       0.f},
		{AbsSize.X, 0.f},
		{AbsSize.X, AbsSize.Y},
		{0.f,       AbsSize.Y}
	};
	for (const FVector2D& Corner : Corners)
	{
		float Deg = FMath::RadiansToDegrees(FMath::Atan2(Corner.Y - Apex.Y, Corner.X - Apex.X));
		while (Deg < StartAngleDeg)              Deg += 360.f;
		while (Deg > StartAngleDeg + 360.f)      Deg -= 360.f;
		if (Deg <= StartAngleDeg + TotalSweepDeg)
			Samples.Add({ Deg, Corner });
	}

	Samples.Sort([](const FSample& A, const FSample& B) { return A.Angle < B.Angle; });

	const int32 N = Samples.Num();
	if (N < 2) return;

	// ?? 踰꾪뀓??援ъ꽦: [inner0, outer0, inner1, outer1, ...]
	const FSlateRenderTransform AccTF = AllottedGeometry.GetAccumulatedRenderTransform();
	TArray<FSlateVertex> Verts;
	TArray<SlateIndex>   Indices;
	Verts.Reserve(N * 2);
	Indices.Reserve((N - 1) * 6);

	for (const FSample& S : Samples)
	{
		const float  Rad     = FMath::DegreesToRadians(S.Angle);
		const FVector2D InnerPt = Apex + FVector2D(FMath::Cos(Rad), FMath::Sin(Rad)) * NearRadiusAbs;

		// ?덉そ 踰꾪뀓??(NearRadius ?먰샇 ??
		Verts.Add(FSlateVertex::Make(AccTF, FVector2f((float)InnerPt.X, (float)InnerPt.Y),
		                             FVector2f(0.f, 0.f), Color));
		// 諛붽묑履?踰꾪뀓??(?붾㈃ ?ｌ?)
		Verts.Add(FSlateVertex::Make(AccTF, FVector2f((float)S.Outer.X, (float)S.Outer.Y),
		                             FVector2f(0.f, 0.f), Color));
	}

	// ?? ?쇨컖???ㅽ듃由? ??(i, i+1) 留덈떎 荑쇰뱶 2媛??앹꽦
	for (int32 i = 0; i < N - 1; ++i)
	{
		const SlateIndex i0 = (SlateIndex)(i * 2);      // inner[i]
		const SlateIndex o0 = (SlateIndex)(i * 2 + 1);  // outer[i]
		const SlateIndex i1 = (SlateIndex)(i * 2 + 2);  // inner[i+1]
		const SlateIndex o1 = (SlateIndex)(i * 2 + 3);  // outer[i+1]

		Indices.Add(i0); Indices.Add(o0); Indices.Add(i1);
		Indices.Add(o0); Indices.Add(o1); Indices.Add(i1);
	}

	if (Indices.Num() < 3) return;

	FSlateDrawElement::MakeCustomVerts(
		OutDrawElements, LayerId,
		WhiteBrush->GetRenderingResource(),
		Verts, Indices, nullptr, 0, 0);
}

// ?????????????????????????????????????????????????????????????????????????????
// DrawConeBoundary
// ?????????????????????????????????????????????????????????????????????????????

void UDS1VisionOverlayWidget::DrawConeBoundary(
	const FGeometry& AllottedGeometry,
	FSlateWindowElementList& OutDrawElements,
	int32 LayerId,
	const FVector2D& Apex,
	float ForwardAngle,
	float HalfCone,
	float ArcRadiusAbs) const
{
	const float SafeStep = FMath::Max(ArcStepDeg, 1.f);

	// ?? ?쇱そ쨌?ㅻⅨ履?寃쎄퀎??
	for (int32 Side = -1; Side <= 1; Side += 2)  // -1 = ?쇱そ, +1 = ?ㅻⅨ履?
	{
		const float AngleDeg = ForwardAngle + Side * HalfCone;
		const FVector2D Dir(FMath::Cos(FMath::DegreesToRadians(AngleDeg)),
		                    FMath::Sin(FMath::DegreesToRadians(AngleDeg)));
		const TArray<FVector2D> Pts = { Apex, Apex + Dir * ArcRadiusAbs };
		FSlateDrawElement::MakeLines(OutDrawElements, LayerId,
			AllottedGeometry.ToPaintGeometry(), Pts,
			ESlateDrawEffect::None, ConeBoundaryColor, true, LineThickness);
	}

	// ?? ??(left ~ right, 媛??援ъ뿭 諛⑺뼢)
	const float LeftAngle  = ForwardAngle - HalfCone;
	const float RightAngle = ForwardAngle + HalfCone;
	const int32 Steps      = FMath::CeilToInt((RightAngle - LeftAngle) / SafeStep);

	TArray<FVector2D> ArcPts;
	ArcPts.Reserve(Steps + 1);
	for (int32 i = 0; i <= Steps; ++i)
	{
		const float Deg = FMath::Min(LeftAngle + SafeStep * i, RightAngle);
		ArcPts.Add(Apex + FVector2D(FMath::Cos(FMath::DegreesToRadians(Deg)),
		                            FMath::Sin(FMath::DegreesToRadians(Deg))) * ArcRadiusAbs);
	}
	FSlateDrawElement::MakeLines(OutDrawElements, LayerId,
		AllottedGeometry.ToPaintGeometry(), ArcPts,
		ESlateDrawEffect::None, ConeBoundaryColor, true, LineThickness);
}

// ?????????????????????????????????????????????????????????????????????????????
// DrawNearCircle
// ?????????????????????????????????????????????????????????????????????????????

void UDS1VisionOverlayWidget::DrawNearCircle(
	const FGeometry& AllottedGeometry,
	FSlateWindowElementList& OutDrawElements,
	int32 LayerId,
	const FVector2D& Apex,
	float RadiusAbs) const
{
	const float SafeStep = FMath::Max(ArcStepDeg, 1.f);
	const int32 Steps    = FMath::CeilToInt(360.f / SafeStep);

	TArray<FVector2D> CirclePts;
	CirclePts.Reserve(Steps + 1);
	for (int32 i = 0; i <= Steps; ++i)
	{
		const float Deg = SafeStep * i;
		CirclePts.Add(Apex + FVector2D(FMath::Cos(FMath::DegreesToRadians(Deg)),
		                               FMath::Sin(FMath::DegreesToRadians(Deg))) * RadiusAbs);
	}

	FSlateDrawElement::MakeLines(OutDrawElements, LayerId,
		AllottedGeometry.ToPaintGeometry(), CirclePts,
		ESlateDrawEffect::None, NearCircleColor, true, LineThickness);
}

