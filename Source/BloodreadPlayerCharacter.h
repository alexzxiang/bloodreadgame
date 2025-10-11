#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Camera/CameraComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Blueprint/UserWidget.h"
#include "BloodreadGameInstance.h"
#include "PracticeDummy.h"
#include "BloodreadPlayerCharacter.generated.h"

USTRUCT(BlueprintType)
struct FPlayerStats
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, EditAnywhere)
    int32 MaxHealth = 100;

    UPROPERTY(BlueprintReadWrite, EditAnywhere)
    int32 CurrentHealth = 100;

    UPROPERTY(BlueprintReadWrite, EditAnywhere)
    int32 Strength = 10;

    UPROPERTY(BlueprintReadWrite, EditAnywhere)
    int32 DamageImmunityTicksRemaining = 0;

    UPROPERTY(BlueprintReadWrite, EditAnywhere)
    bool bCanTakeDamage = true;
};

USTRUCT(BlueprintType)
struct FPlayerLoadout
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite)
    FAbilityData Ability1;

    UPROPERTY(BlueprintReadWrite)
    FAbilityData Ability2;

    UPROPERTY(BlueprintReadWrite)
    FItemData EquippedItem;

    UPROPERTY(BlueprintReadWrite)
    float Ability1CooldownRemaining = 0.0f;

    UPROPERTY(BlueprintReadWrite)
    float Ability2CooldownRemaining = 0.0f;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnHealthChanged, int32, NewHealth, int32, MaxHealth);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPlayerDeath, ABloodreadPlayerCharacter*, DeadPlayer);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnDamageDealt, ABloodreadPlayerCharacter*, Attacker, ABloodreadPlayerCharacter*, Victim, int32, Damage);

UCLASS()
class BLOODREADGAME_API ABloodreadPlayerCharacter : public ACharacter
{
    GENERATED_BODY()

public:
    ABloodreadPlayerCharacter();

protected:
    virtual void BeginPlay() override;

public:
    virtual void Tick(float DeltaTime) override;
    virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

    // Player stats
    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="Player Stats")
    FPlayerStats PlayerStats;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="Player Loadout")
    FPlayerLoadout PlayerLoadout;

    // Combat settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Combat")
    float SwordRange = 200.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Combat")
    float SwordDamage = 15.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Combat")
    float SwordKnockbackForce = 300.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Combat")
    float KnockbackUpwardForce = 100.0f;

    // Movement settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Movement")
    float JumpHeight = 600.0f;

    // Events
    UPROPERTY(BlueprintAssignable)
    FOnHealthChanged OnHealthChanged;

    UPROPERTY(BlueprintAssignable)
    FOnPlayerDeath OnPlayerDeath;

    UPROPERTY(BlueprintAssignable)
    FOnDamageDealt OnDamageDealt;

    // Combat functions
    UFUNCTION(BlueprintCallable, Category="Combat")
    void AttackWithSword();

    UFUNCTION(BlueprintCallable, Category="Combat")
    void UseAbility1();

    UFUNCTION(BlueprintCallable, Category="Combat")
    void UseAbility2();

    UFUNCTION(BlueprintCallable, Category="Combat")
    bool TakeCustomDamage(int32 Damage, ABloodreadPlayerCharacter* Attacker = nullptr);

    UFUNCTION(BlueprintCallable, Category="Combat")
    void ApplyKnockback(FVector KnockbackDirection, float Force);

    UFUNCTION(BlueprintCallable, Category="Combat")
    void Heal(int32 Amount);

    // Test/Debug functions
    UFUNCTION(BlueprintCallable, Category="Debug")
    void TestTakeDamage();
    
    UFUNCTION(BlueprintCallable, Category="Debug")
    void TestHeal();

    // Targeting functions
    UFUNCTION(BlueprintCallable, Category="Combat")
    APracticeDummy* GetCrosshairTarget();

    // Utility functions
    UFUNCTION(BlueprintCallable, Category="Player")
    void InitializeFromLoadout(const FPlayerLoadout& Loadout, int32 HealthBonus, int32 StrengthBonus);

    UFUNCTION(BlueprintCallable, Category="Player")
    FVector GetDirectionToOtherPlayer();

    UFUNCTION(BlueprintCallable, Category="Player")
    ABloodreadPlayerCharacter* FindOtherPlayer();

    // Game tick processing
    UFUNCTION(BlueprintCallable, Category="Game Tick")
    void ProcessGameTick();

    // Getters
    UFUNCTION(BlueprintPure, Category="Player Stats")
    bool IsAlive() const { return PlayerStats.CurrentHealth > 0; }

    UFUNCTION(BlueprintPure, Category="Player Stats")
    float GetHealthPercentage() const;
    
    // Additional health system functions
    UFUNCTION(BlueprintPure, Category="Player Stats")
    int32 GetCurrentHealth() const { return PlayerStats.CurrentHealth; }
    
    UFUNCTION(BlueprintPure, Category="Player Stats")
    int32 GetMaxHealth() const { return PlayerStats.MaxHealth; }
    
    UFUNCTION(BlueprintCallable, Category="Player Stats")
    void SetMaxHealth(int32 NewMaxHealth);
    
    UFUNCTION(BlueprintCallable, Category="Player Stats")
    void SetCurrentHealth(int32 NewHealth);
    
    UFUNCTION(BlueprintPure, Category="Player Stats")
    bool CanHeal() const { return IsAlive() && PlayerStats.CurrentHealth < PlayerStats.MaxHealth; }
    
    UFUNCTION(BlueprintCallable, Category="Player Stats")
    void RestoreToFullHealth();
    
    // UI Update Functions
    UFUNCTION(BlueprintCallable, Category="UI")
    void UpdateHealthDisplay();
    
    UFUNCTION(BlueprintCallable, Category="UI")
    void SetHealthBarWidget(UUserWidget* Widget);
    
    // Ability cooldown system functions
    UFUNCTION(BlueprintPure, Category="Abilities")
    bool CanUseAbility1() const;
    
    UFUNCTION(BlueprintPure, Category="Abilities")
    bool CanUseAbility2() const;
    
    UFUNCTION(BlueprintPure, Category="Abilities")
    float GetAbility1CooldownRemaining() const { return PlayerLoadout.Ability1CooldownRemaining; }
    
    UFUNCTION(BlueprintPure, Category="Abilities")
    float GetAbility2CooldownRemaining() const { return PlayerLoadout.Ability2CooldownRemaining; }
    
    UFUNCTION(BlueprintPure, Category="Abilities")
    float GetAbility1CooldownPercentage() const;
    
    UFUNCTION(BlueprintPure, Category="Abilities")
    float GetAbility2CooldownPercentage() const;

    // Visual effects functions
    UFUNCTION(BlueprintImplementableEvent, BlueprintCallable, Category="Visual Effects")
    void FlashRed();

    UFUNCTION(BlueprintImplementableEvent, BlueprintCallable, Category="Visual Effects")
    void StopFlashRed();

    // Camera adjustment functions for custom meshes
    UFUNCTION(BlueprintCallable, Category="Camera")
    void SetCameraPosition(FVector NewRelativeLocation);
    
    UFUNCTION(BlueprintCallable, Category="Camera")
    void AdjustCameraForMesh(float HeightOffset = 0.0f, float ForwardOffset = 0.0f, float RightOffset = 0.0f);
    
    UFUNCTION(BlueprintPure, Category="Camera")
    FVector GetCurrentCameraPosition() const;

    // Properties for custom mesh camera adjustment
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Camera")
    float MeshScaleMultiplier = 1.0f;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Camera")
    FVector CustomMeshCameraOffset = FVector(-39.56f, 1.75f, 64.0f);

protected:
    // Camera component
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Camera")
    UCameraComponent* FirstPersonCamera;

    // Input functions
    void MoveForward(float Value);
    void MoveRight(float Value);
    void Turn(float Value);
    void LookUp(float Value);
    virtual void Jump() override;

    // Combat helpers
    UFUNCTION(BlueprintCallable, Category="Combat")
    TArray<ABloodreadPlayerCharacter*> GetPlayersInRange(float Range);

    // Get all damageable targets (players and dummies) in range
    struct FDamageableTarget
    {
        AActor* Actor;
        bool bIsPlayer;
        
        FDamageableTarget(AActor* InActor, bool bInIsPlayer) 
            : Actor(InActor), bIsPlayer(bInIsPlayer) {}
    };
    
    TArray<FDamageableTarget> GetDamageableTargetsInRange(float Range);

    UFUNCTION(BlueprintImplementableEvent, Category="Combat")
    void OnSwordAttack();

    UFUNCTION(BlueprintImplementableEvent, Category="Combat")
    void OnAbilityUsed(const FAbilityData& Ability);

    UFUNCTION(BlueprintImplementableEvent, Category="Combat")
    void OnTakeDamage(int32 Damage, ABloodreadPlayerCharacter* Attacker);

    UFUNCTION(BlueprintImplementableEvent, Category="Combat")
    void OnKnockbackApplied(FVector Direction, float Force);

private:
    // Cooldown management
    void UpdateCooldowns(float DeltaTime);
    
    // Reference to other player (for 1v1)
    UPROPERTY()
    ABloodreadPlayerCharacter* OtherPlayer;
    
    // UI References
    UPROPERTY()
    class UUserWidget* HealthBarWidget;
};
