#include "BloodreadGameInstance.h"
#include "Engine/GameInstance.h"
#include "Engine/Engine.h"
#include "Http.h"

DEFINE_LOG_CATEGORY(BloodreadGameInstanceLog);

UBloodreadGameInstance::UBloodreadGameInstance()
{
    DatabaseAPIURL = TEXT("http://localhost/bloodread/api");
}

void UBloodreadGameInstance::Init()
{
    UGameInstance::Init();
    UE_LOG(BloodreadGameInstanceLog, Log, TEXT("BloodreadGameInstance initialized - using Blueprint-based Steam multiplayer"));
}

void UBloodreadGameInstance::Shutdown()
{
    UGameInstance::Shutdown();
}

void UBloodreadGameInstance::MakeHTTPRequest(const FString& Endpoint, const FString& JsonPayload, TFunction<void(bool, const FString&)> Callback)
{
    FString URL = DatabaseAPIURL + Endpoint;
    
    TSharedRef<IHttpRequest, ESPMode::ThreadSafe> HttpRequest = FHttpModule::Get().CreateRequest();
    HttpRequest->SetURL(URL);
    HttpRequest->SetVerb(TEXT("POST"));
    HttpRequest->SetHeader(TEXT("Content-Type"), TEXT("application/json"));
    HttpRequest->SetContentAsString(JsonPayload);

    HttpRequest->OnProcessRequestComplete().BindLambda([Callback](FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful)
    {
        if (bWasSuccessful && Response.IsValid())
        {
            FString ResponseString = Response->GetContentAsString();
            int32 ResponseCode = Response->GetResponseCode();
            
            if (ResponseCode >= 200 && ResponseCode < 300)
            {
                Callback(true, ResponseString);
            }
            else
            {
                UE_LOG(BloodreadGameInstanceLog, Error, TEXT("HTTP request failed with code: %d, Response: %s"), ResponseCode, *ResponseString);
                Callback(false, ResponseString);
            }
        }
        else
        {
            UE_LOG(BloodreadGameInstanceLog, Error, TEXT("HTTP request failed"));
            Callback(false, FString());
        }
    });

    if (!HttpRequest->ProcessRequest())
    {
        UE_LOG(BloodreadGameInstanceLog, Error, TEXT("Failed to start HTTP request"));
        Callback(false, FString());
    }
}

void UBloodreadGameInstance::OnFetchServersResponse(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful)
{
    if (bWasSuccessful && Response.IsValid())
    {
        FString ResponseString = Response->GetContentAsString();
        UE_LOG(BloodreadGameInstanceLog, Log, TEXT("Server fetch response: %s"), *ResponseString);
    }
    else
    {
        UE_LOG(BloodreadGameInstanceLog, Error, TEXT("Failed to fetch servers from database"));
    }
}

void UBloodreadGameInstance::JoinSessionByIP(const FString& IPAddress, int32 Port)
{
    UE_LOG(BloodreadGameInstanceLog, Warning, TEXT("JoinSessionByIP called with IP: %s, Port: %d"), *IPAddress, Port);
    // This method is called from MultiplayerLobbyWidget but the actual session joining
    // logic should be implemented using Advanced Sessions plugin or Blueprint logic
    // For now, this is a placeholder to resolve the compilation error
}

void UBloodreadGameInstance::LoginPlayer(const FString& Username, const FString& Password)
{
    UE_LOG(BloodreadGameInstanceLog, Log, TEXT("LoginPlayer called for user: %s"), *Username);
    
    // Create JSON payload for login request
    FString JsonPayload = FString::Printf(TEXT("{\"username\":\"%s\",\"password\":\"%s\"}"), *Username, *Password);
    
    // Make HTTP request to login endpoint
    MakeHTTPRequest(TEXT("/player_login.php"), JsonPayload, [this](bool bWasSuccessful, const FString& ResponseString)
    {
        if (bWasSuccessful)
        {
            // Parse the response and create player data
            FPlayerData PlayerData;
            PlayerData.Username = TEXT("TestUser"); // TODO: Parse from JSON response
            PlayerData.PlayerId = 1;
            PlayerData.Credits = 100;
            
            UE_LOG(BloodreadGameInstanceLog, Log, TEXT("Login successful for user: %s"), *PlayerData.Username);
            OnLoginComplete.Broadcast(true, PlayerData);
        }
        else
        {
            UE_LOG(BloodreadGameInstanceLog, Error, TEXT("Login failed"));
            FPlayerData EmptyData;
            OnLoginComplete.Broadcast(false, EmptyData);
        }
    });
}

void UBloodreadGameInstance::CreatePlayer(const FString& Username, const FString& Password)
{
    UE_LOG(BloodreadGameInstanceLog, Log, TEXT("CreatePlayer called for user: %s"), *Username);
    
    // Create JSON payload for account creation request
    FString JsonPayload = FString::Printf(TEXT("{\"username\":\"%s\",\"password\":\"%s\"}"), *Username, *Password);
    
    // Make HTTP request to create player endpoint
    MakeHTTPRequest(TEXT("/player_create.php"), JsonPayload, [this](bool bWasSuccessful, const FString& ResponseString)
    {
        if (bWasSuccessful)
        {
            // Parse the response and create player data
            FPlayerData PlayerData;
            PlayerData.Username = TEXT("NewUser"); // TODO: Parse from JSON response
            PlayerData.PlayerId = 1;
            PlayerData.Credits = 100;
            
            UE_LOG(BloodreadGameInstanceLog, Log, TEXT("Account creation successful for user: %s"), *PlayerData.Username);
            OnLoginComplete.Broadcast(true, PlayerData);
        }
        else
        {
            UE_LOG(BloodreadGameInstanceLog, Error, TEXT("Account creation failed"));
            FPlayerData EmptyData;
            OnLoginComplete.Broadcast(false, EmptyData);
        }
    });
}

void UBloodreadGameInstance::OnSteamSessionCreated(bool bWasSuccessful)
{
    UE_LOG(BloodreadGameInstanceLog, Log, TEXT("OnSteamSessionCreated: %s"), bWasSuccessful ? TEXT("Success") : TEXT("Failed"));
    bIsCreatingSession = false;
    OnSessionCreated.Broadcast(bWasSuccessful);
}

void UBloodreadGameInstance::OnSteamSessionJoined(bool bWasSuccessful)
{
    UE_LOG(BloodreadGameInstanceLog, Log, TEXT("OnSteamSessionJoined: %s"), bWasSuccessful ? TEXT("Success") : TEXT("Failed"));
    OnSessionJoined.Broadcast(bWasSuccessful);
}

void UBloodreadGameInstance::OnSteamSessionsFound(bool bWasSuccessful, const TArray<FSessionInfo>& Sessions)
{
    UE_LOG(BloodreadGameInstanceLog, Log, TEXT("OnSteamSessionsFound: %s, Found %d sessions"), 
           bWasSuccessful ? TEXT("Success") : TEXT("Failed"), Sessions.Num());
    bIsSearchingForSessions = false;
    OnSessionsFound.Broadcast(bWasSuccessful, Sessions);
}

FString UBloodreadGameInstance::GetSteamConnectionStatus()
{
    // Placeholder implementation for Steam connection status
    return TEXT("Steam connection status check not implemented");
}

void UBloodreadGameInstance::TestSteamConnectivity()
{
    UE_LOG(BloodreadGameInstanceLog, Log, TEXT("TestSteamConnectivity called"));
    // Placeholder implementation for Steam connectivity test
}

void UBloodreadGameInstance::CancelSessionSearch()
{
    UE_LOG(BloodreadGameInstanceLog, Log, TEXT("CancelSessionSearch called"));
    bIsSearchingForSessions = false;
    // Placeholder implementation for canceling session search
}

void UBloodreadGameInstance::LogBuildIDInfo()
{
    UE_LOG(BloodreadGameInstanceLog, Log, TEXT("LogBuildIDInfo called"));
    // Placeholder implementation for logging build ID info
}

void UBloodreadGameInstance::CreateSession(const FString& SessionName, int32 MaxPlayers, bool bIsLAN)
{
    UE_LOG(BloodreadGameInstanceLog, Log, TEXT("CreateSession called: %s, MaxPlayers: %d, LAN: %s"), 
           *SessionName, MaxPlayers, bIsLAN ? TEXT("true") : TEXT("false"));
    
    bIsCreatingSession = true;
    
    // Call the Blueprint implementation that uses AdvancedSteamSessions
    if (!bIsLAN)
    {
        UE_LOG(BloodreadGameInstanceLog, Log, TEXT("Calling Blueprint HostSession for Steam multiplayer"));
        HostSession(SessionName, MaxPlayers);
    }
    else
    {
        UE_LOG(BloodreadGameInstanceLog, Warning, TEXT("LAN sessions not implemented - use Steam sessions instead"));
        OnSessionCreated.Broadcast(false);
        bIsCreatingSession = false;
    }
}

void UBloodreadGameInstance::FindSessions(bool bIsLAN)
{
    UE_LOG(BloodreadGameInstanceLog, Log, TEXT("FindSessions called, LAN: %s"), bIsLAN ? TEXT("true") : TEXT("false"));
    bIsSearchingForSessions = true;
    
    // Call the Blueprint implementation that uses AdvancedSteamSessions
    if (!bIsLAN)
    {
        UE_LOG(BloodreadGameInstanceLog, Log, TEXT("Calling Blueprint FindAdvancedSteamSessions"));
        FindAdvancedSteamSessions();
    }
    else
    {
        UE_LOG(BloodreadGameInstanceLog, Warning, TEXT("LAN session search not implemented - use Steam sessions instead"));
        TArray<FSessionInfo> EmptySessions;
        OnSessionsFound.Broadcast(false, EmptySessions);
        bIsSearchingForSessions = false;
    }
}

void UBloodreadGameInstance::JoinSessionByIndex(int32 SessionIndex)
{
    UE_LOG(BloodreadGameInstanceLog, Log, TEXT("JoinSessionByIndex called with index: %d"), SessionIndex);
    
    // Call the Blueprint implementation that uses AdvancedSteamSessions
    UE_LOG(BloodreadGameInstanceLog, Log, TEXT("Calling Blueprint JoinAdvancedSteamSession"));
    JoinAdvancedSteamSession(SessionIndex);
}

void UBloodreadGameInstance::JoinSteamSession(int32 SessionIndex)
{
    UE_LOG(BloodreadGameInstanceLog, Log, TEXT("JoinSteamSession called with index: %d"), SessionIndex);
    
    // Call the Blueprint implementation that uses AdvancedSteamSessions
    JoinAdvancedSteamSession(SessionIndex);
}
