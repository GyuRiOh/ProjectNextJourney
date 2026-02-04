// Fill out your copyright notice in the Description page of Project Settings.


#include "Items/DS1Potion.h"

ADS1Potion::ADS1Potion()
{
	PrimaryActorTick.bCanEverTick = false;

	Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("PotionMesh"));
	SetRootComponent(Mesh);
	Mesh->SetCollisionProfileName(UCollisionProfile::NoCollision_ProfileName);
}


