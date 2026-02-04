// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTService.h"
#include "BTService_Strafe.generated.h"

/**
 * 
 */
UCLASS()
class DS1_API UBTService_Strafe : public UBTService
{
	GENERATED_BODY()

protected:
	UPROPERTY(EditAnywhere)
	FBlackboardKeySelector TargetKey;

	UPROPERTY(EditAnywhere)
	float SearchRadius = 300.f;

public:
	UBTService_Strafe();

protected:
	virtual void OnBecomeRelevant(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;

	virtual void OnCeaseRelevant(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;

	virtual void TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;
};
