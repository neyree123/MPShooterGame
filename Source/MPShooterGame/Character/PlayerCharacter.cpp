// Fill out your copyright notice in the Description page of Project Settings.


#include "PlayerCharacter.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/WidgetComponent.h"
#include "Net/UnrealNetwork.h"
#include "MPShooterGame/Weapon/WeaponBase.h"
#include "MPShooterGame/Components/CombatComponent.h"
#include "Components/CapsuleComponent.h"
#include "Kismet/KismetMathLibrary.h"

//Constructor
APlayerCharacter::APlayerCharacter()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	cameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	cameraBoom->SetupAttachment(GetMesh());
	cameraBoom->TargetArmLength = 600.f;
	cameraBoom->bUsePawnControlRotation = true;

	followCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	followCamera->SetupAttachment(cameraBoom, USpringArmComponent::SocketName);
	followCamera->bUsePawnControlRotation = false;

	bUseControllerRotationYaw = false;
	GetCharacterMovement()->bOrientRotationToMovement = true;

	OverheadWidget = CreateDefaultSubobject<UWidgetComponent>(TEXT("OverheadWidget"));
	OverheadWidget->SetupAttachment(RootComponent);

	combat = CreateDefaultSubobject<UCombatComponent>((TEXT("CombatComponent")));
	combat->SetIsReplicated(true);

	GetCharacterMovement()->NavAgentProps.bCanCrouch = true;
	GetCapsuleComponent()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);
	GetMesh()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);
	GetCharacterMovement()->RotationRate = FRotator(0.f, 0.f, 500.f);

	turnInPlaceState = ETurningInPlace::ETIP_NotTurning;
	NetUpdateFrequency = 66.f;
	MinNetUpdateFrequency = 33.f;
}

void APlayerCharacter::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	if (combat)
	{
		combat->player = this;
	}
}

void APlayerCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION(APlayerCharacter, overlappingWeapon, COND_OwnerOnly);
}

// Called when the game starts or when spawned
void APlayerCharacter::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void APlayerCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	AimOffset(DeltaTime);
}

#pragma region PlayerInput

// Called to bind functionality to input
void APlayerCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &APlayerCharacter::Jump);
	PlayerInputComponent->BindAction("Equip", IE_Pressed, this, &APlayerCharacter::EquipButtonPressed);
	PlayerInputComponent->BindAction("Crouch", IE_Pressed, this, &APlayerCharacter::CrouchButtonPressed);
	PlayerInputComponent->BindAction("Aim", IE_Pressed, this, &APlayerCharacter::AimButtonPressed);
	PlayerInputComponent->BindAction("Aim", IE_Released, this, &APlayerCharacter::AimButtonReleased);

	PlayerInputComponent->BindAxis("MoveFoward", this, &APlayerCharacter::MoveFoward);
	PlayerInputComponent->BindAxis("MoveRight", this, &APlayerCharacter::MoveRight);
	PlayerInputComponent->BindAxis("Turn", this, &APlayerCharacter::Turn);
	PlayerInputComponent->BindAxis("LookUp", this, &APlayerCharacter::LookUp);
}

void APlayerCharacter::MoveFoward(float value)
{
	if (Controller != nullptr && value != 0.f)
	{
		const FRotator yawRotation(0.f, Controller->GetControlRotation().Yaw, 0.f);
		const FVector direction(FRotationMatrix(yawRotation).GetUnitAxis(EAxis::X));

		AddMovementInput(direction, value);
	}
}

void APlayerCharacter::MoveRight(float value)
{
	if (Controller != nullptr && value != 0.f)
	{
		const FRotator yawRotation(0.f, Controller->GetControlRotation().Yaw, 0.f);
		const FVector direction(FRotationMatrix(yawRotation).GetUnitAxis(EAxis::Y));

		AddMovementInput(direction, value);
	}
}

void APlayerCharacter::Turn(float value)
{
	AddControllerYawInput(value);
}

void APlayerCharacter::LookUp(float value)
{
	AddControllerPitchInput(value);
}

void APlayerCharacter::Jump()
{
	if (bIsCrouched)
	{
		UnCrouch();
	}
	else
	{
		Super::Jump();
	}
}

void APlayerCharacter::EquipButtonPressed()
{
	if (combat)
	{
		if (HasAuthority())
		{
			combat->EquipWeapon(overlappingWeapon);
		}
		else
		{
			ServerEquippedButtonPressed();
		}
	}
}

void APlayerCharacter::CrouchButtonPressed()
{
	if ( bIsCrouched )
	{
		UnCrouch();
	}
	else
	{
		Crouch(); //Crouch is already replicated by Unreal. Can override implementation if needed.
	}
}

void APlayerCharacter::AimButtonPressed()
{
	if (combat)
	{
		combat->SetAiming(true);
	}
}

void APlayerCharacter::AimButtonReleased()
{
	if (combat)
	{
		combat->SetAiming(false);
	}
}
void APlayerCharacter::AimOffset(float deltaTime)
{
	//Only apply if player has a weapon
	if (combat && combat->equippedWeapon == nullptr) return;

	FVector velocity = GetVelocity();
	velocity.Z = 0.0f;
	float speed = velocity.Size();
	bool bIsInAir = GetCharacterMovement()->IsFalling();

	//Only apply yaw aim offsets if player is not moving or in air
	if (speed == 0.f && !bIsInAir)
	{
		FRotator currentAimRotation = FRotator(0.f, GetBaseAimRotation().Yaw, 0.f);
		FRotator deltaAimRotation = UKismetMathLibrary::NormalizedDeltaRotator(currentAimRotation, startingAimRotation);
		AO_Yaw = deltaAimRotation.Yaw;

		if (turnInPlaceState == ETurningInPlace::ETIP_NotTurning)
		{
			interpAO_Yaw = AO_Yaw;
		}
		bUseControllerRotationYaw = true;

		TurnInPlace(deltaTime);
	}
	//Otherwise track the starting aim rotation
	if (speed > 0.f || bIsInAir)
	{
		startingAimRotation = FRotator(0.f, GetBaseAimRotation().Yaw, 0.f);
		AO_Yaw = 0.f;
		bUseControllerRotationYaw = true;
		turnInPlaceState = ETurningInPlace::ETIP_NotTurning;
	}

	AO_Pitch = GetBaseAimRotation().Pitch;
	//Due to the way Unreal compresses rotators for network communication, we lose our negative values here
	//This check corrects those issues
	if (AO_Pitch > 90.f && !IsLocallyControlled())
	{
		//Map pitch from [270,360] to [-90,0]
		FVector2D inRange(270.f, 360.f);
		FVector2D outRange(-90.f, 0.f);
		AO_Pitch = FMath::GetMappedRangeValueClamped(inRange, outRange, AO_Pitch);
	}
}

void APlayerCharacter::TurnInPlace(float deltaTime)
{
	if (AO_Yaw > 90.f)
	{
		turnInPlaceState = ETurningInPlace::ETIP_Right;
	}
	else if (AO_Yaw < -90.f)
	{
		turnInPlaceState = ETurningInPlace::ETIP_Left;
	}

	if (turnInPlaceState != ETurningInPlace::ETIP_NotTurning)
	{
		interpAO_Yaw = FMath::FInterpTo(interpAO_Yaw, 0.f, deltaTime, 4.f);
		AO_Yaw = interpAO_Yaw;
		if (FMath::Abs(AO_Yaw) < 15.f)
		{
			turnInPlaceState = ETurningInPlace::ETIP_NotTurning;
			startingAimRotation = FRotator(0.f, GetBaseAimRotation().Yaw, 0.f);
		}
	}
}
#pragma endregion

#pragma region WeaponFunctions

AWeaponBase* APlayerCharacter::GetEquippedWeapon()
{
	if(combat == nullptr) return nullptr;

	return combat->equippedWeapon;
}

void APlayerCharacter::ServerEquippedButtonPressed_Implementation() //RPC implmentation for equipping a weapon
{
	if (combat)
	{
		combat->EquipWeapon(overlappingWeapon);
	}
}

void APlayerCharacter::SetOverlappingWeapon(AWeaponBase* weapon) //Only called on the server
{
	//Hide previous overlapping weapon
	if (overlappingWeapon)
	{
		overlappingWeapon->ShowPickupWidget(false);
	}

	overlappingWeapon = weapon;

	//Set the widget for the server player
	if (IsLocallyControlled())
	{
		if (overlappingWeapon)
		{
			overlappingWeapon->ShowPickupWidget(true);
		}
	}
}

//Used to show weapon pickup widget whenever a client is near it
void APlayerCharacter::OnRep_OverlappingWeapon(AWeaponBase* lastWeapon)
{
	if (overlappingWeapon)
	{
		overlappingWeapon->ShowPickupWidget(true);
	}

	//Hide previous overlapping weapon
	if (lastWeapon)
	{
		lastWeapon->ShowPickupWidget(false);
	}
}

bool APlayerCharacter::IsWeaponEquipped()
{
	return(combat && combat->equippedWeapon);
}

bool APlayerCharacter::IsAiming()
{
	return (combat && combat->bAiming);
}
#pragma endregion

