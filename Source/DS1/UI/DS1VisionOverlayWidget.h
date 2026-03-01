// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "DS1VisionOverlayWidget.generated.h"

class UDS1VisibilityComponent;

/**
 * 플레이어 시야(FOV Cone) 밖 영역을 어둡게 칠하는 전체화면 오버레이 위젯.
 *
 * 시각 구성:
 *  ① 근접 원   — NearDetectionRadius 기반 원형. 내부는 항상 밝음
 *  ② FOV 원뿔  — 전방 ConeAngle 범위. NearRadius 바깥부터 ArcRadius까지 밝음
 *  ③ 어두운 영역 — 원뿔 밖 + 근접 원 밖. NearRadius~화면 끝 사이를 어둡게 채움
 */
UCLASS()
class DS1_API UDS1VisionOverlayWidget : public UUserWidget
{
	GENERATED_BODY()

// ── 스타일 프로퍼티 (블루프린트 / 에디터에서 조정 가능) ────────────────────
public:
	/** 시야 밖 어두운 색 (Alpha = 불투명도) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Vision|Style")
	FLinearColor DarkColor = FLinearColor(0.f, 0.f, 0.f, 0.5f);

	/** FOV 원뿔 경계선 색 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Vision|Style")
	FLinearColor ConeBoundaryColor = FLinearColor(1.f, 1.f, 0.85f, 0.75f);

	/** true면 시야각 경계선(양쪽 직선 + 바깥 호)을 그린다 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Vision|Style")
	bool bDrawConeBoundary = false;

	/** 근접 원 아웃라인 색 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Vision|Style")
	FLinearColor NearCircleColor = FLinearColor(0.6f, 0.9f, 1.f, 0.55f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Vision|Style")
	bool bDrawNearCircleBoundary = false;

	/** 경계선 두께 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Vision|Style")
	float LineThickness = 2.0f;

	/**
	 * 시야 투영 기준 높이 (월드 Z 오프셋, Unreal Units).
	 * 0 이면 발 위치 기준 → 원뿔이 우측으로 치우쳐 보임.
	 * 80~100 사이로 설정하면 허리·가슴 높이 기준 → 시각적 중앙 정렬.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Vision|Calibration")
	float ConeApexOffsetZ = 0.f;

	/**
	 * 전방 각도 수동 보정 (degrees).
	 * 원뿔이 캐릭터 실제 이동 방향과 약간 어긋날 때 여기서 조정.
	 * 양수 = 시계 방향, 음수 = 반시계 방향.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Vision|Calibration")
	float ForwardAngleCorrectionDeg = 0.f;

	/** 호(arc)·원 샘플 간격 (degrees). 작을수록 부드럽고 버텍스 증가 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Vision|Style", meta=(ClampMin="1", ClampMax="15"))
	float ArcStepDeg = 4.0f;

	/**
	 * FOV 원뿔 호(arc)를 그릴 반지름 (화면 짧은 변 대비 비율, 0~1).
	 * 0.25 = 화면 짧은 변의 25%.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Vision|Style", meta=(ClampMin="0.05", ClampMax="0.8"))
	float ConeArcRadiusPct = 0.25f;

// ── 내부 ──────────────────────────────────────────────────────────────────
protected:
	virtual void NativeConstruct() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;
	virtual int32 NativePaint(
		const FPaintArgs& Args,
		const FGeometry& AllottedGeometry,
		const FSlateRect& MyCullingRect,
		FSlateWindowElementList& OutDrawElements,
		int32 LayerId,
		const FWidgetStyle& InWidgetStyle,
		bool bParentEnabled) const override;

private:
	TWeakObjectPtr<UDS1VisibilityComponent> CachedVisComp;

	bool     bDataValid              = false;
	FVector2D CachedPlayerAbsPos;          ///< 캐릭터 허리 높이의 Absolute Slate 좌표
	float    CachedForwardAngleDeg   = 0.f; ///< 스크린 공간 전방 각도 (우=0, 아래=90)
	float    CachedHalfConeDeg       = 45.f;
	float    CachedNearRadiusAbs     = 0.f;  ///< 스크린상 근접 원 반지름 (Absolute Slate px)
	FVector2D CachedAbsViewportSize;

	// ── 내부 헬퍼 ──────────────────────────────────────────────────────────

	/** Apex 에서 AngleDeg 방향으로 레이를 쏘아 화면 엣지 교점 반환 */
	FVector2D RayToScreenEdge(const FVector2D& Apex, float AngleDeg, const FVector2D& AbsSize) const;

	/**
	 * 어두운 "도넛 섹터" (근접 원 바깥 ~ 화면 끝) 를 그린다.
	 * StartAngleDeg 에서 시계 방향으로 TotalSweepDeg 만큼.
	 * NearRadiusAbs : 안쪽 경계(밝은 영역 끝) 반지름
	 */
	void DrawDarkAnnularSector(
		const FGeometry& AllottedGeometry,
		FSlateWindowElementList& OutDrawElements,
		int32 LayerId,
		const FVector2D& Apex,
		float StartAngleDeg,
		float TotalSweepDeg,
		float NearRadiusAbs,
		const FColor& Color) const;

	/** FOV 원뿔 경계선 2개 + 호를 그린다 */
	void DrawConeBoundary(
		const FGeometry& AllottedGeometry,
		FSlateWindowElementList& OutDrawElements,
		int32 LayerId,
		const FVector2D& Apex,
		float ForwardAngle,
		float HalfCone,
		float ArcRadiusAbs) const;

	/** 근접 원 아웃라인(전체 원)을 그린다 */
	void DrawNearCircle(
		const FGeometry& AllottedGeometry,
		FSlateWindowElementList& OutDrawElements,
		int32 LayerId,
		const FVector2D& Apex,
		float RadiusAbs) const;
};
