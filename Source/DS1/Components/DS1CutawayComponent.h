// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "DS1CutawayComponent.generated.h"

class USceneCaptureComponent2D;
class UTextureRenderTarget2D;
class UPostProcessComponent;
class UMaterialInterface;
class UMaterialInstanceDynamic;

/**
 * 플레이어 캐릭터에 부착하는 컷어웨이(Cutaway) 시스템 컴포넌트.
 *
 * 동작 개요:
 *  1. 플레이어 메시 전체에 CustomDepth Stencil = 2 를 마킹한다.
 *  2. USceneCaptureComponent2D (카메라에 부착) 로 플레이어 메시만 별도 렌더링하여
 *     UTextureRenderTarget2D 에 저장한다 (플레이어 캡처 버퍼).
 *  3. UPostProcessComponent + M_CutawayReveal 머티리얼이 두 버퍼를 합성한다:
 *       - FOV 원 안 + 플레이어가 가려진 픽셀 → 캡처 버퍼에서 플레이어 색 출력
 *       - FOV 원 밖 → 씬 색에 다크닝 적용
 *
 * 에디터 필수 작업:
 *  - Project Settings → Rendering → Custom Depth-Stencil Pass → Enabled with Stencil
 *  - Content/_Game/Materials/ 에 M_CutawayReveal (Post Process Material) 생성
 *    (DesignDoc/CutawayReveal_2026-04-12.md 의 노드 지침 참고)
 *  - BP_Player → DS1CutawayComponent → CutawayMaterial 에 M_CutawayReveal 지정
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class DS1_API UDS1CutawayComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UDS1CutawayComponent();

	// ── 에디터 프로퍼티 ─────────────────────────────────────────────────────

	/** M_CutawayReveal 포스트 프로세스 머티리얼 (BP 에서 에셋 지정) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Cutaway")
	TObjectPtr<UMaterialInterface> CutawayMaterial;

	/**
	 * FOV 원형 마스크 반지름 (화면 높이 대비 비율, 0~1).
	 * 0.35 = 화면 높이의 35% 반지름 원.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Cutaway", meta=(ClampMin="0.05", ClampMax="1.0"))
	float FOVRadiusFraction = 0.35f;

	/** FOV 원 경계 페이드 폭 (0=날카로움, 클수록 그라데이션 부드러움) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Cutaway", meta=(ClampMin="0.0", ClampMax="0.3"))
	float FOVEdgeSoftness = 0.06f;

	/** FOV 밖 다크닝 강도 (0=없음, 1=완전 검정) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Cutaway", meta=(ClampMin="0.0", ClampMax="1.0"))
	float DarkOutsideFOV = 0.65f;

	/**
	 * 플레이어 캡처 렌더타깃 해상도 배율 (메인 뷰포트 대비).
	 * 0.5 = 절반 해상도 (성능↑, 품질 약간↓).
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Cutaway", meta=(ClampMin="0.25", ClampMax="1.0"))
	float CaptureResolutionFraction = 0.5f;

	/** 컷어웨이 시스템 활성 여부 (런타임 토글 가능) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Cutaway")
	bool bCutawayEnabled = true;

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

private:
	/** 플레이어 메시 전체에 CustomDepth Stencil(2) 설정 */
	void SetupPlayerCustomDepth();

	/** SceneCaptureComponent2D 를 카메라에 부착하고 설정 */
	void SetupSceneCapture();

	/** PostProcessComponent 를 생성하고 동적 머티리얼 인스턴스를 등록 */
	void SetupPostProcess();

	/** 매 프레임 머티리얼 파라미터(플레이어 스크린 좌표, FOV 크기 등) 갱신 */
	void UpdateMaterialParameters();

	// ── 런타임 오브젝트 ────────────────────────────────────────────────────

	UPROPERTY()
	TObjectPtr<USceneCaptureComponent2D> PlayerCapture;

	UPROPERTY()
	TObjectPtr<UTextureRenderTarget2D> PlayerCaptureRT;

	UPROPERTY()
	TObjectPtr<UPostProcessComponent> CutawayPPComp;

	UPROPERTY()
	TObjectPtr<UMaterialInstanceDynamic> CutawayMID;

	// ── M_CutawayReveal 파라미터 이름 (머티리얼 쪽과 동일해야 함) ──────────
	static const FName PN_PlayerCaptureTex;  // Texture2D  param
	static const FName PN_FOVRadius;         // Scalar     param
	static const FName PN_FOVEdgeSoftness;   // Scalar     param
	static const FName PN_DarkOutside;       // Scalar     param
	static const FName PN_PlayerScreenPos;   // Vector4    param (XY=UV, Z=aspect)
};
