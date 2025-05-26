// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ArrowComponent.h"
#include "GameFramework/Actor.h"
#include "TwilightZone.generated.h"

UCLASS()
class CHASINGTWILIGHT_API ATwilightZone : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ATwilightZone();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	// Header file (TwilightZone.h)
	FVector StartLocation;
	FVector EndLocation;

	float Speed = 100.f; // Velocidad en unidades por segundo
	FVector Direction;
	bool bMoving = true;
	UPROPERTY(EditAnywhere, Category = "Zones")
	bool bDarkZone;
	UPROPERTY(EditAnywhere, Category = "Zones")
	bool bSunStorm;

	

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category="Zones")
	USceneComponent* TargetPoint;

};
