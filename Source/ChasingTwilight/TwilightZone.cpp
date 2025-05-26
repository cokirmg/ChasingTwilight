// Fill out your copyright notice in the Description page of Project Settings.


#include "TwilightZone.h"

#include "Components/ArrowComponent.h"
#include "Components/LocalHeightFogComponent.h"

// Sets default values
ATwilightZone::ATwilightZone()
{
	PrimaryActorTick.bCanEverTick = true;
}

// Called when the game starts or when spawned
void ATwilightZone::BeginPlay()
{
	Super::BeginPlay();

	// Buscar el componente Arrow dentro del Blueprint
	TargetPoint = FindComponentByClass<UArrowComponent>();

	if (!TargetPoint)
	{
		UE_LOG(LogTemp, Error, TEXT("TargetPoint no encontrado. Asegúrate de tener un UArrowComponent en el Blueprint."));
		return;
	}

	ULocalHeightFogComponent* FogComp = FindComponentByClass<ULocalHeightFogComponent>();
	if (FogComp)
	{
		if (bDarkZone)
		{
			FogComp->FogDensity = 1.3f;
			FogComp->FogHeightFalloff = 40.2f;
			FogComp->FogHeightOffset = 1.9f;
			FogComp->FogRadialAttenuation = 0.f;
			FogComp->FogAlbedo = FLinearColor(0.0f, 0.08f, 0.48f);
			FogComp->FogEmissive = FLinearColor(0.05f, 0.f, 0.26f);
			FogComp->MarkRenderStateDirty();
			UE_LOG(LogTemp, Warning, TEXT("Dark zone changed"));
		}
		else if (bSunStorm)
		{
			FogComp->FogDensity = 1.3f;
			FogComp->FogHeightFalloff = 40.2f;
			FogComp->FogHeightOffset = 1.9f;
			FogComp->FogRadialAttenuation = 0.f;
			FogComp->FogAlbedo = FLinearColor(0.49f, 0.24f, 0.02f);
			FogComp->FogEmissive = FLinearColor(0.25f, 0.13f, 0.01f);
			FogComp->MarkRenderStateDirty();
			UE_LOG(LogTemp, Warning, TEXT("SunStorm changed"));
		}
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("FogComp is null!"));
	}

	// Solo continuamos si TargetPoint es válido
	StartLocation = GetActorLocation();
	EndLocation = TargetPoint->GetComponentLocation();

	DrawDebugLine(GetWorld(), StartLocation, EndLocation, FColor::Red, false, 2.0f, 0, 5.0f);

	Direction = (EndLocation - StartLocation).GetSafeNormal(); // Dirección normalizada
}

// Called every frame
void ATwilightZone::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// Seguridad por si TargetPoint se pierde en tiempo de ejecución
	if (!TargetPoint)
	{
		return;
	}

	FVector CurrentLocation = GetActorLocation();
	FVector NewLocation = CurrentLocation + Direction * Speed * DeltaTime;

	if (FVector::Dist(NewLocation, StartLocation) >= FVector::Dist(EndLocation, StartLocation))
	{
		SetActorLocation(StartLocation); // Teletransportar al inicio
	}
	else
	{
		SetActorLocation(NewLocation); // Mover hacia adelante
	}
}
