// Fill out your copyright notice in the Description page of Project Settings.


#include "SessMenuBase.h"

void USessMenuBase::menuSetup()
{
	//adds widget to viewport
	AddToViewport();
	//set visible
	SetVisibility(ESlateVisibility::Visible);
	//allow set focus on it
	bIsFocusable = true;

	/*
	we want to set the input mode via PlayerController
	
	*/
	//so let's get world
	UWorld* world = GetWorld();
	if (world) {
		APlayerController* playerController = world->GetFirstPlayerController();
		if (playerController) {
			FInputModeUIOnly uiOnlyMode;
			uiOnlyMode.SetWidgetToFocus(TakeWidget());
			uiOnlyMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
			playerController->SetInputMode(uiOnlyMode);
			playerController->SetShowMouseCursor(true);

		}
		
	}

}
