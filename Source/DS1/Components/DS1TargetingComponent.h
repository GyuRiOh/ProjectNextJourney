// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "DS1Define.h"
#include "Components/ActorComponent.h"
#include "Kismet/KismetSystemLibrary.h"
#include "DS1TargetingComponent.generated.h"


class UCameraComponent;

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class DS1_API UDS1TargetingComponent : public UActorComponent
{
	GENERATED_BODY()

protected:
	/** LockOn 시킬 후보군을 감지할 영역의 크기(반지름) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float TargetingRadius = 500.f;

	/** LockOn시 카메라 회전 속도 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float FaceLockOnRotationSpeed = 20.f;

	/** 디버그 드로우 타입 */
	UPROPERTY(EditAnywhere)
	TEnumAsByte<EDrawDebugTrace::Type> DrawDebugType = EDrawDebugTrace::ForDuration;

protected:
	UPROPERTY()
	ACharacter* Character;

	UPROPERTY()
	UCameraComponent* Camera;

	/** 현재 LockOn된 대상 */
	UPROPERTY()
	AActor* LockedTargetActor;

	/** LockOn 상태 관리 */
	bool bIsLockOn = false;

public:
	UDS1TargetingComponent();

protected:
	virtual void BeginPlay() override;

public:
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
	/** LockedOn On/Off */
	void ToggleLockOn();

	/** LockOn 대상 스위칭 */
	void SwitchingLockedOnActor(ESwitchingDirection InDirection);

	/** LockOn 상태 */
	FORCEINLINE bool IsLockOn() const { return bIsLockOn; }

protected:
	/** LockOn 시킬 후보군을 찾습니다. */
	void FindTargets(OUT TArray<AActor*>& OutTargetingActors) const;

	/** 최종 LockOn 시킬 대상을 찾아 줍니다. */
	AActor* FindClosestTarget(TArray<AActor*>& InTargets, ESwitchingDirection InDirection = ESwitchingDirection::None) const;

	/** 캐릭터가 카메라(ControlRotation)의 회전에 동기화 되도록 설정 */
	void OrientCamera() const;

	/** 캐릭터가 이동 방향으로 회전하도록 설정 */
	void OrientMovement() const;

	/** LockedOn 상태에서 카메라 회전 제어 */
	void FaceLockOnActor() const;

	/** LockOn */
	void LockOnTarget();

	/** LockOn 중지 */
	void StopLockOn();
};
