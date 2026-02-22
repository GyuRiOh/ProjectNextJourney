// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "DS1ItemInstance.generated.h"

class UDS1ItemData;

/**
 * 인벤토리 슬롯 하나에 저장되는 아이템 인스턴스
 */
USTRUCT(BlueprintType)
struct FDS1ItemInstance
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UDS1ItemData* ItemData = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 StackCount = 0;

	bool IsValid() const { return ItemData != nullptr && StackCount > 0; }

	void Clear()
	{
		ItemData = nullptr;
		StackCount = 0;
	}
};
