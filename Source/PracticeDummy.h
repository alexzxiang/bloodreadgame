#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "Components/StaticMeshComponent.h"
#include "Components/WidgetComponent.h"
#include "Components/CapsuleComponent.h"
#include "PracticeDummy.generated.h"

// Forward declarations
class ABloodreadPlayerCharacter;

UCLASS()
class BLOODREADGAME_API APracticeDummy : public APawn
{
    GENERATED_BODY()

public:
    APracticeDummy();

protected:
    virtual void BeginPlay() override;

public:
    virtual void Tick(float DeltaTime) override;

    // Dummy stats
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Dummy Stats")
    int32 MaxHealth = 200;

    UPROPERTY(BlueprintReadOnly, Category="Dummy Stats")
    int32 CurrentHealth = 200;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Dummy")
    float KnockbackResistance = 0.5f; // Reduces knockback force

    // Combat functions
    UFUNCTION(BlueprintCallable, Category="Combat")
    void TakeCustomDamage(int32 Damage, ABloodreadPlayerCharacter* Attacker = nullptr);

    UFUNCTION(BlueprintCallable, Category="Combat")
    void ApplyKnockback(FVector KnockbackDirection, float Force);

    UFUNCTION(BlueprintCallable, Category="Combat")
    void ResetDummy();
    
    UFUNCTION(BlueprintCallable, Category="Combat")
    bool IsAlive() const { return CurrentHealth > 0; }
    
    // Health bar widget management
    UFUNCTION(BlueprintCallable, Category="UI")
    void SetHealthBarWidget(class UUserWidget* Widget);
    
    UFUNCTION(BlueprintCallable, Category="UI") 
    void UpdateHealthDisplay();
    
    // Visual effects
    UFUNCTION(BlueprintCallable, Category="Visual Effects")
    void FlashRed();
    
    UFUNCTION(BlueprintCallable, Category="Visual Effects")
    void StopFlashRed();

    // Game tick processing
    UFUNCTION(BlueprintCallable, Category="Game Tick")
    void ProcessGameTick();

    // Events
    UFUNCTION(BlueprintImplementableEvent, Category="Combat")
    void OnTakeDamage(int32 Damage, ABloodreadPlayerCharacter* Attacker);

    UFUNCTION(BlueprintImplementableEvent, Category="Combat")
    void OnKnockbackApplied(FVector Direction, float Force);

    // Getters
    UFUNCTION(BlueprintPure, Category="Dummy Stats")
    float GetHealthPercentage() const;

    UFUNCTION(BlueprintPure, Category="Dummy Stats")
    int32 GetCurrentHealth() const { return CurrentHealth; }

    UFUNCTION(BlueprintPure, Category="Dummy Stats")
    int32 GetMaxHealth() const { return MaxHealth; }

    // Debug function to manually test health updates
    UFUNCTION(BlueprintCallable, Category="Debug")
    void DebugUpdateHealthBar();

    UFUNCTION(BlueprintCallable, Category="Debug") 
    void DebugTakeDamage() { TakeCustomDamage(25, nullptr); }

    UFUNCTION(BlueprintCallable, Category="Debug")
    void FixHealthBarWidget();

    UFUNCTION(BlueprintCallable, Category="Debug")
    void DebugWidgetInfo();protected:
    // Components
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components")
    UCapsuleComponent* CapsuleComponent;
    
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components")
    UStaticMeshComponent* DummyMesh;
    
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components")
    UWidgetComponent* HealthBarWidgetComponent;

    // Damage immunity (follows game tick system)
    UPROPERTY(BlueprintReadOnly, Category="Combat")
    int32 DamageImmunityTicksRemaining = 0;

    UPROPERTY(BlueprintReadOnly, Category="Combat")
    bool bCanTakeDamage = true;
    
    // Physics and movement
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Physics")
    bool bUseStablePhysics = true;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Physics")
    float KnockbackDecayRate = 5.0f; // How fast knockback velocity decays
    
    UPROPERTY(BlueprintReadOnly, Category="Physics")
    FVector CurrentKnockbackVelocity = FVector::ZeroVector;
    
    UPROPERTY(BlueprintReadOnly, Category="Physics")
    FVector InitialLocation = FVector::ZeroVector;
    
    // Visual effects
    UPROPERTY(BlueprintReadOnly, Category="Visual Effects")
    bool bIsFlashingRed = false;
    
    UPROPERTY(BlueprintReadOnly, Category="Visual Effects")
    FTimerHandle RedFlashTimer;
    
    // Health bar reference
    UPROPERTY(BlueprintReadOnly, Category="UI")
    class UUserWidget* CurrentHealthBarWidget = nullptr;
};
