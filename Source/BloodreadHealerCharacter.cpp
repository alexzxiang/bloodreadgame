#include "BloodreadHealerCharacter.h"
#include "Engine/Engine.h"
#include "Animation/AnimBlueprint.h"
#include "Engine/StreamableManager.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "TimerManager.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/DamageEvents.h"

ABloodreadHealerCharacter::ABloodreadHealerCharacter()
{
    // Set healer-specific defaults
    UE_LOG(LogTemp, Warning, TEXT("HealerCharacter constructor - initializing healer data"));
    InitializeHealerData();
    UE_LOG(LogTemp, Warning, TEXT("HealerCharacter constructor complete - CurrentClass=%d"), (int32)CurrentCharacterClass);
}

void ABloodreadHealerCharacter::BeginPlay()
{
    Super::BeginPlay();
    
    // Apply healer-specific initialization
    InitializeHealerData();
    InitializeFromClassData(CharacterClassData);
}

void ABloodreadHealerCharacter::OnCharacterClassChanged()
{
    Super::OnCharacterClassChanged();
    UE_LOG(LogTemp, Warning, TEXT("Healer character class changed"));
}

bool ABloodreadHealerCharacter::OnAbility1Used()
{
    Super::OnAbility1Used();
    Bond();
    return true;
}

bool ABloodreadHealerCharacter::OnAbility2Used()
{
    Super::OnAbility2Used();
    Regeneration();
    return true;
}

void ABloodreadHealerCharacter::Bond()
{
    // Healer Ability 1: Bond - Heal teammate for 50% of missing health
    UE_LOG(LogTemp, Warning, TEXT("Healer uses Bond!"));
    
    // Note: Mana consumption and cooldown are already handled by BaseCharacter::UseAbility1()
    // This function is called from OnAbility1Used() after the base checks pass
    
    // Get camera direction for targeting
    FVector CameraLocation;
    FRotator CameraRotation;
    GetActorEyesViewPoint(CameraLocation, CameraRotation);
    FVector ForwardVector = CameraRotation.Vector();
    
    // Line trace to find teammate
    FHitResult HitResult;
    FVector StartLocation = CameraLocation;
    FVector EndLocation = StartLocation + (ForwardVector * HealRange);
    
    FCollisionQueryParams QueryParams;
    QueryParams.AddIgnoredActor(this);
    
    if (GetWorld()->LineTraceSingleByChannel(HitResult, StartLocation, EndLocation, ECC_Pawn, QueryParams))
    {
        if (ABloodreadBaseCharacter* Target = Cast<ABloodreadBaseCharacter>(HitResult.GetActor()))
        {
            // TODO: Add team check when multiplayer is implemented
            // For now, heal any character that isn't self
            if (Target != this)
            {
                // Calculate 50% of missing health
                int32 CurrentTargetHealth = Target->GetCurrentHealthFloat();
                int32 MaxTargetHealth = Target->GetMaxHealthFloat();
                int32 MissingHealth = MaxTargetHealth - CurrentTargetHealth;
                int32 HealAmount = FMath::RoundToInt(MissingHealth * 0.5f);
                
                // Heal the target
                int32 NewHealth = FMath::Min(CurrentTargetHealth + HealAmount, MaxTargetHealth);
                Target->SetCurrentHealth(NewHealth);
                
                UE_LOG(LogTemp, Warning, TEXT("Bond healed %s for %d health (from %d to %d)"), 
                       *Target->GetName(), HealAmount, CurrentTargetHealth, NewHealth);
            }
        }
    }
}

void ABloodreadHealerCharacter::Regeneration()
{
    // Healer Ability 2: Regeneration - Make all teammates regenerate health
    UE_LOG(LogTemp, Warning, TEXT("Healer uses Regeneration!"));
    
    // Note: Mana consumption and cooldown are already handled by BaseCharacter::UseAbility2()
    // This function is called from OnAbility2Used() after the base checks pass
    
    // Find all characters in the world (teammates)
    TArray<AActor*> FoundActors;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), ABloodreadBaseCharacter::StaticClass(), FoundActors);
    
    for (AActor* Actor : FoundActors)
    {
        if (ABloodreadBaseCharacter* Character = Cast<ABloodreadBaseCharacter>(Actor))
        {
            // TODO: Add team check when multiplayer is implemented
            // For now, regenerate all characters except self
            if (Character != this)
            {
                // Start regeneration timer for this character
                FTimerHandle RegenTimerHandle;
                
                auto RegenFunction = [this, Character]()
                {
                    if (IsValid(Character))
                    {
                        int32 CurrentHealth = Character->GetCurrentHealthFloat();
                        int32 MaxHealth = Character->GetMaxHealthFloat();
                        
                        if (CurrentHealth < MaxHealth)
                        {
                            int32 NewHealth = FMath::Min(CurrentHealth + TeamRegenRate, MaxHealth);
                            Character->SetCurrentHealth(NewHealth);
                            
                            UE_LOG(LogTemp, Warning, TEXT("Regeneration healed %s for %d health"), 
                                   *Character->GetName(), (int32)TeamRegenRate);
                        }
                    }
                };
                
                // Set timer to regenerate every second for 10 seconds
                GetWorldTimerManager().SetTimer(RegenTimerHandle, RegenFunction, 1.0f, true);
                ActiveRegenTimers.Add(RegenTimerHandle);
                
                // Clear the timer after duration
                FTimerHandle ClearTimerHandle;
                GetWorldTimerManager().SetTimer(ClearTimerHandle, [this, Character, RegenFunction]()
                {
                    // Clear by finding the matching timer for this character
                    for (int32 i = ActiveRegenTimers.Num() - 1; i >= 0; i--)
                    {
                        GetWorldTimerManager().ClearTimer(ActiveRegenTimers[i]);
                        ActiveRegenTimers.RemoveAt(i);
                    }
                }, TeamRegenDuration, false);
            }
        }
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Regeneration activated for all teammates"));
}

void ABloodreadHealerCharacter::Attack()
{
    Super::Attack();
    
    // Healer-specific attack implementation - deals 6 damage, gains 20 mana per hit
    UE_LOG(LogTemp, Warning, TEXT("Healer attacks for 6 damage!"));
    
    // Set healer-specific damage
    float HealerDamage = 6.0f;
    
    // Get camera direction for targeting
    FVector CameraLocation;
    FRotator CameraRotation;
    GetActorEyesViewPoint(CameraLocation, CameraRotation);
    FVector ForwardVector = CameraRotation.Vector();
    
    // Line trace for ranged healing staff attack
    FVector StartLocation = CameraLocation;
    FVector EndLocation = StartLocation + (ForwardVector * 600.0f); // Medium range
    
    FHitResult HitResult;
    FCollisionQueryParams QueryParams;
    QueryParams.AddIgnoredActor(this);
    
    if (GetWorld()->LineTraceSingleByChannel(HitResult, StartLocation, EndLocation, ECC_Pawn, QueryParams))
    {
        if (ABloodreadBaseCharacter* Enemy = Cast<ABloodreadBaseCharacter>(HitResult.GetActor()))
        {
            // Use enhanced damage system with healing knockback
            FVector KnockbackDirection = (Enemy->GetActorLocation() - GetActorLocation()).GetSafeNormal();
            float KnockbackForce = 300.0f; // Lighter knockback for healer attack
            Enemy->DealDamageWithKnockback(HealerDamage, KnockbackDirection, KnockbackForce, this);
            
            // Gain 20 mana per hit
            CurrentMana = FMath::Min(CurrentMana + 20, 400); // Cap at reasonable mana limit
            
            UE_LOG(LogTemp, Warning, TEXT("Healer hit enemy for %f damage, gained 20 mana (current: %d)"), HealerDamage, CurrentMana);
        }
    }
}

void ABloodreadHealerCharacter::InitializeHealerData()
{
    // Set character class
    CurrentCharacterClass = ECharacterClass::Healer;
    
    // Initialize healer stats (based on new specification)
    FCharacterStats HealerStats;
    HealerStats.MaxHealth = 120;   // 120 base health
    HealerStats.Strength = 6;      // Low strength
    HealerStats.Defense = 8;       // Medium defense
    HealerStats.Speed = 10;        // Medium speed
    HealerStats.Mana = 150;        // High mana for healing
    HealerStats.CriticalChance = 0.1f; // Low crit chance
    
    // Initialize healer abilities (new system)
    FCharacterAbilityData BondAbility;
    BondAbility.Name = "Bond";
    BondAbility.Description = "Heal teammate for 50% of their missing health";
    BondAbility.Type = EAbilityType::Heal;
    BondAbility.Cooldown = 30.0f;  // 30 second cooldown
    BondAbility.ManaCost = 100;    // Costs 100 mana
    BondAbility.Damage = 0.0f;     // No damage, healing ability
    BondAbility.Duration = 0.0f;   // Instant heal
    
    FCharacterAbilityData RegenerationAbility;
    RegenerationAbility.Name = "Regeneration";
    RegenerationAbility.Description = "Make all teammates regenerate health over time";
    RegenerationAbility.Type = EAbilityType::Heal;
    RegenerationAbility.Cooldown = 15.0f;  // 15 second cooldown
    RegenerationAbility.ManaCost = 50;     // Costs 50 mana
    RegenerationAbility.Damage = 0.0f;     // No damage, healing ability
    RegenerationAbility.Duration = TeamRegenDuration; // 10 seconds of regeneration
    
    // Set up character class data
    CharacterClassData.CharacterClass = ECharacterClass::Healer;
    CharacterClassData.ClassName = "Healer";
    CharacterClassData.Description = "A support character specializing in healing and team regeneration";
    CharacterClassData.BaseStats = HealerStats;
    CharacterClassData.Ability1 = BondAbility;
    CharacterClassData.Ability2 = RegenerationAbility;
    
    // Set healer mesh (Fey character - nature healer)
    CharacterClassData.CharacterMesh = TSoftObjectPtr<USkeletalMesh>(FSoftObjectPath(TEXT("/Game/ParagonFey/Characters/Heroes/Fey/Meshes/Fey.Fey")));
    
    // Set healer animation blueprint path (will be loaded later by the system)
    CharacterClassData.AnimationBlueprintPath = TEXT("/Game/ParagonFey/Characters/Heroes/Fey/Fey_AnimBlueprint.Fey_AnimBlueprint_C");
    UE_LOG(LogTemp, Warning, TEXT("Set Fey animation blueprint path: %s"), *CharacterClassData.AnimationBlueprintPath);
    
    // Set healer attack animation paths (using actual existing animation files)
    CharacterClassData.BasicAttackAnimationPath = TEXT("/Game/ParagonFey/Characters/Heroes/Fey/Animations/Ability_LMB_A_Slow.Ability_LMB_A_Slow");
    CharacterClassData.Ability1AnimationPath = TEXT("/Game/ParagonFey/Characters/Heroes/Fey/Animations/Ability_Q.Ability_Q");
    CharacterClassData.Ability2AnimationPath = TEXT("/Game/ParagonFey/Characters/Heroes/Fey/Animations/Ability_E.Ability_E");
    
    // Set camera offset adjusted for Muriel character
    CharacterClassData.CameraOffset = FVector(25.0f, 0.0f, 85.0f);
    
    // Apply stats
    CurrentStats = HealerStats;
    CurrentHealth = CurrentStats.MaxHealth;
    CurrentMana = CurrentStats.Mana;
}
