// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotifyState.h"
#include "AnimNotifyState_DS1ComboWindow.generated.h"

/**
 * 
 */
UCLASS(meta=(DisplayName = "Combo Window"))
class DS1_API UAnimNotifyState_DS1ComboWindow : public UAnimNotifyState
{
	GENERATED_BODY()

public:
	UAnimNotifyState_DS1ComboWindow(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());
	virtual void NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration, const FAnimNotifyEventReference& EventReference) override;
	virtual void NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference) override;
};
