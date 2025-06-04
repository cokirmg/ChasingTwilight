#include "MultiplayerVehiclePawn.h"
#include "Components/BoxComponent.h"
#include "Components/StaticMeshComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Net/UnrealNetwork.h"
#include "ChasingTwilightCharacter.h"
#include "InputActionValue.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "ChasingTwilightPlayerState.h"


AMultiplayerVehiclePawn::AMultiplayerVehiclePawn()
{
    PrimaryActorTick.bCanEverTick = true;

    Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
    RootComponent = Root;

    VehicleMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("VehicleMesh"));
    VehicleMesh->SetupAttachment(Root);
    VehicleMesh->SetSimulatePhysics(false);

    InteractionArea = CreateDefaultSubobject<UBoxComponent>(TEXT("InteractionArea"));
    InteractionArea->SetupAttachment(VehicleMesh);

    bReplicates = true;
    SetReplicateMovement(true);

    AceleracionRate = 2.f;
    MaxVelocidad = 1000.f;
    MaxVelocidadReversa = 500.f;
    VelocidadActual = FVector::ZeroVector;
    DireccionGiro = 0.f;
    bAcelerando = false;
    bReversa = false;

    DepositArea = CreateDefaultSubobject<UBoxComponent>(TEXT("DepositoArea"));
    DepositArea->SetupAttachment(RootComponent);
    DepositArea->SetBoxExtent(FVector(100.f));
    DepositArea->SetCollisionProfileName(TEXT("Trigger"));

}

void AMultiplayerVehiclePawn::BeginPlay()
{
    Super::BeginPlay();

    BaseMesh = VehicleMesh;

    InteractionArea->OnComponentBeginOverlap.AddDynamic(this, &AMultiplayerVehiclePawn::OnOverlap);
    InteractionArea->OnComponentEndOverlap.AddDynamic(this, &AMultiplayerVehiclePawn::EndOverlap);
    DepositArea->OnComponentBeginOverlap.AddDynamic(this, &AMultiplayerVehiclePawn::OnDepositoOverlap);

}

void AMultiplayerVehiclePawn::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    UE_LOG(LogTemp, Warning, TEXT("Gasolina: %f"), GasolinaActual);
    const FVector Forward = -BaseMesh->GetForwardVector();
    float TargetSpeed = 0.f;

    if (bAcelerando && TieneCombustible())
    {
        TargetSpeed = MaxVelocidad;
        GasolinaActual -= ConsumoPorSegundo * DeltaTime;
        GasolinaActual = FMath::Clamp(GasolinaActual, 0.f, CapacidadMaxima);
    }
    else if (bReversa)
    {
        TargetSpeed = -MaxVelocidadReversa;
    }

    float CurrentSpeed = FVector::DotProduct(VelocidadActual, Forward);
    float InterpolatedSpeed = FMath::FInterpTo(CurrentSpeed, TargetSpeed, DeltaTime, AceleracionRate);
    VelocidadActual = Forward * InterpolatedSpeed;

    if (FMath::Abs(DireccionGiro) > KINDA_SMALL_NUMBER && FMath::Abs(InterpolatedSpeed) > 0.f)
    {
        float Giro = DireccionGiro * 60.f * DeltaTime;
        float EscalaPorVelocidad = FMath::Clamp(FMath::Abs(InterpolatedSpeed) / MaxVelocidad, 0.f, 1.f);
        Giro *= EscalaPorVelocidad;
        BaseMesh->AddWorldRotation(FRotator(0.f, Giro, 0.f));
    }

    if (FMath::Abs(InterpolatedSpeed) > 1.f)
    {
        BaseMesh->AddWorldOffset(VelocidadActual * DeltaTime, true);
    }
}

void AMultiplayerVehiclePawn::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
    Super::SetupPlayerInputComponent(PlayerInputComponent);

    if (const APlayerController* PC = Cast<APlayerController>(Controller))
    {
        if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PC->GetLocalPlayer()))
        {
            if (VehicleMappingContext)
            {
                Subsystem->AddMappingContext(VehicleMappingContext, 0);
            }
        }
    }

    if (UEnhancedInputComponent* EnhancedInput = Cast<UEnhancedInputComponent>(PlayerInputComponent))
    {
        EnhancedInput->BindAction(AccelerateAction, ETriggerEvent::Triggered, this, &AMultiplayerVehiclePawn::StartAccelerating);
        EnhancedInput->BindAction(AccelerateAction, ETriggerEvent::Completed, this, &AMultiplayerVehiclePawn::StopAccelerating);
        EnhancedInput->BindAction(AccelerateAction, ETriggerEvent::Canceled, this, &AMultiplayerVehiclePawn::StopAccelerating);

        EnhancedInput->BindAction(Brake_ReverseAction, ETriggerEvent::Triggered, this, &AMultiplayerVehiclePawn::StartReversing);
        EnhancedInput->BindAction(Brake_ReverseAction, ETriggerEvent::Completed, this, &AMultiplayerVehiclePawn::StopReversing);
        EnhancedInput->BindAction(Brake_ReverseAction, ETriggerEvent::Canceled, this, &AMultiplayerVehiclePawn::StopReversing);

        EnhancedInput->BindAction(GirarIzquierdaAction, ETriggerEvent::Triggered, this, &AMultiplayerVehiclePawn::GirarIzquierda);
        EnhancedInput->BindAction(GirarIzquierdaAction, ETriggerEvent::Completed, this, &AMultiplayerVehiclePawn::DetenerGiro);
        EnhancedInput->BindAction(GirarIzquierdaAction, ETriggerEvent::Canceled, this, &AMultiplayerVehiclePawn::DetenerGiro);

        EnhancedInput->BindAction(GirarDerechaAction, ETriggerEvent::Triggered, this, &AMultiplayerVehiclePawn::GirarDerecha);
        EnhancedInput->BindAction(GirarDerechaAction, ETriggerEvent::Completed, this, &AMultiplayerVehiclePawn::DetenerGiro);
        EnhancedInput->BindAction(GirarDerechaAction, ETriggerEvent::Canceled, this, &AMultiplayerVehiclePawn::DetenerGiro);

        EnhancedInput->BindAction(ExitVehicleAction, ETriggerEvent::Started, this, &AMultiplayerVehiclePawn::ReleaseControlCheck);
    }
}

void AMultiplayerVehiclePawn::StartAccelerating() {
    bAcelerando = true;
}

void AMultiplayerVehiclePawn::StopAccelerating() {
    bAcelerando = false;
}

void AMultiplayerVehiclePawn::StartReversing() {
    bReversa = true;
}

void AMultiplayerVehiclePawn::StopReversing() {
    bReversa = false;
}

void AMultiplayerVehiclePawn::GirarIzquierda() {
    DireccionGiro = -1.f;
}

void AMultiplayerVehiclePawn::GirarDerecha() {
    DireccionGiro = 1.f;
}

void AMultiplayerVehiclePawn::DetenerGiro() {
    DireccionGiro = 0.f;
}

void AMultiplayerVehiclePawn::OnOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
    AChasingTwilightCharacter* Player = Cast<AChasingTwilightCharacter>(OtherActor);
    if (Player)
    {
        Player->bInteractable = true;
    }

    APawn* OtherPawn = Cast<APawn>(OtherActor);
    if (OtherPawn && OtherPawn->GetController())
    {
        APlayerController* PlayerController = Cast<APlayerController>(OtherPawn->GetController());
        if (PlayerController)
        {
            CurrentController = PlayerController;
        }
    }
}

void AMultiplayerVehiclePawn::EndOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
    AChasingTwilightCharacter* Player = Cast<AChasingTwilightCharacter>(OtherActor);
    if (Player)
    {
        Player->bInteractable = false;
    }
}

void AMultiplayerVehiclePawn::OnDepositoOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
    UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
    AChasingTwilightCharacter* Player = Cast<AChasingTwilightCharacter>(OtherActor);
    if (!Player) return;

    AChasingTwilightPlayerState* PS = Player->GetPlayerState<AChasingTwilightPlayerState>();
    if (!PS || PS->Resource <= 0.f) return;

    float Cantidad = PS->Resource;
    float Espacio = CapacidadMaxima - GasolinaActual;
    float ARecargar = FMath::Min(Cantidad, Espacio);

    GasolinaActual += ARecargar;
    GasolinaActual = FMath::Clamp(GasolinaActual, 0.f, CapacidadMaxima);
    PS->Resource -= ARecargar;

    UE_LOG(LogTemp, Warning, TEXT("Recargado %f de agua. Gasolina ahora: %f"), ARecargar, GasolinaActual);
}


void AMultiplayerVehiclePawn::TakeControl(APlayerController* NewPlayerController)
{
    if (NewPlayerController)
    {
        CachedPlayer = Cast<AChasingTwilightCharacter>(NewPlayerController->GetPawn());

        if (CachedPlayer)
        {
            RelativePosition = GetTransform().InverseTransformPosition(CachedPlayer->GetActorLocation());
            RelativeRotation = GetTransform().InverseTransformRotation(CachedPlayer->GetActorRotation().Quaternion()).Rotator();

            CachedPlayer->SetActorHiddenInGame(true);
            CachedPlayer->SetActorEnableCollision(false);
            CachedPlayer->AttachToActor(this, FAttachmentTransformRules::KeepWorldTransform);
        }

        if (CurrentController)
        {
            CurrentController->UnPossess();
        }

        NewPlayerController->Possess(this);
        CurrentController = NewPlayerController;
    }
}

void AMultiplayerVehiclePawn::ReleaseControlCheck() {
    if (HasAuthority())
    {
        ReleaseControl();
    }
    else
    {
        ServerReleaseControl();
    }
}

void AMultiplayerVehiclePawn::ReleaseControl()
{
    if (CurrentController && CachedPlayer)
    {
        // Mostrar y habilitar al personaje
        CachedPlayer->SetActorHiddenInGame(false);
        CachedPlayer->SetActorEnableCollision(true);
        CachedPlayer->DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);

        FVector WorldPosition = GetTransform().TransformPosition(RelativePosition);
        FRotator WorldRotation = GetTransform().TransformRotation(RelativeRotation.Quaternion()).Rotator();

        CachedPlayer->SetActorLocation(WorldPosition);
        CachedPlayer->SetActorRotation(WorldRotation);

        // Volver a poseer al personaje
        CurrentController->UnPossess();
        CurrentController->Possess(CachedPlayer);

        if (CurrentController->IsLocalController())
        {
            CachedPlayer->EnableInput(CurrentController);
            CachedPlayer->Controller = CurrentController;

            if (UCharacterMovementComponent* MoveComp = CachedPlayer->GetCharacterMovement())
            {
                MoveComp->SetMovementMode(MOVE_Walking);
            }

            if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(CurrentController->GetLocalPlayer()))
            {
                Subsystem->ClearAllMappings();

                if (PlayerMappingContext)
                {
                    Subsystem->AddMappingContext(PlayerMappingContext, 0);
                }
            }

            CachedPlayer->Restart(); // esto reinicia el input correctamente
        }

        // 🔧 Esto es crucial
        CachedPlayer->EnableInput(CurrentController);
        if (UCharacterMovementComponent* MoveComp = CachedPlayer->GetCharacterMovement())
        {
            MoveComp->SetMovementMode(MOVE_Walking); // o el modo que uses normalmente
        }

        if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(CurrentController->GetLocalPlayer()))
        {
            if (VehicleMappingContext)
            {
                Subsystem->RemoveMappingContext(VehicleMappingContext);
                Subsystem->AddMappingContext(PlayerMappingContext, 0);
            }
        }
    }


    CurrentController = nullptr;
    CachedPlayer = nullptr;
}

void AMultiplayerVehiclePawn::ServerReleaseControl_Implementation() {
    ReleaseControl();
}

void AMultiplayerVehiclePawn::PossessedBy(AController* NewController)
{

    Super::PossessedBy(NewController);

    APlayerController* PlayerController = Cast<APlayerController>(NewController);
    if (PlayerController)
    {
        CachedInputSubsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer());

        if (CachedInputSubsystem && VehicleMappingContext)
        {
            CachedInputSubsystem->AddMappingContext(VehicleMappingContext, 0);
        }
    }
}

void AMultiplayerVehiclePawn::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME(AMultiplayerVehiclePawn, VelocidadActual);
    DOREPLIFETIME(AMultiplayerVehiclePawn, DireccionGiro);
    DOREPLIFETIME(AMultiplayerVehiclePawn, bAcelerando);
    DOREPLIFETIME(AMultiplayerVehiclePawn, bReversa);
    DOREPLIFETIME(AMultiplayerVehiclePawn, CurrentController);
    DOREPLIFETIME(AMultiplayerVehiclePawn, GasolinaActual);
}

void AMultiplayerVehiclePawn::ServerTakeControl_Implementation(APlayerController* NewPlayerController)
{
    TakeControl(NewPlayerController);
}
