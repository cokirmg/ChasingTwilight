// VehicleController.cpp
#include "VehicleController.h"
#include "MultiplayerVehiclePawn.h"

AVehicleController::AVehicleController()
{
    // Aquí puedes inicializar cosas específicas del controlador del vehículo
}

void AVehicleController::BeginPlay()
{
    Super::BeginPlay();

    // get the enhanced input subsystem
    if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer()))
    {
        // add the mapping context so we get controls
        Subsystem->AddMappingContext(InputMappingContext, 0);

        UE_LOG(LogTemp, Warning, TEXT("BeginPlay"));
    }

    // Obtener el vehículo controlado
    //VehiclePawn = Cast<AMultiplayerVehiclePawn>(GetPawn());
}

/*oid AVehicleController::SetupInputComponent()
{
    Super::SetupInputComponent();

    // Enlazar las entradas del vehículo
    InputComponent->BindAxis("MoveForward", this, &AVehicleController::MoveForward);
    InputComponent->BindAxis("MoveRight", this, &AVehicleController::MoveRight);
    InputComponent->BindAction("Interact", IE_Pressed, this, &AVehicleController::Interact);
}

void AVehicleController::MoveForward(float Value)
{
    if (Value != 0.0f)
    {
        APawn* ControlledPawn = GetPawn();
        if (ControlledPawn)
        {
            FVector ForwardDirection = ControlledPawn->GetActorForwardVector();
            // Mover el vehículo en esa dirección
            FVector Force = ForwardDirection * Value * MoveSpeed;
            ControlledPawn->AddMovementInput(ForwardDirection, Value);
        }
    }
}

void AVehicleController::MoveRight(float Value)
{
    if (VehiclePawn)
    {
        // Agregar entrada de movimiento lateral
        VehiclePawn->MoveRight(Value);
    }
}

void AVehicleController::Interact()
{
    if (VehiclePawn)
    {
        // Aquí puedes implementar la lógica de interacción (por ejemplo, saltar del vehículo)
    }
}*/
