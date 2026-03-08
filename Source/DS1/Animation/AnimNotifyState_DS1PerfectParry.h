// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotifyState.h"
#include "AnimNotifyState_DS1PerfectParry.generated.h"

/**
 * Perfect Parry window notify state.
 * Place this at the START of the parry montage (shorter than the regular Parry notify).
 * While active, bInPerfectParryWindow on DS1Character is true.
 */
UCLASS(meta=(DisplayName = "Perfect Parry Window"))
class DS1_API UAnimNotifyState_DS1PerfectParry : public UAnimNotifyState
{
	GENERATED_BODY()

public:
	virtual void NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration, const FAnimNotifyEventReference& EventReference) override;

	virtual void NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference) override;
};
