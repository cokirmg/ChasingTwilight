#include "MultiplayerVehiclePawn.h"
#include "Components/BoxComponent.h"
#include "GameFramework/FloatingPawnMovement.h"
#include "Components/StaticMeshComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Net/UnrealNetwork.h"

#include "ChasingTwilightCharacter.h"
#include "InputActionValue.h"

AMultiplayerVehiclePawn::AMultiplayerVehiclePawn()
{
    PrimaryActorTick.bCanEverTick = true;

    Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
    RootComponent = Root;

    //RootComponent = VehicleMesh;
    VehicleMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("VehicleMesh"));
    VehicleMesh->SetupAttachment(Root);
    VehicleMesh->SetSimulatePhysics(false);

    InteractionArea = CreateDefaultSubobject<UBoxComponent>(TEXT("InteractionArea"));
    InteractionArea->SetupAttachment(VehicleMesh);

    bReplicates = true;
    SetReplicateMovement(true);
    CurrentController = nullptr;

    Aceleracion = 1000.f;
    MaxVelocidad = 1000.f;
    VelocidadActual = FVector::ZeroVector;
    DireccionGiro = 0.f;
}

void AMultiplayerVehiclePawn::BeginPlay()
{
    Super::BeginPlay();

    BaseMesh = VehicleMesh;

    InteractionArea->OnComponentBeginOverlap.AddDynamic(this, &AMultiplayerVehiclePawn::OnOverlap);
    InteractionArea->OnComponentEndOverlap.AddDynamic(this, &AMultiplayerVehiclePawn::EndOverlap);
}

void AMultiplayerVehiclePawn::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
   

    if (bAcelerando && VelocidadActual.Size() < MaxVelocidad)
    {
        VelocidadActual += -BaseMesh->GetForwardVector() * Aceleracion * DeltaTime;
    }

    if (VelocidadActual.Size() >= MaxVelocidad)
    {
        VelocidadActual *= Rozamiento;
    }

    if (VelocidadActual.Size() > 0)
    {
        if (FMath::Abs(DireccionGiro) > KINDA_SMALL_NUMBER)
        {
            float Giro = DireccionGiro * 60.f * DeltaTime;
            float EscalaPorVelocidad = FMath::Clamp(VelocidadActual.Size() / MaxVelocidad, 0.f, 1.f);
            Giro *= EscalaPorVelocidad;

            BaseMesh->AddWorldRotation(FRotator(0.f, Giro, 0.f));
          //  AddActorWorldRotation(FRotator(0.f, Giro, 0.f));
        }

        FVector Movimiento = VelocidadActual * DeltaTime;
       BaseMesh->AddWorldOffset(Movimiento, true);
        //AddActorWorldOffset(Movimiento, true);

        if (!bAcelerando)
        {
            VelocidadActual *= Rozamiento;
        }

        if (VelocidadActual.Size() < 10.f)
        {
            VelocidadActual = FVector::ZeroVector;
        }
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


    if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent))
    {
        EnhancedInputComponent->BindAction(AccelerateAction, ETriggerEvent::Triggered, this, &AMultiplayerVehiclePawn::MoveCheck);
        EnhancedInputComponent->BindAction(AccelerateAction, ETriggerEvent::Completed, this, &AMultiplayerVehiclePawn::StopMove);
        EnhancedInputComponent->BindAction(AccelerateAction, ETriggerEvent::Canceled, this, &AMultiplayerVehiclePawn::StopMove);
        EnhancedInputComponent->BindAction(Brake_ReverseAction, ETriggerEvent::Triggered, this, &AMultiplayerVehiclePawn::Brake_Reverse);

        EnhancedInputComponent->BindAction(GirarIzquierdaAction, ETriggerEvent::Triggered, this, &AMultiplayerVehiclePawn::GirarIzquierda);
        EnhancedInputComponent->BindAction(GirarIzquierdaAction, ETriggerEvent::Completed, this, &AMultiplayerVehiclePawn::DetenerGiro);
        EnhancedInputComponent->BindAction(GirarIzquierdaAction, ETriggerEvent::Canceled, this, &AMultiplayerVehiclePawn::DetenerGiro);

        EnhancedInputComponent->BindAction(GirarDerechaAction, ETriggerEvent::Triggered, this, &AMultiplayerVehiclePawn::GirarDerecha);
        EnhancedInputComponent->BindAction(GirarDerechaAction, ETriggerEvent::Completed, this, &AMultiplayerVehiclePawn::DetenerGiro);
        EnhancedInputComponent->BindAction(GirarDerechaAction, ETriggerEvent::Canceled, this, &AMultiplayerVehiclePawn::DetenerGiro);

        EnhancedInputComponent->BindAction(ExitVehicleAction, ETriggerEvent::Started, this, &AMultiplayerVehiclePawn::ReleaseControlCheck);
        EnhancedInputComponent->BindAction(PruebaAction, ETriggerEvent::Started, this, &AMultiplayerVehiclePawn::PruebaCheck);
    }



}


void AMultiplayerVehiclePawn::Prueba() {

}


void AMultiplayerVehiclePawn::PruebaCheck()
{
    UE_LOG(LogTemplateCharacter, Log, TEXT("Me voy a cagar en la puta madre qu epario a panete"));
    if (HasAuthority())
    {
        Prueba(); // si ya somos el servidor
    }
    else
    {
        ServerPrueba(); // si somos cliente, pedimos al servidor que lo haga
    }
}

void AMultiplayerVehiclePawn::DetenerGiro()
{
    DireccionGiro = 0.0f;
}

void AMultiplayerVehiclePawn::Brake_Reverse()
{
    FVector ForwardDir = -BaseMesh->GetForwardVector();
    float VelocidadAdelante = FVector::DotProduct(VelocidadActual, ForwardDir);

    if (VelocidadAdelante > 10.f)
    {
        VelocidadActual *= 0.8f;
    }
    else
    {
        VelocidadActual += ForwardDir * -Aceleracion * Aceleracion * GetWorld()->GetDeltaSeconds();

        float MaxVelocidadReversa = MaxVelocidad * 0.5f;
        if (VelocidadActual.Size() > MaxVelocidadReversa)
        {
            VelocidadActual = VelocidadActual.GetSafeNormal() * MaxVelocidadReversa;
        }
    }
}

void AMultiplayerVehiclePawn::Move()
{
    ForwardDir2 = -BaseMesh->GetForwardVector();
    VelocidadAdelante2 = FVector::DotProduct(VelocidadActual, ForwardDir2);


        VelocidadActual += -ForwardDir2 * -Aceleracion * Aceleracion * GetWorld()->GetDeltaSeconds();

        float MaxVelocidadReversa = MaxVelocidad;
        if (VelocidadActual.Size() > MaxVelocidadReversa)
        {
            VelocidadActual = VelocidadActual.GetSafeNormal() * MaxVelocidadReversa;
        }
    
}

void AMultiplayerVehiclePawn::MoveCheck() {
    UE_LOG(LogTemplateCharacter, Log, TEXT("MOOVE"));
    if (HasAuthority())
    {
        Move(); // si ya somos el servidor
    }
    else
    {
        ServerMove(); // si somos cliente, pedimos al servidor que lo haga
    }
}

void AMultiplayerVehiclePawn::ServerMove_Implementation() {
    Move();
}

void AMultiplayerVehiclePawn::StopMove()
{
    bAcelerando = false;
}

void AMultiplayerVehiclePawn::GirarIzquierda()
{
    DireccionGiro = -1.0f;
}

void AMultiplayerVehiclePawn::GirarDerecha()
{
    DireccionGiro = 1.0f;
}

void AMultiplayerVehiclePawn::ApplyBrake()
{
    VelocidadActual *= 0.5f;
}

void AMultiplayerVehiclePawn::OnOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{

    UE_LOG(LogTemplateCharacter, Log, TEXT("overlap"));
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

void AMultiplayerVehiclePawn::TakeControl(APlayerController* NewPlayerController)
{
    UE_LOG(LogTemplateCharacter, Log, TEXT("Take control"));
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
    UE_LOG(LogTemplateCharacter, Log, TEXT("release control"));
    if (HasAuthority())
    {
        ReleaseControl(); // si ya somos el servidor
    }
    else
    {
        ServerReleaseControl(); // si somos cliente, pedimos al servidor que lo haga
    }

}

void AMultiplayerVehiclePawn::ReleaseControl()
{
    UE_LOG(LogTemplateCharacter, Log, TEXT("Release control"));
    if (CurrentController && CachedPlayer)
    {
        CachedPlayer->SetActorHiddenInGame(false);
        CachedPlayer->SetActorEnableCollision(true);
        CachedPlayer->DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);

        FVector WorldPosition = GetTransform().TransformPosition(RelativePosition);
        FRotator WorldRotation = GetTransform().TransformRotation(RelativeRotation.Quaternion()).Rotator();

        CachedPlayer->SetActorLocation(WorldPosition);
        CachedPlayer->SetActorRotation(WorldRotation);
        CurrentController->UnPossess();
        CurrentController->Possess(CachedPlayer);


        //  Asegúrate de que el sistema de input vuelva a su estado original
        if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(CurrentController->GetLocalPlayer()))
        {


            if (VehicleMappingContext)
            {
                Subsystem->RemoveMappingContext(VehicleMappingContext); // Quitamos el del vehículo
                Subsystem->AddMappingContext(PlayerMappingContext, 0); // Agregamos el del personaje
            }
        }

    }

    CurrentController = nullptr;
    CachedPlayer = nullptr;
}

void AMultiplayerVehiclePawn::ServerReleaseControl_Implementation() {
    ReleaseControl();
}

void AMultiplayerVehiclePawn::ServerPrueba_Implementation() {
    Prueba();
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

    DOREPLIFETIME(AMultiplayerVehiclePawn, CurrentController);
    DOREPLIFETIME(AMultiplayerVehiclePawn, ForwardDir2);
    DOREPLIFETIME(AMultiplayerVehiclePawn, VelocidadAdelante2);
}

void AMultiplayerVehiclePawn::ServerTakeControl_Implementation(APlayerController* NewPlayerController)
{
    TakeControl(NewPlayerController);
}





