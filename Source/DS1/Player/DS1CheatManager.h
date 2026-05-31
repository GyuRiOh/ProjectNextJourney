// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/CheatManager.h"
#include "DS1CheatManager.generated.h"

UCLASS()
class DS1_API UDS1CheatManager : public UCheatManager
{
	GENERATED_BODY()

public:
	/** 콘솔: GiveItem [ItemID] [Count] — 아이템을 인벤토리에 추가 */
	UFUNCTION(Exec)
	void GiveItem(FName ItemID, int32 Count = 1);
};
