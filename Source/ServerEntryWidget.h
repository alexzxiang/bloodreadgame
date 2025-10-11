#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/TextBlock.h"
#include "Components/Button.h"
#include "BloodreadGameInstance.h"
#include "ServerEntryWidget.generated.h"

UCLASS()
class BLOODREADGAME_API UServerEntryWidget : public UUserWidget
{
    GENERATED_BODY()

public:
    UServerEntryWidget(const FObjectInitializer& ObjectInitializer);

    UFUNCTION(BlueprintCallable, Category = "Server Entry")
    void SetSessionInfo(const FSessionInfo& SessionInfo, int32 Index);

    UFUNCTION(BlueprintCallable, Category = "Server Entry")
    void OnServerEntryClicked();

    UFUNCTION(BlueprintCallable, Category = "Server Entry") 
    int32 GetSessionIndex() const { return SessionIndex; }

    UFUNCTION(BlueprintCallable, Category = "Server Entry")
    FSessionInfo GetSessionInfo() const { return CachedSessionInfo; }

protected:
    // Text blocks that will be bound from Blueprint
    UPROPERTY(BlueprintReadOnly, Category = "Server Entry", meta = (BindWidget))
    UTextBlock* SessionNameText;

    UPROPERTY(BlueprintReadOnly, Category = "Server Entry", meta = (BindWidget))
    UTextBlock* HostNameText;

    UPROPERTY(BlueprintReadOnly, Category = "Server Entry", meta = (BindWidget))
    UTextBlock* PlayerCountText;

    UPROPERTY(BlueprintReadOnly, Category = "Server Entry", meta = (BindWidget))
    UButton* ServerButton;

    UPROPERTY(BlueprintReadOnly, Category = "Server Entry")
    FSessionInfo CachedSessionInfo;

    UPROPERTY(BlueprintReadOnly, Category = "Server Entry")
    int32 SessionIndex = -1;

private:
    // Internal function to update the UI text
    void UpdateTextDisplay();
};