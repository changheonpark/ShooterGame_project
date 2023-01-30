// Fill out your copyright notice in the Description page of Project Settings.


#include "ShooterPlayController.h"
#include "Blueprint/UserWidget.h"

AShooterPlayController::AShooterPlayController()
{


}


void AShooterPlayController::BeginPlay()
{
	Super::BeginPlay();

	//Check our HUDOverlayClass TSubclassOf variable
	if (HUDOverlayClass)
	{
		HUDOverlay = CreateWidget<UUserWidget>(this, HUDOverlayClass);
		if (HUDOverlay)
		{
			HUDOverlay->AddToViewport();
			HUDOverlay->SetVisibility(ESlateVisibility::Visible);
		}
	}
}