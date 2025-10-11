#include "MultiplayerLobbyWidget.h"
#include "Components/Button.h"
#include "BloodreadGameInstance.h"
#include "ServerEntryWidget.h"
#include "BloodreadGameMode.h"
#include "Kismet/GameplayStatics.h"

UMultiplayerLobbyWidget::UMultiplayerLobbyWidget(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
{
    SelectedSessionIndex = -1;
    CurrentMaxPlayers = 4;
    bSearchingForSessions = false;
    // Reduced logging verbosity - only log in non-editor builds or when debugging
    UE_LOG(LogTemp, Verbose, TEXT("MultiplayerLobbyWidget constructor initialized"));
}

void UMultiplayerLobbyWidget::NativeConstruct()
{
    Super::NativeConstruct();
    UE_LOG(LogTemp, Log, TEXT("MultiplayerLobbyWidget NativeConstruct called"));
    
    // Get the BloodreadGameInstance
    BloodreadGameInstance = Cast<UBloodreadGameInstance>(GetWorld()->GetGameInstance());
    if (BloodreadGameInstance)
    {
        UE_LOG(LogTemp, Log, TEXT("MultiplayerLobby: Connected to BloodreadGameInstance"));
        
        // Bind to game instance events
        BloodreadGameInstance->OnSessionCreated.AddDynamic(this, &UMultiplayerLobbyWidget::HandleSessionCreated);
        BloodreadGameInstance->OnSessionJoined.AddDynamic(this, &UMultiplayerLobbyWidget::HandleSessionJoined);
        BloodreadGameInstance->OnSessionsFound.AddDynamic(this, &UMultiplayerLobbyWidget::HandleSessionsFound);
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("MultiplayerLobby: Failed to get BloodreadGameInstance"));
    }

    // Bind UI events
    UE_LOG(LogTemp, Verbose, TEXT("MultiplayerLobby: Binding button events"));
    BindButtonEvents();
    
    // Initialize UI state
    UE_LOG(LogTemp, Verbose, TEXT("MultiplayerLobby: Updating initial UI state"));
    UpdateStatusText(TEXT("Ready to connect to Steam multiplayer"));
    UpdateMaxPlayersLabel(4.0f);
    
    UE_LOG(LogTemp, Log, TEXT("MultiplayerLobbyWidget NativeConstruct completed successfully"));
}

void UMultiplayerLobbyWidget::NativeDestruct()
{
    // Clean up delegates if valid
    if (BloodreadGameInstance)
    {
        BloodreadGameInstance->OnSessionCreated.RemoveDynamic(this, &UMultiplayerLobbyWidget::HandleSessionCreated);
        BloodreadGameInstance->OnSessionJoined.RemoveDynamic(this, &UMultiplayerLobbyWidget::HandleSessionJoined);
        BloodreadGameInstance->OnSessionsFound.RemoveDynamic(this, &UMultiplayerLobbyWidget::HandleSessionsFound);
    }
    
    Super::NativeDestruct();
}

void UMultiplayerLobbyWidget::BindButtonEvents()
{
    // Skip binding during asset validation or if world is not valid
    if (!GetWorld() || GetWorld()->WorldType == EWorldType::EditorPreview)
    {
        return;
    }
    
    if (HostGameButton)
    {
        HostGameButton->OnClicked.AddDynamic(this, &UMultiplayerLobbyWidget::OnHostGameClicked);
    }
    
    if (FindGamesButton)
    {
        FindGamesButton->OnClicked.AddDynamic(this, &UMultiplayerLobbyWidget::OnFindGamesClicked);
    }
    
    if (JoinSelectedGameButton)
    {
        JoinSelectedGameButton->OnClicked.AddDynamic(this, &UMultiplayerLobbyWidget::OnJoinGameClicked);
    }
    
    if (RefreshButton)
    {
        RefreshButton->OnClicked.AddDynamic(this, &UMultiplayerLobbyWidget::OnRefreshClicked);
    }
    
    if (BackButton)
    {
        BackButton->OnClicked.AddDynamic(this, &UMultiplayerLobbyWidget::OnBackClicked);
    }
    
    if (MaxPlayersSlider)
    {
        MaxPlayersSlider->OnValueChanged.AddDynamic(this, &UMultiplayerLobbyWidget::OnMaxPlayersChanged);
    }
}

// Button event handlers
void UMultiplayerLobbyWidget::OnHostGameClicked()
{
    CreateSteamSession();
}

void UMultiplayerLobbyWidget::OnFindGamesClicked()
{
    FindSteamSessions();
}

void UMultiplayerLobbyWidget::OnJoinGameClicked()
{
    JoinSelectedSession();
}

void UMultiplayerLobbyWidget::OnRefreshClicked()
{
    RefreshServerList();
}

void UMultiplayerLobbyWidget::OnBackClicked()
{
    // Return to main menu via GameMode
    if (ABloodreadGameMode* GameMode = Cast<ABloodreadGameMode>(GetWorld()->GetAuthGameMode()))
    {
        GameMode->SetGameState(EGameState::MainMenu);
    }
}

void UMultiplayerLobbyWidget::OnMaxPlayersChanged(float Value)
{
    CurrentMaxPlayers = FMath::RoundToInt(Value);
    UpdateMaxPlayersLabel(Value);
}

// Blueprint callable functions
void UMultiplayerLobbyWidget::CreateSteamSession()
{
    if (!BloodreadGameInstance)
    {
        UpdateStatusText(TEXT("Error: Game Instance not found"));
        return;
    }
    
    FString SessionName = TEXT("BloodreadGame Session");
    if (SessionNameTextBox && !SessionNameTextBox->GetText().IsEmpty())
    {
        SessionName = SessionNameTextBox->GetText().ToString();
    }
    
    UpdateStatusText(TEXT("Creating session..."));
    BloodreadGameInstance->CreateSession(SessionName, CurrentMaxPlayers, false);
}

void UMultiplayerLobbyWidget::FindSteamSessions()
{
    if (!BloodreadGameInstance)
    {
        UpdateStatusText(TEXT("Error: Game Instance not found"));
        return;
    }
    
    bSearchingForSessions = true;
    UpdateStatusText(TEXT("Searching for sessions..."));
    ClearServerList();
    
    BloodreadGameInstance->FindSessions(false);
}

void UMultiplayerLobbyWidget::JoinSelectedSession()
{
    if (!BloodreadGameInstance)
    {
        UpdateStatusText(TEXT("Error: Game Instance not found"));
        return;
    }
    
    if (SelectedSessionIndex < 0)
    {
        UpdateStatusText(TEXT("Please select a session to join"));
        return;
    }
    
    if (!CachedSessions.IsValidIndex(SelectedSessionIndex))
    {
        UpdateStatusText(TEXT("Selected session is no longer available"));
        return;
    }
    
    const FSessionInfo& SelectedSession = CachedSessions[SelectedSessionIndex];
    
    UE_LOG(LogTemp, Warning, TEXT("🔌 Attempting to join session: %s"), *SelectedSession.SessionName);
    
    // First, try Steam connection
    UpdateStatusText(TEXT("Connecting via Steam..."));
    
    // Bind to join result to handle fallback
    if (!BloodreadGameInstance->OnSessionJoined.IsAlreadyBound(this, &UMultiplayerLobbyWidget::HandleJoinSessionFallback))
    {
        BloodreadGameInstance->OnSessionJoined.AddDynamic(this, &UMultiplayerLobbyWidget::HandleJoinSessionFallback);
    }
    
    BloodreadGameInstance->JoinSteamSession(SelectedSessionIndex);
}

void UMultiplayerLobbyWidget::HandleJoinSessionFallback(bool bWasSuccessful)
{
    if (bWasSuccessful)
    {
        UE_LOG(LogTemp, Warning, TEXT("✅ Steam connection successful"));
        UpdateStatusText(TEXT("Connected via Steam!"));
        
        // Unbind the fallback handler
        if (BloodreadGameInstance)
        {
            BloodreadGameInstance->OnSessionJoined.RemoveDynamic(this, &UMultiplayerLobbyWidget::HandleJoinSessionFallback);
        }
        return;
    }
    
    // Steam connection failed, try IP fallback
    UE_LOG(LogTemp, Warning, TEXT("❌ Steam connection failed, attempting IP fallback"));
    
    if (!CachedSessions.IsValidIndex(SelectedSessionIndex))
    {
        UpdateStatusText(TEXT("Session no longer available"));
        return;
    }
    
    const FSessionInfo& SelectedSession = CachedSessions[SelectedSessionIndex];
    
    // Check if we have IP information in the session data
    if (!SelectedSession.IPAddress.IsEmpty() && SelectedSession.IPAddress != TEXT("127.0.0.1"))
    {
        UpdateStatusText(FString::Printf(TEXT("Trying direct connection to %s:%d"), *SelectedSession.IPAddress, SelectedSession.Port));
        UE_LOG(LogTemp, Warning, TEXT("🌐 Attempting IP connection to %s:%d"), *SelectedSession.IPAddress, SelectedSession.Port);
        
        BloodreadGameInstance->JoinSessionByIP(SelectedSession.IPAddress, SelectedSession.Port);
    }
    else
    {
        UpdateStatusText(TEXT("Connection failed: No fallback IP available"));
        UE_LOG(LogTemp, Error, TEXT("❌ No IP fallback available for session"));
    }
    
    // Unbind the fallback handler
    if (BloodreadGameInstance)
    {
        BloodreadGameInstance->OnSessionJoined.RemoveDynamic(this, &UMultiplayerLobbyWidget::HandleJoinSessionFallback);
    }
}

void UMultiplayerLobbyWidget::RefreshServerList()
{
    FindSteamSessions();
}

void UMultiplayerLobbyWidget::UpdateMaxPlayersLabel(float Value)
{
    if (MaxPlayersLabel)
    {
        int32 Players = FMath::RoundToInt(Value);
        MaxPlayersLabel->SetText(FText::FromString(FString::Printf(TEXT("Max Players: %d"), Players)));
    }
}

void UMultiplayerLobbyWidget::SetSelectedSession(int32 Index)
{
    SelectedSessionIndex = Index;
    UE_LOG(LogTemp, Log, TEXT("Selected session index: %d"), Index);
}

// Game Instance callbacks
void UMultiplayerLobbyWidget::HandleSessionCreated(bool bWasSuccessful)
{
    if (bWasSuccessful)
    {
        UpdateStatusText(TEXT("Session created successfully! Waiting for players..."));
    }
    else
    {
        UpdateStatusText(TEXT("Failed to create session. Please try again."));
    }
}

void UMultiplayerLobbyWidget::HandleSessionJoined(bool bWasSuccessful)
{
    if (bWasSuccessful)
    {
        UpdateStatusText(TEXT("Joined session successfully!"));
    }
    else
    {
        UpdateStatusText(TEXT("Failed to join session. Please try another."));
    }
}

void UMultiplayerLobbyWidget::HandleSessionsFound(bool bWasSuccessful, const TArray<FSessionInfo>& Sessions)
{
    bSearchingForSessions = false;
    
    // Cache the sessions for fallback connection
    CachedSessions = Sessions;
    UE_LOG(LogTemp, Warning, TEXT("🗂️ Cached %d sessions for fallback connection"), CachedSessions.Num());
    
    if (bWasSuccessful)
    {
        UE_LOG(LogTemp, Warning, TEXT("🎯 OnSessionsFound SUCCESS - %d sessions received"), Sessions.Num());
        
        if (Sessions.Num() > 0)
        {
            UpdateStatusText(FString::Printf(TEXT("Found %d session(s)"), Sessions.Num()));
            UpdateServerList();
            
            UE_LOG(LogTemp, Warning, TEXT("📋 Adding %d sessions to UI..."), Sessions.Num());
            
            // Add sessions to server list
            for (int32 i = 0; i < Sessions.Num(); i++)
            {
                UE_LOG(LogTemp, Warning, TEXT("📋 Adding session[%d]: %s"), i, *Sessions[i].SessionName);
                AddServerEntry(Sessions[i], i);
            }
            
            UE_LOG(LogTemp, Warning, TEXT("✅ Finished adding all sessions to UI"));
        }
        else
        {
            UE_LOG(LogTemp, Warning, TEXT("📭 No sessions found in callback"));
            UpdateStatusText(TEXT("No sessions found. Try hosting your own!"));
        }
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("❌ OnSessionsFound FAILED"));
        UpdateStatusText(TEXT("Failed to search for sessions. Check your connection."));
    }
}

// UI helper functions
void UMultiplayerLobbyWidget::UpdateServerList()
{
    // Implementation depends on your UI setup
    // This would typically refresh the server list display
}

void UMultiplayerLobbyWidget::ClearServerList()
{
    if (ServerListScrollBox)
    {
        ServerListScrollBox->ClearChildren();
    }
    SelectedSessionIndex = -1;
}

void UMultiplayerLobbyWidget::AddServerEntry(const FSessionInfo& SessionInfo, int32 Index)
{
    if (!ServerListScrollBox)
    {
        UE_LOG(LogTemp, Error, TEXT("❌ ServerListScrollBox is NULL - check Blueprint bindings"));
        return;
    }
    
    UE_LOG(LogTemp, Warning, TEXT("📋 Creating ServerEntry widget for session: %s"), *SessionInfo.SessionName);
    
    // Try to use the Blueprint widget class if set, otherwise fallback to C++ class
    UServerEntryWidget* ServerEntry = nullptr;
    
    if (ServerEntryWidgetClass)
    {
        UE_LOG(LogTemp, Warning, TEXT("🎨 Using Blueprint ServerEntryWidgetClass"));
        ServerEntry = CreateWidget<UServerEntryWidget>(this, ServerEntryWidgetClass);
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("⚠️  No ServerEntryWidgetClass set, using C++ fallback"));
        ServerEntry = CreateWidget<UServerEntryWidget>(this, UServerEntryWidget::StaticClass());
    }
    
    if (ServerEntry)
    {
        UE_LOG(LogTemp, Warning, TEXT("✅ ServerEntry widget created successfully"));
        ServerEntry->SetSessionInfo(SessionInfo, Index);
        ServerListScrollBox->AddChild(ServerEntry);
        UE_LOG(LogTemp, Warning, TEXT("✅ ServerEntry added to ScrollBox"));
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("❌ Failed to create ServerEntry widget"));
    }
}

void UMultiplayerLobbyWidget::UpdateStatusText(const FString& Status)
{
    if (StatusLabel)
    {
        StatusLabel->SetText(FText::FromString(Status));
    }
    
    UE_LOG(LogTemp, Log, TEXT("MultiplayerLobby Status: %s"), *Status);
}

void UMultiplayerLobbyWidget::OnServerEntrySelected(int32 SessionIndex)
{
    UE_LOG(LogTemp, Warning, TEXT("MultiplayerLobbyWidget: Session selected - Index: %d"), SessionIndex);
    
    SelectedSessionIndex = SessionIndex;
    
    // Enable the Join Selected Game button
    if (JoinSelectedGameButton)
    {
        JoinSelectedGameButton->SetIsEnabled(true);
        UE_LOG(LogTemp, Warning, TEXT("MultiplayerLobbyWidget: Join button enabled for session %d"), SessionIndex);
    }
    
    // Update status text to show selected session
    UpdateStatusText(FString::Printf(TEXT("Selected session %d - Ready to join!"), SessionIndex + 1));
}
