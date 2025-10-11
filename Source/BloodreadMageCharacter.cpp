#include "BloodreadMageCharacter.h"
#include "Engine/Engine.h"
#include "Animation/AnimBlueprint.h"
#include "Engine/StreamableManager.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Engine/DamageEvents.h"

ABloodreadMageCharacter::ABloodreadMageCharacter()
{
    // Set mage-specific defaults
    UE_LOG(LogTemp, Warning, TEXT("MageCharacter constructor - initializing mage data"));
    InitializeMageData();
    UE_LOG(LogTemp, Warning, TEXT("MageCharacter constructor complete - CurrentClass=%d"), (int32)CurrentCharacterClass);
}

void ABloodreadMageCharacter::BeginPlay()
{
    Super::BeginPlay();
    
    // Apply mage-specific initialization
    InitializeMageData();
    InitializeFromClassData(CharacterClassData);
}

void ABloodreadMageCharacter::OnCharacterClassChanged()
{
    Super::OnCharacterClassChanged();
    UE_LOG(LogTemp, Warning, TEXT("Mage character class changed"));
}

bool ABloodreadMageCharacter::OnAbility1Used()
{
    Super::OnAbility1Used();
    FieryAura();
    return true;
}

bool ABloodreadMageCharacter::OnAbility2Used()
{
    Super::OnAbility2Used();
    Explosion();
    return true;
}

void ABloodreadMageCharacter::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    RegenerateMana(DeltaTime);
}

void ABloodreadMageCharacter::FieryAura()
{
    // Mage Ability 1: Fiery Aura - Spawn spherical damage aura
    UE_LOG(LogTemp, Warning, TEXT("Mage creates Fiery Aura!"));
    
    // Note: Mana consumption and cooldown are already handled by BaseCharacter::UseAbility1()
    // This function is called from OnAbility1Used() after the base checks pass
    
    // Create a timer for the aura damage
    FTimerHandle AuraTimerHandle;
    FVector AuraCenter = GetActorLocation();
    
    // Function to handle aura damage
    auto AuraDamageFunction = [this, AuraCenter]()
    {
        // Find all enemies within aura radius
        TArray<FHitResult> HitResults;
        FVector StartLocation = AuraCenter;
        FVector EndLocation = StartLocation + FVector(0, 0, 1); // Small vertical offset for sphere trace
        
        FCollisionShape Sphere = FCollisionShape::MakeSphere(AuraRadius);
        FCollisionQueryParams QueryParams;
        QueryParams.AddIgnoredActor(this);
        
        bool bHit = GetWorld()->SweepMultiByChannel(
            HitResults,
            StartLocation,
            EndLocation,
            FQuat::Identity,
            ECC_Pawn,
            Sphere,
            QueryParams
        );
        
        if (bHit)
        {
            for (const FHitResult& Hit : HitResults)
            {
                if (ABloodreadBaseCharacter* Enemy = Cast<ABloodreadBaseCharacter>(Hit.GetActor()))
                {
                    if (Enemy != this) // Don't damage self
                    {
                        // Use enhanced damage system with light knockback
                        FVector KnockbackDirection = (Enemy->GetActorLocation() - GetActorLocation()).GetSafeNormal();
                        float KnockbackForce = 200.0f; // Light knockback for DOT effect
                        Enemy->DealDamageWithKnockback(1.0f, KnockbackDirection, KnockbackForce, this);
                        
                        // Mage gains 20 mana per damage dealt
                        CurrentMana = FMath::Min(CurrentMana + 20, 500); // Cap at reasonable limit
                        
                        UE_LOG(LogTemp, Warning, TEXT("Fiery Aura damaged %s, mage gained 20 mana (current: %d)"), *Enemy->GetName(), CurrentMana);
                    }
                }
            }
        }
    };
    
    // Set timer to repeat damage every 0.5 seconds indefinitely (or until character dies/aura is destroyed)
    GetWorldTimerManager().SetTimer(AuraTimerHandle, AuraDamageFunction, AuraDamageInterval, true);
    
    // Store the timer handle so we can track multiple auras
    ActiveAuraTimers.Add(AuraTimerHandle);
    
    UE_LOG(LogTemp, Warning, TEXT("Fiery Aura created at %s with %f radius"), *AuraCenter.ToString(), AuraRadius);
}

void ABloodreadMageCharacter::Explosion()
{
    // Mage Ability 2: Explosion - Omnidirectional knockback and damage
    UE_LOG(LogTemp, Warning, TEXT("Mage triggers Explosion!"));
    
    // Note: Mana consumption and cooldown are already handled by BaseCharacter::UseAbility2()
    // This function is called from OnAbility2Used() after the base checks pass
    
    FVector ExplosionCenter = GetActorLocation();
    
    // Find all enemies within explosion radius
    TArray<FHitResult> HitResults;
    FCollisionShape Sphere = FCollisionShape::MakeSphere(ExplosionRadius);
    FCollisionQueryParams QueryParams;
    QueryParams.AddIgnoredActor(this);
    
    bool bHit = GetWorld()->SweepMultiByChannel(
        HitResults,
        ExplosionCenter,
        ExplosionCenter + FVector(0, 0, 1),
        FQuat::Identity,
        ECC_Pawn,
        Sphere,
        QueryParams
    );
    
    if (bHit)
    {
        for (const FHitResult& Hit : HitResults)
        {
            if (ABloodreadBaseCharacter* Enemy = Cast<ABloodreadBaseCharacter>(Hit.GetActor()))
            {
                if (Enemy != this) // Don't affect self
                {
                    // Use enhanced damage system with explosion knockback
                    FVector KnockbackDirection = (Enemy->GetActorLocation() - ExplosionCenter).GetSafeNormal();
                    KnockbackDirection.Z = 0.3f; // Add upward component
                    float KnockbackForce = ExplosionKnockback * 0.5f; // 50% knockback
                    
                    Enemy->DealDamageWithKnockback(10.0f, KnockbackDirection, KnockbackForce, this);
                    
                    UE_LOG(LogTemp, Warning, TEXT("Explosion hit %s for 10 damage with knockback"), *Enemy->GetName());
                }
            }
        }
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Explosion triggered at %s with %f radius"), *ExplosionCenter.ToString(), ExplosionRadius);
}

void ABloodreadMageCharacter::Attack()
{
    Super::Attack();
    
    // Mage-specific attack implementation - deals 6 damage, gains 10 mana per hit
    UE_LOG(LogTemp, Warning, TEXT("Mage attacks with magic missile for 6 damage!"));
    
    // Set mage-specific damage
    float MageDamage = 6.0f;
    
    // Get camera direction for targeting
    FVector CameraLocation;
    FRotator CameraRotation;
    GetActorEyesViewPoint(CameraLocation, CameraRotation);
    FVector ForwardVector = CameraRotation.Vector();
    
    // Line trace for ranged magic attack
    FVector StartLocation = CameraLocation;
    FVector EndLocation = StartLocation + (ForwardVector * 800.0f); // Longer range than melee
    
    FHitResult HitResult;
    FCollisionQueryParams QueryParams;
    QueryParams.AddIgnoredActor(this);
    
    if (GetWorld()->LineTraceSingleByChannel(HitResult, StartLocation, EndLocation, ECC_Pawn, QueryParams))
    {
        if (ABloodreadBaseCharacter* Enemy = Cast<ABloodreadBaseCharacter>(HitResult.GetActor()))
        {
            // Use enhanced damage system with projectile knockback
            FVector KnockbackDirection = (Enemy->GetActorLocation() - GetActorLocation()).GetSafeNormal();
            float KnockbackForce = 350.0f; // Moderate knockback for projectile
            Enemy->DealDamageWithKnockback(MageDamage, KnockbackDirection, KnockbackForce, this);
            
            // Gain 10 mana per hit
            CurrentMana = FMath::Min(CurrentMana + 10, 500); // Cap at reasonable mana limit
            
            UE_LOG(LogTemp, Warning, TEXT("Mage hit enemy for %f damage, gained 10 mana (current: %d)"), MageDamage, CurrentMana);
        }
    }
}

void ABloodreadMageCharacter::InitializeMageData()
{
    // Set character class
    CurrentCharacterClass = ECharacterClass::Mage;
    
    // Initialize mage stats (based on new specification)
    FCharacterStats MageStats;
    MageStats.MaxHealth = 100;     // 100 base health
    MageStats.Strength = 6;        // Low strength
    MageStats.Defense = 4;         // Low defense
    MageStats.Speed = 12;          // High speed
    MageStats.Mana = 150;          // Starting mana
    MageStats.CriticalChance = 0.2f; // Higher crit for spells
    
    // Initialize mage abilities (new system)
    FCharacterAbilityData FieryAuraAbility;
    FieryAuraAbility.Name = "Fiery Aura";
    FieryAuraAbility.Description = "Spawn a spherical aura that deals damage and grants mana";
    FieryAuraAbility.Type = EAbilityType::Damage;
    FieryAuraAbility.Cooldown = 2.0f;   // 2 second cooldown
    FieryAuraAbility.ManaCost = 100;    // Costs 100 mana
    FieryAuraAbility.Damage = 1.0f;     // Deals 1 damage per tick
    FieryAuraAbility.Duration = 0.0f;   // Permanent until destroyed
    
    FCharacterAbilityData ExplosionAbility;
    ExplosionAbility.Name = "Explosion";
    ExplosionAbility.Description = "Deal knockback and damage in omnidirectional radius";
    ExplosionAbility.Type = EAbilityType::Damage;
    ExplosionAbility.Cooldown = 30.0f;  // 30 second cooldown
    ExplosionAbility.ManaCost = 50;     // Costs 50 mana
    ExplosionAbility.Damage = 10.0f;    // Deals 10 damage
    ExplosionAbility.Duration = 0.0f;   // Instant effect
    
    // Set up character class data
    CharacterClassData.CharacterClass = ECharacterClass::Mage;
    CharacterClassData.ClassName = "Mage";
    CharacterClassData.Description = "A powerful spellcaster with aura creation and explosive abilities";
    CharacterClassData.BaseStats = MageStats;
    CharacterClassData.Ability1 = FieryAuraAbility;
    CharacterClassData.Ability2 = ExplosionAbility;
    
    // Set mage mesh (Aurora character)
    CharacterClassData.CharacterMesh = TSoftObjectPtr<USkeletalMesh>(FSoftObjectPath(TEXT("/Game/ParagonAurora/Characters/Heroes/Aurora/Meshes/Aurora.Aurora")));
    
    // Set mage animation blueprint path (will be loaded later by the system)
    CharacterClassData.AnimationBlueprintPath = TEXT("/Game/ParagonAurora/Characters/Heroes/Aurora/Animations/Aurora_AnimBlueprint.Aurora_AnimBlueprint_C");
    UE_LOG(LogTemp, Warning, TEXT("Set Aurora animation blueprint path: %s"), *CharacterClassData.AnimationBlueprintPath);
    
    // Set mage attack animation paths (using actual existing animation files)
    CharacterClassData.BasicAttackAnimationPath = TEXT("/Game/ParagonAurora/Characters/Heroes/Aurora/Animations/Primary_Attack_Slow_A.Primary_Attack_Slow_A");
    CharacterClassData.Ability1AnimationPath = TEXT("/Game/ParagonAurora/Characters/Heroes/Aurora/Animations/Ability_Q.Ability_Q");
    CharacterClassData.Ability2AnimationPath = TEXT("/Game/ParagonAurora/Characters/Heroes/Aurora/Animations/Ability_E.Ability_E");
    
    // Set camera offset adjusted for Aurora character
    CharacterClassData.CameraOffset = FVector(25.0f, 0.0f, 85.0f);
    
    // Apply stats
    CurrentStats = MageStats;
    CurrentHealth = CurrentStats.MaxHealth;
    CurrentMana = CurrentStats.Mana;
}

void ABloodreadMageCharacter::RegenerateMana(float DeltaTime)
{
    // Mages regenerate mana over time
    if (CurrentMana < CurrentStats.Mana)
    {
        int32 OldMana = CurrentMana;
        float ManaToAdd = 10.0f * DeltaTime; // 10 mana per second
        CurrentMana = FMath::Min(CurrentStats.Mana, CurrentMana + FMath::RoundToInt(ManaToAdd));
        
        if (CurrentMana != OldMana)
        {
            OnManaChanged(OldMana, CurrentMana);
        }
    }
}
