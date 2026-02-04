// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BTTask_FindPatrolPoint.generated.h"

/**
 * 
 */
UCLASS()
class DS1_API UBTTask_FindPatrolPoint : public UBTTaskNode
{
	GENERATED_BODY()

protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FBlackboardKeySelector BlackboardLocation;

public:
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	
};
