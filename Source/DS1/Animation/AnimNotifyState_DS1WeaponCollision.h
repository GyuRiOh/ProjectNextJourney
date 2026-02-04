// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "DS1Define.h"
#include "Animation/AnimNotifies/AnimNotifyState.h"
#include "AnimNotifyState_DS1WeaponCollision.generated.h"

/**
 * 
 */
UCLASS(meta=(DisplayName="Weapon Collision"))
class DS1_API UAnimNotifyState_DS1WeaponCollision : public UAnimNotifyState
{
	GENERATED_BODY()

protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EWeaponCollisionType CollisionType = EWeaponCollisionType::MainCollision;

public:
	UAnimNotifyState_DS1WeaponCollision(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());
	virtual void NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration, const FAnimNotifyEventReference& EventReference) override;
	virtual void NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference) override;
	
};
