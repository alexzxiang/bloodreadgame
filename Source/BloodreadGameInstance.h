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


UCLASS()
class BLOODREADGAME_API UBloodreadGameInstance : public UGameInstance
{
    GENERATED_BODY()

public:
    UBloodreadGameInstance();

    virtual void Init() override;
    virtual void Shutdown() override;

private:
    // Database configuration (keep for player data, etc.)
    FString DatabaseAPIURL = TEXT("https://your-digitalocean-app.ondigitalocean.app/api");
};