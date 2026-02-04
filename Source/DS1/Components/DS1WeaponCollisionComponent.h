// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Kismet/KismetSystemLibrary.h"
#include "DS1WeaponCollisionComponent.generated.h"


DECLARE_MULTICAST_DELEGATE_OneParam(FOnHitActor, const FHitResult&);

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent), meta=(DisplayName = "Weapon Collision") )
class DS1_API UDS1WeaponCollisionComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	FOnHitActor OnHitActor;

protected:
	UPROPERTY(EditAnywhere)
	/** 시작 소켓 */
	FName TraceStartSocketName;

	/** 끝 소켓 */
	UPROPERTY(EditAnywhere)
	FName TraceEndSocketName;

protected:
	/** Sphere 크기(반지름) */
	UPROPERTY(EditAnywhere)
	float TraceRadius = 20.f;

	/** Trace 대상 ObjectType */
	UPROPERTY(EditAnywhere)
	TArray <TEnumAsByte<EObjectTypeQuery>> TraceObjectTypes;

	/** 충돌처리 제외 Actors */
	UPROPERTY(EditAnywhere)
	TArray<AActor*> IgnoredActors;

	/** 디버그 드로우 타입 */
	UPROPERTY(EditAnywhere)
	TEnumAsByte<EDrawDebugTrace::Type> DrawDebugType = EDrawDebugTrace::ForDuration;

protected:
	/** 무기의 MeshComponent */
	// StaticMesh, SkeletalMesh 등
	UPROPERTY()
	UPrimitiveComponent* WeaponMesh;

	UPROPERTY()
	TArray<AActor*> AlreadyHitActors;

	bool bIsCollisionEnabled = false;

public:	
	UDS1WeaponCollisionComponent();

protected:
	virtual void BeginPlay() override;

public:	
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
	void TurnOnCollision();

	void TurnOffCollision();

	void SetWeaponMesh(UPrimitiveComponent* MeshComponent);

	void AddIgnoredActor(AActor* Actor);

	void RemoveIgnoredActor(AActor* Actor);

protected:
	bool CanHitActor(AActor* Actor) const;

	void CollisionTrace();
	
};
