// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "DS1PlayerController.generated.h"

class UDS1InventoryWidget;

/**
 *
 */
UCLASS()
class DS1_API ADS1PlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	/** 인벤토리 UI 토글 */
	void ToggleInventory();

	FORCEINLINE bool IsInventoryOpen() const { return bInventoryOpen; }

protected:
	virtual void BeginPlay() override;

protected:
	/** 인벤토리 위젯 클래스 (블루프린트에서 할당) */
	UPROPERTY(EditAnywhere, Category = "UI")
	TSubclassOf<UDS1InventoryWidget> InventoryWidgetClass;

private:
	UPROPERTY()
	UDS1InventoryWidget* InventoryWidget = nullptr;

	bool bInventoryOpen = false;
};
