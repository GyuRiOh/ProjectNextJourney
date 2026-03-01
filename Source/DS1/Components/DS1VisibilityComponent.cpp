// Fill out your copyright notice in the Description page of Project Settings.

#include "Components/DS1VisibilityComponent.h"

#include "Characters/DS1Enemy.h"
#include "GameFramework/Character.h"
#include "Kismet/GameplayStatics.h"

UDS1VisibilityComponent::UDS1VisibilityComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UDS1VisibilityComponent::BeginPlay()
{
	Super::BeginPlay();

	GetWorld()->GetTimerManager().SetTimer(
		VisibilityTimerHandle,
		this,
		&UDS1VisibilityComponent::CheckEnemyVisibility,
		CheckInterval,
		true
	);
}

void UDS1VisibilityComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	GetWorld()->GetTimerManager().ClearTimer(VisibilityTimerHandle);
	Super::EndPlay(EndPlayReason);
}

void UDS1VisibilityComponent::CheckEnemyVisibility()
{
	AActor* Owner = GetOwner();
	if (!Owner)
	{
		return;
	}

	const FVector PlayerLocation = Owner->GetActorLocation();

	// XY 평면 기준 전방 벡터 (경사 지형에 무관하게 수평 판정)
	const FVector OwnerForward2D = GetVisionForward2D();

	// cone 절반 각도의 cos 값 — 매 체크마다 1회만 계산
	const float HalfAngleCos = FMath::Cos(FMath::DegreesToRadians(ConeAngleDegrees * 0.5f));

	// 씬 내 모든 ADS1Enemy 수집
	TArray<AActor*> AllEnemyActors;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), ADS1Enemy::StaticClass(), AllEnemyActors);

	TSet<TWeakObjectPtr<ADS1Enemy>> NewVisibleEnemies;

	for (AActor* Actor : AllEnemyActors)
	{
		ADS1Enemy* Enemy = Cast<ADS1Enemy>(Actor);
		if (!IsValid(Enemy))
		{
			continue;
		}

		bool bVisible = false;

		const float Distance = FVector::Distance(PlayerLocation, Enemy->GetActorLocation());

		if (NearDetectionRadius > 0.f && Distance <= NearDetectionRadius)
		{
			// 근접 전방위 탐지: 방향 무관, 항상 가시 (인기척)
			bVisible = true;
		}
		else if (Distance <= VisibilityRadius)
		{
			// Cone 체크: 플레이어 전방과 적 방향의 수평 각도 비교
			const FVector ToEnemy2D = (Enemy->GetActorLocation() - PlayerLocation).GetSafeNormal2D();
			const float Dot = FVector::DotProduct(OwnerForward2D, ToEnemy2D);

			if (Dot >= HalfAngleCos)
			{
				// Cone 내부 → LOS 체크
				bVisible = HasLineOfSight(PlayerLocation, Enemy);
			}
		}

		Enemy->SetVisibleToPlayer(bVisible);

		if (bVisible)
		{
			NewVisibleEnemies.Add(Enemy);
		}
	}

	CurrentlyVisibleEnemies = NewVisibleEnemies;
}

FVector UDS1VisibilityComponent::GetVisionForward2D() const
{
	const AActor* Owner = GetOwner();
	if (!Owner)
	{
		return FVector::ForwardVector;
	}

	FVector BaseForward = Owner->GetActorForwardVector();

	if (bUseMeshForwardAsVisionBasis)
	{
		if (const ACharacter* OwnerCharacter = Cast<ACharacter>(Owner))
		{
			if (const USkeletalMeshComponent* Mesh = OwnerCharacter->GetMesh())
			{
				const FVector MeshForward = Mesh->GetForwardVector();
				if (!MeshForward.IsNearlyZero())
				{
					BaseForward = MeshForward;
				}
			}
		}
	}

	FVector Forward2D = FVector(BaseForward.X, BaseForward.Y, 0.f).GetSafeNormal();
	if (Forward2D.IsNearlyZero())
	{
		Forward2D = FVector::ForwardVector;
	}

	if (!FMath::IsNearlyZero(ForwardYawOffsetDegrees))
	{
		Forward2D = FRotator(0.f, ForwardYawOffsetDegrees, 0.f).RotateVector(Forward2D).GetSafeNormal();
	}

	return Forward2D;
}

bool UDS1VisibilityComponent::HasLineOfSight(const FVector& From, const ADS1Enemy* Enemy) const
{
	// 눈높이 보정 (캐릭터 허리~눈 위치)
	const FVector EyeOffset = FVector(0.f, 0.f, 60.f);
	const FVector Start = From + EyeOffset;
	const FVector End = Enemy->GetActorLocation() + EyeOffset;

	FHitResult Hit;
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(GetOwner());

	const bool bHit = GetWorld()->LineTraceSingleByChannel(Hit, Start, End, ECC_Visibility, Params);

	// 아무것도 안 맞거나, 맞은 대상이 적 자신이면 → 시야 확보
	return !bHit || Hit.GetActor() == Enemy;
}
