// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "CT_HUD.generated.h"

/**
 * 
 */
UCLASS()
class CHASINGTWILIGHT_API ACT_HUD : public AHUD
{
	GENERATED_BODY()


	
public:
	virtual void DrawHUD() override;

};
