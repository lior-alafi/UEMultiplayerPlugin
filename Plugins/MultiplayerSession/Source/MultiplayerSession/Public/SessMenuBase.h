// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
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

	//forward declare our plugin subsystem
	class UMultiplayerSessionSubsystem* onlineSessSubsystem;
protected:
	virtual bool Initialize();

public:
	UFUNCTION(BlueprintCallable)
	void menuSetup(FString lobby,int32 maxConnections = 4, FString gameType = TEXT("freeforall"));
};
