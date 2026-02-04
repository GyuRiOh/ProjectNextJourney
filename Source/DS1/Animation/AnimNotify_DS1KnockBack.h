// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotify.h"
#include "AnimNotify_DS1KnockBack.generated.h"

/**
 * 
 */
UCLASS(meta=(DisplayName="Knockback Damage"))
class DS1_API UAnimNotify_DS1KnockBack : public UAnimNotify
{
	GENERATED_BODY()

protected:
	UPROPERTY(EditAnywhere)
	float BaseDamage = 10.f;

	UPROPERTY(EditAnywhere)
	FName LocationSocketName;

	UPROPERTY(EditAnywhere)
	float DamageRadius = 50.f;

	UPROPERTY(EditAnywhere)
	bool bDrawDebug = false;

public:
	virtual void Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference) override;
	
};
