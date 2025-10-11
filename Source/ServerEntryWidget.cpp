#include "ServerEntryWidget.h"
#include "MultiplayerLobbyWidget.h"
#include "Components/TextBlock.h"
#include "Components/Button.h"

UServerEntryWidget::UServerEntryWidget(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
{
    SessionIndex = -1;
    SessionNameText = nullptr;
    HostNameText = nullptr;
    PlayerCountText = nullptr;
    ServerButton = nullptr;
}

void UServerEntryWidget::SetSessionInfo(const FSessionInfo& SessionInfo, int32 Index)
{
    CachedSessionInfo = SessionInfo;
    SessionIndex = Index;
    
    UE_LOG(LogTemp, Warning, TEXT("ServerEntryWidget: Setting session info - Name: %s, Host: %s, Players: %d/%d"), 
           *SessionInfo.SessionName, *SessionInfo.HostName, SessionInfo.CurrentPlayers, SessionInfo.MaxPlayers);
    
    // Update the text display directly in C++
    UpdateTextDisplay();
}

void UServerEntryWidget::UpdateTextDisplay()
{
    // Check if text blocks are valid (they should be bound from Blueprint)
    if (SessionNameText)
    {
        SessionNameText->SetText(FText::FromString(CachedSessionInfo.SessionName));
        UE_LOG(LogTemp, Warning, TEXT("ServerEntryWidget: Set session name text to: %s"), *CachedSessionInfo.SessionName);
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("ServerEntryWidget: SessionNameText is null - make sure it's bound in Blueprint"));
    }

    if (HostNameText)
    {
        HostNameText->SetText(FText::FromString(CachedSessionInfo.HostName));
        UE_LOG(LogTemp, Warning, TEXT("ServerEntryWidget: Set host name text to: %s"), *CachedSessionInfo.HostName);
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("ServerEntryWidget: HostNameText is null - make sure it's bound in Blueprint"));
    }

    if (PlayerCountText)
    {
        FString PlayerCountString = FString::Printf(TEXT("%d/%d"), CachedSessionInfo.CurrentPlayers, CachedSessionInfo.MaxPlayers);
        PlayerCountText->SetText(FText::FromString(PlayerCountString));
        UE_LOG(LogTemp, Warning, TEXT("ServerEntryWidget: Set player count text to: %s"), *PlayerCountString);
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("ServerEntryWidget: PlayerCountText is null - make sure it's bound in Blueprint"));
    }
}

void UServerEntryWidget::OnServerEntryClicked()
{
    UE_LOG(LogTemp, Warning, TEXT("ServerEntryWidget: Session clicked - Index: %d, Name: %s"), 
           SessionIndex, *CachedSessionInfo.SessionName);
    
    // Find the parent MultiplayerLobbyWidget by traversing up the widget tree
    UWidget* CurrentParent = GetParent();
    UMultiplayerLobbyWidget* ParentLobby = nullptr;
    
    // Try to find MultiplayerLobbyWidget up the widget hierarchy
    while (CurrentParent)
    {
        ParentLobby = Cast<UMultiplayerLobbyWidget>(CurrentParent);
        if (ParentLobby)
        {
            break;
        }
        
        // Move up one level in the widget hierarchy
        if (UPanelWidget* PanelParent = Cast<UPanelWidget>(CurrentParent))
        {
            CurrentParent = PanelParent->GetParent();
        }
        else
        {
            CurrentParent = nullptr;
        }
    }
    
    if (ParentLobby)
    {
        ParentLobby->OnServerEntrySelected(SessionIndex);
        UE_LOG(LogTemp, Warning, TEXT("ServerEntryWidget: Notified parent lobby of selection"));
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("ServerEntryWidget: Could not find parent MultiplayerLobbyWidget"));
    }
}