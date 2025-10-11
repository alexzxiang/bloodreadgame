#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "Components/EditableTextBox.h"
#include "Components/ScrollBox.h"
#include "Components/Slider.h"
#include "Components/ListView.h"
#include "Engine/Engine.h"
#include "UObject/ObjectMacros.h"
#include "MultiplayerLobbyWidget.generated.h"

// Forward declarations
class UBloodreadGameInstance;
struct FSessionInfo;

UCLASS(BlueprintType, Blueprintable)
class BLOODREADGAME_API UMultiplayerLobbyWidget : public UUserWidget
{
    GENERATED_BODY()

public:
    UMultiplayerLobbyWidget(const FObjectInitializer& ObjectInitializer);

    virtual void NativeConstruct() override;
    virtual void NativeDestruct() override;

    // UI Components (bind these in Blueprint)
    UPROPERTY(meta = (BindWidget))
    UButton* HostGameButton;
    
    UPROPERTY(meta = (BindWidget))
    UButton* FindGamesButton;
    
    UPROPERTY(meta = (BindWidget))
    UButton* JoinSelectedGameButton;
    
    UPROPERTY(meta = (BindWidget))
    UButton* RefreshButton;
    
    UPROPERTY(meta = (BindWidget))
    UButton* BackButton;

    // Essential Host Game Settings
    UPROPERTY(meta = (BindWidget))
    UEditableTextBox* SessionNameTextBox;
    
    UPROPERTY(meta = (BindWidget))
    USlider* MaxPlayersSlider;
    
    UPROPERTY(meta = (BindWidget))
    UTextBlock* MaxPlayersLabel;

    // Server Browser
    UPROPERTY(meta = (BindWidget))
    UScrollBox* ServerListScrollBox;
    
    UPROPERTY(meta = (BindWidget))
    UTextBlock* StatusLabel;
    
    // Server Entry Widget Class - Set this in Blueprint to your ServerEntry widget blueprint
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI")
    TSubclassOf<class UServerEntryWidget> ServerEntryWidgetClass;

    // Lobby Info
    UPROPERTY(meta = (BindWidget))
    UTextBlock* LobbyInfoLabel;

    // Blueprint callable functions
    UFUNCTION(BlueprintCallable, Category = "Multiplayer")
    void CreateSteamSession();
    
    UFUNCTION(BlueprintCallable, Category = "Multiplayer")
    void FindSteamSessions();
    
    UFUNCTION(BlueprintCallable, Category = "Multiplayer")
    void JoinSelectedSession();
    
    UFUNCTION(BlueprintCallable, Category = "Multiplayer")
    void RefreshServerList();

    UFUNCTION(BlueprintCallable, Category = "Multiplayer")
    void UpdateMaxPlayersLabel(float Value);

    UFUNCTION(BlueprintCallable, Category = "Multiplayer")
    void SetSelectedSession(int32 Index);

    // Server entry selection callback
    UFUNCTION(BlueprintCallable, Category = "Server Selection")
    void OnServerEntrySelected(int32 SessionIndex);

protected:
    // Button event handlers
    UFUNCTION()
    void OnHostGameClicked();
    
    UFUNCTION()
    void OnFindGamesClicked();
    
    UFUNCTION()
    void OnJoinGameClicked();
    
    UFUNCTION()
    void OnRefreshClicked();
    
    UFUNCTION()
    void OnBackClicked();
    
    UFUNCTION()
    void OnMaxPlayersChanged(float Value);

    // Game Instance callbacks (function implementations, not delegates)
    UFUNCTION()
    void HandleSessionCreated(bool bWasSuccessful);

    UFUNCTION()
    void HandleSessionJoined(bool bWasSuccessful);

    UFUNCTION()
    void HandleSessionsFound(bool bWasSuccessful, const TArray<FSessionInfo>& Sessions);
    
    UFUNCTION()
    void HandleJoinSessionFallback(bool bWasSuccessful);

    // UI helper functions
    void UpdateServerList();
    void ClearServerList();
    void AddServerEntry(const FSessionInfo& SessionInfo, int32 Index);
    void UpdateStatusText(const FString& Status);

private:
    // Game Instance reference
    UPROPERTY()
    UBloodreadGameInstance* BloodreadGameInstance;
    
    // Session settings
    int32 SelectedSessionIndex = -1;
    int32 CurrentMaxPlayers = 4;
    bool bSearchingForSessions = false;
    
    // Cache of found sessions for fallback connection (no UPROPERTY to avoid reflection issues)
    TArray<FSessionInfo> CachedSessions;

    // UI binding helper
    void BindButtonEvents();
};
