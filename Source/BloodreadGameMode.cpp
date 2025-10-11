#include "BloodreadGameMode.h"
#include "BloodreadGameInstance.h"
#include "BloodreadPlayerCharacter.h"
#include "BloodreadBaseCharacter.h"
#include "BloodreadWarriorCharacter.h"
#include "BloodreadMageCharacter.h"
#include "BloodreadRogueCharacter.h"
#include "CharacterSelectionManager.h"
#include "PracticeDummy.h"
#include "Engine/World.h"
#include "Engine/PlayerStartPIE.h"
#include "GameFramework/PlayerStart.h"
#include "GameFramework/PlayerController.h"
#include "TimerManager.h"
#include "Kismet/GameplayStatics.h"
#include "Blueprint/UserWidget.h"
#include "BloodreadGamePlayerController.h"
#include "Engine/Engine.h"

ABloodreadGameMode::ABloodreadGameMode()
{
    PrimaryActorTick.bCanEverTick = true;
    
    // Set default classes
    PlayerControllerClass = ABloodreadGamePlayerController::StaticClass();
    DefaultPawnClass = ABloodreadBaseCharacter::StaticClass();  // Keep base class for flexibility
    
    // Initialize Character Selection Manager
    CharacterSelectionManager = CreateDefaultSubobject<UCharacterSelectionManager>(TEXT("CharacterSelectionManager"));
    
    UE_LOG(LogTemp, Warning, TEXT("GameMode constructor: PlayerController set to BloodreadGamePlayerController, DefaultPawn set to BloodreadBaseCharacter"));
}

void ABloodreadGameMode::BeginPlay()
{
    Super::BeginPlay();
    
    UE_LOG(LogTemp, Log, TEXT("BloodreadGameMode started"));
    
    // Initialize game tick timer
    GameTickTimer = 0.0f;
    CurrentTick = 0;
    
    // Initialize Character Selection Manager if not already created
    if (!CharacterSelectionManager)
    {
        CharacterSelectionManager = NewObject<UCharacterSelectionManager>(this);
        UE_LOG(LogTemp, Warning, TEXT("Created Character Selection Manager in BeginPlay"));
    }

    // Start in multiplayer lobby state - the PlayerController will show the lobby UI
    SetGameState(EGameState::MultiplayerLobby);
}

void ABloodreadGameMode::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    if (bMatchInProgress)
    {
        GameTickTimer += DeltaTime;
        
        if (GameTickTimer >= GameTickInterval)
        {
            ProcessGameTick();
            GameTickTimer = 0.0f;
            CurrentTick++;
        }
    }
}

void ABloodreadGameMode::ProcessGameTick()
{
    // Process game tick for all players (now using base character class)
    TArray<AActor*> FoundActors;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), ABloodreadBaseCharacter::StaticClass(), FoundActors);
    
    for (AActor* Actor : FoundActors)
    {
        ABloodreadBaseCharacter* Player = Cast<ABloodreadBaseCharacter>(Actor);
        if (Player)
        {
            // Note: ProcessGameTick is not implemented in base class yet
            // You may want to add this method to ABloodreadBaseCharacter
            UE_LOG(LogTemp, VeryVerbose, TEXT("Processing game tick for character: %s"), *Player->GetName());
        }
    }
    
    // Also check for legacy BloodreadPlayerCharacter instances
    TArray<AActor*> LegacyActors;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), ABloodreadPlayerCharacter::StaticClass(), LegacyActors);
    
    for (AActor* Actor : LegacyActors)
    {
        ABloodreadPlayerCharacter* Player = Cast<ABloodreadPlayerCharacter>(Actor);
        if (Player)
        {
            Player->ProcessGameTick();
        }
    }
    
    // Process game tick for practice dummies
    TArray<AActor*> FoundDummies;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), APracticeDummy::StaticClass(), FoundDummies);
    
    for (AActor* Actor : FoundDummies)
    {
        APracticeDummy* Dummy = Cast<APracticeDummy>(Actor);
        if (Dummy)
        {
            Dummy->ProcessGameTick();
        }
    }
    
    // Call Blueprint event for game tick processing
    OnGameTick(CurrentTick);
}

void ABloodreadGameMode::StartMatch(bool bIsPractice, int32 OpponentPlayerId)
{
    bMatchInProgress = true;
    bIsPracticeMatch = bIsPractice;
    OpponentId = OpponentPlayerId;
    MatchStartTime = GetWorld()->GetTimeSeconds();
    CurrentTick = 0;
    GameTickTimer = 0.0f;
    
    UE_LOG(LogTemp, Log, TEXT("Match started - Practice: %s, Opponent ID: %d"), 
           bIsPractice ? TEXT("Yes") : TEXT("No"), OpponentPlayerId);
    
    OnMatchStarted(bIsPractice);
}

void ABloodreadGameMode::EndMatch(int32 WinnerPlayerId)
{
    if (!bMatchInProgress) return;
    
    bMatchInProgress = false;
    float MatchDuration = GetWorld()->GetTimeSeconds() - MatchStartTime;
    
    UE_LOG(LogTemp, Log, TEXT("Match ended - Winner: %d, Duration: %.2f seconds"), 
           WinnerPlayerId, MatchDuration);
    
    // Record match in database
    UBloodreadGameInstance* GameInstance = Cast<UBloodreadGameInstance>(GetGameInstance());
    if (GameInstance)
    {
        // TODO: Implement match recording for Steam multiplayer
        // You'll need to get actual damage values from your combat system
        int32 PlayerDamage = 0; // TODO: Get from combat system
        int32 OpponentDamage = 0; // TODO: Get from combat system
        
        // GameInstance->RecordMatch(OpponentId, WinnerPlayerId, 
        //     FMath::RoundToInt(MatchDuration), PlayerDamage, OpponentDamage, bIsPracticeMatch);
        
        UE_LOG(LogTemp, Warning, TEXT("Match ended - Winner: %d, Duration: %.2f seconds"), 
            WinnerPlayerId, MatchDuration);
    }
    
    OnMatchEnded(WinnerPlayerId);
}

void ABloodreadGameMode::ShowCharacterSelectionWidget()
{
    UE_LOG(LogTemp, Warning, TEXT("ShowCharacterSelectionWidget called"));
    
    // Get the first player controller
    APlayerController* PlayerController = GetWorld()->GetFirstPlayerController();
    if (!PlayerController)
    {
        UE_LOG(LogTemp, Error, TEXT("No PlayerController found"));
        return;
    }

    // Create the widget if we have a class set
    if (CharacterSelectionWidgetClass)
    {
        CharacterSelectionWidget = CreateWidget<UUserWidget>(GetWorld(), CharacterSelectionWidgetClass);
        if (CharacterSelectionWidget)
        {
            CharacterSelectionWidget->AddToViewport();
            
            // Set input mode to UI only
            PlayerController->SetInputMode(FInputModeUIOnly());
            PlayerController->SetShowMouseCursor(true);
            
            UE_LOG(LogTemp, Warning, TEXT("Character selection widget created and shown"));
        }
        else
        {
            UE_LOG(LogTemp, Error, TEXT("Failed to create character selection widget"));
        }
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("CharacterSelectionWidgetClass is not set"));
    }
}

void ABloodreadGameMode::HideCharacterSelectionWidget()
{
    UE_LOG(LogTemp, Warning, TEXT("HideCharacterSelectionWidget called"));
    
    // Get the first player controller
    APlayerController* PlayerController = GetWorld()->GetFirstPlayerController();
    if (!PlayerController)
    {
        UE_LOG(LogTemp, Error, TEXT("No PlayerController found"));
        return;
    }

    // Hide the widget if it exists
    if (CharacterSelectionWidget)
    {
        CharacterSelectionWidget->RemoveFromParent();
        CharacterSelectionWidget = nullptr;
        
        // Restore game input mode
        PlayerController->SetInputMode(FInputModeGameOnly());
        PlayerController->SetShowMouseCursor(false);
        
        UE_LOG(LogTemp, Warning, TEXT("Character selection widget hidden and input restored"));
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("No character selection widget to hide"));
    }
}

void ABloodreadGameMode::HandleCharacterSelection(int32 CharacterClassIndex)
{
    UE_LOG(LogTemp, Warning, TEXT("=== CHARACTER SELECTION STARTED ==="));
    UE_LOG(LogTemp, Warning, TEXT("Button pressed! Character index: %d"), CharacterClassIndex);
    
    // Get the first player controller
    APlayerController* PlayerController = GetWorld()->GetFirstPlayerController();
    if (!PlayerController)
    {
        UE_LOG(LogTemp, Error, TEXT("ERROR: No PlayerController found"));
        return;
    }

    // Convert index to enum (0=None, 1=Warrior, 2=Mage, 3=Rogue, 4=Healer, 5=Dragon)
    ECharacterClass SelectedClass = static_cast<ECharacterClass>(CharacterClassIndex);
    FString ClassName;
    switch (CharacterClassIndex)
    {
        case 0: 
            ClassName = "None"; 
            break;
        case 1: 
            ClassName = "Warrior"; 
            break;
        case 2: 
            ClassName = "Mage"; 
            break;
        case 3: 
            ClassName = "Rogue"; 
            break;
        case 4: 
            ClassName = "Healer"; 
            break;
        case 5: 
            ClassName = "Dragon"; 
            break;
        default: 
            SelectedClass = ECharacterClass::None;
            ClassName = "Unknown"; 
            break;
    }
    UE_LOG(LogTemp, Warning, TEXT("Selected Character: %s (Index: %d)"), *ClassName, CharacterClassIndex);
    
    // Transition to in-game state (this will hide all UI)
    UE_LOG(LogTemp, Warning, TEXT("Transitioning to In Game state..."));
    SetGameState(EGameState::InGame);
    
    // Small delay before spawning to ensure UI cleanup
    GetWorld()->GetTimerManager().SetTimerForNextTick([this, SelectedClass, PlayerController, ClassName]()
    {
        UE_LOG(LogTemp, Warning, TEXT("UI hidden, now spawning %s character..."), *ClassName);
        SpawnSelectedCharacter(SelectedClass, PlayerController);
    });
}

void ABloodreadGameMode::SpawnSelectedCharacter(ECharacterClass SelectedClass, APlayerController* PlayerController)
{
    if (!PlayerController)
    {
        UE_LOG(LogTemp, Error, TEXT("SpawnSelectedCharacter: PlayerController is null"));
        return;
    }

    // Find a spawn location
    FVector SpawnLocation = FVector(0, 0, 200); // Default spawn location
    FRotator SpawnRotation = FRotator::ZeroRotator;

    // Try to find a PlayerStart
    AActor* PlayerStart = UGameplayStatics::GetActorOfClass(GetWorld(), APlayerStart::StaticClass());
    if (PlayerStart)
    {
        SpawnLocation = PlayerStart->GetActorLocation();
        SpawnRotation = PlayerStart->GetActorRotation();
        UE_LOG(LogTemp, Warning, TEXT("Using PlayerStart location: %s"), *SpawnLocation.ToString());
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("No PlayerStart found, using default location"));
    }

    // Destroy existing pawn if any
    APawn* CurrentPawn = PlayerController->GetPawn();
    if (CurrentPawn)
    {
        PlayerController->UnPossess(); // Unpossess first
        CurrentPawn->Destroy();
        UE_LOG(LogTemp, Warning, TEXT("Destroyed existing player pawn"));
        
        // Wait a frame for destruction to complete
        GetWorld()->GetTimerManager().SetTimerForNextTick([this, SelectedClass, PlayerController, SpawnLocation, SpawnRotation]()
        {
            SpawnNewCharacter(SelectedClass, PlayerController, SpawnLocation, SpawnRotation);
        });
    }
    else
    {
        SpawnNewCharacter(SelectedClass, PlayerController, SpawnLocation, SpawnRotation);
    }
}

ABloodreadBaseCharacter* ABloodreadGameMode::CreateCharacterOfClass(ECharacterClass CharacterClass, FVector SpawnLocation, FRotator SpawnRotation)
{
    if (!CharacterSelectionManager)
    {
        UE_LOG(LogTemp, Error, TEXT("CharacterSelectionManager is null"));
        return nullptr;
    }

    // Use the Character Selection Manager to spawn the character
    ABloodreadBaseCharacter* SpawnedCharacter = CharacterSelectionManager->SpawnCharacterOfClass(
        GetWorld(), CharacterClass, SpawnLocation, SpawnRotation);

    if (SpawnedCharacter)
    {
        UE_LOG(LogTemp, Warning, TEXT("Successfully created character of class: %d at location: %s"), 
               (int32)CharacterClass, *SpawnLocation.ToString());
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to create character of class: %d"), (int32)CharacterClass);
    }

    return SpawnedCharacter;
}

void ABloodreadGameMode::SpawnNewCharacter(ECharacterClass SelectedClass, APlayerController* PlayerController, FVector SpawnLocation, FRotator SpawnRotation)
{
    UE_LOG(LogTemp, Warning, TEXT("=== SPAWNING NEW CHARACTER ==="));
    
    // Spawn the selected character
    ABloodreadBaseCharacter* NewCharacter = CreateCharacterOfClass(SelectedClass, SpawnLocation, SpawnRotation);
    
    if (NewCharacter)
    {
        UE_LOG(LogTemp, Warning, TEXT("Character spawned successfully, setting up possession..."));
        
        // Ensure character is properly set up BEFORE possession
        NewCharacter->SetActorTickEnabled(true);
        NewCharacter->SetActorEnableCollision(true);
        
        // Small delay before possession to ensure character is fully initialized
        GetWorld()->GetTimerManager().SetTimer(PossessionTimerHandle, [this, PlayerController, NewCharacter, SelectedClass]()
        {
            UE_LOG(LogTemp, Warning, TEXT("Starting possession process..."));
            
            // Possess the character FIRST
            PlayerController->Possess(NewCharacter);
            UE_LOG(LogTemp, Warning, TEXT("Character possessed by controller"));
            
            // NOW enable input AFTER possession
            NewCharacter->EnableInput(PlayerController);
            UE_LOG(LogTemp, Warning, TEXT("Input enabled on possessed character"));
            
            // Set up input context after possession
            NewCharacter->SetupInputContext();
            UE_LOG(LogTemp, Warning, TEXT("Input context setup called"));
            
            // Show the player HUD now that character is ready
            ShowPlayerHUD();
            UE_LOG(LogTemp, Warning, TEXT("Player HUD displayed"));
            
            // Ensure input mode is set to game only
            PlayerController->SetInputMode(FInputModeGameOnly());
            PlayerController->SetShowMouseCursor(false);
            
            UE_LOG(LogTemp, Warning, TEXT("=== CHARACTER POSSESSION COMPLETE ==="));
            UE_LOG(LogTemp, Warning, TEXT("Class: %d, Location: %s"), (int32)SelectedClass, *NewCharacter->GetActorLocation().ToString());
            UE_LOG(LogTemp, Warning, TEXT("Input Mode: Game Only, Mouse Cursor: Hidden"));
            UE_LOG(LogTemp, Warning, TEXT("✅ FIXED: EnableInput AND SetupInputContext called AFTER possession!"));
            UE_LOG(LogTemp, Warning, TEXT("Player can now move and look around!"));
            
        }, 0.2f, false); // Increased delay to ensure everything is ready
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("=== FAILED TO SPAWN CHARACTER ==="));
        UE_LOG(LogTemp, Error, TEXT("Failed to spawn character of class: %d"), (int32)SelectedClass);
    }
}

void ABloodreadGameMode::ShowPlayerHUD()
{
    if (!PlayerHUDWidgetClass)
    {
        UE_LOG(LogTemp, Warning, TEXT("PlayerHUDWidgetClass is not set in GameMode"));
        return;
    }

    // Create HUD widget if it doesn't exist
    if (!PlayerHUDWidget)
    {
        PlayerHUDWidget = CreateWidget<UUserWidget>(GetWorld(), PlayerHUDWidgetClass);
        if (PlayerHUDWidget)
        {
            UE_LOG(LogTemp, Warning, TEXT("Player HUD widget created successfully"));
        }
        else
        {
            UE_LOG(LogTemp, Error, TEXT("Failed to create Player HUD widget"));
            return;
        }
    }

    // Add to viewport if not already shown
    if (PlayerHUDWidget && !PlayerHUDWidget->IsInViewport())
    {
        PlayerHUDWidget->AddToViewport();
        UE_LOG(LogTemp, Warning, TEXT("Player HUD widget shown"));
    }
}

void ABloodreadGameMode::HidePlayerHUD()
{
    if (PlayerHUDWidget && PlayerHUDWidget->IsInViewport())
    {
        PlayerHUDWidget->RemoveFromParent();
        UE_LOG(LogTemp, Warning, TEXT("Player HUD widget hidden"));
    }
}

APawn* ABloodreadGameMode::SpawnDefaultPawnFor_Implementation(AController* NewPlayer, AActor* StartSpot)
{
    UE_LOG(LogTemp, Warning, TEXT("=== SpawnDefaultPawnFor called ==="));
    UE_LOG(LogTemp, Warning, TEXT("Deferring character spawn until character selection"));
    
    // Don't spawn any character initially - wait for character selection
    // This allows the UI to show character selection first
    return nullptr;
}

void ABloodreadGameMode::SpawnCharacterForPlayer(int32 CharacterClassIndex, APlayerController* PlayerController)
{
    UE_LOG(LogTemp, Warning, TEXT("=== SpawnCharacterForPlayer called ==="));
    
    // Convert index to character class (matches enum values: 0=None, 1=Warrior, 2=Mage, 3=Rogue, 4=Healer, 5=Dragon)
    ECharacterClass SelectedClass = static_cast<ECharacterClass>(CharacterClassIndex);
    FString ClassName = "Unknown";
    
    switch (CharacterClassIndex)
    {
        case 1:
            ClassName = "Warrior";
            break;
        case 2:
            ClassName = "Mage";
            break;
        case 3:
            ClassName = "Rogue";
            break;
        case 4:
            ClassName = "Healer";
            break;
        case 5:
            ClassName = "Dragon";
            break;
        default:
            UE_LOG(LogTemp, Error, TEXT("Invalid character class index: %d"), CharacterClassIndex);
            return;
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Player selected %s (class %d)"), *ClassName, CharacterClassIndex);
    
    // Get the player controller (use provided one or get first player)
    APlayerController* TargetController = PlayerController;
    if (!TargetController)
    {
        TargetController = GetWorld()->GetFirstPlayerController();
    }
    
    if (!TargetController)
    {
        UE_LOG(LogTemp, Error, TEXT("No player controller found!"));
        return;
    }
    
    // Find a spawn location (use player start or default)
    FVector SpawnLocation = FVector(0, 0, 100); // Default spawn location
    FRotator SpawnRotation = FRotator::ZeroRotator;
    
    AActor* PlayerStart = FindPlayerStart(TargetController);
    if (PlayerStart)
    {
        SpawnLocation = PlayerStart->GetActorLocation();
        SpawnRotation = PlayerStart->GetActorRotation();
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Spawning %s at location: %s"), *ClassName, *SpawnLocation.ToString());
    
    // Destroy existing pawn if any
    if (TargetController->GetPawn())
    {
        UE_LOG(LogTemp, Warning, TEXT("Destroying existing pawn"));
        TargetController->GetPawn()->Destroy();
    }
    
    // Spawn the selected character
    ABloodreadBaseCharacter* NewCharacter = CreateCharacterOfClass(SelectedClass, SpawnLocation, SpawnRotation);
    
    if (NewCharacter)
    {
        UE_LOG(LogTemp, Warning, TEXT("Successfully spawned %s: %s"), *ClassName, *NewCharacter->GetName());
        
        // Possess the new character
        TargetController->Possess(NewCharacter);
        UE_LOG(LogTemp, Warning, TEXT("Player controller now possessing %s"), *NewCharacter->GetName());
        
        // Hide character selection UI (if it exists)
        HideCharacterSelectionWidget();
        
        // Show game HUD
        ShowPlayerHUD();
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to spawn %s!"), *ClassName);
    }
}

// Legacy team management functions (placeholders)
void ABloodreadGameMode::SpawnAITeammates(int32 TeamSize, int32 TeamId)
{
    UE_LOG(LogTemp, Warning, TEXT("SpawnAITeammates called - not implemented. TeamSize: %d, TeamId: %d"), TeamSize, TeamId);
}

void ABloodreadGameMode::AssignPlayerToTeam(ABloodreadBaseCharacter* PlayerCharacter)
{
    UE_LOG(LogTemp, Warning, TEXT("AssignPlayerToTeam called - not implemented. Player: %s"), PlayerCharacter ? *PlayerCharacter->GetName() : TEXT("None"));
}

void ABloodreadGameMode::SetupBalancedMatch()
{
    UE_LOG(LogTemp, Warning, TEXT("SetupBalancedMatch called - not implemented"));
}

void ABloodreadGameMode::InitializeTeamManager()
{
    UE_LOG(LogTemp, Warning, TEXT("InitializeTeamManager called - not implemented"));
}

// Multiplayer session management functions
void ABloodreadGameMode::PostLogin(APlayerController* NewPlayer)
{
    Super::PostLogin(NewPlayer);
    
    ConnectedPlayers++;
    UE_LOG(LogTemp, Warning, TEXT("Player joined! Total players: %d/%d"), ConnectedPlayers, MaxPlayers);
    
    // Call Blueprint event
    OnPlayerJoined(ConnectedPlayers, MaxPlayers);
    
    // Auto-start match when enough players join
    if (bAutoStartWhenReady && ConnectedPlayers >= MinPlayersToStart)
    {
        UE_LOG(LogTemp, Warning, TEXT("Enough players connected, starting multiplayer match!"));
        StartMultiplayerMatch();
    }
}

void ABloodreadGameMode::Logout(AController* Exiting)
{
    Super::Logout(Exiting);
    
    ConnectedPlayers = FMath::Max(0, ConnectedPlayers - 1);
    UE_LOG(LogTemp, Warning, TEXT("Player left! Total players: %d/%d"), ConnectedPlayers, MaxPlayers);
    
    // Call Blueprint event
    OnPlayerLeft(ConnectedPlayers, MaxPlayers);
}

void ABloodreadGameMode::StartMultiplayerMatch()
{
    UE_LOG(LogTemp, Warning, TEXT("Starting multiplayer match with %d players!"), ConnectedPlayers);
    
    // Initialize all connected players
    for (FConstPlayerControllerIterator Iterator = GetWorld()->GetPlayerControllerIterator(); Iterator; ++Iterator)
    {
        if (ABloodreadGamePlayerController* PC = Cast<ABloodreadGamePlayerController>(*Iterator))
        {
            // Give players time to select characters or spawn with default
            UE_LOG(LogTemp, Warning, TEXT("Initializing player controller for multiplayer: %s"), *PC->GetName());
        }
    }
    
    // Start the match (can be overridden in Blueprint)
    StartMatch(false, 0);
}

int32 ABloodreadGameMode::GetConnectedPlayerCount()
{
    return ConnectedPlayers;
}

// ============================================================================
// GAME STATE MANAGEMENT
// ============================================================================

void ABloodreadGameMode::SetGameState(EGameState NewState)
{
    if (CurrentGameState == NewState)
    {
        UE_LOG(LogTemp, Warning, TEXT("Game State already %d - no change needed"), (int32)NewState);
        return; // No change needed
    }

    UE_LOG(LogTemp, Warning, TEXT("=== GAME STATE CHANGE: %d -> %d ==="), (int32)CurrentGameState, (int32)NewState);
    CurrentGameState = NewState;

    // Handle state transitions
    switch (NewState)
    {
        case EGameState::MultiplayerLobby:
            UE_LOG(LogTemp, Warning, TEXT("Entering Multiplayer Lobby state - calling ShowMultiplayerLobby"));
            ShowMultiplayerLobby();
            break;
            
        case EGameState::CharacterSelection:
            UE_LOG(LogTemp, Warning, TEXT("Entering Character Selection state"));
            ShowCharacterSelectionWidget();
            break;
            
        case EGameState::InGame:
            UE_LOG(LogTemp, Warning, TEXT("Entering In Game state"));
            HideCharacterSelectionWidget();
            HideMultiplayerLobby();
            break;
            
        default:
            UE_LOG(LogTemp, Warning, TEXT("Unknown game state: %d"), (int32)NewState);
            break;
    }
    
    UE_LOG(LogTemp, Warning, TEXT("=== GAME STATE CHANGE COMPLETE ==="));
}

// Special function for Blueprint GameMode to call
void ABloodreadGameMode::InitializeGameModeState()
{
    UE_LOG(LogTemp, Warning, TEXT("=== InitializeGameModeState called from Blueprint ==="));
    UE_LOG(LogTemp, Warning, TEXT("This function allows Blueprint GameMode to trigger C++ logic"));
    
    // Initialize game tick timer
    GameTickTimer = 0.0f;
    CurrentTick = 0;
    
    // Initialize Character Selection Manager if not already created
    if (!CharacterSelectionManager)
    {
        CharacterSelectionManager = NewObject<UCharacterSelectionManager>(this);
        UE_LOG(LogTemp, Warning, TEXT("Created Character Selection Manager in InitializeGameModeState"));
    }

    // Start in multiplayer lobby state
    UE_LOG(LogTemp, Warning, TEXT("Setting game state to MultiplayerLobby from Blueprint call"));
    SetGameState(EGameState::MultiplayerLobby);
    
    UE_LOG(LogTemp, Warning, TEXT("=== InitializeGameModeState completed ==="));
}

// Simple function to show lobby widget immediately
void ABloodreadGameMode::ShowLobbyWidgetNow()
{
    UE_LOG(LogTemp, Warning, TEXT("=== ShowLobbyWidgetNow called from Blueprint BeginPlay ==="));
    
    // Get the first player controller
    APlayerController* PlayerController = GetWorld()->GetFirstPlayerController();
    if (!PlayerController)
    {
        UE_LOG(LogTemp, Error, TEXT("No PlayerController found - cannot show lobby"));
        return;
    }

    UE_LOG(LogTemp, Warning, TEXT("PlayerController found: %s"), *PlayerController->GetName());

    // Create and show the multiplayer lobby widget using the Blueprint class if set
    if (MultiplayerLobbyWidgetClass)
    {
        UE_LOG(LogTemp, Warning, TEXT("Creating MultiplayerLobbyWidget using Blueprint class: %s"), *MultiplayerLobbyWidgetClass->GetName());
        MultiplayerLobbyWidget = CreateWidget<UMultiplayerLobbyWidget>(GetWorld(), MultiplayerLobbyWidgetClass);
        
        if (MultiplayerLobbyWidget)
        {
            UE_LOG(LogTemp, Warning, TEXT("Blueprint widget created successfully! Adding to viewport"));
            MultiplayerLobbyWidget->AddToViewport();
            
            // Set input mode to UI only
            PlayerController->SetInputMode(FInputModeUIOnly());
            PlayerController->SetShowMouseCursor(true);
            
            UE_LOG(LogTemp, Warning, TEXT("✅ WBP_MULTIPLAYERLOBBY SHOWN! Input mode: UI Only, Mouse cursor: Visible"));
            UE_LOG(LogTemp, Warning, TEXT("Your Blueprint lobby widget should now be visible on screen!"));
        }
        else
        {
            UE_LOG(LogTemp, Error, TEXT("❌ FAILED to create widget from Blueprint class: %s"), *MultiplayerLobbyWidgetClass->GetName());
        }
    }
    else
    {
        // Fallback to C++ class if no Blueprint class is set
        UE_LOG(LogTemp, Warning, TEXT("No Blueprint MultiplayerLobbyWidgetClass set! Using C++ fallback"));
        UE_LOG(LogTemp, Warning, TEXT("You should set MultiplayerLobbyWidgetClass to WBP_MultiplayerLobby in your GameMode Blueprint"));
        
        MultiplayerLobbyWidget = CreateWidget<UMultiplayerLobbyWidget>(GetWorld(), UMultiplayerLobbyWidget::StaticClass());
        if (MultiplayerLobbyWidget)
        {
            UE_LOG(LogTemp, Warning, TEXT("C++ fallback widget created, adding to viewport"));
            MultiplayerLobbyWidget->AddToViewport();
            
            // Set input mode to UI only
            PlayerController->SetInputMode(FInputModeUIOnly());
            PlayerController->SetShowMouseCursor(true);
            
            UE_LOG(LogTemp, Warning, TEXT("✅ C++ FALLBACK WIDGET SHOWN! Input mode: UI Only, Mouse cursor: Visible"));
        }
        else
        {
            UE_LOG(LogTemp, Error, TEXT("❌ FAILED to create C++ fallback widget"));
        }
    }
    
    UE_LOG(LogTemp, Warning, TEXT("=== ShowLobbyWidgetNow completed ==="));
}

// ============================================================================
// MULTIPLAYER LOBBY MANAGEMENT
// ============================================================================

void ABloodreadGameMode::ShowMultiplayerLobby()
{
    UE_LOG(LogTemp, Warning, TEXT("=== GameMode: ShowMultiplayerLobby called ==="));
    
    // Get the first player controller
    APlayerController* PlayerController = GetWorld()->GetFirstPlayerController();
    if (!PlayerController)
    {
        UE_LOG(LogTemp, Error, TEXT("No PlayerController found for lobby"));
        return;
    }

    UE_LOG(LogTemp, Warning, TEXT("PlayerController found: %s"), *PlayerController->GetName());

    // Create the lobby widget directly in GameMode (same pattern as character selection)
    if (MultiplayerLobbyWidgetClass)
    {
        UE_LOG(LogTemp, Warning, TEXT("Using Blueprint MultiplayerLobbyWidgetClass: %s"), *MultiplayerLobbyWidgetClass->GetName());
        MultiplayerLobbyWidget = CreateWidget<UMultiplayerLobbyWidget>(GetWorld(), MultiplayerLobbyWidgetClass);
        if (MultiplayerLobbyWidget)
        {
            UE_LOG(LogTemp, Warning, TEXT("Widget created successfully, adding to viewport"));
            MultiplayerLobbyWidget->AddToViewport();
            
            // Set input mode to UI only
            PlayerController->SetInputMode(FInputModeUIOnly());
            PlayerController->SetShowMouseCursor(true);
            
            UE_LOG(LogTemp, Warning, TEXT("Multiplayer lobby widget created and shown, input mode set to UI"));
        }
        else
        {
            UE_LOG(LogTemp, Error, TEXT("Failed to create multiplayer lobby widget from Blueprint class"));
        }
    }
    else
    {
        // Fallback to C++ class if no Blueprint class is set
        UE_LOG(LogTemp, Warning, TEXT("No Blueprint class set, using C++ MultiplayerLobbyWidget fallback"));
        MultiplayerLobbyWidget = CreateWidget<UMultiplayerLobbyWidget>(GetWorld(), UMultiplayerLobbyWidget::StaticClass());
        if (MultiplayerLobbyWidget)
        {
            UE_LOG(LogTemp, Warning, TEXT("C++ widget created successfully, adding to viewport"));
            MultiplayerLobbyWidget->AddToViewport();
            
            // Set input mode to UI only
            PlayerController->SetInputMode(FInputModeUIOnly());
            PlayerController->SetShowMouseCursor(true);
            
            UE_LOG(LogTemp, Warning, TEXT("Multiplayer lobby widget created using C++ class fallback and shown"));
        }
        else
        {
            UE_LOG(LogTemp, Error, TEXT("Failed to create multiplayer lobby widget (C++ fallback)"));
        }
    }
    
    UE_LOG(LogTemp, Warning, TEXT("=== GameMode: ShowMultiplayerLobby completed ==="));
}

void ABloodreadGameMode::HideMultiplayerLobby()
{
    UE_LOG(LogTemp, Warning, TEXT("GameMode: HideMultiplayerLobby called"));
    
    APlayerController* PlayerController = GetWorld()->GetFirstPlayerController();
    if (!PlayerController)
    {
        return;
    }

    // Hide the lobby widget directly in GameMode (same pattern as character selection)
    if (MultiplayerLobbyWidget && IsValid(MultiplayerLobbyWidget))
    {
        MultiplayerLobbyWidget->RemoveFromParent();
        MultiplayerLobbyWidget = nullptr;
        
        // Set input mode back to game
        PlayerController->SetInputMode(FInputModeGameOnly());
        PlayerController->SetShowMouseCursor(false);
        
        UE_LOG(LogTemp, Warning, TEXT("Multiplayer lobby widget hidden"));
    }
}

void ABloodreadGameMode::OnSessionCreated(bool bSuccess)
{
    UE_LOG(LogTemp, Warning, TEXT("GameMode: Session creation %s"), bSuccess ? TEXT("SUCCESS") : TEXT("FAILED"));
    
    if (bSuccess)
    {
        // Session created successfully, transition to character selection
        UE_LOG(LogTemp, Warning, TEXT("Transitioning to Character Selection after session creation"));
        SetGameState(EGameState::CharacterSelection);
    }
    else
    {
        // Session creation failed, stay in lobby
        UE_LOG(LogTemp, Error, TEXT("Session creation failed, staying in lobby"));
    }
}

void ABloodreadGameMode::OnSessionJoined(bool bSuccess)
{
    UE_LOG(LogTemp, Warning, TEXT("GameMode: Session join %s"), bSuccess ? TEXT("SUCCESS") : TEXT("FAILED"));
    
    if (bSuccess)
    {
        // Session joined successfully, transition to character selection
        UE_LOG(LogTemp, Warning, TEXT("Transitioning to Character Selection after joining session"));
        SetGameState(EGameState::CharacterSelection);
    }
    else
    {
        // Session join failed, stay in lobby
        UE_LOG(LogTemp, Error, TEXT("Session join failed, staying in lobby"));
    }
}
