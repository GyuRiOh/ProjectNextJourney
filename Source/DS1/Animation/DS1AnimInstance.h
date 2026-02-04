// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "DS1Define.h"
#include "Animation/AnimInstance.h"
#include "DS1AnimInstance.generated.h"

class UCharacterMovementComponent;

/**
 * 
 */
UCLASS()
class DS1_API UDS1AnimInstance : public UAnimInstance
{
	GENERATED_BODY()

protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "References")
	ACharacter* Character;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "References")
	UCharacterMovementComponent* MovementComponent;

protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement Data")
	FVector Velocity;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement Data")
	float GroundSpeed;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement Data")
	bool bShouldMove;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement Data")
	bool bIsFalling;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement Data")
	float Direction;

protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Data")
	bool bCombatEnabled = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat Data")
	ECombatType CombatType = ECombatType::None;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat Data")
	bool bShouldBlocking = false;

public:
	UDS1AnimInstance();

	virtual void NativeInitializeAnimation() override;
	virtual void NativeUpdateAnimation(float DeltaSeconds) override;

public:
	UFUNCTION()
	void AnimNotify_ResetMovementInput();

	UFUNCTION()
	void AnimNotify_ResetState();

// Animation
public:
	void UpdateCombatMode(const ECombatType InCombatType);
	void UpdateBlocking(bool InShouldBlocking);

	// Delegate functions
protected:
	void OnChangedCombat(const bool bInCombatEnabled);
	
};
