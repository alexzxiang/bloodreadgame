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