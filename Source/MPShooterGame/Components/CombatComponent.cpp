// Fill out your copyright notice in the Description page of Project Settings.


#include "CombatComponent.h"
#include "MPShooterGame/Weapon/WeaponBase.h"
#include "MPShooterGame/Character/PlayerCharacter.h"
#include "Engine/SkeletalMeshSocket.h"
#include "Components/SphereComponent.h"
#include "Net/UnrealNetwork.h"
#include "GameFramework/CharacterMovementComponent.h"

// Sets default values for this component's properties
UCombatComponent::UCombatComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UCombatComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UCombatComponent, equippedWeapon);
	DOREPLIFETIME(UCombatComponent, bAiming);
}

// Called when the game starts
void UCombatComponent::BeginPlay()
{
	Super::BeginPlay();

	if (player)
	{
		player->GetCharacterMovement()->MaxWalkSpeed = baseWalkSpeed;
	}
}

// Called every frame
void UCombatComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}
#pragma region Aiming

void UCombatComponent::SetAiming(bool bIsAiming)
{
	bAiming = bIsAiming;
	ServerSetAiming(bIsAiming);

	if (player)
	{
		player->GetCharacterMovement()->MaxWalkSpeed = bIsAiming ? aimWalkSpeed : baseWalkSpeed;
	}
}

void UCombatComponent::ServerSetAiming_Implementation(bool bIsAiming)
{
	bAiming = bIsAiming;

	if (player)
	{
		player->GetCharacterMovement()->MaxWalkSpeed = bIsAiming ? aimWalkSpeed : baseWalkSpeed;
	}
}
#pragma endregion

#pragma region EquippingWeapons

void UCombatComponent::EquipWeapon(AWeaponBase* weaponToEquip)
{
	if (player == nullptr || weaponToEquip == nullptr) return;

	equippedWeapon = weaponToEquip;
	equippedWeapon->SetWeaponState(EWeaponState::EWS_Equipped);
	const USkeletalMeshSocket* handSocket = player->GetMesh()->GetSocketByName(FName("rightHandSocket"));
	if (handSocket)
	{
		handSocket->AttachActor(equippedWeapon, player->GetMesh());
	}

	equippedWeapon->SetOwner(player);
	player->GetCharacterMovement()->bOrientRotationToMovement = false;
	player->bUseControllerRotationYaw = true;
}

void UCombatComponent::OnRep_EquippedWeapon()
{
	if (equippedWeapon && player)
	{
		player->GetCharacterMovement()->bOrientRotationToMovement = false;
		player->bUseControllerRotationYaw = true;
	}
}
#pragma endregion