// Fill out your copyright notice in the Description page of Project Settings.

#include "PlayerCharacterAnimInstance.h"
#include "PlayerCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"

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

	bIsAccelerating = playerCharacter->GetCharacterMovement()->GetCurrentAcceleration().Size() > 0.f ? true : false;
}
