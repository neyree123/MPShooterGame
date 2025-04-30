// Fill out your copyright notice in the Description page of Project Settings.


#include "PlayerCharacter.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/WidgetComponent.h"
#include "Net/UnrealNetwork.h"
#include "MPShooterGame/Weapon/WeaponBase.h"
#include "MPShooterGame/Components/CombatComponent.h"

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
}

#pragma region PlayerInput

// Called to bind functionality to input
void APlayerCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &ACharacter::Jump);
	PlayerInputComponent->BindAction("Equip", IE_Pressed, this, &APlayerCharacter::EquipButtonPressed);
	PlayerInputComponent->BindAction("Crouch", IE_Pressed, this, &APlayerCharacter::CrouchButtonPressed);

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
#pragma endregion

#pragma region WeaponFunctions

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
#pragma endregion

