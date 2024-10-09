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
	//allow more sessions to be opened
	SessSettings->BuildUniqueId = 1;
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
	if (OnlineSession) {
		OnlineSession->ClearOnFindSessionsCompleteDelegate_Handle(OnSessFind_handle);
	}
	if (searchSessSettings->SearchResults.Num() <= 0) {
		MultiplayerOnSessionFindDelegate.Broadcast(TArray<FOnlineSessionSearchResult>(), false);
		return;
	}
	MultiplayerOnSessionFindDelegate.Broadcast(searchSessSettings->SearchResults, bWasSuccessful);
}

void UMultiplayerSessionSubsystem::OnJoinSessionComplete(FName SessionName, EOnJoinSessionCompleteResult::Type resultStatus)
{
	if (OnlineSession) {
		OnlineSession->ClearOnJoinSessionCompleteDelegate_Handle(OnSessJoin_handle);
	}

	MultiplayerOnSessionJoinDelegate.Broadcast(resultStatus);

}

void UMultiplayerSessionSubsystem::OnDestroySessionComplete(FName SessionName, bool bWasSuccessful)
{
	if (OnlineSession)
	{
		OnlineSession->ClearOnDestroySessionCompleteDelegate_Handle(OnSessDestroy_handle);
	}

	if (bCreateSessionOnDestry && bWasSuccessful) {
		bCreateSessionOnDestry = false;
		CreateSession(numOfPublicAcceptedConnections, MatchType);
	}

	MultiplayerOnSessionDestroyDelegate.Broadcast(bWasSuccessful);
}

void UMultiplayerSessionSubsystem::OnStartSessionComplete(FName SessionName, bool bWasSuccessful)
{
	if (OnlineSession)
	{
		OnlineSession->ClearOnStartSessionCompleteDelegate_Handle(OnSessStart_handle);
	}
	MultiplayerOnSessionStartDelegate.Broadcast(bWasSuccessful);
}

UMultiplayerSessionSubsystem::UMultiplayerSessionSubsystem() :
	OnSessCreateDelegate(FOnCreateSessionCompleteDelegate::CreateUObject(this,&ThisClass::OnCreateSessionComplete)),
	OnSessFindDelegate(FOnFindSessionsCompleteDelegate::CreateUObject(this,&ThisClass::OnFindSessionsComplete)),
	OnSessJoinDelegate(FOnJoinSessionCompleteDelegate::CreateUObject(this,&ThisClass::OnJoinSessionComplete)),
	OnSessDestroyDelegate(FOnDestroySessionCompleteDelegate::CreateUObject(this,&ThisClass::OnDestroySessionComplete)),
	OnSessStartDelegate(FOnStartSessionCompleteDelegate::CreateUObject(this,&ThisClass::OnStartSessionComplete))
{
	MatchType = FString(TEXT("freeforall"));

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

	numOfPublicAcceptedConnections = numPublicPlayers;
	MatchType = matchType;
	//check if a session exists if so we disconnect and connect to a new one
	auto existingSession = OnlineSession->GetNamedSession(NAME_GameSession);

	//do not replace this to !existingSession IT WILL NOT CLOSE SESSIONS
	if (existingSession != nullptr)
	{
		DestroySession();
		bCreateSessionOnDestry = true;
	}

	//store delegate in fdelegate, we need to remove the delegate later
	OnSessCreate_handle = OnlineSession->AddOnCreateSessionCompleteDelegate_Handle(OnSessCreateDelegate);
	
	PrepareSessionSettings(numOfPublicAcceptedConnections, MatchType);
	
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
	if (!OnlineSession.IsValid()) {
		MultiplayerOnSessionFindDelegate.Broadcast(TArray<FOnlineSessionSearchResult>(), false);
		return;
	}
	OnSessFind_handle = OnlineSession->AddOnFindSessionsCompleteDelegate_Handle(OnSessFindDelegate);

	const int32 uniqueId = GetWorld()->GetFirstLocalPlayerFromController()->GetUniqueID();

	searchSessSettings = MakeShareable(new FOnlineSessionSearch());
	searchSessSettings->bIsLanQuery = IOnlineSubsystem::Get()->GetSubsystemName() == "NULL";
	searchSessSettings->MaxSearchResults = numOfSessionsResults;

	/** Search for presence sessions only (because when we created the session we allowed presence */
	searchSessSettings->QuerySettings.Set(SEARCH_PRESENCE, true, EOnlineComparisonOp::Equals);
	if (!OnlineSession->FindSessions(uniqueId, searchSessSettings.ToSharedRef()))
	{
		OnlineSession->ClearOnFindSessionsCompleteDelegate_Handle(OnSessFind_handle);
		MultiplayerOnSessionFindDelegate.Broadcast(TArray<FOnlineSessionSearchResult>(), false);
	}
}

void UMultiplayerSessionSubsystem::JoinSession(const FOnlineSessionSearchResult& sessResult)
{
	if (!OnlineSession.IsValid()) {
		MultiplayerOnSessionJoinDelegate.Broadcast(EOnJoinSessionCompleteResult::UnknownError);
		return;
	}

	OnSessJoin_handle = OnlineSession->AddOnJoinSessionCompleteDelegate_Handle(OnSessJoinDelegate);

	//get uniqueID
	const auto uniqueId = GetWorld()->GetFirstLocalPlayerFromController()->GetPreferredUniqueNetId();

	if (!OnlineSession->JoinSession(*uniqueId,NAME_GameSession,sessResult)) {
		OnlineSession->ClearOnJoinSessionCompleteDelegate_Handle(OnSessJoin_handle);
		MultiplayerOnSessionJoinDelegate.Broadcast(EOnJoinSessionCompleteResult::UnknownError);
	}
}

void UMultiplayerSessionSubsystem::DestroySession()
{
	if (!OnlineSession.IsValid()) {
		MultiplayerOnSessionDestroyDelegate.Broadcast(false);
		return;
	}

	OnSessDestroy_handle = OnlineSession->AddOnDestroySessionCompleteDelegate_Handle(OnSessDestroyDelegate);

	if (!OnlineSession->DestroySession(NAME_GameSession)) {
		OnlineSession->ClearOnDestroySessionCompleteDelegate_Handle(OnSessDestroy_handle);
		MultiplayerOnSessionDestroyDelegate.Broadcast(false);
	}
}

void UMultiplayerSessionSubsystem::StartSession()
{
	if (!OnlineSession.IsValid()) {
		MultiplayerOnSessionStartDelegate.Broadcast(false);
		return;
	}
	OnSessStart_handle = OnlineSession->AddOnStartSessionCompleteDelegate_Handle(OnSessStartDelegate);

	if (!OnlineSession->StartSession(NAME_GameSession))
	{
		OnlineSession->ClearOnStartSessionCompleteDelegate_Handle(OnSessStart_handle);
		MultiplayerOnSessionStartDelegate.Broadcast(false);
	}
	
}
