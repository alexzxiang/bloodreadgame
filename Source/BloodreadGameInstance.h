#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "Http.h"
#include "Json.h"

#include "BloodreadGameInstance.generated.h"

// Log category declaration
DECLARE_LOG_CATEGORY_EXTERN(BloodreadGameInstanceLog, Log, All);

// Legacy structs for backward compatibility (kept for existing code)
USTRUCT(BlueprintType)
struct FPlayerData
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite)
    int32 PlayerId = 0;

    UPROPERTY(BlueprintReadWrite)
    FString Username;

    UPROPERTY(BlueprintReadWrite)
    int32 Credits = 100;

    UPROPERTY(BlueprintReadWrite)
    int32 TotalWins = 0;

    UPROPERTY(BlueprintReadWrite)
    int32 TotalLosses = 0;
};

USTRUCT(BlueprintType)
struct FAbilityData
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite)
    int32 Id = 0;

    UPROPERTY(BlueprintReadWrite)
    FString Name;

    UPROPERTY(BlueprintReadWrite)
    FString Description;

    UPROPERTY(BlueprintReadWrite)
    int32 Cost = 0;

    UPROPERTY(BlueprintReadWrite)
    int32 Damage = 0;

    UPROPERTY(BlueprintReadWrite)
    float Cooldown = 0.0f;

    UPROPERTY(BlueprintReadWrite)
    float KnockbackForce = 0.0f;
};

USTRUCT(BlueprintType)
struct FItemData
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite)
    int32 Id = 0;

    UPROPERTY(BlueprintReadWrite)
    FString Name;

    UPROPERTY(BlueprintReadWrite)
    FString Description;

    UPROPERTY(BlueprintReadWrite)
    int32 Cost = 0;

    UPROPERTY(BlueprintReadWrite)
    int32 HealthBonus = 0;

    UPROPERTY(BlueprintReadWrite)
    int32 StrengthBonus = 0;
};

USTRUCT(BlueprintType)
struct FSessionInfo
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite)
    FString SessionName;

    UPROPERTY(BlueprintReadWrite)
    FString HostName;

    UPROPERTY(BlueprintReadWrite)
    FString MapName;

    UPROPERTY(BlueprintReadWrite)
    int32 CurrentPlayers = 0;

    UPROPERTY(BlueprintReadWrite)
    int32 MaxPlayers = 8;

    UPROPERTY(BlueprintReadWrite)
    int32 Ping = 0;

    UPROPERTY(BlueprintReadWrite)
    bool bIsLAN = false;
    
    UPROPERTY(BlueprintReadWrite)
    FString IPAddress;
    
    UPROPERTY(BlueprintReadWrite)
    int32 Port = 7777;
};

USTRUCT(BlueprintType)
struct FDiscoveredServer
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite)
    FString SessionName;

    UPROPERTY(BlueprintReadWrite)
    FString IPAddress;

    UPROPERTY(BlueprintReadWrite)
    int32 Port = 7777;

    UPROPERTY(BlueprintReadWrite)
    int32 MaxPlayers = 8;

    UPROPERTY(BlueprintReadWrite)
    int32 CurrentPlayers = 0;

    UPROPERTY(BlueprintReadWrite)
    FString MapName;

    UPROPERTY(BlueprintReadWrite)
    bool bIsActive = true;
};

// Blueprint assignable delegates for backward compatibility
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnSessionCreated, bool, bWasSuccessful);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnSessionJoined, bool, bWasSuccessful);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnSessionsFound, bool, bWasSuccessful, const TArray<FSessionInfo>&, Sessions);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnPlayerLoginComplete, bool, bWasSuccessful, const FPlayerData&, PlayerData);

UCLASS()
class BLOODREADGAME_API UBloodreadGameInstance : public UGameInstance
{
    GENERATED_BODY()

public:
    UBloodreadGameInstance();

    virtual void Init() override;
    virtual void Shutdown() override;

    // Steam multiplayer system using AdvancedSteamSessions
    UFUNCTION(BlueprintImplementableEvent, Category="Steam Multiplayer")
    void HostSession(const FString& ServerName, int32 AmountOfSlots);

    UFUNCTION(BlueprintImplementableEvent, Category="Steam Multiplayer")
    void FindAdvancedSteamSessions();

    UFUNCTION(BlueprintImplementableEvent, Category="Steam Multiplayer")
    void JoinAdvancedSteamSession(int32 SessionIndex);

    UFUNCTION(BlueprintImplementableEvent, Category="Dedicated Server")
    void OnServersRefreshedEvent(bool bWasSuccessful, const TArray<FDiscoveredServer>& Servers);

    UFUNCTION(BlueprintCallable, Category="Multiplayer")
    void JoinSteamSession(int32 SessionIndex);

    // Database/Player management methods
    UFUNCTION(BlueprintCallable, Category="Database")
    void LoginPlayer(const FString& Username, const FString& Password);

    UFUNCTION(BlueprintCallable, Category="Database")
    void CreatePlayer(const FString& Username, const FString& Password);

    // Steam connectivity check for debugging
    UFUNCTION(BlueprintCallable, BlueprintPure, Category="Multiplayer")
    FString GetSteamConnectionStatus();

    UFUNCTION(BlueprintCallable, Category="Multiplayer")
    void TestSteamConnectivity();

    // Session management utilities
    UFUNCTION(BlueprintCallable, Category="Multiplayer")
    void CancelSessionSearch();

    // Removed ForceResetSessionStates() - root-cause solution uses proper state management instead

    UFUNCTION(BlueprintCallable, Category="Multiplayer")
    void LogBuildIDInfo();

    UFUNCTION(BlueprintCallable, BlueprintPure, Category="Multiplayer")
    bool IsSearchingForSessions() const { return bIsSearchingForSessions; }

    UFUNCTION(BlueprintCallable, BlueprintPure, Category="Multiplayer")
    bool IsCreatingSession() const { return bIsCreatingSession; }

    // Legacy session methods that wrap Advanced Sessions functionality
    UFUNCTION(BlueprintCallable, Category="Multiplayer")
    void CreateSession(const FString& SessionName, int32 MaxPlayers, bool bIsLAN);

    UFUNCTION(BlueprintCallable, Category="Multiplayer") 
    void FindSessions(bool bIsLAN);

    UFUNCTION(BlueprintCallable, Category="Multiplayer")
    void JoinSessionByIndex(int32 SessionIndex);

    UFUNCTION(BlueprintCallable, Category="Multiplayer")
    void JoinSessionByIP(const FString& IPAddress, int32 Port);

    // Blueprint assignable delegates for backward compatibility
    UPROPERTY(BlueprintAssignable)
    FOnSessionCreated OnSessionCreated;

    UPROPERTY(BlueprintAssignable)
    FOnSessionJoined OnSessionJoined;

    UPROPERTY(BlueprintAssignable)
    FOnSessionsFound OnSessionsFound;

    UPROPERTY(BlueprintAssignable)
    FOnPlayerLoginComplete OnLoginComplete;

    // Blueprint callable functions to be called from AdvancedSteamSessions callbacks
    UFUNCTION(BlueprintCallable, Category="Multiplayer")
    void OnSteamSessionCreated(bool bWasSuccessful);

    UFUNCTION(BlueprintCallable, Category="Multiplayer")
    void OnSteamSessionJoined(bool bWasSuccessful);

    UFUNCTION(BlueprintCallable, Category="Multiplayer")
    void OnSteamSessionsFound(bool bWasSuccessful, const TArray<FSessionInfo>& Sessions);

private:
    // Session state tracking
    bool bIsSearchingForSessions = false;
    bool bIsCreatingSession = false;
    
    // Database configuration (keep for player data, etc.)
    FString DatabaseAPIURL = TEXT("https://your-digitalocean-app.ondigitalocean.app/api");
    
    // HTTP request helpers for database integration (keep for player data)
    void MakeHTTPRequest(const FString& Endpoint, const FString& JsonPayload, TFunction<void(bool, const FString&)> Callback);
    void OnFetchServersResponse(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful);
};