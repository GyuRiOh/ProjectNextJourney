// Fill out your copyright notice in the Description page of Project Settings.


#include "Characters/DS1EnemyBoss.h"

#include "Components/AudioComponent.h"
#include "Components/WidgetComponent.h"
#include "Kismet/GameplayStatics.h"
#include "UI/DS1BossHealthBarWidget.h"

void ADS1EnemyBoss::SeesTarget(AActor* InTargetActor)
{
	if (IsValid(InTargetActor))
	{
		if (BossHealthBarWidget)
		{
			BossHealthBarWidget->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
		}
	}

	// 보스 배경음악
	if (BossMusicAsset)
	{
		if (!bStartedBossMusic)
		{
			bStartedBossMusic = true;
			BossMusic = UGameplayStatics::SpawnSound2D(this, BossMusicAsset);
			BossMusic->FadeIn(1.f);
		}
	}
}

void ADS1EnemyBoss::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	GEngine->AddOnScreenDebugMessage(1, 1.5f, FColor::Cyan, FString::Printf(TEXT("%f"), AttributeComponent->GetBaseStamina()));
}

void ADS1EnemyBoss::BeginPlay()
{
	Super::BeginPlay();

	// 체력바 텀포넌트 삭제
	// 보스는 사용안함.
	if (HealthBarWidgetComponent)
	{
		HealthBarWidgetComponent->DestroyComponent();
	}

	// 보스 체력바 생성
	if (BossHealthBarWidgetClass)
	{
		BossHealthBarWidget = CreateWidget<UDS1BossHealthBarWidget>(GetWorld(), BossHealthBarWidgetClass);
		if (BossHealthBarWidget)
		{
			BossHealthBarWidget->Init(AttributeComponent);
			BossHealthBarWidget->AddToViewport();
			BossHealthBarWidget->SetVisibility(ESlateVisibility::Hidden);
		}
	}
}

void ADS1EnemyBoss::OnDeath()
{
	Super::OnDeath();

	if (BossHealthBarWidget)
	{
		BossHealthBarWidget->SetVisibility(ESlateVisibility::Hidden);
	}

	if (IsValid(BossMusic))
	{
		BossMusic->FadeOut(2.f, 0);
	}
}
