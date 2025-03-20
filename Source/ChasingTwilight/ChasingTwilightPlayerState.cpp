// Fill out your copyright notice in the Description page of Project Settings.


#include "ChasingTwilightPlayerState.h"
#include "Net/UnrealNetwork.h"


AChasingTwilightPlayerState::AChasingTwilightPlayerState(const FObjectInitializer& ObjectInitializer)
: Super(ObjectInitializer)
{
	Health = 100.0f;
}

void AChasingTwilightPlayerState::GetLifetimeReplicatedProps
(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AChasingTwilightPlayerState, Health);
}