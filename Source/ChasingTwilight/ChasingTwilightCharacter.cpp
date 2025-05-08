// Copyright Epic Games, Inc. All Rights Reserved.

#include "ChasingTwilightCharacter.h"
#include "ChasingTwilightProjectile.h"
#include "Animation/AnimInstance.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputActionValue.h"
#include "Engine/LocalPlayer.h"
#include "ChasingTwilightPlayerState.h"
#include "GameFramework/DamageType.h"
#include "Engine/DamageEvents.h"
#include <Kismet/GameplayStatics.h>
#include "ChasingTwilight/AC_Pickable.h"


DEFINE_LOG_CATEGORY(LogTemplateCharacter);

//////////////////////////////////////////////////////////////////////////
// AChasingTwilightCharacter

AChasingTwilightCharacter::AChasingTwilightCharacter()
{
	// Character doesnt have a rifle at start
	bHasRifle = false;
    
	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(55.f, 96.0f);
        
	// Create a CameraComponent    
	FirstPersonCameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("FirstPersonCamera"));
	FirstPersonCameraComponent->SetupAttachment(GetCapsuleComponent());
	FirstPersonCameraComponent->SetRelativeLocation(FVector(-10.f, 0.f, 60.f)); // Position the camera
	FirstPersonCameraComponent->bUsePawnControlRotation = true;

	// Create a mesh component that will be used when being viewed from a '1st person' view (when controlling this pawn)
	Mesh1P = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("CharacterMesh1P"));
	Mesh1P->SetOnlyOwnerSee(true);
	Mesh1P->SetupAttachment(FirstPersonCameraComponent);
	Mesh1P->bCastDynamicShadow = false;
	Mesh1P->CastShadow = false;
	Mesh1P->SetRelativeLocation(FVector(-30.f, 0.f, -150.f));

	// Configurar el Mesh de ACharacter en lugar de usar Mesh3P
	GetMesh()->SetOwnerNoSee(true);  // Visible para los demás jugadores
	GetMesh()->SetOnlyOwnerSee(false);
	GetMesh()->SetupAttachment(GetCapsuleComponent()); // Mantenerlo en el CapsuleComponent

	// Configurar el arma en el Mesh de ACharacter
	TP_Gun = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("TP_Gun"));
	TP_Gun->SetOwnerNoSee(true); // Oculto para el dueño
	TP_Gun->SetupAttachment(GetMesh(), TEXT("hand_rSocket")); // Adjuntar al esqueleto de GetMesh()

	//Not seeing itself
	GetMesh()->SetOwnerNoSee(true);
}

void AChasingTwilightCharacter::BeginPlay()
{
	// Call the base class  
	Super::BeginPlay();

	// Add Input Mapping Context
	if (APlayerController* PlayerController = Cast<APlayerController>(Controller))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
		{
			Subsystem->AddMappingContext(DefaultMappingContext, 0);
		}
	}

	TArray<AActor*> FoundActors;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), AMultiplayerVehiclePawn::StaticClass(), FoundActors);
	Vehicle = Cast<AMultiplayerVehiclePawn>(FoundActors[0]);


}

//////////////////////////////////////////////////////////////////////////// Input

void AChasingTwilightCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	// Set up action bindings
	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		// Jumping
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Started, this, &ACharacter::Jump);
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &ACharacter::StopJumping);

		// Moving
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &AChasingTwilightCharacter::Move);

		// Looking
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &AChasingTwilightCharacter::Look);

		// Interacting
		EnhancedInputComponent->BindAction(InteractAction, ETriggerEvent::Started, this, &AChasingTwilightCharacter::Interact);
	}
	else
	{
		UE_LOG(LogTemplateCharacter, Error, TEXT("'%s' Failed to find an Enhanced Input Component! This template is built to use the Enhanced Input system. If you intend to use the legacy system, then you will need to update this C++ file."), *GetNameSafe(this));
	}
}


void AChasingTwilightCharacter::Move(const FInputActionValue& Value)
{
	// input is a Vector2D
	FVector2D MovementVector = Value.Get<FVector2D>();

	if (Controller != nullptr)
	{
		// add movement 
		AddMovementInput(GetActorForwardVector(), MovementVector.Y);
		AddMovementInput(GetActorRightVector(), MovementVector.X);
	}
}

void AChasingTwilightCharacter::Look(const FInputActionValue& Value)
{
	// input is a Vector2D
	FVector2D LookAxisVector = Value.Get<FVector2D>();

	if (Controller != nullptr)
	{
		// add yaw and pitch input to controller
		AddControllerYawInput(LookAxisVector.X);
		AddControllerPitchInput(LookAxisVector.Y);
	}
}

void AChasingTwilightCharacter::Interact(const FInputActionValue& Value)
{
	if (bInteractable && Vehicle)
	{
		APlayerController* PlayerController = Cast<APlayerController>(GetController());
		if (PlayerController)
		{
			Vehicle->TakeControl(PlayerController);
		}
	}
	else
	{
		UE_LOG(LogTemplateCharacter, Log, TEXT("No puedes interactuar con el vehículo"));
	}
}


void AChasingTwilightCharacter::SetHasRifle(bool bNewHasRifle)
{
	bHasRifle = bNewHasRifle;
}

bool AChasingTwilightCharacter::GetHasRifle()
{
	return bHasRifle;
}

float AChasingTwilightCharacter::TakeDamage(float Damage, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	Super::TakeDamage(Damage, DamageEvent, EventInstigator, DamageCauser);
	AChasingTwilightPlayerState* CTPlayerState = GetPlayerState<AChasingTwilightPlayerState>();
	CTPlayerState->Health -= Damage;

	return Damage;
}

void AChasingTwilightCharacter::Fire(const FVector pos, const FVector dir)
{
	DrawDebugLine(GetWorld(), pos, dir, FColor::Red, true, 100, 0, 5.0f);

	FCollisionObjectQueryParams ObjQuery;
	// Use defined channel. Look in "DefaultEngine.ini"
	ObjQuery.AddObjectTypesToQuery(ECC_GameTraceChannel1);
	FCollisionQueryParams ColQuery;
	ColQuery.AddIgnoredActor(this);

	FHitResult HitRes;
	GetWorld()->LineTraceSingleByObjectType(HitRes, pos, dir, ObjQuery, ColQuery);
	if (HitRes.bBlockingHit)
	{
		UE_LOG(LogTemp, Warning, TEXT("ACTOR %s"), *HitRes.GetActor()->GetName());
		AChasingTwilightCharacter* OtherChar = Cast<AChasingTwilightCharacter>(HitRes.GetActor());
		FDamageEvent thisEvent(UDamageType::StaticClass());
		OtherChar->TakeDamage(10.0f, thisEvent, GetController(), GetOwner());
	}

}

void AChasingTwilightCharacter::Pick(const FVector pos, const FVector dir)
{

	FCollisionObjectQueryParams ObjQuery;
	// Use defined channel. Look in "DefaultEngine.ini"
	ObjQuery.AddObjectTypesToQuery(ECC_GameTraceChannel1);
	FCollisionQueryParams ColQuery;
	ColQuery.AddIgnoredActor(this);

	FHitResult HitRes;
	GetWorld()->LineTraceSingleByObjectType(HitRes, pos, dir, ObjQuery, ColQuery);
	if (HitRes.bBlockingHit)
	{
		if (UAC_Pickable* comp = HitRes.GetActor()->FindComponentByClass<UAC_Pickable>())
		{
			UE_LOG(LogTemp, Warning, TEXT("Picking Shit"));
		}

		UE_LOG(LogTemp, Warning, TEXT("No Shit to Pick"));
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Nothing near for pick"));
	}

}

bool AChasingTwilightCharacter::ServerPick_Validate(const FVector pos, const FVector dir)
{
	if (pos != FVector(ForceInit) && dir != FVector(ForceInit))
	{
		return true;
	}
	else
	{
		return false;
	}
}

void AChasingTwilightCharacter::ServerPick_Implementation(const FVector pos, const FVector dir)
{
	Pick(pos, dir);
	//MultiCastShootEffects();
}

bool AChasingTwilightCharacter::ServerFire_Validate(const FVector pos, const FVector dir)
{
	if (pos != FVector(ForceInit) && dir != FVector(ForceInit))
	{
		return true;
	}
	else
	{
		return false;
	}
}

void AChasingTwilightCharacter::ServerFire_Implementation(const FVector pos, const FVector dir)
{
	Fire(pos, dir);
	MultiCastShootEffects();
}

void AChasingTwilightCharacter::MultiCastShootEffects_Implementation()
{
	// try and play a firing animation if specified
	if (TP_FireAnimation != nullptr)
	{
		// Get the animation object for the arms mesh
		//AChasingTwilightCharacter* character = Cast<AChasingTwilightCharacter>(Character);
		UAnimInstance* AnimInstance = GetMesh3P()->GetAnimInstance();
		if (AnimInstance != nullptr)
		{
			AnimInstance->Montage_Play(TP_FireAnimation, 1.f);
		}
	}
	// try and play the sound if specified
	/*if (FireSound != nullptr)
	{
		UGameplayStatics::PlaySoundAtLocation(this, FireSound, GetOwner()->GetActorLocation());
	}*/
	// TODO: Try to play particles for the shot and for the bullet
}