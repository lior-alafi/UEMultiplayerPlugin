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

	//bind the button to a bp widget button with the same name
	UPROPERTY(meta = (BindWidget))
	//must be the same name as on the BP widget
	class UButton* hostBtn;

	UPROPERTY(meta = (BindWidget))
	UButton* joinBtn;


protected:

public:
	UFUNCTION(BlueprintCallable)
	void menuSetup();
};
