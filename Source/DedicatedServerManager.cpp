#include "DedicatedServerManager.h"
#include "Engine/Engine.h"
#include "HAL/PlatformProcess.h"
#include "HAL/PlatformFilemanager.h"
#include "Misc/Guid.h"
#include "Misc/DateTime.h"
#include "Engine/World.h"
#include "HttpModule.h"
#include "Interfaces/IHttpResponse.h"
#include "Dom/JsonObject.h"
#include "Serialization/JsonSerializer.h"
#include "Serialization/JsonWriter.h"

UDedicatedServerManager::UDedicatedServerManager()
{
    // Initialize with empty server list
}

bool UDedicatedServerManager::StartDedicatedServer(const FString& SessionName, const FString& MapName, int32 MaxPlayers, int32& OutPort, FString& OutServerID)
{
    if (RunningServers.Num() >= MaxConcurrentServers)
    {
        UE_LOG(LogTemp, Error, TEXT("Cannot start server: Maximum concurrent servers (%d) reached"), MaxConcurrentServers);
        return false;
    }

    // Generate unique server ID and get available port
    OutServerID = GenerateServerID();
    OutPort = GetNextAvailablePort();

    UE_LOG(LogTemp, Warning, TEXT("🚀 Starting dedicated server: %s on port %d"), *OutServerID, OutPort);
    UE_LOG(LogTemp, Warning, TEXT("   Session: %s, Map: %s, Max Players: %d"), *SessionName, *MapName, MaxPlayers);

    // Launch the dedicated server process
    FProcHandle ProcessHandle;
    if (!LaunchServerProcess(SessionName, MapName, OutPort, MaxPlayers, ProcessHandle))
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to launch dedicated server process"));
        return false;
    }

    // Create server record
    FRunningServer NewServer;
    NewServer.ServerID = OutServerID;
    NewServer.SessionName = SessionName;
    NewServer.Port = OutPort;
    NewServer.CurrentPlayers = 0;
    NewServer.MaxPlayers = MaxPlayers;
    NewServer.MapName = MapName;
    NewServer.StartTime = FDateTime::Now();
    NewServer.bIsActive = true;
    NewServer.ProcessHandle = ProcessHandle;

    RunningServers.Add(NewServer);

    // Register with database
    RegisterServerWithDatabase(OutServerID, SessionName, OutPort, MapName, MaxPlayers);

    UE_LOG(LogTemp, Warning, TEXT("✅ Dedicated server started successfully: %s"), *OutServerID);
    return true;
}

bool UDedicatedServerManager::StopDedicatedServer(const FString& ServerID)
{
    for (int32 i = 0; i < RunningServers.Num(); i++)
    {
        if (RunningServers[i].ServerID == ServerID)
        {
            UE_LOG(LogTemp, Warning, TEXT("🛑 Stopping dedicated server: %s"), *ServerID);

            // Kill the process
            if (RunningServers[i].ProcessHandle.IsValid() && !KillServerProcess(RunningServers[i].ProcessHandle))
            {
                UE_LOG(LogTemp, Warning, TEXT("Failed to kill server process gracefully"));
            }

            // Unregister from database
            UnregisterServerFromDatabase(ServerID);

            // Remove from array
            RunningServers.RemoveAt(i);

            UE_LOG(LogTemp, Warning, TEXT("✅ Dedicated server stopped: %s"), *ServerID);
            return true;
        }
    }

    UE_LOG(LogTemp, Warning, TEXT("Server not found for stopping: %s"), *ServerID);
    return false;
}

TArray<FRunningServer> UDedicatedServerManager::GetRunningServers()
{
    return RunningServers;
}

void UDedicatedServerManager::UpdateServerStatus()
{
    UE_LOG(LogTemp, Log, TEXT("Updating status for %d running servers"), RunningServers.Num());

    for (int32 i = RunningServers.Num() - 1; i >= 0; i--)
    {
        FRunningServer& Server = RunningServers[i];
        
        // Check if process is still running
        if (Server.ProcessHandle.IsValid() && !IsProcessRunning(Server.ProcessHandle))
        {
            UE_LOG(LogTemp, Warning, TEXT("Server process died: %s"), *Server.ServerID);
            
            // Unregister from database
            UnregisterServerFromDatabase(Server.ServerID);
            
            // Remove from list
            RunningServers.RemoveAt(i);
            continue;
        }

        // TODO: Query actual player count from server
        // For now, we'll keep the existing values
    }
}

void UDedicatedServerManager::RegisterServerWithDatabase(const FString& ServerID, const FString& SessionName, int32 Port, const FString& MapName, int32 MaxPlayers)
{
    UE_LOG(LogTemp, Warning, TEXT("📝 Registering server with database: %s"), *ServerID);

    // Create HTTP request
    FHttpRequestRef Request = FHttpModule::Get().CreateRequest();
    Request->SetURL(TEXT("http://bloodread.games/api/server_register.php"));
    Request->SetVerb(TEXT("POST"));
    Request->SetHeader(TEXT("Content-Type"), TEXT("application/json"));

    // Create JSON payload
    TSharedPtr<FJsonObject> JsonObject = MakeShareable(new FJsonObject);
    JsonObject->SetStringField(TEXT("server_id"), ServerID);
    JsonObject->SetStringField(TEXT("session_name"), SessionName);
    JsonObject->SetNumberField(TEXT("port"), Port);
    JsonObject->SetStringField(TEXT("map_name"), MapName);
    JsonObject->SetNumberField(TEXT("max_players"), MaxPlayers);
    JsonObject->SetNumberField(TEXT("current_players"), 0);
    JsonObject->SetStringField(TEXT("server_ip"), TEXT("127.0.0.1")); // TODO: Get actual server IP
    JsonObject->SetStringField(TEXT("status"), TEXT("active"));

    FString OutputString;
    TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&OutputString);
    FJsonSerializer::Serialize(JsonObject.ToSharedRef(), Writer);

    Request->SetContentAsString(OutputString);

    // Bind response delegate
    Request->OnProcessRequestComplete().BindLambda([ServerID](FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful)
    {
        if (bWasSuccessful && Response.IsValid())
        {
            UE_LOG(LogTemp, Warning, TEXT("✅ Server registered with database: %s"), *ServerID);
        }
        else
        {
            UE_LOG(LogTemp, Error, TEXT("❌ Failed to register server with database: %s"), *ServerID);
        }
    });

    Request->ProcessRequest();
}

void UDedicatedServerManager::UnregisterServerFromDatabase(const FString& ServerID)
{
    UE_LOG(LogTemp, Warning, TEXT("🗑️ Unregistering server from database: %s"), *ServerID);

    // Create HTTP request
    FHttpRequestRef Request = FHttpModule::Get().CreateRequest();
    Request->SetURL(FString::Printf(TEXT("http://bloodread.games/api/server_unregister.php?server_id=%s"), *ServerID));
    Request->SetVerb(TEXT("DELETE"));

    // Bind response delegate
    Request->OnProcessRequestComplete().BindLambda([ServerID](FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful)
    {
        if (bWasSuccessful)
        {
            UE_LOG(LogTemp, Warning, TEXT("✅ Server unregistered from database: %s"), *ServerID);
        }
        else
        {
            UE_LOG(LogTemp, Error, TEXT("❌ Failed to unregister server from database: %s"), *ServerID);
        }
    });

    Request->ProcessRequest();
}

bool UDedicatedServerManager::IsDedicatedServer()
{
    return IsRunningDedicatedServer();
}

int32 UDedicatedServerManager::GetNextAvailablePort()
{
    int32 Port = BaseServerPort;
    
    // Find the next available port
    while (true)
    {
        bool bPortInUse = false;
        
        for (const FRunningServer& Server : RunningServers)
        {
            if (Server.Port == Port)
            {
                bPortInUse = true;
                break;
            }
        }
        
        if (!bPortInUse)
        {
            return Port;
        }
        
        Port++;
    }
}

FString UDedicatedServerManager::GenerateServerID()
{
    FGuid NewGuid = FGuid::NewGuid();
    return FString::Printf(TEXT("SERVER_%s"), *NewGuid.ToString());
}

bool UDedicatedServerManager::LaunchServerProcess(const FString& SessionName, const FString& MapName, int32 Port, int32 MaxPlayers, FProcHandle& OutProcessHandle)
{
    // Use regular game executable as server with -server flag
    FString ExecutablePath = FPaths::Combine(FPaths::ProjectDir(), TEXT("Binaries/Win64/BloodreadGame.exe"));
    
    // Build command line arguments for headless server mode
    FString CommandLine = FString::Printf(TEXT("%s?listen -server -log -port=%d -MaxPlayers=%d -nullrhi -nosound -nosteam"), 
                                         *MapName, Port, MaxPlayers);

    UE_LOG(LogTemp, Warning, TEXT("Launching server: %s %s"), *ExecutablePath, *CommandLine);

    // Launch the process
    OutProcessHandle = FPlatformProcess::CreateProc(*ExecutablePath, *CommandLine, false, false, false, nullptr, 0, nullptr, nullptr);
    
    if (!OutProcessHandle.IsValid())
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to create server process"));
        return false;
    }

    return true;
}

bool UDedicatedServerManager::KillServerProcess(FProcHandle& ProcessHandle)
{
    if (!ProcessHandle.IsValid())
    {
        return false;
    }

    FPlatformProcess::TerminateProc(ProcessHandle, true);
    FPlatformProcess::CloseProc(ProcessHandle);
    return true;
}

bool UDedicatedServerManager::IsProcessRunning(FProcHandle& ProcessHandle)
{
    if (!ProcessHandle.IsValid())
    {
        return false;
    }

    return FPlatformProcess::IsProcRunning(ProcessHandle);
}