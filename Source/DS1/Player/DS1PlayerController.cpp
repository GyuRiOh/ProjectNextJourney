// Fill out your copyright notice in the Description page of Project Settings.

#include "Player/DS1PlayerController.h"

#include "Blueprint/UserWidget.h"
#include "UI/DS1InventoryWidget.h"

void ADS1PlayerController::BeginPlay()
{
	Super::BeginPlay();

	if (InventoryWidgetClass)
	{
		InventoryWidget = CreateWidget<UDS1InventoryWidget>(this, InventoryWidgetClass);
	}
}

void ADS1PlayerController::ToggleInventory()
{
	if (!InventoryWidget)
	{
		return;
	}

	if (bInventoryOpen)
	{
		// 인벤토리 닫기
		InventoryWidget->RemoveFromParent();
		bInventoryOpen = false;

		SetShowMouseCursor(false);
		SetInputMode(FInputModeGameOnly());
	}
	else
	{
		// 인벤토리 열기
		InventoryWidget->AddToViewport(10);
		bInventoryOpen = true;

		SetShowMouseCursor(true);
		FInputModeGameAndUI InputMode;
		InputMode.SetWidgetToFocus(InventoryWidget->TakeWidget());
		InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
		SetInputMode(InputMode);
	}
}
