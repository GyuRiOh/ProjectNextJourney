// Fill out your copyright notice in the Description page of Project Settings.

#include "Components/DS1CutawayComponent.h"

#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/DS1VisibilityComponent.h"
#include "Components/PostProcessComponent.h"
#include "Components/SceneCaptureComponent2D.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/TextureRenderTarget2D.h"
#include "GameFramework/Character.h"
#include "Materials/MaterialInstanceDynamic.h"

const FName UDS1CutawayComponent::PN_PlayerCaptureTex(TEXT("PlayerCaptureTex"));
const FName UDS1CutawayComponent::PN_FOVRadius(TEXT("FOVRadius"));
const FName UDS1CutawayComponent::PN_FOVEdgeSoftness(TEXT("FOVEdgeSoftness"));
const FName UDS1CutawayComponent::PN_DarkOutside(TEXT("DarkOutside"));
const FName UDS1CutawayComponent::PN_PlayerScreenPos(TEXT("PlayerScreenPos"));

UDS1CutawayComponent::UDS1CutawayComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.TickGroup = TG_PostUpdateWork;
}

void UDS1CutawayComponent::BeginPlay()
{
	Super::BeginPlay();

	if (!bCutawayEnabled || !CutawayMaterial)
	{
		SetComponentTickEnabled(false);
		return;
	}

	if (ACharacter* Owner = Cast<ACharacter>(GetOwner()))
	{
		CachedVisibilityComponent = Owner->FindComponentByClass<UDS1VisibilityComponent>();
	}

	SetupPlayerCustomDepth();
	SetupSceneCapture();
	SetupPostProcess();
}

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

void UDS1CutawayComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	UpdateMaterialParameters();
}

void UDS1CutawayComponent::SetupPlayerCustomDepth()
{
	ACharacter* Owner = Cast<ACharacter>(GetOwner());
	if (!Owner)
	{
		return;
	}

	constexpr int32 CutawayStencil = 2;

	auto MarkMesh = [CutawayStencil](UPrimitiveComponent* Primitive)
	{
		if (!Primitive)
		{
			return;
		}

		Primitive->SetRenderCustomDepth(true);
		Primitive->SetCustomDepthStencilValue(CutawayStencil);
	};

	MarkMesh(Owner->GetMesh());

	TArray<USkeletalMeshComponent*> SkeletalMeshes;
	Owner->GetComponents<USkeletalMeshComponent>(SkeletalMeshes);
	for (USkeletalMeshComponent* SkeletalMesh : SkeletalMeshes)
	{
		MarkMesh(SkeletalMesh);
	}
}

void UDS1CutawayComponent::SetupSceneCapture()
{
	ACharacter* Owner = Cast<ACharacter>(GetOwner());
	if (!Owner)
	{
		return;
	}

	UCameraComponent* Camera = Owner->FindComponentByClass<UCameraComponent>();
	if (!Camera)
	{
		return;
	}

	FVector2D ViewportSize(1920.f, 1080.f);
	if (GEngine && GEngine->GameViewport)
	{
		GEngine->GameViewport->GetViewportSize(ViewportSize);
	}

	const int32 RTW = FMath::Max(1, FMath::RoundToInt(ViewportSize.X * CaptureResolutionFraction));
	const int32 RTH = FMath::Max(1, FMath::RoundToInt(ViewportSize.Y * CaptureResolutionFraction));

	PlayerCaptureRT = NewObject<UTextureRenderTarget2D>(this, TEXT("PlayerCutawayRT"));
	PlayerCaptureRT->RenderTargetFormat = ETextureRenderTargetFormat::RTF_RGBA8;
	PlayerCaptureRT->ClearColor = FLinearColor::Black;
	PlayerCaptureRT->bAutoGenerateMips = false;
	PlayerCaptureRT->InitAutoFormat(RTW, RTH);
	PlayerCaptureRT->UpdateResourceImmediate(true);

	PlayerCapture = NewObject<USceneCaptureComponent2D>(Owner, TEXT("PlayerCutawayCapture"));
	PlayerCapture->AttachToComponent(Camera, FAttachmentTransformRules::SnapToTargetNotIncludingScale);
	PlayerCapture->RegisterComponent();

	PlayerCapture->TextureTarget = PlayerCaptureRT;
	PlayerCapture->CaptureSource = ESceneCaptureSource::SCS_FinalColorLDR;
	PlayerCapture->PrimitiveRenderMode = ESceneCapturePrimitiveRenderMode::PRM_UseShowOnlyList;
	PlayerCapture->ShowOnlyActors.Add(Owner);

	PlayerCapture->ShowFlags.SetAtmosphere(false);
	PlayerCapture->ShowFlags.SetFog(false);
	PlayerCapture->ShowFlags.SetBloom(false);
	PlayerCapture->ShowFlags.SetAmbientOcclusion(false);
	PlayerCapture->ShowFlags.SetDynamicShadows(false);
	PlayerCapture->ShowFlags.SetAntiAliasing(false);
	PlayerCapture->ShowFlags.SetLensFlares(false);
	PlayerCapture->ShowFlags.SetVignette(false);

	PlayerCapture->FOVAngle = Camera->FieldOfView;
	PlayerCapture->bCaptureEveryFrame = true;
	PlayerCapture->bCaptureOnMovement = false;
}

void UDS1CutawayComponent::SetupPostProcess()
{
	if (!CutawayMaterial || !PlayerCaptureRT)
	{
		return;
	}

	ACharacter* Owner = Cast<ACharacter>(GetOwner());
	if (!Owner)
	{
		return;
	}

	CutawayMID = UMaterialInstanceDynamic::Create(CutawayMaterial, this);
	if (!CutawayMID)
	{
		return;
	}

	CutawayMID->SetTextureParameterValue(PN_PlayerCaptureTex, PlayerCaptureRT);

	CutawayPPComp = NewObject<UPostProcessComponent>(Owner, TEXT("CutawayPP"));
	CutawayPPComp->RegisterComponent();
	CutawayPPComp->bUnbound = true;
	CutawayPPComp->Priority = 5.f;

	FWeightedBlendable WeightedBlendable;
	WeightedBlendable.Weight = 1.0f;
	WeightedBlendable.Object = CutawayMID;
	CutawayPPComp->Settings.WeightedBlendables.Array.Add(WeightedBlendable);

	UpdateMaterialParameters();
}

float UDS1CutawayComponent::ResolveRevealRadiusFraction(APlayerController* PC, const FVector& PlayerCenter, const FVector2D& ScreenPos, int32 ViewY)
{
	if (bUseVisibilityNearRadiusForReveal)
	{
		if (!CachedVisibilityComponent.IsValid())
		{
			if (ACharacter* Owner = Cast<ACharacter>(GetOwner()))
			{
				CachedVisibilityComponent = Owner->FindComponentByClass<UDS1VisibilityComponent>();
			}
		}

		if (CachedVisibilityComponent.IsValid())
		{
			const float NearWorldRadius = CachedVisibilityComponent->GetNearDetectionRadius();
			if (NearWorldRadius > KINDA_SMALL_NUMBER)
			{
				FVector2D NearScreenPos;
				if (PC->ProjectWorldLocationToScreen(PlayerCenter + FVector(NearWorldRadius, 0.f, 0.f), NearScreenPos, true))
				{
					const float ScreenRadiusPx = FVector2D::Distance(ScreenPos, NearScreenPos);
					if (ScreenRadiusPx > KINDA_SMALL_NUMBER)
					{
						return FMath::Clamp(ScreenRadiusPx / static_cast<float>(ViewY), 0.01f, 1.0f);
					}
				}
			}
		}
	}

	return FMath::Clamp(FOVRadiusFraction, 0.05f, 1.0f);
}

float UDS1CutawayComponent::ResolveDarkOutsideAmount() const
{
	return bEnableCutawayDarkening ? FMath::Clamp(DarkOutsideFOV, 0.f, 1.f) : 0.f;
}

void UDS1CutawayComponent::UpdateMaterialParameters()
{
	if (!CutawayMID)
	{
		return;
	}

	ACharacter* Owner = Cast<ACharacter>(GetOwner());
	if (!Owner)
	{
		return;
	}

	APlayerController* PC = Cast<APlayerController>(Owner->GetController());
	if (!PC)
	{
		return;
	}

	int32 ViewX = 0;
	int32 ViewY = 0;
	PC->GetViewportSize(ViewX, ViewY);
	if (ViewX <= 0 || ViewY <= 0)
	{
		return;
	}

	float HalfHeight = 90.f;
	if (UCapsuleComponent* Capsule = Owner->GetCapsuleComponent())
	{
		HalfHeight = Capsule->GetScaledCapsuleHalfHeight();
	}

	const FVector PlayerCenter = Owner->GetActorLocation() + FVector(0.f, 0.f, HalfHeight * 0.5f);

	FVector2D ScreenPos;
	if (!PC->ProjectWorldLocationToScreen(PlayerCenter, ScreenPos, true))
	{
		return;
	}

	const FLinearColor ScreenPosParam(
		ScreenPos.X / static_cast<float>(ViewX),
		ScreenPos.Y / static_cast<float>(ViewY),
		static_cast<float>(ViewX) / static_cast<float>(ViewY),
		0.f);

	CutawayMID->SetScalarParameterValue(PN_FOVRadius, ResolveRevealRadiusFraction(PC, PlayerCenter, ScreenPos, ViewY));
	CutawayMID->SetScalarParameterValue(PN_FOVEdgeSoftness, FMath::Clamp(FOVEdgeSoftness, 0.f, 0.3f));
	CutawayMID->SetScalarParameterValue(PN_DarkOutside, ResolveDarkOutsideAmount());
	CutawayMID->SetVectorParameterValue(PN_PlayerScreenPos, ScreenPosParam);
}
