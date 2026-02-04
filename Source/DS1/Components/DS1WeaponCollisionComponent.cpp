// Fill out your copyright notice in the Description page of Project Settings.


#include "Components/DS1WeaponCollisionComponent.h"

#include "Kismet/KismetSystemLibrary.h"

UDS1WeaponCollisionComponent::UDS1WeaponCollisionComponent()
{
	PrimaryComponentTick.bCanEverTick = true;

	TraceObjectTypes.Add(UEngineTypes::ConvertToObjectType(ECC_Pawn));
}

void UDS1WeaponCollisionComponent::BeginPlay()
{
	Super::BeginPlay();

	
}

void UDS1WeaponCollisionComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (bIsCollisionEnabled)
	{
		CollisionTrace();
	}
}

void UDS1WeaponCollisionComponent::TurnOnCollision()
{
	AlreadyHitActors.Empty();
	bIsCollisionEnabled = true;
}

void UDS1WeaponCollisionComponent::TurnOffCollision()
{
	bIsCollisionEnabled = false;
}

void UDS1WeaponCollisionComponent::SetWeaponMesh(UPrimitiveComponent* MeshComponent)
{
	WeaponMesh = MeshComponent;
}

void UDS1WeaponCollisionComponent::AddIgnoredActor(AActor* Actor)
{
	IgnoredActors.Add(Actor);
}

void UDS1WeaponCollisionComponent::RemoveIgnoredActor(AActor* Actor)
{
	IgnoredActors.Remove(Actor);
}

bool UDS1WeaponCollisionComponent::CanHitActor(AActor* Actor) const
{
	return AlreadyHitActors.Contains(Actor) == false;
}

void UDS1WeaponCollisionComponent::CollisionTrace()
{
    TArray<FHitResult> OutHits;

    const FVector Start = WeaponMesh->GetSocketLocation(TraceStartSocketName);
    const FVector End = WeaponMesh->GetSocketLocation(TraceEndSocketName);

    bool const bHit = UKismetSystemLibrary::SphereTraceMultiForObjects(
        GetOwner(),
        Start,
        End,
        TraceRadius,
        TraceObjectTypes,
        false,
        IgnoredActors,
        DrawDebugType,
        OutHits,
        true);

    if (bHit)
    {
        for (const FHitResult& Hit : OutHits)
        {
            AActor* HitActor = Hit.GetActor();

            if (HitActor == nullptr)
            {
                continue;
            }

            if (CanHitActor(HitActor))
            {
                AlreadyHitActors.Add(HitActor);

                // Call OnHitActor Broadcast
                if (OnHitActor.IsBound())
                {
                    OnHitActor.Broadcast(Hit);
                }
            }
        }
    }
}

