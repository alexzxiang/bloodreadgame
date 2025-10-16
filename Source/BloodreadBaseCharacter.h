#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "Components/InputComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/WidgetComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputActionValue.h"
#include "Engine/Engine.h"
#include "Net/UnrealNetwork.h"
#include "BloodreadBaseCharacter.generated.h"

// Forward declarations
class UInputMappingContext;
class UInputAction;
class APracticeDummy;
class ABloodreadWarriorCharacter;
class ABloodreadMageCharacter;
class ABloodreadRogueCharacter;
class ABloodreadHealerCharacter;
class ABloodreadDragonCharacter;
class ABloodreadPlayerCharacter;

// Structure for any targetable entity (players or dummies)
USTRUCT(BlueprintType)
struct FTargetableActor
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, Category = "Targeting")
    AActor* Actor;
    
    UPROPERTY(BlueprintReadWrite, Category = "Targeting")
    bool bIsPlayer;
    
    UPROPERTY(BlueprintReadWrite, Category = "Targeting")
    bool bIsDummy;
    
    // Default constructor for Blueprint compatibility
    FTargetableActor()
        : Actor(nullptr), bIsPlayer(false), bIsDummy(false)
    {
    }
    
    // Constructor with actor parameter
    explicit FTargetableActor(AActor* InActor) 
        : Actor(InActor), bIsPlayer(false), bIsDummy(false)
    {
        if (Actor)
        {
            // We'll set these in the implementation file to avoid forward declaration issues
            bIsPlayer = false;
            bIsDummy = false;
        }
    }
};

// Enums for character system
UENUM(BlueprintType)
enum class ECharacterClass : uint8
{
    None        UMETA(DisplayName = "None"),
    Warrior     UMETA(DisplayName = "Warrior"),
    Mage        UMETA(DisplayName = "Mage"),
    Rogue       UMETA(DisplayName = "Rogue"),
    Healer      UMETA(DisplayName = "Healer"),
    Dragon      UMETA(DisplayName = "Dragon"),
    Archer      UMETA(DisplayName = "Archer"),
    Berserker   UMETA(DisplayName = "Berserker"),
    Paladin     UMETA(DisplayName = "Paladin")
};

UENUM(BlueprintType)
enum class ETeam : uint8
{
    None        UMETA(DisplayName = "None"),
    Player      UMETA(DisplayName = "Player"),
    Enemy       UMETA(DisplayName = "Enemy"),
    Neutral     UMETA(DisplayName = "Neutral")
};

UENUM(BlueprintType)
enum class EAbilityType : uint8
{
    None        UMETA(DisplayName = "None"),
    Damage      UMETA(DisplayName = "Damage"),
    Heal        UMETA(DisplayName = "Heal"),
    Buff        UMETA(DisplayName = "Buff"),
    Debuff      UMETA(DisplayName = "Debuff"),
    Movement    UMETA(DisplayName = "Movement"),
    Utility     UMETA(DisplayName = "Utility"),
    Defense     UMETA(DisplayName = "Defense")
};

// Structs for character data
USTRUCT(BlueprintType)
struct FCharacterStats
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
    int32 MaxHealth = 100;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
    int32 Strength = 10;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
    int32 Defense = 5;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
    int32 Speed = 10;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
    int32 Mana = 50;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
    float CriticalChance = 0.1f;

    FCharacterStats()
    {
        MaxHealth = 100;
        Strength = 10;
        Defense = 5;
        Speed = 10;
        Mana = 50;
        CriticalChance = 0.1f;
    }
};

USTRUCT(BlueprintType)
struct FCharacterAbilityData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ability")
    FString Name = "Default Ability";

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ability")
    FString Description = "A basic ability";

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ability")
    EAbilityType Type = EAbilityType::None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ability")
    float Cooldown = 5.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ability")
    int32 ManaCost = 10;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ability")
    float Damage = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ability")
    float HealAmount = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ability")
    float Duration = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ability")
    float CooldownRemaining = 0.0f;

    FCharacterAbilityData()
    {
        Name = "Default Ability";
        Description = "A basic ability";
        Type = EAbilityType::None;
        Cooldown = 5.0f;
        ManaCost = 10;
        Damage = 0.0f;
        HealAmount = 0.0f;
        Duration = 0.0f;
        CooldownRemaining = 0.0f;
    }
};

USTRUCT(BlueprintType)
struct FCharacterClassData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Class")
    ECharacterClass CharacterClass = ECharacterClass::None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Class")
    FString ClassName = "Default";

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Class")
    FString Description = "A basic character class";

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Class")
    FCharacterStats BaseStats;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Class")
    FCharacterAbilityData Ability1;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Class")
    FCharacterAbilityData Ability2;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Class")
    TSoftObjectPtr<USkeletalMesh> CharacterMesh;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Class")
    FString AnimationBlueprintPath;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Class")
    FString BasicAttackAnimationPath;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Class")
    FString Ability1AnimationPath;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Class")
    FString Ability2AnimationPath;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Class")
    FVector CameraOffset;

    FCharacterClassData()
    {
        CharacterClass = ECharacterClass::None;
        ClassName = "Default";
        Description = "A basic character class";
        BaseStats = FCharacterStats();
        Ability1 = FCharacterAbilityData();
        Ability2 = FCharacterAbilityData();
        CharacterMesh = nullptr;
        AnimationBlueprintPath = "";
        BasicAttackAnimationPath = "";
        Ability1AnimationPath = "";
        Ability2AnimationPath = "";
        CameraOffset = FVector(-39.56f, 1.75f, 64.0f); // Default camera offset
    }
};

UCLASS()
class BLOODREADGAME_API ABloodreadBaseCharacter : public ACharacter
{
    GENERATED_BODY()

public:
    ABloodreadBaseCharacter();

    // Network replication
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

protected:
    virtual void BeginPlay() override;
    virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;
    virtual void Tick(float DeltaTime) override;

    // Character class system
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Class")
    ECharacterClass CurrentCharacterClass = ECharacterClass::Warrior;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Class")
    FCharacterClassData CharacterClassData;

    // Core stats
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
    FCharacterStats CurrentStats;

    UPROPERTY(ReplicatedUsing = OnRep_Health, EditAnywhere, BlueprintReadWrite, Category = "Stats")
    int32 CurrentHealth = 100;

    UPROPERTY(Replicated, EditAnywhere, BlueprintReadWrite, Category = "Stats")
    int32 CurrentMana = 50;

    // Movement tuning properties (exposed for tweaking in editor/blueprints)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement|Tuning")
    float CharacterGravityScale = 0.8f; // Reduced from 1.0f for better air control and abilities

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement|Tuning")
    float CharacterJumpZVelocity = 420.0f; // Increased from 100.0f to more reasonable value (UE5 default is ~420)

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Knockback|Tuning")
    float HorizontalKnockbackMultiplier = 2.5f; // Increased horizontal push for stronger knockback

    // Mesh positioning and rotation fixes for character meshes
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mesh|Positioning")
    FVector MeshLocationOffset = FVector(0.0f, 0.0f, -110.0f); // Lower mesh by 110 units

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mesh|Positioning")
    FRotator MeshRotationOffset = FRotator(0.0f, -90.0f, 0.0f); // Rotate mesh -90 degrees on Yaw

    // Network replication functions
    UFUNCTION()
    void OnRep_Health();

    // Mana regeneration system
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
    float ManaRegenRate = 1.0f; // Mana per second

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
    float ManaRegenTimer = 0.0f; // Timer for mana regeneration

    // Basic Attack System
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    float BasicAttackRange = 200.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    float BasicAttackDamage = 15.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    float BasicAttackKnockbackForce = 300.0f;

    // Camera system
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
    UCameraComponent* FirstPersonCamera;

    // Health bar widget component
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "UI")
    UWidgetComponent* HealthBarWidgetComponent;

    // Health bar widget class (loaded in constructor)
    UPROPERTY()
    TSubclassOf<UUserWidget> OverheadHealthWidgetClass;

    // Health bar reference (like practice dummy)
    UPROPERTY(BlueprintReadOnly, Category = "UI")
    class UUserWidget* CurrentHealthBarWidget = nullptr;

    // Distance-based health bar visibility
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI")
    float MaxHealthBarVisibilityDistance = 2000.0f; // Max distance to show health bars (in UE units)
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI")
    float HealthBarVisibilityCheckInterval = 0.5f; // How often to check distance (in seconds)

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera")
    FVector CustomMeshCameraOffset = FVector(25.0f, 1.75f, 85.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera")
    float MeshScaleMultiplier = 1.0f;

    // Input system
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input", meta = (AllowPrivateAccess = "true"))
    UInputMappingContext* DefaultMappingContext;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input", meta = (AllowPrivateAccess = "true"))
    UInputAction* JumpAction;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input", meta = (AllowPrivateAccess = "true"))
    UInputAction* MoveAction;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input", meta = (AllowPrivateAccess = "true"))
    UInputAction* LookAction;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input", meta = (AllowPrivateAccess = "true"))
    UInputAction* AttackAction;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input", meta = (AllowPrivateAccess = "true"))
    UInputAction* Ability1Action;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input", meta = (AllowPrivateAccess = "true"))
    UInputAction* Ability2Action;

public:
    // Input setup function
    UFUNCTION(BlueprintCallable, Category = "Input")
    void SetupInputContext();

    // Character class functions
    UFUNCTION(BlueprintCallable, Category = "Character Class")
    void SetCharacterClass(ECharacterClass NewClass);

    UFUNCTION(BlueprintCallable, Category = "Character Class")
    void InitializeFromClassData(const FCharacterClassData& ClassData);

    UFUNCTION(BlueprintPure, Category = "Character Class")
    ECharacterClass GetCharacterClass() const { return CurrentCharacterClass; }

    UFUNCTION(BlueprintPure, Category = "Character Class")
    FCharacterClassData GetCharacterClassData() const { return CharacterClassData; }

    // Blueprint-callable function to set mesh on any skeletal mesh component
    UFUNCTION(BlueprintCallable, Category = "Character Class")
    bool SetMeshOnComponent(USkeletalMeshComponent* MeshComponent, const FString& MeshPath);

    // Blueprint-callable function to apply class data to specific mesh component
    UFUNCTION(BlueprintCallable, Category = "Character Class")
    bool ApplyClassDataToMeshComponent(USkeletalMeshComponent* MeshComponent, ECharacterClass CharacterClass);

    // Blueprint-callable function to set animation blueprint on any skeletal mesh component
    UFUNCTION(BlueprintCallable, Category = "Character Class")
    bool SetAnimationBlueprintOnComponent(USkeletalMeshComponent* MeshComponent, const FString& AnimBPPath);

    // Force initialization of character systems (called by PlayerController on possess)
    UFUNCTION(BlueprintCallable, Category = "Character Class")
    void ForceInitializeCharacterSystems();

    // Debug function to test mesh loading paths
    UFUNCTION(BlueprintCallable, Category = "Character Class")
    void TestAllMeshPaths();

    // Manual initialization functions for Blueprint characters
    void InitializeWarriorDataManually();
    void InitializeMageDataManually();
    void InitializeRogueDataManually();
    void InitializeHealerDataManually();
    void InitializeDragonDataManually();

    // Health system - Helper functions for Blueprint Interface
    UFUNCTION(BlueprintCallable, Category = "Health")
    void DealDamage(float DamageAmount);

    UFUNCTION(BlueprintCallable, Category = "Health") 
    void HealCharacter(float HealAmount);
    
    // Combined damage and knockback function - preferred for attacks
    UFUNCTION(BlueprintCallable, Category = "Health")
    void DealDamageWithKnockback(float DamageAmount, FVector KnockbackDirection, float KnockbackForce, ABloodreadBaseCharacter* Attacker = nullptr);

    // Multiplayer RPCs
    UFUNCTION(Server, Reliable, BlueprintCallable, Category = "Multiplayer")
    void Server_UseAbility(int32 AbilityIndex, FVector TargetLocation);

    UFUNCTION(NetMulticast, Reliable, BlueprintCallable, Category = "Multiplayer")
    void Multicast_PlayAbilityAnimation(int32 AbilityIndex);

    UFUNCTION(Server, Reliable, Category = "Multiplayer")
    void Server_TakeDamage(float DamageAmount, ABloodreadBaseCharacter* DamageSource);

    UFUNCTION(NetMulticast, Reliable, Category = "Multiplayer")
    void Multicast_OnHealthChanged(int32 NewHealth, int32 MaxHealth);

    UFUNCTION(Server, Reliable, Category = "Multiplayer")
    void Server_BasicAttack(FVector TargetLocation);

    UFUNCTION(NetMulticast, Reliable, Category = "Multiplayer")
    void Multicast_PlayHitAnimation();

    UFUNCTION(BlueprintPure, Category = "Health")
    bool GetIsAlive() const { return CurrentHealth > 0; }

    UFUNCTION(BlueprintPure, Category = "Health")
    float GetCurrentHealthFloat() const { return static_cast<float>(CurrentHealth); }

    UFUNCTION(BlueprintPure, Category = "Health")
    float GetMaxHealthFloat() const { return static_cast<float>(CurrentStats.MaxHealth); }

    UFUNCTION(BlueprintPure, Category = "Health")
    float GetHealthPercent() const;

    UFUNCTION(BlueprintPure, Category = "Health")
    FString GetHealthDisplayText() const;

    // Additional health methods for compatibility
    UFUNCTION(BlueprintPure, Category = "Health")
    int32 GetCurrentHealth() const { return CurrentHealth; }

    UFUNCTION(BlueprintCallable, Category = "Health")
    void SetCurrentHealth(int32 NewHealth) { 
        int32 OldHealth = CurrentHealth;
        CurrentHealth = FMath::Clamp(NewHealth, 0, CurrentStats.MaxHealth);
        OnHealthChanged(OldHealth, CurrentHealth);
    }

    UFUNCTION(BlueprintPure, Category = "Health")
    int32 GetMaxHealth() const { return CurrentStats.MaxHealth; }

    UFUNCTION(BlueprintPure, Category = "Health")
    FString GetHealthText() const;

    UFUNCTION(BlueprintPure, Category = "Health")
    bool IsAlive() const { return CurrentHealth > 0; }

    // Health bar UI system (copied from PracticeDummy approach)
    UFUNCTION(BlueprintCallable, Category = "UI")
    void SetHealthBarWidget(class UUserWidget* Widget);
    
    UFUNCTION(BlueprintCallable, Category = "UI") 
    void UpdateHealthDisplay();

    UFUNCTION(BlueprintCallable, Category = "UI")
    void InitializeHealthBar();

    UFUNCTION(BlueprintCallable, Category = "UI")
    void UpdateHealthBarVisibility();

    // Initialize editor-spawned characters manually
    UFUNCTION(BlueprintCallable, Category = "Debug")
    void InitializeEditorSpawnedCharacter();

    // Debug functions for testing damage and knockback
    UFUNCTION(BlueprintCallable, Category = "Debug")
    void TestTakeDamage(int32 DamageAmount = 50);

    UFUNCTION(BlueprintCallable, Category = "Debug")
    void TestKnockback(FVector Direction = FVector(1.0f, 0.0f, 0.5f), float Force = 500.0f);

    // Helper function to get class name as string
    UFUNCTION(BlueprintPure, Category = "Character Info")
    FString GetOwnerClassName() const;

    // Force enable physics settings for knockback
    UFUNCTION(BlueprintCallable, Category = "Physics")
    void ForceEnableKnockbackPhysics();

    // Timer handle for distance checking
    UPROPERTY()
    FTimerHandle HealthBarVisibilityTimerHandle;

    // Timer handle for knockback recovery
    UPROPERTY()
    FTimerHandle KnockbackRecoveryTimerHandle;

    // Timer handle for animation blend back
    UPROPERTY()
    FTimerHandle AnimationBlendBackTimer;

    // Mana system
    UFUNCTION(BlueprintCallable, Category = "Mana")
    bool UseMana(int32 ManaAmount);

    UFUNCTION(BlueprintCallable, Category = "Mana")
    void RestoreMana(int32 ManaAmount);

    UFUNCTION(BlueprintPure, Category = "Mana")
    float GetManaPercentage() const;

    UFUNCTION(BlueprintPure, Category = "Mana")
    int32 GetCurrentMana() const { return CurrentMana; }

    UFUNCTION(BlueprintPure, Category = "Mana")
    int32 GetMaxMana() const { return CurrentStats.Mana; }

    // Ability system
    UFUNCTION(BlueprintCallable, Category = "Abilities")
    virtual void UseAbility1();

    UFUNCTION(BlueprintCallable, Category = "Abilities")
    virtual void UseAbility2();

    UFUNCTION(BlueprintPure, Category = "Abilities")
    bool CanUseAbility1() const;

    UFUNCTION(BlueprintPure, Category = "Abilities")
    bool CanUseAbility2() const;

    UFUNCTION(BlueprintPure, Category = "Abilities")
    float GetAbility1CooldownPercentage() const;

    UFUNCTION(BlueprintPure, Category = "Abilities")
    float GetAbility2CooldownPercentage() const;

    UFUNCTION(BlueprintPure, Category = "Abilities")
    FString GetAbility1Name() const;

    UFUNCTION(BlueprintPure, Category = "Abilities")
    FString GetAbility2Name() const;

    UFUNCTION(BlueprintPure, Category = "Abilities")
    float GetAbility1RemainingCooldown() const;

    UFUNCTION(BlueprintPure, Category = "Abilities")
    float GetAbility2RemainingCooldown() const;

    // Combat system
    UFUNCTION(BlueprintCallable, Category = "Combat")
    APracticeDummy* GetCrosshairTarget();

    // New universal targeting function that can target both players and dummies
    UFUNCTION(BlueprintCallable, Category = "Combat")
    FTargetableActor GetCrosshairTargetActor();

    // Enhanced attack function that works on both players and dummies
    UFUNCTION(BlueprintCallable, Category = "Combat")
    void AttackTarget();

    // Knockback system - with networking support
    UFUNCTION(BlueprintCallable, Category = "Combat")
    virtual void ApplyKnockback(FVector KnockbackDirection, float Force);

    // Networked knockback - called on server, replicated to all clients
    UFUNCTION(NetMulticast, Reliable, Category = "Combat")
    void MulticastApplyKnockback(FVector KnockbackDirection, float Force);

    // Server-side knockback initiation (called from attacking player)
    UFUNCTION(Server, Reliable, Category = "Combat")
    void ServerApplyKnockback(FVector KnockbackDirection, float Force);

    // Server RPC to apply knockback to another player (called by attacker)
    UFUNCTION(Server, Reliable, Category = "Combat")
    void ServerApplyKnockbackToTarget(ACharacter* TargetCharacter, FVector KnockbackDirection, float Force);

private:
    // Internal knockback implementation (does the actual physics work)
    void ApplyKnockbackInternal(FVector KnockbackDirection, float Force);

public:

    // Combat damage system (virtual so subclasses can override)
    UFUNCTION(BlueprintCallable, Category = "Combat")
    virtual bool TakeCustomDamage(int32 Damage, ABloodreadBaseCharacter* Attacker = nullptr);

    UFUNCTION(BlueprintImplementableEvent, Category = "Combat")
    void OnBasicAttack();

    // Animation playback functions
    UFUNCTION(BlueprintCallable, Category = "Animation")
    void PlayBasicAttackAnimation();
    
    UFUNCTION(BlueprintCallable, Category = "Animation") 
    void PlayAbility1Animation();
    
    UFUNCTION(BlueprintCallable, Category = "Animation")
    void PlayAbility2Animation();
    
    UFUNCTION(BlueprintCallable, Category = "Animation")
    bool PlayAnimationFromPath(const FString& AnimationPath);

    // Animation event callbacks (called from Animation Notifies or Blueprint events)
    UFUNCTION(BlueprintImplementableEvent, Category = "Animation")
    void OnAttackHit();

    // Combat events
    UFUNCTION(BlueprintImplementableEvent, Category = "Combat")
    void OnKnockbackApplied(FVector Direction, float Force);

    UFUNCTION(BlueprintImplementableEvent, Category = "Combat")
    void OnTakeDamage(int32 Damage, ABloodreadBaseCharacter* Attacker);

    // Camera adjustment functions
    UFUNCTION(BlueprintCallable, Category = "Camera")
    void SetCameraPosition(FVector NewRelativeLocation);

    UFUNCTION(BlueprintCallable, Category = "Camera")
    void AdjustCameraForMesh(float HeightOffset, float ForwardOffset, float RightOffset);

    UFUNCTION(BlueprintPure, Category = "Camera")
    FVector GetCurrentCameraPosition() const;

    // Visual effects
    UFUNCTION(BlueprintImplementableEvent, Category = "Visual Effects")
    void FlashRed();

    UFUNCTION(BlueprintImplementableEvent, Category = "Visual Effects")
    void StopFlashRed();

protected:
    // Enhanced Input handling
    virtual void Move(const FInputActionValue& Value);
    virtual void Look(const FInputActionValue& Value);
    
    // Traditional Input handling
    virtual void MoveForward(float Value);
    virtual void MoveRight(float Value);
    virtual void Turn(float Value);
    virtual void LookUp(float Value);
    
    // Actions
    virtual void Attack();
    virtual void TestDamage();
    virtual void TestHeal();

    // Virtual functions for subclasses to override
    virtual void OnCharacterClassChanged() {}
    virtual bool OnAbility1Used() { return false; }
    virtual bool OnAbility2Used() { return false; }
    virtual void OnHealthChanged(int32 OldHealth, int32 NewHealth) {}
    virtual void OnManaChanged(int32 OldMana, int32 NewMana) {}

public:
    // AI-related functions
    virtual TArray<ABloodreadBaseCharacter*> GetEnemiesInRadius(float Radius) { return TArray<ABloodreadBaseCharacter*>(); }
    virtual TArray<ABloodreadBaseCharacter*> GetAlliesInRadius(float Radius) { return TArray<ABloodreadBaseCharacter*>(); }
    virtual TArray<ABloodreadBaseCharacter*> GetAllAllies() { return TArray<ABloodreadBaseCharacter*>(); }
    virtual void ActivateAbility1() {}
    virtual void ActivateAbility2() {}
    virtual void PerformAttack() {}
    
    // Additional methods
    void ReactivateAnimationBlueprints();

    UFUNCTION(BlueprintCallable, Category = "UI")
    FString GetManaText() const;

private:
    void UpdateAbilityCooldowns(float DeltaTime);
};