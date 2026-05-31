// Fill out your copyright notice in the Description page of Project Settings.

#include "Player/DS1CheatManager.h"
#include "Components/DS1InventoryComponent.h"
#include "Data/DS1ItemData.h"
#include "Data/DS1ItemDataRegistry.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"

void UDS1CheatManager::GiveItem(FName ItemID, int32 Count)
{
	APlayerController* PC = GetOuterAPlayerController();
	if (!PC) return;

	UGameInstance* GI = UGameplayStatics::GetGameInstance(GetWorld());
	UDS1ItemDataRegistry* Registry = GI ? GI->GetSubsystem<UDS1ItemDataRegistry>() : nullptr;
	if (!Registry)
	{
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("GiveItem: ItemDataRegistry 없음"));
		return;
	}

	UDS1ItemData* ItemData = Registry->FindItemDataByID(ItemID);
	if (!ItemData)
	{
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red,
			FString::Printf(TEXT("GiveItem: '%s' 아이템 없음"), *ItemID.ToString()));
		return;
	}

	APawn* Pawn = PC->GetPawn();
	UDS1InventoryComponent* InvComp = Pawn ? Pawn->FindComponentByClass<UDS1InventoryComponent>() : nullptr;
	if (!InvComp)
	{
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("GiveItem: InventoryComponent 없음"));
		return;
	}

	const int32 Added = InvComp->AddItem(ItemData, Count);
	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green,
		FString::Printf(TEXT("GiveItem: %s x%d 추가"), *ItemID.ToString(), Added));
}
