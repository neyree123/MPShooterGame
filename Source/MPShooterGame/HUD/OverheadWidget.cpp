// Fill out your copyright notice in the Description page of Project Settings.


#include "OverheadWidget.h"
#include "Components/TextBlock.h"

void UOverheadWidget::SetDisplayText(FString textToDesiplay)
{
	if (DisplayText)
	{
		DisplayText->SetText(FText::FromString(textToDesiplay));
	}
}

void UOverheadWidget::ShowPlayerNetRole(APawn* inPawn)
{
	ENetRole localRole = inPawn->GetLocalRole();
	FString role;

	switch (localRole)
	{
	case ROLE_SimulatedProxy:
		role = "Simulated Proxy";
		break;
	case ROLE_AutonomousProxy:
		role = "Autonomous Proxy";
		break;
	case ROLE_Authority:
		role = "Authority";
		break;
	case ROLE_None:
	default:
		role = "None";
		break;
	}
	FString localRoleString = FString::Printf(TEXT("Local Role: %s"), *role);

	SetDisplayText(localRoleString);
}

void UOverheadWidget::ShowPlayerName(APawn* inPawn)
{
	
}

void UOverheadWidget::NativeDestruct()
{
	RemoveFromParent();
	Super::NativeDestruct();
}
