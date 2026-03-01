// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "DS1VisibilityComponent.generated.h"

class ADS1Enemy;

/**
 * 플레이어 캐릭터에 부착되어 주변 적의 가시성(Visibility)을 관리하는 컴포넌트.
 *
 * 동작 방식:
 *   - CheckInterval마다 타이머로 CheckEnemyVisibility() 실행
 *   - VisibilityRadius 내, ConeAngleDegrees 범위 내 적을 수집
 *   - 플레이어 → 적 방향으로 LineTrace (ECC_Visibility) 수행
 *   - 장애물에 가려진 적은 SetVisibleToPlayer(false), 시야 확보된 적은 (true)
 *   - cone 밖이거나 반경 밖 적은 자동으로 숨김
 *
 * Forward 벡터: GetActorForwardVector() (Option A)
 *   - bOrientRotationToMovement=true이므로 이동 방향 = 캐릭터 전방
 *   - 탐험/발견 긴장감 부여
 *   - XY 평면 기준 dot product → 지형 경사에 무관
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class DS1_API UDS1VisibilityComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UDS1VisibilityComponent();

	/** 가시성 체크 주기 (초) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Visibility")
	float CheckInterval = 0.15f;

	/** 가시성 판정 최대 반경 (Unreal Units) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Visibility")
	float VisibilityRadius = 2000.f;

	/** 시야 cone 전체 각도 (degrees). 예: 90 → 좌우 각 45° */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Visibility", meta=(ClampMin="0", ClampMax="360"))
	float ConeAngleDegrees = 90.f;

	/**
	 * 시야 전방축 Yaw 보정값 (degrees).
	 * 메시/액터 축이 어긋난 경우 시야 콘 방향을 미세 조정한다.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Visibility")
	float ForwardYawOffsetDegrees = 0.f;

	/**
	 * true면 메시 전방축을 시야 기준으로 사용한다.
	 * 기본값 false: 액터 전방축 기반(기존 동작).
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Visibility")
	bool bUseMeshForwardAsVisionBasis = false;

	/**
	 * 근접 전방위 탐지 반경 (Unreal Units).
	 * 이 거리 안에 있는 적은 뒤쪽에 있어도 항상 보임 (인기척 개념).
	 * 0 이하이면 근접 탐지 비활성화.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Visibility", meta=(ClampMin="0"))
	float NearDetectionRadius = 400.f;

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

private:
	FTimerHandle VisibilityTimerHandle;

	/** 현재 보이는 상태로 관리 중인 적 목록 */
	TSet<TWeakObjectPtr<ADS1Enemy>> CurrentlyVisibleEnemies;

	void CheckEnemyVisibility();

	/**
	 * From에서 Enemy까지 LineTrace(ECC_Visibility)를 수행하여 시야 확보 여부를 반환.
	 * 첫 번째 히트가 Enemy 자신이거나 아무것도 안 맞으면 true (시야 확보).
	 */
	bool HasLineOfSight(const FVector& From, const ADS1Enemy* Enemy) const;

public:
	/** 시야 계산에 사용하는 수평 전방 벡터(정규화) */
	FVector GetVisionForward2D() const;

	/** 시야 원뿔 절반 각도 (degrees). 외부에서 오버레이 위젯이 읽는 용도 */
	FORCEINLINE float GetConeHalfAngleDegrees() const { return ConeAngleDegrees * 0.5f; }

	/** 시야 반경 (Unreal Units) */
	FORCEINLINE float GetVisibilityRadius() const { return VisibilityRadius; }

	/** 근접 전방위 탐지 반경 (Unreal Units) */
	FORCEINLINE float GetNearDetectionRadius() const { return NearDetectionRadius; }
};
