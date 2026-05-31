// Fill out your copyright notice in the Description page of Project Settings.

#include "Player/DS1PlayerController.h"

#include "Blueprint/WidgetBlueprintLibrary.h"
#include "Components/DS1InventoryComponent.h"
#include "GameFramework/Pawn.h"
#include "Player/DS1CheatManager.h"
#include "UI/DS1InventoryWidget.h"

ADS1PlayerController::ADS1PlayerController()
{
	CheatClass = UDS1CheatManager::StaticClass();
}

void ADS1PlayerController::BeginPlay()
{
	Super::BeginPlay();

	InventoryWidget = CreateWidget<UDS1InventoryWidget>(this, UDS1InventoryWidget::StaticClass());
}

void ADS1PlayerController::ToggleInventory()
{
	if (!InventoryWidget)
	{
		return;
	}

	TArray<UUserWidget*> ExistingInventoryWidgets;
	UWidgetBlueprintLibrary::GetAllWidgetsOfClass(this, ExistingInventoryWidgets, UDS1InventoryWidget::StaticClass(), false);

	if (bInventoryOpen)
	{
		for (UUserWidget* Widget : ExistingInventoryWidgets)
		{
			if (Widget)
			{
				Widget->RemoveFromParent();
			}
		}

		InventoryWidget->RemoveFromParent();
		bInventoryOpen = false;

		SetShowMouseCursor(false);
		SetInputMode(FInputModeGameOnly());
		return;
	}

	for (UUserWidget* Widget : ExistingInventoryWidgets)
	{
		if (Widget)
		{
			Widget->RemoveFromParent();
		}
	}

	InventoryWidget->AddToViewport(10);
	bInventoryOpen = true;

	if (APawn* ControlledPawn = GetPawn())
	{
		if (UDS1InventoryComponent* InventoryComponent = ControlledPawn->FindComponentByClass<UDS1InventoryComponent>())
		{
			InventoryWidget->InitInventory(InventoryComponent);
		}
	}

	SetShowMouseCursor(true);
	FInputModeGameAndUI InputMode;
	InputMode.SetWidgetToFocus(InventoryWidget->TakeWidget());
	InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
	SetInputMode(InputMode);
}
