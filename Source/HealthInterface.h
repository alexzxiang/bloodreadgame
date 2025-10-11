#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "HealthInterface.generated.h"

// This class does not need to be modified.
UINTERFACE(MinimalAPI, Blueprintable)
class UHealthInterface : public UInterface
{
    GENERATED_BODY()
};

/**
 * Health Interface for characters that can take damage and heal
 */
class BLOODREADGAME_API IHealthInterface
{
    GENERATED_BODY()

public:
    // Health query functions
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Health")
    float GetCurrentHealth() const;
    virtual float GetCurrentHealth_Implementation() const = 0;

    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Health")
    float GetMaxHealth() const;
    virtual float GetMaxHealth_Implementation() const = 0;

    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Health")
    float GetHealthPercentage() const;
    virtual float GetHealthPercentage_Implementation() const = 0;

    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Health")
    bool IsAlive() const;
    virtual bool IsAlive_Implementation() const = 0;

    // Health modification functions
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Health")
    void ReceiveDamage(float DamageAmount);
    virtual void ReceiveDamage_Implementation(float DamageAmount) = 0;

    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Health")
    void Heal(float HealAmount);
    virtual void Heal_Implementation(float HealAmount) = 0;

    // Utility functions for UI
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Health")
    FString GetHealthText() const;
    virtual FString GetHealthText_Implementation() const = 0;
};
