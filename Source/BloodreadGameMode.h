#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "BloodreadBaseCharacter.h"
#include "CharacterSelectionManager.h"
#include "MultiplayerLobbyWidget.h"
#include "Blueprint/UserWidget.h"
#include "Components/Widget.h"
#include "BloodreadGameMode.generated.h"

UENUM(BlueprintType)
enum class EGameState : uint8
{
    MainMenu        UMETA(DisplayName = "Main Menu"),
    MultiplayerLobby UMETA(DisplayName = "Multiplayer Lobby"), 
    CharacterSelection UMETA(DisplayName = "Character Selection"),
    InGame          UMETA(DisplayName = "In Game"),
    GameOver        UMETA(DisplayName = "Game Over")
};

UCLASS()
class BLOODREADGAME_API ABloodreadGameMode : public AGameModeBase
{
    GENERATED_BODY()

public:
    ABloodreadGameMode();

    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;
    
    // Override spawn to use character selection system
    virtual APawn* SpawnDefaultPawnFor_Implementation(AController* NewPlayer, AActor* StartSpot) override;

    // Multiplayer session management
    virtual void PostLogin(APlayerController* NewPlayer) override;
    virtual void Logout(AController* Exiting) override;
    
    UFUNCTION(BlueprintCallable, Category="Multiplayer")
    void StartMultiplayerMatch();
    
    UFUNCTION(BlueprintCallable, Category="Multiplayer")
    int32 GetConnectedPlayerCount();

    UFUNCTION(BlueprintImplementableEvent, Category="Multiplayer")
    void OnPlayerJoined(int32 CurrentPlayers, int32 MaxPlayersCount);

    UFUNCTION(BlueprintImplementableEvent, Category="Multiplayer")
    void OnPlayerLeft(int32 CurrentPlayers, int32 MaxPlayersCount);

    // Game tick system (1/16 seconds = 0.0625)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Game Settings")
    float GameTickInterval = 0.0625f;

    // Combat settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Combat")
    int32 BasePlayerHealth = 100;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Combat")
    int32 BasePlayerStrength = 10;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Combat")
    float KnockbackForce = 500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Combat")
    int32 DamageImmunityTicks = 3;

    // Multiplayer settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Multiplayer")
    int32 MaxPlayers = 4;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Multiplayer")
    int32 MinPlayersToStart = 2;

    UPROPERTY(BlueprintReadOnly, Category="Multiplayer")
    int32 ConnectedPlayers = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Multiplayer")
    bool bAutoStartWhenReady = true;

    // Game State Management
    UPROPERTY(BlueprintReadOnly, Category="Game State")
    EGameState CurrentGameState = EGameState::MainMenu;

    UFUNCTION(BlueprintCallable, Category="Game State")
    void SetGameState(EGameState NewState);

    UFUNCTION(BlueprintCallable, Category="Game State")
    EGameState GetGameState() const { return CurrentGameState; }

    // Special function for Blueprint GameMode to call when it's ready
    UFUNCTION(BlueprintCallable, Category="Game State")
    void InitializeGameModeState();

    // Simple function to show lobby widget immediately (call from Blueprint BeginPlay)
    UFUNCTION(BlueprintCallable, Category="UI")
    void ShowLobbyWidgetNow();

    // Multiplayer Lobby Management
    UFUNCTION(BlueprintCallable, Category="Multiplayer Lobby")
    void ShowMultiplayerLobby();

    UFUNCTION(BlueprintCallable, Category="Multiplayer Lobby")
    void HideMultiplayerLobby();

    UFUNCTION(BlueprintCallable, Category="Multiplayer Lobby")
    void OnSessionCreated(bool bSuccess);

    UFUNCTION(BlueprintCallable, Category="Multiplayer Lobby")
    void OnSessionJoined(bool bSuccess);

    // Match management
    UFUNCTION(BlueprintCallable, Category="Match")
    void StartMatch(bool bIsPractice = false, int32 OpponentPlayerId = 0);

    UFUNCTION(BlueprintCallable, Category="Match")
    void EndMatch(int32 WinnerPlayerId);

    UFUNCTION(BlueprintImplementableEvent, Category="Match")
    void OnMatchStarted(bool bIsPractice);

    UFUNCTION(BlueprintImplementableEvent, Category="Match")
    void OnMatchEnded(int32 WinnerPlayerId);

    // Character Selection System
    UFUNCTION(BlueprintCallable, Category="Character Selection")
    void ShowCharacterSelectionWidget();

    UFUNCTION(BlueprintCallable, Category="Character Selection")
    void HideCharacterSelectionWidget();

    UFUNCTION(BlueprintCallable, Category="HUD Management")
    void ShowPlayerHUD();

    UFUNCTION(BlueprintCallable, Category="HUD Management")
    void HidePlayerHUD();

    UFUNCTION(BlueprintCallable, Category="Character Selection")
    void HandleCharacterSelection(int32 CharacterClassIndex);    UFUNCTION(BlueprintCallable, Category="Character Selection")
    void SpawnSelectedCharacter(ECharacterClass SelectedClass, APlayerController* PlayerController);
    
    // New function for spawning character after UI selection
    UFUNCTION(BlueprintCallable, Category="Character Selection")
    void SpawnCharacterForPlayer(int32 CharacterClassIndex, APlayerController* PlayerController = nullptr);

    UFUNCTION(BlueprintCallable, Category="Character Selection")
    ABloodreadBaseCharacter* CreateCharacterOfClass(ECharacterClass CharacterClass, FVector SpawnLocation, FRotator SpawnRotation);

    // Team management functions (legacy placeholders)
    UFUNCTION(BlueprintCallable, Category="Team Management")
    void SpawnAITeammates(int32 TeamSize, int32 TeamId);

    UFUNCTION(BlueprintCallable, Category="Team Management")
    void AssignPlayerToTeam(ABloodreadBaseCharacter* PlayerCharacter);

    UFUNCTION(BlueprintCallable, Category="Team Management")
    void SetupBalancedMatch();

    UFUNCTION(BlueprintCallable, Category="Team Management")
    void InitializeTeamManager();

    // Widget references for management
    UPROPERTY(BlueprintReadWrite, Category="Multiplayer Lobby")
    UMultiplayerLobbyWidget* MultiplayerLobbyWidget;

    UPROPERTY(BlueprintReadWrite, Category="Character Selection")
    class UUserWidget* CharacterSelectionWidget;

    // HUD widget reference for management
    UPROPERTY(BlueprintReadWrite, Category="HUD Management")
    class UUserWidget* PlayerHUDWidget;

    // Widget class references (EDITABLE IN BLUEPRINT)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Multiplayer Lobby")
    TSubclassOf<UMultiplayerLobbyWidget> MultiplayerLobbyWidgetClass;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="HUD Management")
    TSubclassOf<UUserWidget> PlayerHUDWidgetClass;

protected:
    // Character Selection Manager
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Character Selection")
    UCharacterSelectionManager* CharacterSelectionManager;

    // Widget class reference for character selection
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Character Selection")
    TSubclassOf<UUserWidget> CharacterSelectionWidgetClass;

    UPROPERTY(BlueprintReadOnly, Category="Match")
    bool bMatchInProgress = false;

    UPROPERTY(BlueprintReadOnly, Category="Match")
    bool bIsPracticeMatch = false;

    UPROPERTY(BlueprintReadOnly, Category="Match")
    int32 OpponentId = 0;

    UPROPERTY(BlueprintReadOnly, Category="Match")
    float MatchStartTime = 0.0f;

    // Game tick counter
    float GameTickTimer = 0.0f;
    int32 CurrentTick = 0;

    // Timer handle for character possession
    FTimerHandle PossessionTimerHandle;

    UFUNCTION()
    void ProcessGameTick();

    UFUNCTION(BlueprintImplementableEvent, Category="Game Tick")
    void OnGameTick(int32 TickNumber);

    // Helper function for spawning characters with proper timing
    void SpawnNewCharacter(ECharacterClass SelectedClass, APlayerController* PlayerController, FVector SpawnLocation, FRotator SpawnRotation);
};
