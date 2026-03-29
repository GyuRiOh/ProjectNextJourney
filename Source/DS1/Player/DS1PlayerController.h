// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "DS1PlayerController.generated.h"

class UDS1InventoryWidget;

UCLASS()
class DS1_API ADS1PlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	void ToggleInventory();

	FORCEINLINE bool IsInventoryOpen() const { return bInventoryOpen; }

protected:
	virtual void BeginPlay() override;

private:
	UPROPERTY()
	UDS1InventoryWidget* InventoryWidget = nullptr;

	bool bInventoryOpen = false;
};
