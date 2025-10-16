#include "BloodreadPlayerCharacter.h"
#include "BloodreadGameMode.h"
#include "BloodreadBaseCharacter.h"
#include "Engine/World.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"

ABloodreadPlayerCharacter::ABloodreadPlayerCharacter()
{
    PrimaryActorTick.bCanEverTick = true;

    // Set up camera
    FirstPersonCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FirstPersonCamera"));
    FirstPersonCamera->SetupAttachment(RootComponent);
    FirstPersonCamera->SetRelativeLocation(FVector(-39.56f, 1.75f, 64.0f));
    FirstPersonCamera->bUsePawnControlRotation = true;

    // Configure character movement
    GetCharacterMovement()->JumpZVelocity = JumpHeight;
    GetCharacterMovement()->AirControl = 0.2f;

    // Initialize player stats
    PlayerStats.MaxHealth = 100;
    PlayerStats.CurrentHealth = 100;
    PlayerStats.Strength = 10;
    PlayerStats.DamageImmunityTicksRemaining = 0;
    PlayerStats.bCanTakeDamage = true;
    
    // Debug: Log constructor values
    UE_LOG(LogTemp, Warning, TEXT("Constructor: MaxHealth = %d, CurrentHealth = %d"), PlayerStats.MaxHealth, PlayerStats.CurrentHealth);
}

void ABloodreadPlayerCharacter::BeginPlay()
{
    Super::BeginPlay();
    
    // Debug: Log BeginPlay values
    UE_LOG(LogTemp, Warning, TEXT("BeginPlay: MaxHealth = %d, CurrentHealth = %d"), PlayerStats.MaxHealth, PlayerStats.CurrentHealth);
    
    // Find other player in the world (for 1v1)
    OtherPlayer = FindOtherPlayer();
}

void ABloodreadPlayerCharacter::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    // Update ability cooldowns
    UpdateCooldowns(DeltaTime);
}

void ABloodreadPlayerCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
    Super::SetupPlayerInputComponent(PlayerInputComponent);

    // Movement
    PlayerInputComponent->BindAxis("MoveForward", this, &ABloodreadPlayerCharacter::MoveForward);
    PlayerInputComponent->BindAxis("MoveRight", this, &ABloodreadPlayerCharacter::MoveRight);
    PlayerInputComponent->BindAxis("Turn", this, &ABloodreadPlayerCharacter::Turn);
    PlayerInputComponent->BindAxis("LookUp", this, &ABloodreadPlayerCharacter::LookUp);

    // Actions
    PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &ABloodreadPlayerCharacter::Jump);
    PlayerInputComponent->BindAction("Attack", IE_Pressed, this, &ABloodreadPlayerCharacter::AttackWithSword);
    PlayerInputComponent->BindAction("Ability1", IE_Pressed, this, &ABloodreadPlayerCharacter::UseAbility1);
    PlayerInputComponent->BindAction("Ability2", IE_Pressed, this, &ABloodreadPlayerCharacter::UseAbility2);
    
    // Debug/Test actions
    PlayerInputComponent->BindAction("TestDamage", IE_Pressed, this, &ABloodreadPlayerCharacter::TestTakeDamage);
    PlayerInputComponent->BindAction("TestHeal", IE_Pressed, this, &ABloodreadPlayerCharacter::TestHeal);
}

void ABloodreadPlayerCharacter::MoveForward(float Value)
{
    if (Value != 0.0f)
    {
        AddMovementInput(GetActorForwardVector(), Value);
    }
}

void ABloodreadPlayerCharacter::MoveRight(float Value)
{
    if (Value != 0.0f)
    {
        AddMovementInput(GetActorRightVector(), Value);
    }
}

void ABloodreadPlayerCharacter::Turn(float Value)
{
    AddControllerYawInput(Value);
}

void ABloodreadPlayerCharacter::LookUp(float Value)
{
    AddControllerPitchInput(Value);
}

void ABloodreadPlayerCharacter::Jump()
{
    Super::Jump();
}

void ABloodreadPlayerCharacter::AttackWithSword()
{
    if (!IsAlive()) return;

    UE_LOG(LogTemp, Warning, TEXT("AttackWithSword called - using crosshair targeting"));

    // Use crosshair targeting for precise combat
    APracticeDummy* TargetDummy = GetCrosshairTarget();
    
    if (TargetDummy)
    {
        // Check if dummy is within sword range
        float Distance = FVector::Dist(GetActorLocation(), TargetDummy->GetActorLocation());
        if (Distance <= SwordRange)
        {
            UE_LOG(LogTemp, Warning, TEXT("AttackWithSword: Attacking crosshair target at distance %.1f"), Distance);
            
            // Calculate damage with strength bonus
            int32 TotalDamage = SwordDamage + PlayerStats.Strength;
            
            // Apply damage
            TargetDummy->TakeCustomDamage(TotalDamage, this);
            
            // Apply knockback
            FVector KnockbackDirection = (TargetDummy->GetActorLocation() - GetActorLocation()).GetSafeNormal();
            KnockbackDirection.Z = 0.2f; // Add slight upward force
            TargetDummy->ApplyKnockback(KnockbackDirection, SwordKnockbackForce);
            
            UE_LOG(LogTemp, Log, TEXT("Player dealt %d damage with sword to crosshair target"), TotalDamage);
        }
        else
        {
            UE_LOG(LogTemp, Warning, TEXT("AttackWithSword: Target dummy too far (%.1f > %.1f)"), Distance, SwordRange);
        }
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("AttackWithSword: No dummy in crosshair - attack requires precise targeting"));
    }
    
    // Call Blueprint event
    OnSwordAttack();
}

void ABloodreadPlayerCharacter::UseAbility1()
{
    if (!IsAlive() || PlayerLoadout.Ability1CooldownRemaining > 0.0f) return;
    
    if (PlayerLoadout.Ability1.Id > 0)
    {
        // Start cooldown
        PlayerLoadout.Ability1CooldownRemaining = PlayerLoadout.Ability1.Cooldown;
        
        // Apply ability effects based on ability type
        if (PlayerLoadout.Ability1.Name == "Heal")
        {
            Heal(PlayerLoadout.Ability1.Damage); // Damage field used for heal amount
        }
        else
        {
            // Damage ability - find targets in range
            TArray<FDamageableTarget> TargetsInRange = GetDamageableTargetsInRange(300.0f); // Ability range
            
            for (const FDamageableTarget& Target : TargetsInRange)
            {
                if (!Target.Actor) continue;
                
                if (Target.bIsPlayer)
                {
                    // Handle player target
                    ABloodreadPlayerCharacter* PlayerTarget = Cast<ABloodreadPlayerCharacter>(Target.Actor);
                    if (PlayerTarget && PlayerTarget->IsAlive() && PlayerTarget->PlayerStats.bCanTakeDamage)
                    {
                        PlayerTarget->TakeCustomDamage(PlayerLoadout.Ability1.Damage, this);
                        
                        if (PlayerLoadout.Ability1.KnockbackForce > 0.0f)
                        {
                            FVector KnockbackDirection = (PlayerTarget->GetActorLocation() - GetActorLocation()).GetSafeNormal();
                            
                            // Use networked knockback routing
                            if (IsValid(PlayerTarget))
                            {
                                if (GetWorld()->GetNetMode() == NM_Standalone)
                                {
                                    // Single player - apply directly
                                    PlayerTarget->ApplyKnockback(KnockbackDirection, PlayerLoadout.Ability1.KnockbackForce);
                                }
                                else
                                {
                                    // Multiplayer - route through our owned RPC
                                    ServerApplyKnockbackToTarget(PlayerTarget, KnockbackDirection, PlayerLoadout.Ability1.KnockbackForce);
                                }
                            }
                        }
                    }
                }
                else
                {
                    // Handle dummy target
                    APracticeDummy* DummyTarget = Cast<APracticeDummy>(Target.Actor);
                    if (DummyTarget && DummyTarget->IsAlive())
                    {
                        DummyTarget->TakeCustomDamage(PlayerLoadout.Ability1.Damage, this);
                        
                        if (PlayerLoadout.Ability1.KnockbackForce > 0.0f)
                        {
                            FVector KnockbackDirection = (DummyTarget->GetActorLocation() - GetActorLocation()).GetSafeNormal();
                            DummyTarget->ApplyKnockback(KnockbackDirection, PlayerLoadout.Ability1.KnockbackForce);
                        }
                    }
                }
            }
        }
        
        OnAbilityUsed(PlayerLoadout.Ability1);
        UE_LOG(LogTemp, Log, TEXT("Used ability: %s"), *PlayerLoadout.Ability1.Name);
    }
}

void ABloodreadPlayerCharacter::UseAbility2()
{
    if (!IsAlive() || PlayerLoadout.Ability2CooldownRemaining > 0.0f) return;
    
    if (PlayerLoadout.Ability2.Id > 0)
    {
        // Start cooldown
        PlayerLoadout.Ability2CooldownRemaining = PlayerLoadout.Ability2.Cooldown;
        
        // Apply ability effects (similar to Ability1)
        if (PlayerLoadout.Ability2.Name == "Heal")
        {
            Heal(PlayerLoadout.Ability2.Damage);
        }
        else
        {
            TArray<FDamageableTarget> TargetsInRange = GetDamageableTargetsInRange(300.0f);
            
            for (const FDamageableTarget& Target : TargetsInRange)
            {
                if (!Target.Actor) continue;
                
                if (Target.bIsPlayer)
                {
                    // Handle player target
                    ABloodreadPlayerCharacter* PlayerTarget = Cast<ABloodreadPlayerCharacter>(Target.Actor);
                    if (PlayerTarget && PlayerTarget->IsAlive() && PlayerTarget->PlayerStats.bCanTakeDamage)
                    {
                        PlayerTarget->TakeCustomDamage(PlayerLoadout.Ability2.Damage, this);
                        
                        if (PlayerLoadout.Ability2.KnockbackForce > 0.0f)
                        {
                            FVector KnockbackDirection = (PlayerTarget->GetActorLocation() - GetActorLocation()).GetSafeNormal();
                            
                            // Use networked knockback routing
                            if (IsValid(PlayerTarget))
                            {
                                if (GetWorld()->GetNetMode() == NM_Standalone)
                                {
                                    // Single player - apply directly
                                    PlayerTarget->ApplyKnockback(KnockbackDirection, PlayerLoadout.Ability2.KnockbackForce);
                                }
                                else
                                {
                                    // Multiplayer - route through our owned RPC
                                    ServerApplyKnockbackToTarget(PlayerTarget, KnockbackDirection, PlayerLoadout.Ability2.KnockbackForce);
                                }
                            }
                        }
                    }
                }
                else
                {
                    // Handle dummy target
                    APracticeDummy* DummyTarget = Cast<APracticeDummy>(Target.Actor);
                    if (DummyTarget && DummyTarget->IsAlive())
                    {
                        DummyTarget->TakeCustomDamage(PlayerLoadout.Ability2.Damage, this);
                        
                        if (PlayerLoadout.Ability2.KnockbackForce > 0.0f)
                        {
                            FVector KnockbackDirection = (DummyTarget->GetActorLocation() - GetActorLocation()).GetSafeNormal();
                            DummyTarget->ApplyKnockback(KnockbackDirection, PlayerLoadout.Ability2.KnockbackForce);
                        }
                    }
                }
            }
        }
        
        OnAbilityUsed(PlayerLoadout.Ability2);
        UE_LOG(LogTemp, Log, TEXT("Used ability: %s"), *PlayerLoadout.Ability2.Name);
    }
}

bool ABloodreadPlayerCharacter::TakeCustomDamage(int32 Damage, ABloodreadPlayerCharacter* Attacker)
{
    // Early exit conditions - no damage taken
    if (!IsAlive() || !PlayerStats.bCanTakeDamage || PlayerStats.DamageImmunityTicksRemaining > 0 || Damage <= 0)
    {
        return false;
    }

    // Store previous health for comparison
    int32 PreviousHealth = PlayerStats.CurrentHealth;
    
    // Apply damage
    PlayerStats.CurrentHealth = FMath::Max(0, PlayerStats.CurrentHealth - Damage);
    
    // Set damage immunity
    ABloodreadGameMode* GameMode = Cast<ABloodreadGameMode>(UGameplayStatics::GetGameMode(this));
    if (GameMode)
    {
        PlayerStats.bCanTakeDamage = false;
    }
    
    // Broadcast events
    OnHealthChanged.Broadcast(PlayerStats.CurrentHealth, PlayerStats.MaxHealth);
    OnTakeDamage(Damage, Attacker);
    
    // Flash red when taking damage
    FlashRed();
    
    // Update health display
    UpdateHealthDisplay();
    
    UE_LOG(LogTemp, Log, TEXT("Player took %d damage, health: %d/%d"), 
           Damage, PlayerStats.CurrentHealth, PlayerStats.MaxHealth);
    
    // Check for death
    if (PlayerStats.CurrentHealth <= 0)
    {
        OnPlayerDeath.Broadcast(this);
        
        // End the match
        if (GameMode)
        {
            int32 WinnerId = Attacker ? 1 : 0; // You'll need to map this to actual player IDs
        }
        
        UE_LOG(LogTemp, Warning, TEXT("Player has died from damage!"));
    }
    
    return true; // Damage was successfully applied
}

void ABloodreadPlayerCharacter::ApplyKnockback(FVector KnockbackDirection, float Force)
{
    if (!IsAlive()) return;

    UE_LOG(LogTemp, Warning, TEXT("PlayerChar ApplyKnockback: Original direction: %s, Force: %.2f"), *KnockbackDirection.ToString(), Force);
    
    // Enhanced knockback: Don't normalize to preserve force magnitudes
    FVector Horizontal = FVector(KnockbackDirection.X, KnockbackDirection.Y, 0.0f);
    Horizontal = Horizontal.GetSafeNormal();
    
    // Apply multipliers to create enhanced force components
    float HorizontalForce = Force * HorizontalKnockbackMultiplier;
    float VerticalForce = Force * 0.6f; // Strong vertical component
    
    // Create final knockback vector with proper proportions
    FVector EnhancedKnockback = Horizontal * HorizontalForce;
    EnhancedKnockback.Z = VerticalForce; // Always add upward force
    
    UE_LOG(LogTemp, Warning, TEXT("PlayerChar ApplyKnockback: Enhanced knockback: %s (H:%.1f V:%.1f)"), 
           *EnhancedKnockback.ToString(), HorizontalForce, VerticalForce);
    
    // Apply enhanced knockback directly
    GetCharacterMovement()->AddImpulse(EnhancedKnockback, true);
    
    OnKnockbackApplied(EnhancedKnockback.GetSafeNormal(), EnhancedKnockback.Size());
    UE_LOG(LogTemp, Log, TEXT("PlayerChar Knockback applied with enhanced force: %.2f"), EnhancedKnockback.Size());
}

void ABloodreadPlayerCharacter::ServerApplyKnockbackToTarget_Implementation(ACharacter* TargetCharacter, FVector KnockbackDirection, float Force)
{
    UE_LOG(LogTemp, Warning, TEXT("PlayerChar ServerApplyKnockbackToTarget: Applying knockback to %s from %s"), 
           TargetCharacter ? *TargetCharacter->GetName() : TEXT("NULL"), *GetName());
    
    if (!TargetCharacter)
    {
        UE_LOG(LogTemp, Warning, TEXT("PlayerChar ServerApplyKnockbackToTarget: NULL target"));
        return;
    }
    
    // Handle different character types
    if (ABloodreadBaseCharacter* BaseCharacter = Cast<ABloodreadBaseCharacter>(TargetCharacter))
    {
        if (!BaseCharacter->GetIsAlive())
        {
            UE_LOG(LogTemp, Warning, TEXT("PlayerChar ServerApplyKnockbackToTarget: BaseCharacter is dead"));
            return;
        }
        
        // Apply knockback to BloodreadBaseCharacter
        BaseCharacter->ApplyKnockback(KnockbackDirection, Force);
    }
    else if (ABloodreadPlayerCharacter* PlayerCharacter = Cast<ABloodreadPlayerCharacter>(TargetCharacter))
    {
        if (!PlayerCharacter->IsAlive())
        {
            UE_LOG(LogTemp, Warning, TEXT("PlayerChar ServerApplyKnockbackToTarget: PlayerCharacter is dead"));
            return;
        }
        
        // Apply knockback to BloodreadPlayerCharacter
        PlayerCharacter->ApplyKnockback(KnockbackDirection, Force);
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("PlayerChar ServerApplyKnockbackToTarget: Unknown character type: %s"), *TargetCharacter->GetClass()->GetName());
    }
}

void ABloodreadPlayerCharacter::Heal(int32 Amount)
{
    if (!IsAlive() || Amount <= 0) return;

    int32 PreviousHealth = PlayerStats.CurrentHealth;
    PlayerStats.CurrentHealth = FMath::Min(PlayerStats.MaxHealth, PlayerStats.CurrentHealth + Amount);
    
    // Only broadcast if health actually changed
    if (PlayerStats.CurrentHealth != PreviousHealth)
    {
        OnHealthChanged.Broadcast(PlayerStats.CurrentHealth, PlayerStats.MaxHealth);
        
        // Update health display
        UpdateHealthDisplay();
        
        UE_LOG(LogTemp, Log, TEXT("Player healed for %d, health: %d/%d"), 
               PlayerStats.CurrentHealth - PreviousHealth, PlayerStats.CurrentHealth, PlayerStats.MaxHealth);
    }
}

void ABloodreadPlayerCharacter::TestTakeDamage()
{
    UE_LOG(LogTemp, Warning, TEXT("TestTakeDamage called - targeting dummies with crosshair"));
    
    // Perform crosshair-based raycast to find target dummy
    APracticeDummy* TargetDummy = GetCrosshairTarget();
    
    if (TargetDummy)
    {
        UE_LOG(LogTemp, Warning, TEXT("TestTakeDamage: Found crosshair target dummy"));
        
        FVector PlayerLocation = GetActorLocation();
        FVector KnockbackDirection = (TargetDummy->GetActorLocation() - PlayerLocation).GetSafeNormal();
        float KnockbackForce = 150.0f;
        
        // Apply damage to dummy
        TargetDummy->TakeCustomDamage(10, this);
        
        // Apply knockback to dummy
        if (TargetDummy->IsAlive())
        {
            TargetDummy->ApplyKnockback(KnockbackDirection, KnockbackForce);
            UE_LOG(LogTemp, Warning, TEXT("TestTakeDamage: Applied 10 damage and knockback to crosshair target"));
        }
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("TestTakeDamage: No dummy in crosshair - F key only targets dummies now"));
    }
}

void ABloodreadPlayerCharacter::TestHeal()
{
    Heal(10);
}

APracticeDummy* ABloodreadPlayerCharacter::GetCrosshairTarget()
{
    // Get camera location and forward direction
    FVector CameraLocation;
    FRotator CameraRotation;
    GetActorEyesViewPoint(CameraLocation, CameraRotation);
    FVector CameraForward = CameraRotation.Vector();
    
    // Perform raycast from center of screen
    FVector TraceStart = CameraLocation;
    FVector TraceEnd = TraceStart + (CameraForward * 1000.0f); // 10 meter range
    
    FHitResult HitResult;
    FCollisionQueryParams CollisionParams;
    CollisionParams.AddIgnoredActor(this);
    
    bool bHit = GetWorld()->LineTraceSingleByChannel(
        HitResult,
        TraceStart,
        TraceEnd,
        ECollisionChannel::ECC_Visibility,
        CollisionParams
    );
    
    if (bHit && HitResult.GetActor())
    {
        // Check if hit actor is a practice dummy
        APracticeDummy* HitDummy = Cast<APracticeDummy>(HitResult.GetActor());
        if (HitDummy && HitDummy->IsAlive())
        {
            UE_LOG(LogTemp, Log, TEXT("GetCrosshairTarget: Found dummy %s at distance %.1f"), 
                   *HitDummy->GetName(), HitResult.Distance);
            return HitDummy;
        }
    }
    
    return nullptr;
}

void ABloodreadPlayerCharacter::SetMaxHealth(int32 NewMaxHealth)
{
    if (NewMaxHealth <= 0)
    {
        UE_LOG(LogTemp, Warning, TEXT("Attempted to set invalid max health: %d"), NewMaxHealth);
        return;
    }
    
    int32 PreviousMaxHealth = PlayerStats.MaxHealth;
    PlayerStats.MaxHealth = NewMaxHealth;
    
    // If current health exceeds new max health, clamp it
    if (PlayerStats.CurrentHealth > PlayerStats.MaxHealth)
    {
        PlayerStats.CurrentHealth = PlayerStats.MaxHealth;
    }
    
    OnHealthChanged.Broadcast(PlayerStats.CurrentHealth, PlayerStats.MaxHealth);
    
    // Update health display
    UpdateHealthDisplay();
    
    UE_LOG(LogTemp, Log, TEXT("Max health changed from %d to %d, current health: %d"), 
           PreviousMaxHealth, PlayerStats.MaxHealth, PlayerStats.CurrentHealth);
}

void ABloodreadPlayerCharacter::SetCurrentHealth(int32 NewHealth)
{
    if (NewHealth < 0)
    {
        UE_LOG(LogTemp, Warning, TEXT("Attempted to set negative health: %d"), NewHealth);
        return;
    }
    
    int32 PreviousHealth = PlayerStats.CurrentHealth;
    bool WasAlive = IsAlive();
    
    PlayerStats.CurrentHealth = FMath::Min(NewHealth, PlayerStats.MaxHealth);
    
    OnHealthChanged.Broadcast(PlayerStats.CurrentHealth, PlayerStats.MaxHealth);
    
    // Update health display
    UpdateHealthDisplay();
    
    // Check for death state change
    if (WasAlive && !IsAlive())
    {
        OnPlayerDeath.Broadcast(this);
        UE_LOG(LogTemp, Warning, TEXT("Player died from health being set to %d"), PlayerStats.CurrentHealth);
    }
    
    UE_LOG(LogTemp, Log, TEXT("Health set from %d to %d"), PreviousHealth, PlayerStats.CurrentHealth);
}

void ABloodreadPlayerCharacter::RestoreToFullHealth()
{
    if (PlayerStats.CurrentHealth < PlayerStats.MaxHealth)
    {
        PlayerStats.CurrentHealth = PlayerStats.MaxHealth;
        OnHealthChanged.Broadcast(PlayerStats.CurrentHealth, PlayerStats.MaxHealth);
        
        // Update health display
        UpdateHealthDisplay();
        
        UE_LOG(LogTemp, Log, TEXT("Player health restored to full: %d/%d"), 
               PlayerStats.CurrentHealth, PlayerStats.MaxHealth);
    }
}

void ABloodreadPlayerCharacter::UpdateHealthDisplay()
{
    if (HealthBarWidget && IsValid(HealthBarWidget))
    {
        // Try common Progress Bar names
        UProgressBar* HealthProgressBar = nullptr;
        TArray<FString> ProgressBarNames = {TEXT("HealthProgressBar"), TEXT("ProgressBar"), TEXT("HealthBar"), TEXT("Health_ProgressBar")};
        
        for (const FString& Name : ProgressBarNames)
        {
            HealthProgressBar = Cast<UProgressBar>(HealthBarWidget->GetWidgetFromName(*Name));
            if (HealthProgressBar)
            {
                HealthProgressBar->SetPercent(GetHealthPercentage());
                UE_LOG(LogTemp, Log, TEXT("Updated Progress Bar '%s' to %f%%"), *Name, GetHealthPercentage() * 100.0f);
                break;
            }
        }
        
        // Try common Text Block names  
        UTextBlock* HealthText = nullptr;
        TArray<FString> TextBlockNames = {TEXT("HealthText"), TEXT("TextBlock"), TEXT("HealthLabel"), TEXT("Health_Text")};
        
        for (const FString& Name : TextBlockNames)
        {
            HealthText = Cast<UTextBlock>(HealthBarWidget->GetWidgetFromName(*Name));
            if (HealthText)
            {
                FString HealthString = FString::Printf(TEXT("%d/%d"), PlayerStats.CurrentHealth, PlayerStats.MaxHealth);
                HealthText->SetText(FText::FromString(HealthString));
                UE_LOG(LogTemp, Log, TEXT("Updated Text Block '%s' to '%s'"), *Name, *HealthString);
                break;
            }
        }
        
        if (!HealthProgressBar && !HealthText)
        {
            UE_LOG(LogTemp, Warning, TEXT("Could not find Progress Bar or Text Block components in widget"));
        }
        
        UE_LOG(LogTemp, Log, TEXT("UpdateHealthDisplay called: %d/%d (%f%%)"), 
               PlayerStats.CurrentHealth, PlayerStats.MaxHealth, GetHealthPercentage() * 100.0f);
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("HealthBarWidget is null or invalid - need to call SetHealthBarWidget first"));
    }
}

void ABloodreadPlayerCharacter::SetHealthBarWidget(UUserWidget* Widget)
{
    HealthBarWidget = Widget;
    UE_LOG(LogTemp, Log, TEXT("Health bar widget set"));
}

bool ABloodreadPlayerCharacter::CanUseAbility1() const
{
    return IsAlive() && PlayerLoadout.Ability1CooldownRemaining <= 0.0f;
}

bool ABloodreadPlayerCharacter::CanUseAbility2() const
{
    return IsAlive() && PlayerLoadout.Ability2CooldownRemaining <= 0.0f;
}

float ABloodreadPlayerCharacter::GetAbility1CooldownPercentage() const
{
    if (PlayerLoadout.Ability1.Cooldown <= 0.0f) return 0.0f;
    return FMath::Clamp(PlayerLoadout.Ability1CooldownRemaining / PlayerLoadout.Ability1.Cooldown, 0.0f, 1.0f);
}

float ABloodreadPlayerCharacter::GetAbility2CooldownPercentage() const
{
    if (PlayerLoadout.Ability2.Cooldown <= 0.0f) return 0.0f;
    return FMath::Clamp(PlayerLoadout.Ability2CooldownRemaining / PlayerLoadout.Ability2.Cooldown, 0.0f, 1.0f);
}

void ABloodreadPlayerCharacter::SetCameraPosition(FVector NewRelativeLocation)
{
    if (FirstPersonCamera)
    {
        FirstPersonCamera->SetRelativeLocation(NewRelativeLocation);
        CustomMeshCameraOffset = NewRelativeLocation;
        UE_LOG(LogTemp, Warning, TEXT("Camera position set to: %s"), *NewRelativeLocation.ToString());
    }
}

void ABloodreadPlayerCharacter::AdjustCameraForMesh(float HeightOffset, float ForwardOffset, float RightOffset)
{
    FVector BaseOffset = FVector(-39.56f, 1.75f, 64.0f); // Default position
    FVector AdjustedOffset = BaseOffset + FVector(ForwardOffset, RightOffset, HeightOffset);
    
    // Apply mesh scale multiplier
    AdjustedOffset *= MeshScaleMultiplier;
    
    SetCameraPosition(AdjustedOffset);
    
    UE_LOG(LogTemp, Warning, TEXT("Camera adjusted for custom mesh - Height: %.1f, Forward: %.1f, Right: %.1f"), 
           HeightOffset, ForwardOffset, RightOffset);
}

FVector ABloodreadPlayerCharacter::GetCurrentCameraPosition() const
{
    return FirstPersonCamera ? FirstPersonCamera->GetRelativeLocation() : FVector::ZeroVector;
}

void ABloodreadPlayerCharacter::InitializeFromLoadout(const FPlayerLoadout& Loadout, int32 HealthBonus, int32 StrengthBonus)
{
    PlayerLoadout = Loadout;
    
    // Apply bonuses from items
    PlayerStats.MaxHealth += HealthBonus;
    PlayerStats.CurrentHealth = PlayerStats.MaxHealth;
    PlayerStats.Strength += StrengthBonus;
    
    OnHealthChanged.Broadcast(PlayerStats.CurrentHealth, PlayerStats.MaxHealth);
    
    UE_LOG(LogTemp, Log, TEXT("Player initialized - Health: %d, Strength: %d"), 
           PlayerStats.MaxHealth, PlayerStats.Strength);
}

FVector ABloodreadPlayerCharacter::GetDirectionToOtherPlayer()
{
    if (OtherPlayer)
    {
        return (OtherPlayer->GetActorLocation() - GetActorLocation()).GetSafeNormal();
    }
    return FVector::ZeroVector;
}

ABloodreadPlayerCharacter* ABloodreadPlayerCharacter::FindOtherPlayer()
{
    TArray<AActor*> FoundActors;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), ABloodreadPlayerCharacter::StaticClass(), FoundActors);
    
    for (AActor* Actor : FoundActors)
    {
        ABloodreadPlayerCharacter* Player = Cast<ABloodreadPlayerCharacter>(Actor);
        if (Player && Player != this)
        {
            return Player;
        }
    }
    return nullptr;
}

void ABloodreadPlayerCharacter::ProcessGameTick()
{
    // Decrease damage immunity
    if (PlayerStats.DamageImmunityTicksRemaining > 0)
    {
        PlayerStats.DamageImmunityTicksRemaining--;
        if (PlayerStats.DamageImmunityTicksRemaining == 0)
        {
            PlayerStats.bCanTakeDamage = true;
        }
    }
}

float ABloodreadPlayerCharacter::GetHealthPercentage() const
{
    if (PlayerStats.MaxHealth == 0) return 0.0f;
    return (float)PlayerStats.CurrentHealth / (float)PlayerStats.MaxHealth;
}

TArray<ABloodreadPlayerCharacter*> ABloodreadPlayerCharacter::GetPlayersInRange(float Range)
{
    TArray<ABloodreadPlayerCharacter*> PlayersInRange;
    TArray<AActor*> FoundActors;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), ABloodreadPlayerCharacter::StaticClass(), FoundActors);
    
    for (AActor* Actor : FoundActors)
    {
        ABloodreadPlayerCharacter* Player = Cast<ABloodreadPlayerCharacter>(Actor);
        if (Player && Player != this)
        {
            float Distance = FVector::Dist(GetActorLocation(), Player->GetActorLocation());
            if (Distance <= Range)
            {
                PlayersInRange.Add(Player);
            }
        }
    }
    
    return PlayersInRange;
}

TArray<ABloodreadPlayerCharacter::FDamageableTarget> ABloodreadPlayerCharacter::GetDamageableTargetsInRange(float Range)
{
    TArray<FDamageableTarget> TargetsInRange;
    
    // Get all players in range
    TArray<AActor*> FoundPlayers;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), ABloodreadPlayerCharacter::StaticClass(), FoundPlayers);
    
    for (AActor* Actor : FoundPlayers)
    {
        ABloodreadPlayerCharacter* Player = Cast<ABloodreadPlayerCharacter>(Actor);
        if (Player && Player != this)
        {
            float Distance = FVector::Dist(GetActorLocation(), Player->GetActorLocation());
            if (Distance <= Range)
            {
                TargetsInRange.Add(FDamageableTarget(Player, true));
            }
        }
    }
    
    // Get all practice dummies in range
    TArray<AActor*> FoundDummies;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), APracticeDummy::StaticClass(), FoundDummies);
    
    for (AActor* Actor : FoundDummies)
    {
        APracticeDummy* Dummy = Cast<APracticeDummy>(Actor);
        if (Dummy)
        {
            float Distance = FVector::Dist(GetActorLocation(), Dummy->GetActorLocation());
            if (Distance <= Range)
            {
                TargetsInRange.Add(FDamageableTarget(Dummy, false));
            }
        }
    }
    
    return TargetsInRange;
}

void ABloodreadPlayerCharacter::UpdateCooldowns(float DeltaTime)
{
    if (PlayerLoadout.Ability1CooldownRemaining > 0.0f)
    {
        PlayerLoadout.Ability1CooldownRemaining = FMath::Max(0.0f, PlayerLoadout.Ability1CooldownRemaining - DeltaTime);
    }
    
    if (PlayerLoadout.Ability2CooldownRemaining > 0.0f)
    {
        PlayerLoadout.Ability2CooldownRemaining = FMath::Max(0.0f, PlayerLoadout.Ability2CooldownRemaining - DeltaTime);
    }
}
