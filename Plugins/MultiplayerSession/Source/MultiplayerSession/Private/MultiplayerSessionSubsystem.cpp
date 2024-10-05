// Fill out your copyright notice in the Description page of Project Settings.


#include "MultiplayerSessionSubsystem.h"
#include "OnlineSubsystem.h"
#include "OnlineSessionSettings.h" // for session creation settings

#include "Online/OnlineSessionNames.h" //for SEARCH_PRESENCE


bool UMultiplayerSessionSubsystem::PrepareSessionSettings(int32 numPublicPlayers, FString matchType)
{
	SessSettings = MakeShareable(new FOnlineSessionSettings());
	
	//subsystem named NULL are basically LAN
	SessSettings->bIsLANMatch = IOnlineSubsystem::Get()->GetSubsystemName() == "NULL";
	//allow join ongoing session
	SessSettings->bAllowJoinInProgress = true;
	SessSettings->bAllowJoinViaPresence = true;
	SessSettings->bIsDedicated = false;
	
	SessSettings->bShouldAdvertise = true;
	SessSettings->bUseLobbiesIfAvailable = true;
	SessSettings->bUsesPresence = true;
	SessSettings->NumPublicConnections = numPublicPlayers;
	SessSettings->Set(FName("MatchType"), matchType,EOnlineDataAdvertisementType::ViaOnlineServiceAndPing);

	return true;
}

void UMultiplayerSessionSubsystem::OnCreateSessionComplete(FName SessionName, bool bWasSuccessful)
{
	if (OnlineSession) {
		//if fail let's clear the delegate from list
		OnlineSession->ClearOnCreateSessionCompleteDelegate_Handle(OnSessCreate_handle);
	}
	//multicast is broadcast
	MultiPlayerOnSessionCreatedDelegate.Broadcast(bWasSuccessful);	
}

void UMultiplayerSessionSubsystem::OnFindSessionsComplete(bool bWasSuccessful)
{
}

void UMultiplayerSessionSubsystem::OnJoinSessionComplete(FName SessionName, EOnJoinSessionCompleteResult::Type resultStatus)
{
}

void UMultiplayerSessionSubsystem::OnDestroySessionComplete(FName SessionName, bool bWasSuccessful)
{
}

void UMultiplayerSessionSubsystem::OnStartSessionComplete(FName SessionName, bool bWasSuccessful)
{
}

UMultiplayerSessionSubsystem::UMultiplayerSessionSubsystem() :
	OnSessCreateDelegate(FOnCreateSessionCompleteDelegate::CreateUObject(this,&ThisClass::OnCreateSessionComplete)),
	OnSessFindDelegate(FOnFindSessionsCompleteDelegate::CreateUObject(this,&ThisClass::OnFindSessionsComplete)),
	OnSessJoinDelegate(FOnJoinSessionCompleteDelegate::CreateUObject(this,&ThisClass::OnJoinSessionComplete)),
	OnSessDestroyDelegate(FOnDestroySessionCompleteDelegate::CreateUObject(this,&ThisClass::OnDestroySessionComplete)),
	OnSessStartDelegate(FOnStartSessionCompleteDelegate::CreateUObject(this,&ThisClass::OnStartSessionComplete))
{
	

	IOnlineSubsystem* subsystem = IOnlineSubsystem::Get();
	if (subsystem) {
		//log
		OnlineSession = subsystem->GetSessionInterface();
		
	}
	
}

void UMultiplayerSessionSubsystem::CreateSession(int32 numPublicPlayers, FString matchType)
{
	if (!OnlineSession.IsValid()) {
		return;
	}
	//check if a session exists if so we disconnect and connect to a new one
	auto existingSession = OnlineSession->GetNamedSession(NAME_GameSession);

	//do not replace this to !existingSession IT WILL NOT CLOSE SESSIONS
	if (existingSession != nullptr)
	{
		OnlineSession->DestroySession(NAME_GameSession);
	}

	//store delegate in fdelegate, we need to remove the delegate later
	OnSessCreate_handle = OnlineSession->AddOnCreateSessionCompleteDelegate_Handle(OnSessCreateDelegate);
	
	PrepareSessionSettings(numPublicPlayers, matchType);
	
	const int32 uniqueId = GetWorld()->GetFirstPlayerController()->GetUniqueID();	
	if (!OnlineSession->CreateSession(uniqueId, NAME_GameSession, *SessSettings))
	{
		//if we failed to create a session clear delegate

		OnlineSession->ClearOnCreateSessionCompleteDelegate_Handle(OnSessCreate_handle);
		MultiPlayerOnSessionCreatedDelegate.Broadcast(false);
	}
}

void UMultiplayerSessionSubsystem::FindSessions(int32 numOfSessionsResults)
{
}

void UMultiplayerSessionSubsystem::JoinSession(const FOnlineSessionSearchResult& sessResult)
{
}

void UMultiplayerSessionSubsystem::DestroySession()
{
}

void UMultiplayerSessionSubsystem::StartSession()
{
}
