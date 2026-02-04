// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Interfaces/DS1CombatInterface.h"
#include "Interfaces/DS1Targeting.h"
#include "DS1Enemy.generated.h"

class UDS1RotationComponent;
class ADS1Weapon;
class UDS1CombatComponent;
class ATargetPoint;
class UWidgetComponent;
class USphereComponent;
class UDS1StateComponent;
class UDS1AttributeComponent;

UCLASS()
class DS1_API ADS1Enemy : public ACharacter, public IDS1Targeting, public IDS1CombatInterface
{
	GENERATED_BODY()

protected:
	/** 타겟팅 충돌 체크용 */
	UPROPERTY(VisibleAnywhere)
	USphereComponent* TargetingSphereComponent;

	UPROPERTY(VisibleAnywhere)
	UDS1AttributeComponent* AttributeComponent;

	UPROPERTY(VisibleAnywhere)
	UDS1StateComponent* StateComponent;

	UPROPERTY(VisibleAnywhere)
	UDS1CombatComponent* CombatComponent;

	UPROPERTY(VisibleAnywhere)
	UDS1RotationComponent* RotationComponent;

	/** LockOn UI Widget */
	UPROPERTY(VisibleAnywhere)
	UWidgetComponent* LockOnWidgetComponent;

	/** HealthBar */
	UPROPERTY(VisibleAnywhere)
	UWidgetComponent* HealthBarWidgetComponent;

// Effect Section
protected:
	UPROPERTY(EditAnywhere, Category="Effect")
	USoundCue* ImpactSound;

	UPROPERTY(EditAnywhere, Category="Effect")
	UParticleSystem* ImpactParticle;

protected:
	UPROPERTY(EditAnywhere, Category= "AI | Patrol")
	TArray<ATargetPoint*> PatrolPoints;

	UPROPERTY(VisibleAnywhere, Category = "AI | Patrol")
	int32 PatrolIndex = 0;

// 무기
protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSubclassOf<ADS1Weapon> DefaultWeaponClass;

protected:
	FTimerHandle ParriedDelayTimerHandle;
	FTimerHandle StunnedDelayTimerHandle;

	/** 피격시 스턴 확률 */
	UPROPERTY(EditAnywhere)
	int StunnedRate = 0;

public:
	ADS1Enemy();

protected:
	virtual void BeginPlay() override;

public:
	virtual void Tick(float DeltaTime) override;
	virtual float TakeDamage(float Damage, const FDamageEvent& DamageEvent, AController* EventInstigator, AActor* DamageCauser) override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

protected:
	virtual void OnDeath();
	void SetDeathState();
	void OnAttributeChanged(EDS1AttributeType AttributeType, float InValue);
	void SetupHealthBar();

protected:
	void ImpactEffect(const FVector& Location);
	void HitReaction(const AActor* Attacker);

public:
	// IDS1Targeting 구현.
	/** 타겟킹시 처리할 로직 처리 */
	virtual void OnTargeted(bool bTargeted) override;
	/** 타겟팅 가능한지 체크 */
	virtual bool CanBeTargeted() override;

	// IDS1CombatInterface 구현.
	virtual void ActivateWeaponCollision(EWeaponCollisionType WeaponCollisionType) override;
	virtual void DeactivateWeaponCollision(EWeaponCollisionType WeaponCollisionType) override;
	virtual void PerformAttack(FGameplayTag& AttackTypeTag, FOnMontageEnded& MontageEndedDelegate) override;
	virtual void Parried() override;

	// 체력바 토글
	void ToggleHealthBarVisibility(bool bVisibility);
	// 타겟 본다.
	virtual void SeesTarget(AActor* InTargetActor);

public:
	FORCEINLINE ATargetPoint* GetPatrolPoint()
	{
		return PatrolPoints.Num() >= (PatrolIndex + 1) ? PatrolPoints[PatrolIndex] : nullptr;
	}
	FORCEINLINE void IncrementPatrolIndex()
	{
		PatrolIndex = (PatrolIndex + 1) % PatrolPoints.Num();
	}
};
