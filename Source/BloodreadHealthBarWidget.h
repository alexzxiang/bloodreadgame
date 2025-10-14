#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "BloodreadHealthBarWidget.generated.h"

// Forward declarations
class ABloodreadBaseCharacter;
class UProgressBar;
class UTextBlock;

UCLASS()
class BLOODREADGAME_API UBloodreadHealthBarWidget : public UUserWidget
{
    GENERATED_BODY()

public:
    UBloodreadHealthBarWidget(const FObjectInitializer& ObjectInitializer);

protected:
    virtual void NativeConstruct() override;
    virtual void NativeDestruct() override;

public:
    UFUNCTION(BlueprintCallable, Category = "Health Bar")
    void InitializeHealthBar(ABloodreadBaseCharacter* Character);

    UFUNCTION(BlueprintCallable, Category = "Health Bar")
    void UpdateHealthDisplay();

    UFUNCTION(BlueprintCallable, Category = "Health Bar")
    void SetUpdateFrequency(float NewUpdateFrequency);

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Health Bar")
    FString GetHealthText() const;

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Health Bar")
    FString GetManaText() const;

protected:
    UFUNCTION()
    void BindHealthEvents(ABloodreadBaseCharacter* Character);

    UFUNCTION()
    void UnbindHealthEvents();

    UFUNCTION()
    void OnCharacterHealthChanged(int32 OldHealth, int32 NewHealth);

protected:
    // Widget components (optional bindings)
    UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
    UProgressBar* HealthBar;

    UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
    UProgressBar* HealthProgressBar;

    UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
    UTextBlock* HealthText;

    UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
    UTextBlock* CharacterClassText;

    // Character reference
    UPROPERTY(BlueprintReadOnly, Category = "Character")
    ABloodreadBaseCharacter* TargetCharacter;

    UPROPERTY(BlueprintReadOnly, Category = "Character")
    ABloodreadBaseCharacter* PlayerCharacterRef;

    // Update frequency
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Health Bar")
    float UpdateFrequency = 0.1f;

    // Timer handles
    FTimerHandle UpdateTimer;
    FTimerHandle UpdateTimerHandle;

    // Colors for health bar
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Health Bar")
    FLinearColor HealthyColor;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Health Bar")
    FLinearColor DamagedColor;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Health Bar")
    FLinearColor CriticalColor;
};