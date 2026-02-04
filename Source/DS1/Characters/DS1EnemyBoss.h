// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Characters/DS1Enemy.h"
#include "DS1EnemyBoss.generated.h"

class UDS1BossHealthBarWidget;
/**
 * 
 */
UCLASS()
class DS1_API ADS1EnemyBoss : public ADS1Enemy
{
	GENERATED_BODY()

protected:
	UPROPERTY(EditAnywhere)
	TSubclassOf<UUserWidget> BossHealthBarWidgetClass;

	UPROPERTY()
	UDS1BossHealthBarWidget* BossHealthBarWidget;

protected:
	UPROPERTY(EditAnywhere)
	USoundWave* BossMusicAsset;

	UPROPERTY()
	UAudioComponent* BossMusic;

	bool bStartedBossMusic = false;

public:
	virtual void SeesTarget(AActor* InTargetActor) override;
	virtual void Tick(float DeltaSeconds) override;

protected:
	virtual void BeginPlay() override;

	virtual void OnDeath() override;
	
};
