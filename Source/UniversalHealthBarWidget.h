#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"
#include "BloodreadBaseCharacter.h"
#include "PracticeDummy.h"
#include "UniversalHealthBarWidget.generated.h"

/**
 * Universal health bar widget that works with both BloodreadBaseCharacter and PracticeDummy
 * Replaces the dummy-specific widget to support all character types
 */
UCLASS()
class BLOODREADGAME_API UUniversalHealthBarWidget : public UUserWidget
{
    GENERATED_BODY()

protected:
    // Widget components (automatically bound from Blueprint)
    UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
    class UProgressBar* HealthProgressBar;

    UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
    class UTextBlock* HealthText;

    UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
    class UTextBlock* CharacterNameText;

    // References to owner actors
    UPROPERTY(BlueprintReadOnly, Category = "Character")
    ABloodreadBaseCharacter* OwnerCharacter;

    UPROPERTY(BlueprintReadOnly, Category = "Character")
    APracticeDummy* OwnerDummy;

    // Current health data
    UPROPERTY(BlueprintReadOnly, Category = "Health")
    int32 CurrentHealth;

    UPROPERTY(BlueprintReadOnly, Category = "Health")
    int32 MaxHealth;

    UPROPERTY(BlueprintReadOnly, Category = "Health")
    float HealthPercentage;

public:
    // Initialize with either character type
    UFUNCTION(BlueprintCallable, Category = "Health")
    void InitializeWithCharacter(ABloodreadBaseCharacter* Character);

    UFUNCTION(BlueprintCallable, Category = "Health")
    void InitializeWithDummy(APracticeDummy* Dummy);

    // Universal update function
    UFUNCTION(BlueprintCallable, Category = "Health")
    void UpdateHealthDisplay();

    // Blueprint-bindable functions for UI
    UFUNCTION(BlueprintPure, Category = "Health")
    float GetHealthPercentage() const;

    UFUNCTION(BlueprintPure, Category = "Health")
    FText GetHealthText() const;

    UFUNCTION(BlueprintPure, Category = "Health")
    FText GetCharacterName() const;

    UFUNCTION(BlueprintPure, Category = "Health")
    FText GetCharacterClass() const;

    // Helper functions
    UFUNCTION(BlueprintPure, Category = "Health")
    bool IsOwnerAlive() const;
    
    // Additional Blueprint-callable functions for easy binding
    UFUNCTION(BlueprintPure, Category = "Health")
    int32 GetCurrentHealth() const { return CurrentHealth; }
    
    UFUNCTION(BlueprintPure, Category = "Health")
    int32 GetMaxHealth() const { return MaxHealth; }
    
    // Auto-update system for real-time health tracking
    UFUNCTION(BlueprintCallable, Category = "Health")
    void StartAutoUpdate();
    
    UFUNCTION(BlueprintCallable, Category = "Health")
    void StopAutoUpdate();
    
    // Override UUserWidget functions for automatic initialization
    virtual void NativeConstruct() override;
    virtual void NativeDestruct() override;

protected:
    // Timer for auto-updating health display
    UPROPERTY()
    FTimerHandle UpdateTimerHandle;
    
    // Update interval (configurable in Blueprint)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Health Bar Settings")
    float UpdateInterval = 0.1f;
    
    // Automatically start updating when constructed
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Health Bar Settings")
    bool bAutoStartUpdating = true;

private:
    // Internal helper to get health data from either owner type
    void RefreshHealthData();
    FString GetOwnerDisplayName() const;
    FString GetOwnerClassName() const;
    
    // Auto-update function called by timer
    void UpdateDisplay();
    
    // Apply visual updates to widget components
    void ApplyHealthToWidgets();
};
