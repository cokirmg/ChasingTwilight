#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "Components/BoxComponent.h"
#include "GameFramework/PlayerController.h"
#include "EnhancedInputSubsystems.h"
#include "InputMappingContext.h"

#include "MultiplayerVehiclePawn.generated.h"

class AChasingTwilightCharacter;
class UInputAction;

UCLASS()
class CHASINGTWILIGHT_API AMultiplayerVehiclePawn : public APawn
{
    GENERATED_BODY()

public:
    AMultiplayerVehiclePawn();

    // --- Mapping Contexts ---
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input)
    UInputMappingContext* VehicleMappingContext;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input)
    UInputMappingContext* PlayerMappingContext;

    // --- Control del vehículo ---
    UFUNCTION(BlueprintCallable, Category = "Vehicle")
    void TakeControl(APlayerController* NewController);


protected:
    // --- Input Actions ---
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input)
    UInputAction* AccelerateAction;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input)
    UInputAction* Brake_ReverseAction;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input)
    UInputAction* GirarDerechaAction;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input)
    UInputAction* GirarIzquierdaAction;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input)
    UInputAction* ExitVehicleAction;

    // --- Componentes ---
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    USceneComponent* Root;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UStaticMeshComponent* VehicleMesh;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UBoxComponent* InteractionArea;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UStaticMeshComponent* BaseMesh;

    // --- Movimiento ---
    UPROPERTY(EditAnywhere, Category = "Movimiento")
    float MaxVelocidad = 1000.0f;

    UPROPERTY(EditAnywhere, Category = "Movimiento")
    float MaxVelocidadReversa = 500.0f;

    UPROPERTY(EditAnywhere, Category = "Movimiento")
    float AceleracionRate = 2.0f; // velocidad con la que interpola

    UPROPERTY(EditAnywhere, Category = "Movimiento")
    float VelocidadGiro = 60.0f;

    // --- Estado actual ---
    UPROPERTY(Replicated)
    FVector VelocidadActual;

    UPROPERTY(Replicated)
    float DireccionGiro;

    UPROPERTY(Replicated)
    bool bAcelerando;

    UPROPERTY(Replicated)
    bool bReversa;

    // --- Control ---
    UPROPERTY(Replicated)
    APlayerController* CurrentController;

    AChasingTwilightCharacter* CachedPlayer;
    FVector RelativePosition;
    FRotator RelativeRotation;

    UPROPERTY()
    UEnhancedInputLocalPlayerSubsystem* CachedInputSubsystem;

    // --- Overrides ---
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;
    virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;
    virtual void PossessedBy(AController* NewController) override;
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

    // --- Input Handlers ---
    void StartAccelerating();
    void StopAccelerating();
    void StartReversing();
    void StopReversing();
    void GirarIzquierda();
    void GirarDerecha();
    void DetenerGiro();
    void ReleaseControlCheck();


    UFUNCTION(BlueprintCallable, Category = "Vehicle")
    void ReleaseControl();

    // --- Red ---
    UFUNCTION(Server, Reliable)
    void ServerTakeControl(APlayerController* NewPlayerController);

    UFUNCTION(Server, Reliable)
    void ServerReleaseControl();

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combustible")
    UBoxComponent* DepositArea;

    UPROPERTY(EditAnywhere, Replicated, BlueprintReadWrite, Category = "Combustible")
    float GasolinaActual = 100.0f;


    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combustible")
    float CapacidadMaxima = 100.0f;

    // Consumo por segundo al acelerar
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combustible")
    float ConsumoPorSegundo = 5.0f;

    // ¿Puede moverse actualmente?
    bool TieneCombustible() const { return GasolinaActual > 0.f; }


private:
    // --- Overlap con jugadores ---
    UFUNCTION()
    void OnOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp,
        int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

    UFUNCTION()
    void EndOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp,
        int32 OtherBodyIndex);

    void OnDepositoOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);


};
