// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "DS1RotationComponent.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class DS1_API UDS1RotationComponent : public UActorComponent
{
	GENERATED_BODY()

protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	AActor* TargetActor;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bShouldRotate = false;

public:	
	UDS1RotationComponent();

public:
	FORCEINLINE void SetTargetActor(AActor* InActor) { TargetActor = InActor; }
	FORCEINLINE void ToggleShouldRotate(const bool bRotate) { bShouldRotate = bRotate; }

protected:
	virtual void BeginPlay() override;

public:	
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

		
};
