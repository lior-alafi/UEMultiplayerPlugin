// Fill out your copyright notice in the Description page of Project Settings.


#include "SessMenuBase.h"
#include "Components/Button.h"
#include "MultiplayerSessionSubsystem.h"

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

	//lay host a session basic 
	if (onlineSessSubsystem) {
		onlineSessSubsystem->CreateSession(MaxConnections, GameType);
		UWorld* world = GetWorld();
		if (world)
		{
			//travel to lobby
			world->ServerTravel(FString::Printf(TEXT("%s?listen"), *LobbyPath));
		}
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
	//lay join session basic
	if (onlineSessSubsystem) {
		onlineSessSubsystem->FindSessions(10000);
	}
}

void USessMenuBase::menuTeardown()
{
	RemoveFromParent();
	UWorld* world = GetWorld();
	if (world) {
		APlayerController* playerCtrler = world->GetFirstPlayerController();
		if (playerCtrler)
		{
			//make sure we can move our character again
			FInputModeGameOnly gameOnly;
			playerCtrler->SetInputMode(gameOnly);

			//hide mouse (we no longer need it(?))
			playerCtrler->SetShowMouseCursor(false);
		}
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

void USessMenuBase::NativeDestruct()
{
	menuTeardown();

	Super::NativeDestruct();
}

void USessMenuBase::menuSetup(FString lobby,int32 maxConns, FString gameType)
{

	MaxConnections = maxConns;
	GameType = gameType;
	LobbyPath = lobby;
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

			//focus mouse on the widget
			FInputModeUIOnly uiOnlyMode;
			uiOnlyMode.SetWidgetToFocus(TakeWidget());
			uiOnlyMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
			playerController->SetInputMode(uiOnlyMode);
			playerController->SetShowMouseCursor(true);
		}
	}

	UGameInstance* instance = GetGameInstance();
	if (!instance) return;
	onlineSessSubsystem = instance->GetSubsystem<UMultiplayerSessionSubsystem>();

}
