// Fill out your copyright notice in the Description page of Project Settings.


#include "WeaponBase.h"
#include "Components/SphereComponent.h"
#include "Components/WidgetComponent.h"
#include "MPShooterGame/Character/PlayerCharacter.h"
#include "Net/UnrealNetwork.h"

// Sets default values
AWeaponBase::AWeaponBase()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true; //Needed for weapons to be handled via the server

	weaponMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("WeaponMesh"));
	weaponMesh->SetupAttachment(RootComponent);
	SetRootComponent(weaponMesh);

	weaponMesh->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Block);
	weaponMesh->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Ignore);
	weaponMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	areaSphere = CreateDefaultSubobject<USphereComponent>(TEXT("areaSphere"));
	areaSphere->SetupAttachment(RootComponent);
	areaSphere->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	areaSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	pickupWidget = CreateDefaultSubobject<UWidgetComponent>(TEXT("PickupWidget"));
	pickupWidget->SetupAttachment(RootComponent);
	ShowPickupWidget(false);
}

// Called when the game starts or when spawned
void AWeaponBase::BeginPlay()
{
	Super::BeginPlay();

	//Only enable collison on the server
	if (HasAuthority())
	{
		areaSphere->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		areaSphere->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Overlap);
		areaSphere->OnComponentBeginOverlap.AddDynamic(this, &AWeaponBase::OnSphereOverlap);
		areaSphere->OnComponentEndOverlap.AddDynamic(this, &AWeaponBase::OnEndSphereOverlap);
	}
}

// Called every frame
void AWeaponBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void AWeaponBase::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AWeaponBase, weaponState);
}

void AWeaponBase::OnSphereOverlap(UPrimitiveComponent* overlappedComponet, AActor* otherActor, UPrimitiveComponent* otherComp, int32 otherBodyIndex, bool bFromSweep, const FHitResult& sweepResult)
{
	APlayerCharacter* player = Cast<APlayerCharacter>(otherActor);
	if (player)
	{
		player->SetOverlappingWeapon(this);
	}
}

void AWeaponBase::OnEndSphereOverlap(UPrimitiveComponent* overlappedComponet, AActor* otherActor, UPrimitiveComponent* otherComp, int32 otherBodyIndex)
{
	APlayerCharacter* player = Cast<APlayerCharacter>(otherActor);
	if (player)
	{
		player->SetOverlappingWeapon(nullptr);
	}
}

void AWeaponBase::SetWeaponState(EWeaponState state)
{
	weaponState = state;

	switch (weaponState)
	{
	case EWeaponState::EWS_Initial:
		break;
	case EWeaponState::EWS_Equipped:	
		ShowPickupWidget(false);
		GetAreaSphere()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		break;
	case EWeaponState::EWS_Dropped:
		break;
	case EWeaponState::EWS_MAX:
		break;
	default:
		break;
	}
}

void AWeaponBase::OnRep_WeaponState()
{
	switch (weaponState)
	{
	case EWeaponState::EWS_Initial:
		break;
	case EWeaponState::EWS_Equipped:
		ShowPickupWidget(false);
		areaSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		break;
	case EWeaponState::EWS_Dropped:
		break;
	case EWeaponState::EWS_MAX:
		break;
	default:
		break;
	}
}

void AWeaponBase::ShowPickupWidget(bool bShowWidget)
{
	if (pickupWidget)
	{
		pickupWidget->SetVisibility(bShowWidget);
	}
}
