// Fill out your copyright notice in the Description page of Project Settings.


#include "SessMenuBase.h"
#include "Components/Button.h"

void USessMenuBase::hostButtonClicked()
{
	if (GEngine) {
		GEngine->AddOnScreenDebugMessage(
			-1,
			15.f,
			FColor::Yellow,
			FString(TEXT("hosting..."))
		);
	}
}

void USessMenuBase::joinButtonClicked()
{
	if (GEngine) {
		GEngine->AddOnScreenDebugMessage(
			-1,
			15.f,
			FColor::Yellow,
			FString(TEXT("joining..."))
		);
	}
}

bool USessMenuBase::Initialize()
{
	if (!Super::Initialize())
	{
		return false;
	}

	if (!hostBtn || !joinBtn) {
		return false;
	}
	//bind the delegate to relevant function
	hostBtn->OnClicked.AddDynamic(this, &ThisClass::hostButtonClicked);
	joinBtn->OnClicked.AddDynamic(this, &ThisClass::joinButtonClicked);
	return true;
}

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
