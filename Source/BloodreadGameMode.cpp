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
    UE_LOG(LogTemp, Warning, TEXT("=== CHARACTER SELECTION STARTED ==="));
    UE_LOG(LogTemp, Warning, TEXT("Button pressed! Character index: %d"), CharacterClassIndex);
    
    // If no PlayerController provided, use the first player controller for backwards compatibility
    if (!PlayerController)
    {
        PlayerController = GetWorld()->GetFirstPlayerController();
        UE_LOG(LogTemp, Warning, TEXT("No PlayerController provided, using first player controller"));
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
    
    // Call the character selected event first
    OnCharacterSelected(CharacterClassIndex);
    
    // Small delay before spawning to ensure UI cleanup
    GetWorld()->GetTimerManager().SetTimerForNextTick([this, SelectedClass, PlayerController, ClassName]()
    {
        UE_LOG(LogTemp, Warning, TEXT("UI hidden, now spawning %s character..."), *ClassName);
        SpawnSelectedCharacter(SelectedClass, PlayerController);
    });
}

void ABloodreadGameMode::OnCharacterSelected(int32 CharacterClassIndex)
{
    UE_LOG(LogTemp, Warning, TEXT("OnCharacterSelected called with index: %d"), CharacterClassIndex);
    
    // Get the first player controller
    APlayerController* PlayerController = GetWorld()->GetFirstPlayerController();
    if (!PlayerController)
    {
        UE_LOG(LogTemp, Error, TEXT("OnCharacterSelected: No PlayerController found"));
        return;
    }
    
    // Cast to our custom player controller to access health bar functionality
    ABloodreadGamePlayerController* BloodreadController = Cast<ABloodreadGamePlayerController>(PlayerController);
    if (BloodreadController)
    {
        // Get the character that should be possessed by now
        ABloodreadBaseCharacter* PlayerCharacter = Cast<ABloodreadBaseCharacter>(BloodreadController->GetPawn());
        if (PlayerCharacter)
        {
            UE_LOG(LogTemp, Warning, TEXT("OnCharacterSelected: Initializing health bar widget for character: %s"), *PlayerCharacter->GetName());
            BloodreadController->InitializeHealthBarWidget(PlayerCharacter);
        }
        else
        {
            UE_LOG(LogTemp, Warning, TEXT("OnCharacterSelected: Character not possessed yet, will initialize health bar later"));
        }
    }
    
    // Also initialize the player UI
    InitializePlayerUI(PlayerController);
}

void ABloodreadGameMode::InitializePlayerUI(APlayerController* PlayerController)
{
    UE_LOG(LogTemp, Warning, TEXT("InitializePlayerUI called"));
    
    if (!PlayerController)
    {
        UE_LOG(LogTemp, Error, TEXT("InitializePlayerUI: PlayerController is null"));
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
            UE_LOG(LogTemp, Warning, TEXT("InitializePlayerUI: Initializing health bar widget for character: %s"), *PlayerCharacter->GetName());
            BloodreadController->InitializeHealthBarWidget(PlayerCharacter);
        }
        else
        {
            UE_LOG(LogTemp, Warning, TEXT("InitializePlayerUI: No character possessed yet"));
        }
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("InitializePlayerUI: PlayerController is not BloodreadGamePlayerController"));
    }
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
    UE_LOG(LogTemp, Warning, TEXT("ShowPlayerHUD called - initializing health bar widget"));
    
    // Get the first player controller
    APlayerController* PlayerController = GetWorld()->GetFirstPlayerController();
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