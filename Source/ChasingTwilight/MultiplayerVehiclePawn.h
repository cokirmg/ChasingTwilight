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

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
    UInputMappingContext* VehicleMappingContext;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
    UInputMappingContext* PlayerMappingContext;

protected:
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
    UInputAction* AccelerateAction;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
    UInputAction* Brake_ReverseAction;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
    UInputAction* GirarDerechaAction;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
    UInputAction* GirarIzquierdaAction;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
    UInputAction* FrenarAction;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
    UInputAction* ExitVehicleAction;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    USceneComponent* Root;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UStaticMeshComponent* VehicleMesh;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UBoxComponent* InteractionArea;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UStaticMeshComponent* BaseMesh;

    virtual void BeginPlay() override;

public:
    virtual void Tick(float DeltaTime) override;
    virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
    virtual void PossessedBy(AController* NewController) override;

    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

    void Move(const FInputActionValue& Value);
    void StopMove();
    void GirarIzquierda();
    void GirarDerecha();
    void DetenerGiro();
    void Brake_Reverse();
    void ApplyBrake();


    void TakeControl(APlayerController* NewController);

    UFUNCTION(BlueprintCallable, Category = "Vehicle")
    void ReleaseControl();


private:
    UFUNCTION()
    void OnOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp,
        int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
    
    UFUNCTION()
    void EndOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

    AChasingTwilightCharacter* CachedPlayer;
    APlayerController* CurrentController;

    FVector RelativePosition;
    FRotator RelativeRotation;

    UPROPERTY()
    UEnhancedInputLocalPlayerSubsystem* CachedInputSubsystem;

    //UPROPERTY(EditAnywhere, Category = "Movimiento")
    float Aceleracion = 5000.0f;

    UPROPERTY(EditAnywhere, Category = "Movimiento")
    float MaxVelocidad = 1000.0f;

    UPROPERTY(EditAnywhere, Category = "Movimiento")
    float Rozamiento = 0.90f;

    UPROPERTY(EditAnywhere, Category = "Movimiento")
    float VelocidadGiro = 60.0f;

    UPROPERTY(Replicated)
    FVector VelocidadActual;

    UPROPERTY(Replicated)
    float DireccionGiro;

    UPROPERTY(Replicated)
    bool bAcelerando;

    float VelocidadLineal = 0.f; // usás un float en vez de FVector
};
