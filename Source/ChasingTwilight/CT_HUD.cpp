// Fill out your copyright notice in the Description page of Project Settings.


#include "CT_HUD.h"
#include "Kismet/GameplayStatics.h"
#include "ChasingTwilightPlayerState.h"
#include "ChasingTwilightCharacter.h"

void ACT_HUD::DrawHUD()
{
	
	AChasingTwilightCharacter* ThisChar =Cast<AChasingTwilightCharacter>(UGameplayStatics::GetPlayerPawn(GetWorld(), 0));
	AChasingTwilightPlayerState* PlayerState = ThisChar ? Cast<AChasingTwilightPlayerState>(ThisChar->GetPlayerState()) : nullptr;
	if (PlayerState)
	{
		FString HUDHealth = FString::Printf(TEXT("Health: %f"), PlayerState->Health);
		DrawText(HUDHealth, FColor::Yellow, 50, 50);
		FString HUDResource = FString::Printf(TEXT("Resource: %f"), PlayerState->Resource);
		DrawText(HUDResource, FColor::Blue, 50, 75);
	}
}