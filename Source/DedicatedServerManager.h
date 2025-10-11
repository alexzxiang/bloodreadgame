#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "HAL/Platform.h"
#include "DedicatedServerManager.generated.h"

USTRUCT(BlueprintType)
struct FRunningServer
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly)
    FString ServerID;
    
    UPROPERTY(BlueprintReadOnly)
    FString SessionName;
    
    UPROPERTY(BlueprintReadOnly)
    int32 Port;
    
    UPROPERTY(BlueprintReadOnly)
    int32 CurrentPlayers;
    
    UPROPERTY(BlueprintReadOnly)
    int32 MaxPlayers;
    
    UPROPERTY(BlueprintReadOnly)
    FString MapName;
    
    UPROPERTY(BlueprintReadOnly)
    FDateTime StartTime;
    
    UPROPERTY(BlueprintReadOnly)
    bool bIsActive;

    // Process handle for the dedicated server (not exposed to Blueprint)
    FProcHandle ProcessHandle;

    FRunningServer()
    {
        ServerID = TEXT("");
        SessionName = TEXT("");
        Port = 0;
        CurrentPlayers = 0;
        MaxPlayers = 4;
        MapName = TEXT("");
        StartTime = FDateTime::Now();
        bIsActive = false;
        ProcessHandle = FProcHandle();
    }
};

UCLASS(BlueprintType)
class BLOODREADGAME_API UDedicatedServerManager : public UObject
{
    GENERATED_BODY()

public:
    UDedicatedServerManager();

    // Start a new dedicated server instance
    UFUNCTION(BlueprintCallable, Category = "Server Manager")
    bool StartDedicatedServer(const FString& SessionName, const FString& MapName, int32 MaxPlayers, int32& OutPort, FString& OutServerID);

    // Stop a dedicated server instance
    UFUNCTION(BlueprintCallable, Category = "Server Manager")
    bool StopDedicatedServer(const FString& ServerID);

    // Get all running servers
    UFUNCTION(BlueprintCallable, Category = "Server Manager")
    TArray<FRunningServer> GetRunningServers();

    // Update server status (called periodically)
    UFUNCTION(BlueprintCallable, Category = "Server Manager")
    void UpdateServerStatus();

    // Register this server with the database
    UFUNCTION(BlueprintCallable, Category = "Server Manager")
    void RegisterServerWithDatabase(const FString& ServerID, const FString& SessionName, int32 Port, const FString& MapName, int32 MaxPlayers);

    // Unregister server from database
    UFUNCTION(BlueprintCallable, Category = "Server Manager")
    void UnregisterServerFromDatabase(const FString& ServerID);

    // Check if we're running as a dedicated server
    UFUNCTION(BlueprintCallable, Category = "Server Manager")
    static bool IsDedicatedServer();

    // Get the next available port
    UFUNCTION(BlueprintCallable, Category = "Server Manager")
    int32 GetNextAvailablePort();

protected:
    // Array of running server instances
    UPROPERTY(BlueprintReadOnly, Category = "Server Manager")
    TArray<FRunningServer> RunningServers;

    // Base port for dedicated servers
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Server Manager")
    int32 BaseServerPort = 7777;

    // Maximum number of concurrent servers
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Server Manager")
    int32 MaxConcurrentServers = 10;

private:
    // Generate unique server ID
    FString GenerateServerID();
    
    // Launch dedicated server process
    bool LaunchServerProcess(const FString& SessionName, const FString& MapName, int32 Port, int32 MaxPlayers, FProcHandle& OutProcessHandle);
    
    // Kill server process
    bool KillServerProcess(FProcHandle& ProcessHandle);
    
    // Check if process is still running
    bool IsProcessRunning(FProcHandle& ProcessHandle);
};