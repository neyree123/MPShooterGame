// Fill out your copyright notice in the Description page of Project Settings.

#include "PlayerCharacterAnimInstance.h"
#include "PlayerCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetMathLibrary.h"

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

	bIsInAir = playerCharacter->GetCharacterMovement()->IsFalling();
	bIsAccelerating = playerCharacter->GetCharacterMovement()->GetCurrentAcceleration().Size() > 0.1f ? true : false;
	bIsCrouched = playerCharacter->bIsCrouched;

	bWeaponEquipped = playerCharacter->IsWeaponEquipped();
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
}
