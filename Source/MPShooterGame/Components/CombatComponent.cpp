// Fill out your copyright notice in the Description page of Project Settings.


#include "CombatComponent.h"
#include "MPShooterGame/Weapon/WeaponBase.h"
#include "MPShooterGame/Character/PlayerCharacter.h"
#include "Engine/SkeletalMeshSocket.h"
#include "Components/SphereComponent.h"
#include "Net/UnrealNetwork.h"

// Sets default values for this component's properties
UCombatComponent::UCombatComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

// Called when the game starts
void UCombatComponent::BeginPlay()
{
	Super::BeginPlay();
}


// Called every frame
void UCombatComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

void UCombatComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UCombatComponent, equippedWeapon);
}

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
}

