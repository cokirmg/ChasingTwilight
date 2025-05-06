// VehicleController.h
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "EnhancedInputSubsystems.h"
#include "InputMappingContext.h"
#include "VehicleController.generated.h"

UCLASS()
class CHASINGTWILIGHT_API AVehicleController : public APlayerController
{
    GENERATED_BODY()

public:
    AVehicleController();
    float MoveSpeed = 1000.0f;

protected:
    virtual void BeginPlay() override;


    /** Input Mapping Context to be used for player input */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input)
    UInputMappingContext* InputMappingContext;

public:
    //virtual void SetupInputComponent() override;

    // Métodos de entrada para controlar el vehículo
   // void MoveForward(float Value);
   // void MoveRight(float Value);
    //void Interact();

private:
    //class AMultiplayerVehiclePawn* VehiclePawn;
};
