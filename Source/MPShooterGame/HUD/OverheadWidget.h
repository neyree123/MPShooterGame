// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "OverheadWidget.generated.h"

/**
 * 
 */
UCLASS()
class MPSHOOTERGAME_API UOverheadWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:
	UPROPERTY(meta = (BindWidget))
	class UTextBlock* DisplayText;

	void SetDisplayText(FString textToDesiplay);

	UFUNCTION(BlueprintCallable)
	void ShowPlayerNetRole(APawn* inPawn);

	UFUNCTION(BlueprintCallable)
	void ShowPlayerName(APawn* inPawn);

protected:
	virtual void NativeDestruct() override;
};
