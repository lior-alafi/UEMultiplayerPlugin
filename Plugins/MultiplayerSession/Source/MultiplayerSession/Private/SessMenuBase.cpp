// Fill out your copyright notice in the Description page of Project Settings.


#include "SessMenuBase.h"
#include "Components/Button.h"
#include "MultiplayerSessionSubsystem.h"

#include "OnlineSessionSettings.h" 

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

void USessMenuBase::OnCreateSession(bool bWasSuccessful)
{
	
	if (GEngine) {
		FColor color = bWasSuccessful ? FColor::Green : FColor::Red;
		FString msg = bWasSuccessful ? FString(TEXT("host session successfully")) : FString(TEXT("failed to host session"));
		GEngine->AddOnScreenDebugMessage(
			-1, 15.f, color, msg);
	}
	if (bWasSuccessful) {
		UWorld* world = GetWorld();
		if (!world) {
			return;
		}
		//travel to lobby
		world->ServerTravel(FString::Printf(TEXT("%s?listen"), *LobbyPath));
	}
}

void USessMenuBase::OnFindSessions(const TArray<FOnlineSessionSearchResult>& sess, bool bWasSuccessful)
{
	if (!bWasSuccessful && GEngine) {
		GEngine->AddOnScreenDebugMessage(-1, 15.f, FColor::Red, FString("failed to find a session"));
		return;
	}
	if (onlineSessSubsystem == nullptr) {
		return;
	}
	for (const auto& res : sess)
	{
		if (res.IsValid())
		{
			FString val;
			//if value for this key("MatchType") is the same type as value provide(Fstring) it will fill that variable
			res.Session.SessionSettings.Get("MatchType", val);

			if (val == GameType)
			{
				onlineSessSubsystem->JoinSession(res);
				return;
			}
		} 
	}
}

void USessMenuBase::OnJoinSession(EOnJoinSessionCompleteResult::Type result)
{
	//1. get correct address 
	//2. client travel to that address
	IOnlineSubsystem* subsystem = IOnlineSubsystem::Get();
	if (subsystem) {
		IOnlineSessionPtr  OnlineSessionInterface = subsystem->GetSessionInterface();
		FString Address;
		if (GEngine && OnlineSessionInterface->GetResolvedConnectString(NAME_GameSession, Address)) {
			//get playercontroller of this instance
			APlayerController* playerCtrller = GetGameInstance()->GetFirstLocalPlayerController();
			if (playerCtrller) {
				//travel to this address
				playerCtrller->ClientTravel(Address, ETravelType::TRAVEL_Absolute);
			}
		}
	}
}

void USessMenuBase::OnDestroySession(bool bWasSuccessful)
{
}

void USessMenuBase::OnStartSession(bool bWasSuccessful)
{
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
	if (onlineSessSubsystem) {
		//bind custom delegates
		onlineSessSubsystem->MultiPlayerOnSessionCreatedDelegate.AddDynamic(this, &ThisClass::OnCreateSession);
		onlineSessSubsystem->MultiplayerOnSessionDestroyDelegate.AddDynamic(this, &ThisClass::OnDestroySession);
		onlineSessSubsystem->MultiplayerOnSessionStartDelegate.AddDynamic(this, &ThisClass::OnStartSession);
		
		//not dynamic session delegates
		onlineSessSubsystem->MultiplayerOnSessionFindDelegate.AddUObject(this, &ThisClass::OnFindSessions);
		onlineSessSubsystem->MultiplayerOnSessionJoinDelegate.AddUObject(this, &ThisClass::OnJoinSession);
	}
}
