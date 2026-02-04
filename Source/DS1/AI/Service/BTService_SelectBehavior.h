// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "DS1Define.h"
#include "BehaviorTree/BTService.h"
#include "BTService_SelectBehavior.generated.h"

class ADS1Enemy;
/**
 * 
 */
UCLASS()
class DS1_API UBTService_SelectBehavior : public UBTService
{
	GENERATED_BODY()

protected:
	UPROPERTY(EditAnywhere)
	FBlackboardKeySelector TargetKey;

	UPROPERTY(EditAnywhere)
	FBlackboardKeySelector BehaviorKey;

	UPROPERTY(EditAnywhere)
	float AttackRangeDistance = 500.f;

public:
	UBTService_SelectBehavior();

protected:
	virtual void OnBecomeRelevant(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual void TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;

protected:
	void SetBehaviorKey(UBlackboardComponent* BlackboardComp, EDS1AIBehavior Behavior) const;
	virtual void UpdateBehavior(UBlackboardComponent* BlackboardComp, ADS1Enemy* ControlledEnemy) const;
};
