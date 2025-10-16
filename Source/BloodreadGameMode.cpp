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
    
    // Initialize Character Selection Manager if not already created
    if (!CharacterSelectionManager)
    {
        CharacterSelectionManager = NewObject<UCharacterSelectionManager>(this);
        UE_LOG(LogTemp, Warning, TEXT("Created Character Selection Manager in BeginPlay"));
    }
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
    UE_LOG(LogTemp, Warning, TEXT("GameMode: HideCharacterSelectionWidget called"));
    
    // Get the first player controller
    APlayerController* PlayerController = GetWorld()->GetFirstPlayerController();
    if (!PlayerController)
    {
        UE_LOG(LogTemp, Error, TEXT("GameMode: No PlayerController found"));
        return;
    }

    // Cast to our custom player controller and call its hide function
    ABloodreadGamePlayerController* BloodreadController = Cast<ABloodreadGamePlayerController>(PlayerController);
    if (BloodreadController)
    {
        UE_LOG(LogTemp, Warning, TEXT("GameMode: Delegating character selection widget hiding to PlayerController"));
        BloodreadController->HideCharacterSelectionWidget();
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("GameMode: PlayerController is not BloodreadGamePlayerController"));
    }

    // Also hide the GameMode's widget if it exists (fallback)
    if (CharacterSelectionWidget)
    {
        UE_LOG(LogTemp, Warning, TEXT("GameMode: Also hiding GameMode's character selection widget"));
        CharacterSelectionWidget->RemoveFromParent();
        CharacterSelectionWidget = nullptr;
        
        // Restore game input mode
        PlayerController->SetInputMode(FInputModeGameOnly());
        PlayerController->SetShowMouseCursor(false);
        
        UE_LOG(LogTemp, Warning, TEXT("GameMode: Character selection widget hidden and input restored"));
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("GameMode: No GameMode character selection widget to hide"));
    }
}

void ABloodreadGameMode::HandleCharacterSelection(int32 CharacterClassIndex, APlayerController* PlayerController)
{
    UE_LOG(LogTemp, Error, TEXT("🚨🚨🚨 HANDLE CHARACTER SELECTION CALLED 🚨🚨🚨"));
    UE_LOG(LogTemp, Warning, TEXT("=== CHARACTER SELECTION STARTED ==="));
    UE_LOG(LogTemp, Warning, TEXT("Button pressed! Character index: %d"), CharacterClassIndex);
    UE_LOG(LogTemp, Error, TEXT("🚨 GAMEMODE: Received PlayerController parameter: %s 🚨"), PlayerController ? *PlayerController->GetName() : TEXT("NULL"));
    UE_LOG(LogTemp, Warning, TEXT("GAMEMODE: PlayerController pointer address: %p"), PlayerController);
    UE_LOG(LogTemp, Warning, TEXT("GAMEMODE: PlayerController IsValid: %s"), IsValid(PlayerController) ? TEXT("TRUE") : TEXT("FALSE"));
    
    // Enhanced PlayerController logging
    if (PlayerController)
    {
        UE_LOG(LogTemp, Warning, TEXT("GAMEMODE: PlayerController provided: %s"), *PlayerController->GetName());
        UE_LOG(LogTemp, Warning, TEXT("GAMEMODE: PlayerController Class: %s"), *PlayerController->GetClass()->GetName());
        UE_LOG(LogTemp, Warning, TEXT("GAMEMODE: PlayerController HasAuthority: %s"), PlayerController->HasAuthority() ? TEXT("TRUE") : TEXT("FALSE"));
        UE_LOG(LogTemp, Warning, TEXT("GAMEMODE: PlayerController IsLocalController: %s"), PlayerController->IsLocalController() ? TEXT("TRUE") : TEXT("FALSE"));
        
        // Check current pawn
        APawn* CurrentPawn = PlayerController->GetPawn();
        if (CurrentPawn)
        {
            UE_LOG(LogTemp, Warning, TEXT("GAMEMODE: PlayerController already has pawn: %s"), *CurrentPawn->GetName());
        }
        else
        {
            UE_LOG(LogTemp, Warning, TEXT("GAMEMODE: PlayerController has no current pawn"));
        }
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("GAMEMODE: No PlayerController provided, using first player controller"));
        PlayerController = GetWorld()->GetFirstPlayerController();
        if (PlayerController)
        {
            UE_LOG(LogTemp, Warning, TEXT("GAMEMODE: Found first player controller: %s"), *PlayerController->GetName());
        }
    }
    
    if (!PlayerController)
    {
        UE_LOG(LogTemp, Error, TEXT("ERROR: No PlayerController found"));
        return;
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Using PlayerController: %s"), *PlayerController->GetName());

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
    
    UE_LOG(LogTemp, Error, TEXT("🚨 ABOUT TO SET SPAWN TIMER 🚨"));
    
    // Small delay before spawning to ensure UI cleanup
    GetWorld()->GetTimerManager().SetTimerForNextTick([this, SelectedClass, PlayerController, ClassName, CharacterClassIndex]()
    {
        UE_LOG(LogTemp, Error, TEXT("🚨 SPAWN TIMER EXECUTED 🚨"));
        UE_LOG(LogTemp, Warning, TEXT("UI hidden, now spawning %s character..."), *ClassName);
        UE_LOG(LogTemp, Warning, TEXT("SPAWN TIMER: PlayerController: %s"), PlayerController ? *PlayerController->GetName() : TEXT("NULL"));
        UE_LOG(LogTemp, Warning, TEXT("SPAWN TIMER: About to call SpawnSelectedCharacter"));
        SpawnSelectedCharacter(SelectedClass, PlayerController, CharacterClassIndex);
    });
}

void ABloodreadGameMode::SpawnSelectedCharacter(ECharacterClass SelectedClass, APlayerController* PlayerController, int32 CharacterClassIndex)
{
    UE_LOG(LogTemp, Error, TEXT("🚨 SPAWN SELECTED CHARACTER CALLED 🚨"));
    UE_LOG(LogTemp, Warning, TEXT("SPAWN SELECTED: PlayerController: %s"), PlayerController ? *PlayerController->GetName() : TEXT("NULL"));
    UE_LOG(LogTemp, Warning, TEXT("SPAWN SELECTED: SelectedClass: %d"), (int32)SelectedClass);
    
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
        GetWorld()->GetTimerManager().SetTimerForNextTick([this, SelectedClass, PlayerController, SpawnLocation, SpawnRotation, CharacterClassIndex]()
        {
            SpawnNewCharacter(SelectedClass, PlayerController, SpawnLocation, SpawnRotation, CharacterClassIndex);
        });
    }
    else
    {
        SpawnNewCharacter(SelectedClass, PlayerController, SpawnLocation, SpawnRotation, CharacterClassIndex);
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

void ABloodreadGameMode::SpawnNewCharacter(ECharacterClass SelectedClass, APlayerController* PlayerController, FVector SpawnLocation, FRotator SpawnRotation, int32 CharacterClassIndex)
{
    UE_LOG(LogTemp, Error, TEXT("🚨 SPAWN NEW CHARACTER CALLED 🚨"));
    UE_LOG(LogTemp, Warning, TEXT("=== SPAWNING NEW CHARACTER ==="));
    UE_LOG(LogTemp, Warning, TEXT("SPAWN: PlayerController: %s"), PlayerController ? *PlayerController->GetName() : TEXT("NULL"));
    UE_LOG(LogTemp, Warning, TEXT("SPAWN: SelectedClass: %d"), (int32)SelectedClass);
    
    // Spawn the selected character
    ABloodreadBaseCharacter* NewCharacter = CreateCharacterOfClass(SelectedClass, SpawnLocation, SpawnRotation);
    
    if (NewCharacter)
    {
        UE_LOG(LogTemp, Warning, TEXT("Character spawned successfully, setting up possession..."));
        UE_LOG(LogTemp, Warning, TEXT("SPAWN: NewCharacter: %s"), *NewCharacter->GetName());
        UE_LOG(LogTemp, Warning, TEXT("SPAWN: NewCharacter bReplicates: %s"), NewCharacter->GetIsReplicated() ? TEXT("TRUE") : TEXT("FALSE"));
        UE_LOG(LogTemp, Warning, TEXT("SPAWN: NewCharacter NetMode: %d"), (int32)NewCharacter->GetNetMode());
        
        // Ensure character is properly set up BEFORE possession
        NewCharacter->SetActorTickEnabled(true);
        NewCharacter->SetActorEnableCollision(true);
        
        // Force replication settings
        NewCharacter->SetReplicates(true);
        NewCharacter->SetReplicateMovement(true);
        UE_LOG(LogTemp, Warning, TEXT("SPAWN: Forced replication settings on character"));
        
        UE_LOG(LogTemp, Error, TEXT("🚨 ABOUT TO SET POSSESSION TIMER 🚨"));
        
        // Small delay before possession to ensure character is fully initialized
        GetWorld()->GetTimerManager().SetTimer(PossessionTimerHandle, [this, PlayerController, NewCharacter, SelectedClass, CharacterClassIndex]()
        {
            UE_LOG(LogTemp, Error, TEXT("🚨🚨🚨 POSSESSION TIMER EXECUTED 🚨🚨🚨"));
            UE_LOG(LogTemp, Warning, TEXT("=== POSSESSION TIMER STARTED ==="));
            UE_LOG(LogTemp, Warning, TEXT("POSSESSION: PlayerController: %s"), PlayerController ? *PlayerController->GetName() : TEXT("NULL"));
            UE_LOG(LogTemp, Warning, TEXT("POSSESSION: NewCharacter: %s"), NewCharacter ? *NewCharacter->GetName() : TEXT("NULL"));
            UE_LOG(LogTemp, Warning, TEXT("POSSESSION: PlayerController IsValid: %s"), IsValid(PlayerController) ? TEXT("TRUE") : TEXT("FALSE"));
            UE_LOG(LogTemp, Warning, TEXT("POSSESSION: NewCharacter IsValid: %s"), IsValid(NewCharacter) ? TEXT("TRUE") : TEXT("FALSE"));
            
            if (!PlayerController)
            {
                UE_LOG(LogTemp, Error, TEXT("POSSESSION: PlayerController is NULL in timer!"));
                return;
            }
            
            if (!NewCharacter)
            {
                UE_LOG(LogTemp, Error, TEXT("POSSESSION: NewCharacter is NULL in timer!"));
                return;
            }
            
            UE_LOG(LogTemp, Warning, TEXT("Starting possession process..."));
            
            // Check if PlayerController already has a pawn
            APawn* CurrentPawn = PlayerController->GetPawn();
            if (CurrentPawn)
            {
                UE_LOG(LogTemp, Warning, TEXT("POSSESSION: PlayerController already possesses: %s"), *CurrentPawn->GetName());
            }
            else
            {
                UE_LOG(LogTemp, Warning, TEXT("POSSESSION: PlayerController has no current pawn"));
            }
            
            // Possess the character FIRST
            UE_LOG(LogTemp, Error, TEXT("🚨🚨🚨 ABOUT TO POSSESS 🚨🚨🚨"));
            UE_LOG(LogTemp, Warning, TEXT("POSSESSION: About to call PlayerController->Possess()"));
            UE_LOG(LogTemp, Warning, TEXT("POSSESSION: PlayerController Name: %s"), *PlayerController->GetName());
            UE_LOG(LogTemp, Warning, TEXT("POSSESSION: PlayerController IsLocalController: %s"), PlayerController->IsLocalController() ? TEXT("TRUE") : TEXT("FALSE"));
            UE_LOG(LogTemp, Warning, TEXT("POSSESSION: NewCharacter Name: %s"), *NewCharacter->GetName());
            UE_LOG(LogTemp, Warning, TEXT("POSSESSION: NewCharacter bReplicates: %s"), NewCharacter->GetIsReplicated() ? TEXT("TRUE") : TEXT("FALSE"));
            UE_LOG(LogTemp, Warning, TEXT("POSSESSION: NewCharacter NetMode: %d"), (int32)NewCharacter->GetNetMode());
            PlayerController->Possess(NewCharacter);
            UE_LOG(LogTemp, Error, TEXT("🚨🚨🚨 POSSESS CALL COMPLETED 🚨🚨🚨"));
            UE_LOG(LogTemp, Warning, TEXT("POSSESSION: PlayerController->Possess() completed"));
            
            // Force a second possession attempt with delay to ensure replication
            GetWorld()->GetTimerManager().SetTimer(RepossessionTimerHandle, [this, PlayerController, NewCharacter]()
            {
                UE_LOG(LogTemp, Warning, TEXT("REPOSSESSION: Attempting second possession to force replication"));
                UE_LOG(LogTemp, Warning, TEXT("REPOSSESSION: PlayerController: %s"), *PlayerController->GetName());
                UE_LOG(LogTemp, Warning, TEXT("REPOSSESSION: Character: %s"), *NewCharacter->GetName());
                
                // Force unpossess and re-possess to trigger replication
                PlayerController->UnPossess();
                UE_LOG(LogTemp, Warning, TEXT("REPOSSESSION: Unpossessed"));
                
                // Small delay before re-possessing
                GetWorld()->GetTimerManager().SetTimerForNextTick([this, PlayerController, NewCharacter]()
                {
                    PlayerController->Possess(NewCharacter);
                    UE_LOG(LogTemp, Warning, TEXT("REPOSSESSION: Re-possessed character"));
                });
                
            }, 0.5f, false);
            
            // Verify possession worked
            APawn* NewCurrentPawn = PlayerController->GetPawn();
            if (NewCurrentPawn == NewCharacter)
            {
                UE_LOG(LogTemp, Warning, TEXT("POSSESSION: SUCCESS - PlayerController now possesses the new character"));
                
                // Notify the client via RPC that possession happened
                if (ABloodreadGamePlayerController* BloodreadPC = Cast<ABloodreadGamePlayerController>(PlayerController))
                {
                    UE_LOG(LogTemp, Warning, TEXT("POSSESSION: Calling Client RPC to notify client of possession"));
                    BloodreadPC->ClientNotifyCharacterPossession(NewCharacter);
                    UE_LOG(LogTemp, Warning, TEXT("POSSESSION: Client RPC called"));
                }
            }
            else
            {
                UE_LOG(LogTemp, Error, TEXT("POSSESSION: FAILED - PlayerController->GetPawn() != NewCharacter"));
                UE_LOG(LogTemp, Error, TEXT("POSSESSION: Expected: %s, Got: %s"), 
                       *NewCharacter->GetName(), 
                       NewCurrentPawn ? *NewCurrentPawn->GetName() : TEXT("NULL"));
                return;
            }
            
            UE_LOG(LogTemp, Warning, TEXT("Character possessed by controller"));
            
            // NOW enable input AFTER possession
            NewCharacter->EnableInput(PlayerController);
            UE_LOG(LogTemp, Warning, TEXT("Input enabled on possessed character"));
            
            // Set up input context after possession
            NewCharacter->SetupInputContext();
            UE_LOG(LogTemp, Warning, TEXT("Input context setup called"));
            
            // NOTE: Health bar initialization is now handled by PlayerController in OnPossess
            // No need to call ShowPlayerHUD here as it would cause double initialization on server
            UE_LOG(LogTemp, Warning, TEXT("Character setup complete - UI handled by PlayerController"));
            
            // Ensure input mode is set to game only (server-side, but clients will handle their own UI)
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

void ABloodreadGameMode::ShowPlayerHUD(APlayerController* TargetPlayerController)
{
    UE_LOG(LogTemp, Warning, TEXT("ShowPlayerHUD called - initializing health bar widget"));
    UE_LOG(LogTemp, Warning, TEXT("ShowPlayerHUD: TargetPlayerController: %s"), TargetPlayerController ? *TargetPlayerController->GetName() : TEXT("NULL"));
    
    // Use the provided PlayerController instead of GetFirstPlayerController
    APlayerController* PlayerController = TargetPlayerController;
    if (!PlayerController)
    {
        UE_LOG(LogTemp, Error, TEXT("ShowPlayerHUD: No PlayerController provided, falling back to first player controller"));
        PlayerController = GetWorld()->GetFirstPlayerController();
        if (PlayerController)
        {
            UE_LOG(LogTemp, Warning, TEXT("ShowPlayerHUD: Using fallback PlayerController: %s"), *PlayerController->GetName());
        }
    }
    
    if (!PlayerController)
    {
        UE_LOG(LogTemp, Error, TEXT("ShowPlayerHUD: No PlayerController found"));
        return;
    }
    
    // Cast to our custom player controller to access health bar functionality
    ABloodreadGamePlayerController* BloodreadController = Cast<ABloodreadGamePlayerController>(PlayerController);
    if (BloodreadController)
    {
        // Get the possessed character
        ABloodreadBaseCharacter* PlayerCharacter = Cast<ABloodreadBaseCharacter>(BloodreadController->GetPawn());
        if (PlayerCharacter)
        {
            UE_LOG(LogTemp, Warning, TEXT("ShowPlayerHUD: Initializing health bar widget for character: %s"), *PlayerCharacter->GetName());
            BloodreadController->InitializeHealthBarWidget(PlayerCharacter);
        }
        else
        {
            UE_LOG(LogTemp, Error, TEXT("ShowPlayerHUD: No BloodreadBaseCharacter found on PlayerController"));
        }
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("ShowPlayerHUD: PlayerController is not BloodreadGamePlayerController"));
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
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to spawn %s!"), *ClassName);
    }
}