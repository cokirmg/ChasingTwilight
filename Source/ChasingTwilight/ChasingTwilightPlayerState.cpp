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

float AChasingTwilightPlayerState::TakeDamage(float Damage, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	Super::TakeDamage(Damage, DamageEvent, EventInstigator, DamageCauser);
	AChasingTwilightPlayerState* CTPlayerState = Cast<AChasingTwilightPlayerState>(this);
	if (GetLocalRole() == ROLE_Authority && DamageCauser != this &&
		CTPlayerState && CTPlayerState->Health > 0.f)
	{
		CTPlayerState->Health -= Damage;
	}
	return Damage;
}
