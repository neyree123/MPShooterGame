// Fill out your copyright notice in the Description page of Project Settings.

#include "PlayerCharacterAnimInstance.h"
#include "PlayerCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "MPShooterGame/Weapon/WeaponBase.h"

void UPlayerCharacterAnimInstance::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();

	playerCharacter = Cast<APlayerCharacter>(TryGetPawnOwner());
}

void UPlayerCharacterAnimInstance::NativeUpdateAnimation(float deltaTime)
{
	Super::NativeUpdateAnimation(deltaTime);

	if (playerCharacter == nullptr)
	{
		playerCharacter = Cast<APlayerCharacter>(TryGetPawnOwner());
	}

	if (playerCharacter == nullptr)
	{
		return;
	}

	FVector velocity = playerCharacter->GetVelocity();
	velocity.Z = 0.0f;
	speed = velocity.Size();

	//Movement variables
	bIsInAir = playerCharacter->GetCharacterMovement()->IsFalling();
	bIsAccelerating = playerCharacter->GetCharacterMovement()->GetCurrentAcceleration().Size() > 0.1f ? true : false;
	bIsCrouched = playerCharacter->bIsCrouched;
	turnInPlaceState = playerCharacter->GetTurnInPlaceState();

	//Weapon equipped variables
	bWeaponEquipped = playerCharacter->IsWeaponEquipped();
	equippedWeapon = playerCharacter->GetEquippedWeapon();
	bAiming = playerCharacter->IsAiming();

	//Offset yaw for strafing
	FRotator aimRotation = playerCharacter->GetBaseAimRotation();
	FRotator movementRotation = UKismetMathLibrary::MakeRotFromX(velocity);
	FRotator deltaRot = UKismetMathLibrary::NormalizedDeltaRotator(movementRotation, aimRotation);
	deltaRotation = FMath::RInterpTo(deltaRot, deltaRotation, deltaTime, 6.f);
	yawOffset = deltaRotation.Yaw;

	//Set lean value
	playerRotationLastFrame = playerRotation;
	playerRotation = playerCharacter->GetActorRotation();
	const FRotator delta = UKismetMathLibrary::NormalizedDeltaRotator(playerRotation, playerRotationLastFrame);
	const float target = delta.Yaw / deltaTime;
	const float Interp = FMath::FInterpTo(lean, target, deltaTime, 6.f);
	lean = FMath::Clamp(Interp, -90.f, 90.f);

	//Set aim offsets
	AO_Yaw = playerCharacter->GetAO_Yaw();
	AO_Pitch = playerCharacter->GetAO_Pitch();

	//Attach player's hand to the relevant socket if they have a weapon equipped
	if (bWeaponEquipped && equippedWeapon && equippedWeapon->GetWeaponMesh() && playerCharacter->GetMesh())
	{
		leftHandTransform = equippedWeapon->GetWeaponMesh()->GetSocketTransform(FName("LeftHandSocket"), ERelativeTransformSpace::RTS_World);


		FVector outPos;
		FRotator outRot;
		playerCharacter->GetMesh()->TransformToBoneSpace(FName("hand_r"), leftHandTransform.GetLocation(), FRotator::ZeroRotator, outPos, outRot);

		leftHandTransform.SetLocation(outPos);
		leftHandTransform.SetRotation(FQuat(outRot));
	}
}
