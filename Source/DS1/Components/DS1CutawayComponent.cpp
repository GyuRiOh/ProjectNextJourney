// Fill out your copyright notice in the Description page of Project Settings.

#include "Components/DS1CutawayComponent.h"

#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/PostProcessComponent.h"
#include "Components/SceneCaptureComponent2D.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/TextureRenderTarget2D.h"
#include "GameFramework/Character.h"
#include "Materials/MaterialInstanceDynamic.h"

// ── 파라미터 이름 상수 ──────────────────────────────────────────────────────
const FName UDS1CutawayComponent::PN_PlayerCaptureTex(TEXT("PlayerCaptureTex"));
const FName UDS1CutawayComponent::PN_FOVRadius       (TEXT("FOVRadius"));
const FName UDS1CutawayComponent::PN_FOVEdgeSoftness (TEXT("FOVEdgeSoftness"));
const FName UDS1CutawayComponent::PN_DarkOutside     (TEXT("DarkOutside"));
const FName UDS1CutawayComponent::PN_PlayerScreenPos (TEXT("PlayerScreenPos"));

// ── 생성자 ──────────────────────────────────────────────────────────────────
UDS1CutawayComponent::UDS1CutawayComponent()
{
	// PostUpdateWork 그룹: 카메라 이동 후 파라미터 갱신
	PrimaryComponentTick.bCanEverTick   = true;
	PrimaryComponentTick.TickGroup      = TG_PostUpdateWork;
}

// ── BeginPlay ───────────────────────────────────────────────────────────────
void UDS1CutawayComponent::BeginPlay()
{
	Super::BeginPlay();

	if (!bCutawayEnabled || !CutawayMaterial)
	{
		SetComponentTickEnabled(false);
		return;
	}

	SetupPlayerCustomDepth();
	SetupSceneCapture();
	SetupPostProcess();
}

// ── EndPlay ─────────────────────────────────────────────────────────────────
void UDS1CutawayComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (PlayerCapture)
	{
		PlayerCapture->DestroyComponent();
		PlayerCapture = nullptr;
	}
	if (CutawayPPComp)
	{
		CutawayPPComp->DestroyComponent();
		CutawayPPComp = nullptr;
	}
	Super::EndPlay(EndPlayReason);
}

// ── TickComponent ───────────────────────────────────────────────────────────
void UDS1CutawayComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	UpdateMaterialParameters();
}

// ── SetupPlayerCustomDepth ──────────────────────────────────────────────────
void UDS1CutawayComponent::SetupPlayerCustomDepth()
{
	ACharacter* Owner = Cast<ACharacter>(GetOwner());
	if (!Owner) return;

	// Stencil 값 2 = 컷어웨이 대상 (기존 OcclusionReveal 이 1 을 썼으므로 충돌 방지)
	constexpr int32 CutawayStencil = 2;

	auto MarkMesh = [CutawayStencil](UPrimitiveComponent* Mesh)
	{
		if (!Mesh) return;
		Mesh->SetRenderCustomDepth(true);
		Mesh->SetCustomDepthStencilValue(CutawayStencil);
	};

	// 기본 골격 메시
	MarkMesh(Owner->GetMesh());

	// DS1Character 의 TorsoMesh / LegsMesh / FeetMesh 등 추가 SkeletalMesh 일괄 처리
	TArray<USkeletalMeshComponent*> SkelMeshes;
	Owner->GetComponents<USkeletalMeshComponent>(SkelMeshes);
	for (USkeletalMeshComponent* SM : SkelMeshes)
	{
		MarkMesh(SM);
	}
}

// ── SetupSceneCapture ───────────────────────────────────────────────────────
void UDS1CutawayComponent::SetupSceneCapture()
{
	ACharacter* Owner = Cast<ACharacter>(GetOwner());
	if (!Owner) return;

	UCameraComponent* Camera = Owner->FindComponentByClass<UCameraComponent>();
	if (!Camera) return;

	// ① 렌더 타깃 생성 ─────────────────────────────────────────────────────
	FVector2D ViewportSize(1920.f, 1080.f);
	if (GEngine && GEngine->GameViewport)
	{
		GEngine->GameViewport->GetViewportSize(ViewportSize);
	}

	const int32 RTW = FMath::Max(1, FMath::RoundToInt(ViewportSize.X * CaptureResolutionFraction));
	const int32 RTH = FMath::Max(1, FMath::RoundToInt(ViewportSize.Y * CaptureResolutionFraction));

	PlayerCaptureRT = NewObject<UTextureRenderTarget2D>(this, TEXT("PlayerCutawayRT"));
	PlayerCaptureRT->RenderTargetFormat = ETextureRenderTargetFormat::RTF_RGBA8;
	PlayerCaptureRT->ClearColor         = FLinearColor::Black;
	PlayerCaptureRT->bAutoGenerateMips  = false;
	PlayerCaptureRT->InitAutoFormat(RTW, RTH);
	PlayerCaptureRT->UpdateResourceImmediate(true);

	// ② SceneCaptureComponent2D 생성 및 카메라에 부착 ─────────────────────
	PlayerCapture = NewObject<USceneCaptureComponent2D>(Owner, TEXT("PlayerCutawayCapture"));
	PlayerCapture->AttachToComponent(Camera, FAttachmentTransformRules::SnapToTargetNotIncludingScale);
	PlayerCapture->RegisterComponent();

	PlayerCapture->TextureTarget    = PlayerCaptureRT;
	PlayerCapture->CaptureSource    = ESceneCaptureSource::SCS_FinalColorLDR;

	// 플레이어 메시만 렌더링
	PlayerCapture->PrimitiveRenderMode = ESceneCapturePrimitiveRenderMode::PRM_UseShowOnlyList;
	PlayerCapture->ShowOnlyActors.Add(Owner);

	// 불필요한 렌더 피처 비활성화 (성능 절약)
	PlayerCapture->ShowFlags.SetAtmosphere(false);
	PlayerCapture->ShowFlags.SetFog(false);
	PlayerCapture->ShowFlags.SetBloom(false);
	PlayerCapture->ShowFlags.SetAmbientOcclusion(false);
	PlayerCapture->ShowFlags.SetDynamicShadows(false);
	PlayerCapture->ShowFlags.SetAntiAliasing(false);
	PlayerCapture->ShowFlags.SetLensFlares(false);
	PlayerCapture->ShowFlags.SetVignette(false);

	// 메인 카메라 FOV 와 동일하게 맞춤
	PlayerCapture->FOVAngle           = Camera->FieldOfView;
	PlayerCapture->bCaptureEveryFrame = true;
	PlayerCapture->bCaptureOnMovement = false;
}

// ── SetupPostProcess ────────────────────────────────────────────────────────
void UDS1CutawayComponent::SetupPostProcess()
{
	if (!CutawayMaterial || !PlayerCaptureRT) return;

	ACharacter* Owner = Cast<ACharacter>(GetOwner());
	if (!Owner) return;

	// ① 동적 머티리얼 인스턴스 생성 ───────────────────────────────────────
	CutawayMID = UMaterialInstanceDynamic::Create(CutawayMaterial, this);
	if (!CutawayMID) return;

	// 텍스처 파라미터: 플레이어 캡처 렌더타깃
	CutawayMID->SetTextureParameterValue(PN_PlayerCaptureTex, PlayerCaptureRT);

	// 스칼라 파라미터 초기값
	CutawayMID->SetScalarParameterValue(PN_FOVRadius,       FOVRadiusFraction);
	CutawayMID->SetScalarParameterValue(PN_FOVEdgeSoftness, FOVEdgeSoftness);
	CutawayMID->SetScalarParameterValue(PN_DarkOutside,     DarkOutsideFOV);

	// ② PostProcessComponent 생성 ────────────────────────────────────────
	CutawayPPComp = NewObject<UPostProcessComponent>(Owner, TEXT("CutawayPP"));
	CutawayPPComp->RegisterComponent();
	CutawayPPComp->bUnbound  = true;   // 볼륨 경계 없이 전체 씬에 적용
	CutawayPPComp->Priority  = 5.f;

	// 머티리얼 등록
	FWeightedBlendable WB;
	WB.Weight = 1.0f;
	WB.Object = CutawayMID;
	CutawayPPComp->Settings.WeightedBlendables.Array.Add(WB);
}

// ── UpdateMaterialParameters ────────────────────────────────────────────────
void UDS1CutawayComponent::UpdateMaterialParameters()
{
	if (!CutawayMID) return;

	ACharacter* Owner = Cast<ACharacter>(GetOwner());
	if (!Owner) return;

	APlayerController* PC = Cast<APlayerController>(Owner->GetController());
	if (!PC) return;

	// 뷰포트 크기
	int32 ViewX = 0, ViewY = 0;
	PC->GetViewportSize(ViewX, ViewY);
	if (ViewX <= 0 || ViewY <= 0) return;

	// 플레이어 허리 높이(캡슐 절반) 를 기준점으로 사용
	float HalfHeight = 90.f;
	if (UCapsuleComponent* Capsule = Owner->GetCapsuleComponent())
	{
		HalfHeight = Capsule->GetScaledCapsuleHalfHeight();
	}
	const FVector PlayerCenter = Owner->GetActorLocation() + FVector(0.f, 0.f, HalfHeight * 0.5f);

	// 월드 좌표 → 스크린 좌표 투영
	FVector2D ScreenPos;
	if (!PC->ProjectWorldLocationToScreen(PlayerCenter, ScreenPos, true)) return;

	// 0~1 정규화 + 종횡비 (Z) 패킹
	const FLinearColor ScreenPosParam(
		ScreenPos.X / static_cast<float>(ViewX),
		ScreenPos.Y / static_cast<float>(ViewY),
		static_cast<float>(ViewX) / static_cast<float>(ViewY), // aspect ratio (W/H)
		0.f
	);
	CutawayMID->SetVectorParameterValue(PN_PlayerScreenPos, ScreenPosParam);
}
