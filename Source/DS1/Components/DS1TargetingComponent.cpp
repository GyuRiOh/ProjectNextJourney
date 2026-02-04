// Fill out your copyright notice in the Description page of Project Settings.


#include "DS1TargetingComponent.h"

#include "Camera/CameraComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Interfaces/DS1Targeting.h"
#include "Kismet/KismetMathLibrary.h"
#include "Kismet/KismetSystemLibrary.h"


UDS1TargetingComponent::UDS1TargetingComponent()
{
	PrimaryComponentTick.bCanEverTick = true;

}

void UDS1TargetingComponent::BeginPlay()
{
	Super::BeginPlay();

	Character = Cast<ACharacter>(GetOwner());
	if (Character)
	{
		Camera = Character->GetComponentByClass<UCameraComponent>();
	}
	
}

void UDS1TargetingComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (bIsLockOn == false)
	{
		return;
	}

	if (IsValid(Character) == false)
	{
		return;
	}

	if (IsValid(LockedTargetActor) == false)
	{
		return;
	}

	const float Distance = FVector::Distance(Character->GetActorLocation(), LockedTargetActor->GetActorLocation());

	if (IDS1Targeting* Targeting = Cast<IDS1Targeting>(LockedTargetActor))
	{
		// TargetingRadius 보다 멀어지면 LockedOn을 중단한다.
		if (Targeting->CanBeTargeted() == false || Distance > TargetingRadius)
		{
			StopLockOn();
		}
		else
		{
			FaceLockOnActor();
		}
	}

}

void UDS1TargetingComponent::ToggleLockOn()
{
	if (bIsLockOn)
	{
		StopLockOn();
	}
	else
	{
		LockOnTarget();
	}
}

void UDS1TargetingComponent::SwitchingLockedOnActor(ESwitchingDirection InDirection)
{
	if (::IsValid(LockedTargetActor))
	{
		if (IDS1Targeting* Targeting = Cast<IDS1Targeting>(LockedTargetActor))
		{
			// 기존 타겟 해제.
			Targeting->OnTargeted(false);

			// 신규 타겟 지정.
			TArray<AActor*> OutTargets;
			FindTargets(OutTargets);
			AActor* TargetActor = FindClosestTarget(OutTargets, InDirection);

			if (::IsValid(TargetActor))
			{
				if (IDS1Targeting* NewTargeting = Cast<IDS1Targeting>(TargetActor))
				{
					LockedTargetActor = TargetActor;
					NewTargeting->OnTargeted(true);
				}
			}
			else
			{
				StopLockOn();
			}
		}
	}
}

void UDS1TargetingComponent::FindTargets(TArray<AActor*>& OutTargetingActors) const
{
	TArray<FHitResult> OutHits;
	const FVector Start = Character->GetActorLocation();
	const FVector End = Start;

	TArray<TEnumAsByte<EObjectTypeQuery>> ObjectTypes;
	ObjectTypes.Add(UEngineTypes::ConvertToObjectType(COLLISION_OBJECT_TARGETING));

	TArray<AActor*> ActorsToIgnore;

	const bool bHit = UKismetSystemLibrary::SphereTraceMultiForObjects(
		GetOwner(),
		Start,
		End,
		TargetingRadius,
		ObjectTypes,
		false,
		ActorsToIgnore,
		DrawDebugType,
		OutHits,
		true);

	if (bHit)
	{
		for (const FHitResult& HitResult : OutHits)
		{
			AActor* HitActor = HitResult.GetActor();
			if (IDS1Targeting* Targeting = Cast<IDS1Targeting>(HitActor))
			{
				if (Targeting->CanBeTargeted())
				{
					OutTargetingActors.Add(HitActor);
				}
			}
		}
	}
}

AActor* UDS1TargetingComponent::FindClosestTarget(TArray<AActor*>& InTargets, ESwitchingDirection InDirection) const
{
	float TargetCompareValue = 0.f;
	AActor* ClosestTarget = nullptr;

	for (const AActor* TargetActor : InTargets)
	{
		if (InDirection != ESwitchingDirection::None && LockedTargetActor == TargetActor)
		{
			continue;
		}

		FHitResult OutHit;
		const FVector Start = Camera->GetComponentLocation();
		const FVector End = TargetActor->GetActorLocation();

		TArray<AActor*> ActorsToIgnore;


		// 대상이 ECC_Visibility 채널에 대해서 Response가 Block으로 설정되어 있는지 확인 필요.
		const bool bHit = UKismetSystemLibrary::LineTraceSingle(
			GetOwner(),
			Start,
			End,
			UEngineTypes::ConvertToTraceType(ECC_Visibility),
			false,
			ActorsToIgnore,
			DrawDebugType,
			OutHit,
			true);

		if (bHit)
		{
			// 카메라의 시야를 기준으로 하기 때문에 내적 계산은 카메라 기준으로 처리합니다.
			// Left 방향에 따른 내적 계산.
			if (InDirection == ESwitchingDirection::Left)
			{
				// 비교대상 Vector가 Normal 방향이기 때문에 반대로 계산됩니다.
				if (FVector::DotProduct(Camera->GetRightVector(), OutHit.Normal) > 0.f == false)
				{
					continue;
				}
			}

			// Right 방향에 따른 내적 계산.
			if ( InDirection == ESwitchingDirection::Right)
			{
				if (FVector::DotProduct(Camera->GetRightVector(), OutHit.Normal) < 0.f == false)
				{
					continue;
				}
			}

			AActor* HitActor = OutHit.GetActor();

			// 플레이어와 가장 가까운 타겟을 찾는다.
			// 전방에 있는 가장 가까운 타겟을 찾기 위해서 카메라의 ForwardVector와 LookAt Vector를 내적합니다.
			const FRotator LookAtRotation = UKismetMathLibrary::FindLookAtRotation(Character->GetActorLocation(), HitActor->GetActorLocation());
			float CheckValue = FVector::DotProduct(Camera->GetForwardVector(), LookAtRotation.Vector());
			
			if (CheckValue > TargetCompareValue)
			{
				TargetCompareValue = CheckValue;
				ClosestTarget = HitActor;
			}
		}
	}

	return ClosestTarget;
}

void UDS1TargetingComponent::OrientCamera() const
{
	Character->GetCharacterMovement()->bOrientRotationToMovement = false;
	Character->GetCharacterMovement()->bUseControllerDesiredRotation = true;
}

void UDS1TargetingComponent::OrientMovement() const
{
	Character->GetCharacterMovement()->bOrientRotationToMovement = true;
	Character->GetCharacterMovement()->bUseControllerDesiredRotation = false;
}

void UDS1TargetingComponent::FaceLockOnActor() const
{
	const FRotator CurrentControlRotation = Character->GetControlRotation();

	const FVector TargetLocation = LockedTargetActor->GetActorLocation() - FVector(0.f, 0.f, 150.f);
	const FRotator TargetLookAtRotation = UKismetMathLibrary::FindLookAtRotation(Character->GetActorLocation(), TargetLocation);

	FRotator InterpRotation = FMath::RInterpTo(CurrentControlRotation, TargetLookAtRotation, GetWorld()->GetDeltaSeconds(), FaceLockOnRotationSpeed);

	// Roll은 기존 ControlRotation.Roll값으로 유지하고 Pitch/Yaw 값만 InterpRotation의 값으로 적용.
	Character->GetController()->SetControlRotation(FRotator(InterpRotation.Pitch, InterpRotation.Yaw, CurrentControlRotation.Roll));
}

void UDS1TargetingComponent::LockOnTarget()
{
	TArray<AActor*> OutTargets;
	FindTargets(OutTargets);
	AActor* TargetActor = FindClosestTarget(OutTargets);

	if (::IsValid(TargetActor))
	{
		LockedTargetActor = TargetActor;
		bIsLockOn = true;
		if (IDS1Targeting* Targeting = Cast<IDS1Targeting>(LockedTargetActor))
		{
			Targeting->OnTargeted(bIsLockOn);
		}

		OrientCamera();
	}
}

void UDS1TargetingComponent::StopLockOn()
{
	bIsLockOn = false;
	if (IDS1Targeting* Targeting = Cast<IDS1Targeting>(LockedTargetActor))
	{
		Targeting->OnTargeted(bIsLockOn);
	}
	LockedTargetActor = nullptr;
	OrientMovement();
}


