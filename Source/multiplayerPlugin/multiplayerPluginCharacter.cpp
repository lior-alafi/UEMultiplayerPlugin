// Copyright Epic Games, Inc. All Rights Reserved.

#include "multiplayerPluginCharacter.h"
#include "Engine/LocalPlayer.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/Controller.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputActionValue.h"
#include "OnlineSessionSettings.h" // for session creation settings

#include "Online/OnlineSessionNames.h" //for SEARCH_PRESENCE

DEFINE_LOG_CATEGORY(LogTemplateCharacter);

//////////////////////////////////////////////////////////////////////////
// AmultiplayerPluginCharacter

AmultiplayerPluginCharacter::AmultiplayerPluginCharacter():
	/* &ThisClass is used instead of &AmultiplayerPluginCharacter 
		create a delegate bounded to OncreateComplete 
	*/
	createSessionCompleteDelegate(FOnCreateSessionCompleteDelegate::CreateUObject(this,&ThisClass::OnCreateSessionComplete)),
	findSessionCompleteDelegate(FOnFindSessionsCompleteDelegate::CreateUObject(this,&ThisClass::OnFindSessionComplete))
{
	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);
		
	// Don't rotate when the controller rotates. Let that just affect the camera.
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	// Configure character movement
	GetCharacterMovement()->bOrientRotationToMovement = true; // Character moves in the direction of input...	
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 500.0f, 0.0f); // ...at this rotation rate

	// Note: For faster iteration times these variables, and many more, can be tweaked in the Character Blueprint
	// instead of recompiling to adjust them
	GetCharacterMovement()->JumpZVelocity = 700.f;
	GetCharacterMovement()->AirControl = 0.35f;
	GetCharacterMovement()->MaxWalkSpeed = 500.f;
	GetCharacterMovement()->MinAnalogWalkSpeed = 20.f;
	GetCharacterMovement()->BrakingDecelerationWalking = 2000.f;
	GetCharacterMovement()->BrakingDecelerationFalling = 1500.0f;

	// Create a camera boom (pulls in towards the player if there is a collision)
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 400.0f; // The camera follows at this distance behind the character	
	CameraBoom->bUsePawnControlRotation = true; // Rotate the arm based on the controller

	// Create a follow camera
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName); // Attach the camera to the end of the boom and let the boom adjust to match the controller orientation
	FollowCamera->bUsePawnControlRotation = false; // Camera does not rotate relative to arm

	// Note: The skeletal mesh and anim blueprint references on the Mesh component (inherited from Character) 
	// are set in the derived blueprint asset named ThirdPersonCharacter (to avoid direct content references in C++)


	//multiplayer
	IOnlineSubsystem* onlineSubsystem = IOnlineSubsystem::Get();
	if (onlineSubsystem) {
		OnlineSessionInterface = onlineSubsystem->GetSessionInterface();

		//print log to screen
		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(
				-1, // keep previous msgs
				15.f, //display for 15s
				FColor::Blue,
				FString::Printf(TEXT("Found a subsystem: %s"),*onlineSubsystem->GetSubsystemName().ToString())
				);
		}
	}
}

void AmultiplayerPluginCharacter::BeginPlay()
{
	// Call the base class  
	Super::BeginPlay();
}

void AmultiplayerPluginCharacter::CreateGameSession()
{
	if (!OnlineSessionInterface.IsValid()) {
		GEngine->AddOnScreenDebugMessage(
			-1, 15.f, FColor::Blue,
			FString::Printf(TEXT("no game session")));
		return;
	}

	//we can only have 1 session alive at a time, using "NAME_GameSession" make sure we call the same one
	auto existingSession = OnlineSessionInterface->GetNamedSession(NAME_GameSession);

	if (existingSession != nullptr) {
		// if exist destroy so we can create a new one
		OnlineSessionInterface->DestroySession(NAME_GameSession);
	}

	//adding delegate
	OnlineSessionInterface->AddOnCreateSessionCompleteDelegate_Handle(createSessionCompleteDelegate);


	//session settings
	TSharedPtr<FOnlineSessionSettings> SessSettings = MakeShareable(new FOnlineSessionSettings());
	SessSettings->bIsLANMatch = false;
	//how many players
	SessSettings->NumPublicConnections = 4; 
	//allow people to join while session is alive
	SessSettings->bAllowJoinInProgress = true;
	// find players by region
	SessSettings->bAllowJoinViaPresence = true;
	// steam advertize the session
	SessSettings->bShouldAdvertise = true;
	//use presence(regions) info 
	SessSettings->bUsesPresence = true;

	SessSettings->bIsDedicated = false;

	//if cannot find sessions enable this
	SessSettings->bUseLobbiesIfAvailable = true;
	
	//host player id
	const ULocalPlayer* localPlayer = GetWorld()->GetFirstLocalPlayerFromController();
	
	bool isSuccess = OnlineSessionInterface->CreateSession(*localPlayer->GetPreferredUniqueNetId(), NAME_GameSession, *SessSettings);
	
	if (isSuccess) {
		GEngine->AddOnScreenDebugMessage(
			-1, 15.f, FColor::Blue,
			FString::Printf(TEXT("created session")));
	}


}

void AmultiplayerPluginCharacter::JoinGameSession()
{
	if (!OnlineSessionInterface.IsValid()) {
		GEngine->AddOnScreenDebugMessage(
			-1, 15.f, FColor::Red,
			FString::Printf(TEXT("invalid session")));
		return;
	}
	//add onfind delegate
	OnlineSessionInterface->AddOnFindSessionsCompleteDelegate_Handle(findSessionCompleteDelegate);

	auto UserUniqueId = GetWorld()->GetFirstLocalPlayerFromController()->GetPreferredUniqueNetId();
	sessSearchSettings = MakeShareable(new FOnlineSessionSearch());
	
	//in normal case we should select something lower(4) but since our steam id is shared(480)
	//many users use that
	sessSearchSettings->MaxSearchResults = 10000;
	//it's not a LAN game
	sessSearchSettings->bIsLanQuery = false;
	/** Search for presence sessions only (because when we created the session we allowed presence */	
	sessSearchSettings->QuerySettings.Set(SEARCH_PRESENCE, true, EOnlineComparisonOp::Equals);



	OnlineSessionInterface->FindSessions(*UserUniqueId, sessSearchSettings.ToSharedRef());
}

void AmultiplayerPluginCharacter::OnCreateSessionComplete(FName SessionName, bool bWasSuccessful)
{
	if (bWasSuccessful)
	{
		GEngine->AddOnScreenDebugMessage(
			-1, 15.f, FColor::Blue,
			FString::Printf(TEXT("%s created successfuly"), *SessionName.ToString()));
	}
	else
	{
		GEngine->AddOnScreenDebugMessage(
			-1, 15.f, FColor::Red,
			FString::Printf(TEXT("failed to create session")));
	}
}

void AmultiplayerPluginCharacter::OnFindSessionComplete(bool bWasSuccessful)
{

	if (!bWasSuccessful)
	{
		GEngine->AddOnScreenDebugMessage(
			-1, 15.f, FColor::Red,
			FString::Printf(TEXT("failed to join session")));
		return;
	}

	for(auto result : sessSearchSettings->SearchResults)
	{
		FString sessId = result.GetSessionIdStr();
		FString user = result.Session.OwningUserName;

		GEngine->AddOnScreenDebugMessage(
			-1, 15.f, FColor::Green,
			FString::Printf(TEXT("id: %s user: %s"),*sessId,*user));
	}
}

//////////////////////////////////////////////////////////////////////////
// Input

void AmultiplayerPluginCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	// Add Input Mapping Context
	if (APlayerController* PlayerController = Cast<APlayerController>(GetController()))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
		{
			Subsystem->AddMappingContext(DefaultMappingContext, 0);
		}
	}
	
	// Set up action bindings
	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent)) {
		
		// Jumping
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Started, this, &ACharacter::Jump);
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &ACharacter::StopJumping);

		// Moving
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &AmultiplayerPluginCharacter::Move);

		// Looking
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &AmultiplayerPluginCharacter::Look);
	}
	else
	{
		UE_LOG(LogTemplateCharacter, Error, TEXT("'%s' Failed to find an Enhanced Input component! This template is built to use the Enhanced Input system. If you intend to use the legacy system, then you will need to update this C++ file."), *GetNameSafe(this));
	}
}

void AmultiplayerPluginCharacter::Move(const FInputActionValue& Value)
{
	// input is a Vector2D
	FVector2D MovementVector = Value.Get<FVector2D>();

	if (Controller != nullptr)
	{
		// find out which way is forward
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		// get forward vector
		const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
	
		// get right vector 
		const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

		// add movement 
		AddMovementInput(ForwardDirection, MovementVector.Y);
		AddMovementInput(RightDirection, MovementVector.X);
	}
}

void AmultiplayerPluginCharacter::Look(const FInputActionValue& Value)
{
	// input is a Vector2D
	FVector2D LookAxisVector = Value.Get<FVector2D>();

	if (Controller != nullptr)
	{
		// add yaw and pitch input to controller
		AddControllerYawInput(LookAxisVector.X);
		AddControllerPitchInput(LookAxisVector.Y);
	}
}