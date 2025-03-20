// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "ChasingTwilightPlayerState.generated.h"

/**
 * 
 */
UCLASS()
class CHASINGTWILIGHT_API AChasingTwilightPlayerState : public APlayerState
{
	GENERATED_UCLASS_BODY()
    UPROPERTY(Replicated)
    float Health;

};
