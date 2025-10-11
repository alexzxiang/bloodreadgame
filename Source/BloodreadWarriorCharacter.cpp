#include "BloodreadWarriorCharacter.h"
#include "Engine/Engine.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Animation/AnimBlueprint.h"
#include "Engine/StreamableManager.h"
#include "Engine/DamageEvents.h"

ABloodreadWarriorCharacter::ABloodreadWarriorCharacter()
{
    // Set warrior-specific defaults
    UE_LOG(LogTemp, Warning, TEXT("WarriorCharacter constructor - initializing warrior data"));
    InitializeWarriorData();
    UE_LOG(LogTemp, Warning, TEXT("WarriorCharacter constructor complete - CurrentClass=%d"), (int32)CurrentCharacterClass);
}

void ABloodreadWarriorCharacter::BeginPlay()
{
    Super::BeginPlay();
    
    // Apply warrior-specific initialization
    InitializeWarriorData();
    InitializeFromClassData(CharacterClassData);
}

void ABloodreadWarriorCharacter::OnCharacterClassChanged()
{
    Super::OnCharacterClassChanged();
    UE_LOG(LogTemp, Warning, TEXT("Warrior character class changed"));
}

bool ABloodreadWarriorCharacter::OnAbility1Used()
{
    Super::OnAbility1Used();
    HexPunch();
    return true;
}

bool ABloodreadWarriorCharacter::OnAbility2Used()
{
    Super::OnAbility2Used();
    PowerShield();
    return true;
}

void ABloodreadWarriorCharacter::HexPunch()
{
    // Warrior Ability 1: Hex Punch - Pull enemies to your location and damage them
    UE_LOG(LogTemp, Warning, TEXT("Warrior uses Hex Punch!"));
    
    // Note: Mana consumption and cooldown are already handled by BaseCharacter::UseAbility1()
    // This function is called from OnAbility1Used() after the base checks pass
    
    // Get camera direction for targeting
    FVector CameraLocation;
    FRotator CameraRotation;
    GetActorEyesViewPoint(CameraLocation, CameraRotation);
    FVector ForwardVector = CameraRotation.Vector();
    
    // Line trace to find enemies within range
    FHitResult HitResult;
    FVector StartLocation = CameraLocation;
    FVector EndLocation = StartLocation + (ForwardVector * HexPunchRange);
    
    FCollisionQueryParams QueryParams;
    QueryParams.AddIgnoredActor(this);
    
    if (GetWorld()->LineTraceSingleByChannel(HitResult, StartLocation, EndLocation, ECC_Pawn, QueryParams))
    {
        if (ABloodreadBaseCharacter* Enemy = Cast<ABloodreadBaseCharacter>(HitResult.GetActor()))
        {
            // Pull enemy towards warrior with smooth interpolation
            FVector EnemyLocation = Enemy->GetActorLocation();
            FVector WarriorLocation = GetActorLocation();
            
            // Calculate pull direction and launch velocity
            FVector PullDirection = (WarriorLocation - EnemyLocation).GetSafeNormal();
            FVector LaunchVelocity = PullDirection * 800.0f + FVector(0, 0, 200); // Add some vertical lift
            
            // Use enhanced damage system that includes knockback
            FVector KnockbackDirection = PullDirection;
            float KnockbackForce = 600.0f; // Moderate knockback for pull effect
            
            // Use DealDamageWithKnockback for combined damage and knockback
            Enemy->DealDamageWithKnockback(10.0f, KnockbackDirection, KnockbackForce, this);
            
            UE_LOG(LogTemp, Warning, TEXT("Hex Punch hit enemy: %s with enhanced damage system"), *Enemy->GetName());
        }
    }
}

void ABloodreadWarriorCharacter::PowerShield()
{
    // Warrior Ability 2: Power Shield - Temporary health shield with regeneration
    UE_LOG(LogTemp, Warning, TEXT("Warrior uses Power Shield!"));
    
    // Note: Mana consumption and cooldown are already handled by BaseCharacter::UseAbility2()
    // This function is called from OnAbility2Used() after the base checks pass
    
    // Add temporary health (shield)
    int32 NewMaxHealth = CurrentStats.MaxHealth + ShieldHealthBonus;
    int32 NewCurrentHealth = CurrentHealth + ShieldHealthBonus;
    
    // Set temporary max health
    CurrentStats.MaxHealth = NewMaxHealth;
    CurrentHealth = NewCurrentHealth;
    
    // Start shield duration timer
    FTimerHandle ShieldTimerHandle;
    GetWorldTimerManager().SetTimer(ShieldTimerHandle, [this]()
    {
        // Remove shield after duration
        CurrentStats.MaxHealth -= ShieldHealthBonus;
        CurrentHealth = FMath::Min(CurrentHealth, CurrentStats.MaxHealth);
        UE_LOG(LogTemp, Warning, TEXT("Power Shield expired"));
    }, ShieldDuration, false);
    
    // Start health regeneration
    FTimerHandle RegenTimerHandle;
    GetWorldTimerManager().SetTimer(RegenTimerHandle, [this]()
    {
        if (CurrentHealth < CurrentStats.MaxHealth)
        {
            CurrentHealth = FMath::Min(CurrentHealth + ShieldRegenRate, CurrentStats.MaxHealth);
        }
    }, 1.0f, true, 0.0f); // Start immediately, repeat every second
    
    // Clear regen timer after 4 seconds
    FTimerHandle RegenStopTimerHandle;
    GetWorldTimerManager().SetTimer(RegenStopTimerHandle, [this, RegenTimerHandle]() mutable
    {
        GetWorldTimerManager().ClearTimer(RegenTimerHandle);
        UE_LOG(LogTemp, Warning, TEXT("Power Shield regeneration ended"));
    }, 4.0f, false);
    
    // TODO: Add visual effect - glowing blue sphere around player
    UE_LOG(LogTemp, Warning, TEXT("Power Shield activated - Shield: +%d health, Regen: %d health over 4 seconds"), (int32)ShieldHealthBonus, 20);
}

void ABloodreadWarriorCharacter::Attack()
{
    Super::Attack();
    
    // Warrior-specific attack implementation - deals 7 damage, gains 10 mana per hit
    UE_LOG(LogTemp, Warning, TEXT("Warrior attacks with melee weapon for 7 damage!"));
    
    // Set warrior-specific damage
    float WarriorDamage = 7.0f;
    
    // Line trace for melee attack
    FVector StartLocation = GetActorLocation();
    FVector ForwardVector = GetActorForwardVector();
    FVector EndLocation = StartLocation + (ForwardVector * 150.0f); // Melee range
    
    FHitResult HitResult;
    FCollisionQueryParams QueryParams;
    QueryParams.AddIgnoredActor(this);
    
    if (GetWorld()->LineTraceSingleByChannel(HitResult, StartLocation, EndLocation, ECC_Pawn, QueryParams))
    {
        if (ABloodreadBaseCharacter* Enemy = Cast<ABloodreadBaseCharacter>(HitResult.GetActor()))
        {
            // Deal damage
            // Use enhanced damage system with knockback
            FVector KnockbackDirection = (Enemy->GetActorLocation() - GetActorLocation()).GetSafeNormal();
            float KnockbackForce = 400.0f;
            Enemy->DealDamageWithKnockback(WarriorDamage, KnockbackDirection, KnockbackForce, this);
            
            // Gain 10 mana per hit
            CurrentMana = FMath::Min(CurrentMana + 10, 200); // Cap at reasonable mana limit
            
            UE_LOG(LogTemp, Warning, TEXT("Warrior hit enemy for %f damage, gained 10 mana (current: %d)"), WarriorDamage, CurrentMana);
        }
    }
}

void ABloodreadWarriorCharacter::InitializeWarriorData()
{
    // Set character class
    CurrentCharacterClass = ECharacterClass::Warrior;
    
    // Initialize warrior stats (based on new specification)
    FCharacterStats WarriorStats;
    WarriorStats.MaxHealth = 120;  // 120 base health
    WarriorStats.Strength = 18;    // High strength
    WarriorStats.Defense = 12;     // High defense
    WarriorStats.Speed = 8;        // Lower speed
    WarriorStats.Mana = 100;       // Starting mana
    WarriorStats.CriticalChance = 0.15f;
    
    // Initialize warrior abilities (new system)
    FCharacterAbilityData HexPunchAbility;
    HexPunchAbility.Name = "Hex Punch";
    HexPunchAbility.Description = "Pull enemies to your location in a smooth motion and deal damage";
    HexPunchAbility.Type = EAbilityType::Damage;
    HexPunchAbility.Cooldown = 15.0f;  // 15 second cooldown
    HexPunchAbility.ManaCost = 100;    // Costs 100 mana
    HexPunchAbility.Damage = 10.0f;    // Deals 10 damage
    HexPunchAbility.Duration = 0.5f;   // Pull duration
    
    FCharacterAbilityData PowerShieldAbility;
    PowerShieldAbility.Name = "Power Shield";
    PowerShieldAbility.Description = "Gain a shield that adds temporary health and regeneration";
    PowerShieldAbility.Type = EAbilityType::Defense;
    PowerShieldAbility.Cooldown = 15.0f;  // 15 second cooldown
    PowerShieldAbility.ManaCost = 30;     // Costs 30 mana
    PowerShieldAbility.Damage = 0.0f;     // No direct damage
    PowerShieldAbility.Duration = 10.0f;  // Shield lasts 10 seconds
    
    // Set up character class data
    CharacterClassData.CharacterClass = ECharacterClass::Warrior;
    CharacterClassData.ClassName = "Warrior";
    CharacterClassData.Description = "A mighty melee fighter with high health and devastating close-combat abilities";
    CharacterClassData.BaseStats = WarriorStats;
    CharacterClassData.Ability1 = HexPunchAbility;
    CharacterClassData.Ability2 = PowerShieldAbility;
    
    // Set warrior mesh (Gideon character)
    CharacterClassData.CharacterMesh = TSoftObjectPtr<USkeletalMesh>(FSoftObjectPath(TEXT("/Game/ParagonGideon/Characters/Heroes/Gideon/Meshes/Gideon.Gideon")));
    
    // Set warrior animation blueprint path (will be loaded later by the system)
    CharacterClassData.AnimationBlueprintPath = TEXT("/Game/ParagonGideon/Characters/Heroes/Gideon/Animations/Gideon_AnimBlueprint.Gideon_AnimBlueprint_C");
    UE_LOG(LogTemp, Warning, TEXT("Set Gideon animation blueprint path: %s"), *CharacterClassData.AnimationBlueprintPath);
    
    // Set warrior attack animation paths (using actual existing animation files)
    CharacterClassData.BasicAttackAnimationPath = TEXT("/Game/ParagonGideon/Characters/Heroes/Gideon/Animations/Primary_Attack_A_Medium.Primary_Attack_A_Medium");
    CharacterClassData.Ability1AnimationPath = TEXT("/Game/ParagonGideon/Characters/Heroes/Gideon/Animations/Burden_Start.Burden_Start");
    CharacterClassData.Ability2AnimationPath = TEXT("/Game/ParagonGideon/Characters/Heroes/Gideon/Animations/Cosmic_Rift.Cosmic_Rift");
    
    // Set camera offset adjusted for Gideon character
    CharacterClassData.CameraOffset = FVector(25.0f, 0.0f, 85.0f);
    
    // Apply stats
    CurrentStats = WarriorStats;
    CurrentHealth = CurrentStats.MaxHealth;
    CurrentMana = CurrentStats.Mana;
}
