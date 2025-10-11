// Copyright Epic Games, Inc. All Rights Reserved.




#include "BloodreadGamePlayerController.h"
#include "Blueprint/UserWidget.h"
#include "Engine/Engine.h"
#include "Engine/AssetManager.h"
#include "UObject/ConstructorHelpers.h"


ABloodreadGamePlayerController::ABloodreadGamePlayerController()
{
   UE_LOG(LogTemp, Warning, TEXT("BloodreadGamePlayerController constructor - initializing for Blueprint-based Steam multiplayer"));
   
   // Initialize widget pointers
   MultiplayerLobbyWidget = nullptr;
   WB_MainMenu = nullptr;
   WB_CreateServer = nullptr;
   WB_ServerBrowser = nullptr;
   
   // Load widget classes using ConstructorHelpers (must be in constructor)
   static ConstructorHelpers::FClassFinder<UUserWidget> MainMenuWidgetBPClass(TEXT("/Game/WB_MainMenu"));
   if (MainMenuWidgetBPClass.Class != nullptr)
   {
       MainMenuWidgetClass = MainMenuWidgetBPClass.Class;
       UE_LOG(LogTemp, Warning, TEXT("Successfully loaded WB_MainMenu Blueprint class"));
   }
   else
   {
       UE_LOG(LogTemp, Error, TEXT("Failed to load WB_MainMenu Blueprint class from /Game/WB_MainMenu"));
       MainMenuWidgetClass = nullptr;
   }
   
   static ConstructorHelpers::FClassFinder<UUserWidget> CreateServerWidgetBPClass(TEXT("/Game/WB_CreateServer"));
   if (CreateServerWidgetBPClass.Class != nullptr)
   {
       CreateServerWidgetClass = CreateServerWidgetBPClass.Class;
       UE_LOG(LogTemp, Warning, TEXT("Successfully loaded WB_CreateServer Blueprint class"));
   }
   else
   {
       UE_LOG(LogTemp, Error, TEXT("Failed to load WB_CreateServer Blueprint class from /Game/WB_CreateServer"));
       CreateServerWidgetClass = nullptr;
   }
   
   static ConstructorHelpers::FClassFinder<UUserWidget> ServerBrowserWidgetBPClass(TEXT("/Game/WB_ServerBrowser"));
   if (ServerBrowserWidgetBPClass.Class != nullptr)
   {
       ServerBrowserWidgetClass = ServerBrowserWidgetBPClass.Class;
       UE_LOG(LogTemp, Warning, TEXT("Successfully loaded WB_ServerBrowser Blueprint class"));
   }
   else
   {
       UE_LOG(LogTemp, Error, TEXT("Failed to load WB_ServerBrowser Blueprint class from /Game/WB_ServerBrowser"));
       ServerBrowserWidgetClass = nullptr;
   }
}


void ABloodreadGamePlayerController::BeginPlay()
{
   Super::BeginPlay();

   UE_LOG(LogTemp, Warning, TEXT("BloodreadGamePlayerController BeginPlay - Setting up Blueprint-based Steam multiplayer UI"));
   
   // Check if this is the local controller
   if (IsLocalController())
   {
       UE_LOG(LogTemp, Warning, TEXT("Local controller detected - creating Steam multiplayer UI widgets"));
       
       // Create UI widgets (widget classes already loaded in constructor)
       CreateUIWidgets();
   }
   else
   {
       UE_LOG(LogTemp, Warning, TEXT("Non-local controller - skipping UI creation"));
   }
}


void ABloodreadGamePlayerController::OnPossess(APawn* InPawn)
{
   Super::OnPossess(InPawn);


   UE_LOG(LogTemp, Warning, TEXT("=== PlayerController::OnPossess START ==="));
   UE_LOG(LogTemp, Warning, TEXT("PlayerController possessing pawn: %s"), InPawn ? *InPawn->GetName() : TEXT("NULL"));
  
   if (InPawn)
   {
       UE_LOG(LogTemp, Warning, TEXT("Pawn class: %s"), *InPawn->GetClass()->GetName());
      
       // Try to cast to our base character and initialize mesh
       if (ABloodreadBaseCharacter* BloodreadCharacter = Cast<ABloodreadBaseCharacter>(InPawn))
       {
           UE_LOG(LogTemp, Warning, TEXT("SUCCESS: Cast to BloodreadBaseCharacter %s - Current Class: %d"),
                  *BloodreadCharacter->GetName(), (int32)BloodreadCharacter->GetCharacterClass());
           InitializeCharacterMesh(BloodreadCharacter);
       }
       else
       {
           UE_LOG(LogTemp, Error, TEXT("FAILED: Possessed pawn is not a BloodreadBaseCharacter - Class: %s"), *InPawn->GetClass()->GetName());
       }
   }
   else
   {
       UE_LOG(LogTemp, Error, TEXT("InPawn is NULL!"));
   }
  
   UE_LOG(LogTemp, Warning, TEXT("=== PlayerController::OnPossess END ==="));
}


void ABloodreadGamePlayerController::InitializeCharacterMesh(ABloodreadBaseCharacter* InCharacter)
{
   if (!InCharacter)
   {
       UE_LOG(LogTemp, Error, TEXT("InitializeCharacterMesh: InCharacter is null"));
       return;
   }


   UE_LOG(LogTemp, Warning, TEXT("InitializeCharacterMesh: Starting mesh initialization for %s"), *InCharacter->GetName());


   // Force the character to initialize its class data
   InCharacter->ForceInitializeCharacterSystems();


   // Try to find CharacterMesh0 component (commonly used in Blueprints)
   if (USkeletalMeshComponent* CharacterMesh0 = InCharacter->FindComponentByClass<USkeletalMeshComponent>())
   {
       UE_LOG(LogTemp, Warning, TEXT("Found SkeletalMeshComponent, applying class-based mesh"));
      
       ECharacterClass CharacterClass = InCharacter->GetCharacterClass();
       if (InCharacter->ApplyClassDataToMeshComponent(CharacterMesh0, CharacterClass))
       {
           UE_LOG(LogTemp, Warning, TEXT("SUCCESS: Mesh applied to CharacterMesh0 component"));
       }
       else
       {
           UE_LOG(LogTemp, Error, TEXT("FAILED: Could not apply mesh to CharacterMesh0 component"));
       }
   }
   else
   {
       UE_LOG(LogTemp, Warning, TEXT("No CharacterMesh0 found, using default character mesh"));
      
       // Fallback to the default character mesh
       ECharacterClass CharacterClass = InCharacter->GetCharacterClass();
       if (InCharacter->ApplyClassDataToMeshComponent(InCharacter->GetMesh(), CharacterClass))
       {
           UE_LOG(LogTemp, Warning, TEXT("SUCCESS: Mesh applied to default character mesh"));
       }
       else
       {
           UE_LOG(LogTemp, Error, TEXT("FAILED: Could not apply mesh to default character mesh"));
       }
   }
}


void ABloodreadGamePlayerController::ShowMultiplayerLobby()
{
   UE_LOG(LogTemp, Warning, TEXT("ShowMultiplayerLobby called"));
  
   // If we already have a lobby widget, just show it
   if (MultiplayerLobbyWidget && IsValid(MultiplayerLobbyWidget))
   {
       MultiplayerLobbyWidget->AddToViewport();
       SetInputMode(FInputModeUIOnly());
       SetShowMouseCursor(true);
       UE_LOG(LogTemp, Warning, TEXT("Showing existing multiplayer lobby"));
       return;
   }
  
   UE_LOG(LogTemp, Warning, TEXT("Creating new multiplayer lobby widget..."));
  
   // Check if we have a Blueprint class set
   if (MultiplayerLobbyWidgetClass)
   {
       UE_LOG(LogTemp, Warning, TEXT("Using Blueprint MultiplayerLobbyWidget class: %s"), *MultiplayerLobbyWidgetClass->GetName());
       MultiplayerLobbyWidget = CreateWidget<UMultiplayerLobbyWidget>(this, MultiplayerLobbyWidgetClass);
   }
   else
   {
       // Fallback to C++ class if no Blueprint class is set
       UE_LOG(LogTemp, Warning, TEXT("Using C++ MultiplayerLobbyWidget class (no Blueprint class set)"));
       MultiplayerLobbyWidget = CreateWidget<UMultiplayerLobbyWidget>(this, UMultiplayerLobbyWidget::StaticClass());
   }
  
   if (MultiplayerLobbyWidget)
   {
       UE_LOG(LogTemp, Warning, TEXT("Widget created successfully, adding to viewport"));
       MultiplayerLobbyWidget->AddToViewport();
       SetInputMode(FInputModeUIOnly());
       SetShowMouseCursor(true);
       UE_LOG(LogTemp, Warning, TEXT("Multiplayer lobby created and shown - Input mode set to UI"));
   }
   else
   {
       UE_LOG(LogTemp, Error, TEXT("Failed to create multiplayer lobby widget"));
   }
}


void ABloodreadGamePlayerController::HideMultiplayerLobby()
{
   if (MultiplayerLobbyWidget && IsValid(MultiplayerLobbyWidget))
   {
       MultiplayerLobbyWidget->RemoveFromParent();
       SetInputMode(FInputModeGameOnly());
       SetShowMouseCursor(false);
       UE_LOG(LogTemp, Warning, TEXT("Multiplayer lobby hidden"));
   }
}

void ABloodreadGamePlayerController::CreateUIWidgets()
{
   UE_LOG(LogTemp, Warning, TEXT("CreateUIWidgets: Creating Steam multiplayer UI widgets"));
   
   // Create WB_MainMenu widget and add to viewport
   if (MainMenuWidgetClass)
   {
       WB_MainMenu = CreateWidget<UUserWidget>(this, MainMenuWidgetClass);
       if (WB_MainMenu)
       {
           WB_MainMenu->AddToViewport();
           UE_LOG(LogTemp, Warning, TEXT("WB_MainMenu created and added to viewport"));
           
           // Set input mode for UI interaction
           SetInputMode(FInputModeUIOnly());
           SetShowMouseCursor(true);
       }
       else
       {
           UE_LOG(LogTemp, Error, TEXT("Failed to create WB_MainMenu widget"));
       }
   }
   else
   {
       UE_LOG(LogTemp, Error, TEXT("MainMenuWidgetClass is null - cannot create WB_MainMenu"));
   }
   
   // Create WB_CreateServer widget (but don't add to viewport yet)
   if (CreateServerWidgetClass)
   {
       WB_CreateServer = CreateWidget<UUserWidget>(this, CreateServerWidgetClass);
       if (WB_CreateServer)
       {
           UE_LOG(LogTemp, Warning, TEXT("WB_CreateServer widget created successfully"));
       }
       else
       {
           UE_LOG(LogTemp, Error, TEXT("Failed to create WB_CreateServer widget"));
       }
   }
   else
   {
       UE_LOG(LogTemp, Error, TEXT("CreateServerWidgetClass is null - cannot create WB_CreateServer"));
   }
   
   // Create WB_ServerBrowser widget (but don't add to viewport yet)
   if (ServerBrowserWidgetClass)
   {
       WB_ServerBrowser = CreateWidget<UUserWidget>(this, ServerBrowserWidgetClass);
       if (WB_ServerBrowser)
       {
           UE_LOG(LogTemp, Warning, TEXT("WB_ServerBrowser widget created successfully"));
           // Set as the current server browser (user requirement)
           UE_LOG(LogTemp, Warning, TEXT("WB_ServerBrowser set as current server browser widget"));
       }
       else
       {
           UE_LOG(LogTemp, Error, TEXT("Failed to create WB_ServerBrowser widget"));
       }
   }
   else
   {
       UE_LOG(LogTemp, Error, TEXT("ServerBrowserWidgetClass is null - cannot create WB_ServerBrowser"));
   }
   
   UE_LOG(LogTemp, Warning, TEXT("UI widget creation complete - Main Menu displayed, Create Server and Server Browser widgets ready"));
}



