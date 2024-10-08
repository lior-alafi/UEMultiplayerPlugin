// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "MultiplayerSessionSubsystem.generated.h"

//define delegates
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMultiplayerOnSessionCreated, bool, bWasSuccessful);
DECLARE_MULTICAST_DELEGATE_TwoParams(FMultiplayerOnSessionFind,const TArray<FOnlineSessionSearchResult> &sess, bool bWasSuccessful);
DECLARE_MULTICAST_DELEGATE_OneParam(FMultiplayerOnSessionJoin, EOnJoinSessionCompleteResult::Type result);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMultiplayerOnSessionDestroy,bool,bWasSuccessful);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMultiplayerOnSessionStart,bool,bWasSuccessful);

/**
 * 
 */
UCLASS()
class MULTIPLAYERSESSION_API UMultiplayerSessionSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()
	
private:
	IOnlineSessionPtr OnlineSession;
	bool bCreateSessionOnDestry{ false };
	
	//createSession variables
	int32 numOfPublicAcceptedConnections = 4;
	FString MatchType;

	//settings
	TSharedPtr<FOnlineSessionSettings> SessSettings;
	TSharedPtr<FOnlineSessionSearch> searchSessSettings;

	//delegates
	FOnCreateSessionCompleteDelegate OnSessCreateDelegate;
	FOnFindSessionsCompleteDelegate OnSessFindDelegate;
	FOnJoinSessionCompleteDelegate OnSessJoinDelegate;
	FOnDestroySessionCompleteDelegate OnSessDestroyDelegate;
	FOnStartSessionCompleteDelegate OnSessStartDelegate;

	//delegates handles
	FDelegateHandle OnSessCreate_handle;
	FDelegateHandle OnSessFind_handle;
	FDelegateHandle OnSessJoin_handle;
	FDelegateHandle OnSessDestroy_handle;
	FDelegateHandle OnSessStart_handle;

	bool PrepareSessionSettings(int32 numPublicPlayers, FString matchType);
protected:
	//calbacks
	void OnCreateSessionComplete(FName SessionName, bool bWasSuccessful);
	void OnFindSessionsComplete(bool bWasSuccessful);
	void OnJoinSessionComplete(FName SessionName, EOnJoinSessionCompleteResult::Type resultStatus);
	void OnDestroySessionComplete(FName SessionName, bool bWasSuccessful);
	void OnStartSessionComplete(FName SessionName, bool bWasSuccessful);
public:
	UMultiplayerSessionSubsystem();
	
	void CreateSession(int32 numPublicPlayers, FString matchType);
	void FindSessions(int32 numOfSessionsResults);
	void JoinSession(const FOnlineSessionSearchResult& sessResult);
	void DestroySession();
	void StartSession();

	//custom delegates

	FMultiplayerOnSessionCreated MultiPlayerOnSessionCreatedDelegate;
	FMultiplayerOnSessionFind MultiplayerOnSessionFindDelegate;
	FMultiplayerOnSessionJoin MultiplayerOnSessionJoinDelegate;
	FMultiplayerOnSessionDestroy MultiplayerOnSessionDestroyDelegate;
	FMultiplayerOnSessionStart MultiplayerOnSessionStartDelegate;
};
