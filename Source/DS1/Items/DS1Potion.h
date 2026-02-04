// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "DS1Potion.generated.h"

UCLASS()
class DS1_API ADS1Potion : public AActor
{
	GENERATED_BODY()

protected:
	UPROPERTY(EditAnywhere)
	UStaticMeshComponent* Mesh;
	
public:	
	ADS1Potion();
};
