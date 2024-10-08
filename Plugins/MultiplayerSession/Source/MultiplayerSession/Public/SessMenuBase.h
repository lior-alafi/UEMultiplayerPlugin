// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "SessMenuBase.generated.h"


///**
// * 
// */
UCLASS()
class MULTIPLAYERSESSION_API USessMenuBase : public UUserWidget
{
	GENERATED_BODY()
	
private:
	int32 MaxConnections;
	FString GameType;
	FString LobbyPath;

	//bind the button to a bp widget button with the same name
	UPROPERTY(meta = (BindWidget))
	//must be the same name as on the BP widget
	class UButton* hostBtn;

	UPROPERTY(meta = (BindWidget))
	UButton* joinBtn;

	UFUNCTION()
	void hostButtonClicked();

	UFUNCTION()
	void joinButtonClicked();

	void menuTeardown();

	//forward declare our plugin subsystem
	class UMultiplayerSessionSubsystem* onlineSessSubsystem;
protected:
	virtual bool Initialize();

	//destroy widget 
	virtual void NativeDestruct() override;

//custom delegates callback
	UFUNCTION()
	virtual void OnCreateSession(bool bWasSuccessful);

	virtual void OnFindSessions(const TArray<FOnlineSessionSearchResult>& sess, bool bWasSuccessful);
	virtual void OnJoinSession(EOnJoinSessionCompleteResult::Type result);
	UFUNCTION()
	virtual void OnDestroySession(bool bWasSuccessful);

	UFUNCTION()
	virtual void OnStartSession(bool bWasSuccessful);

public:
	UFUNCTION(BlueprintCallable)
	void menuSetup(FString lobby,int32 maxConnections = 4, FString gameType = TEXT("freeforall"));


};
