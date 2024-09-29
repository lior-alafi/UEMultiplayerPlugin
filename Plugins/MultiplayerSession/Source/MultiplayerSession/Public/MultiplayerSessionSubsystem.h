// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "MultiplayerSessionSubsystem.generated.h"

/**
 * 
 */
UCLASS()
class MULTIPLAYERSESSION_API UMultiplayerSessionSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()
	
private:
	IOnlineSessionPtr OnlineSession;
	
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
};
