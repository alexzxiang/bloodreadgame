#include "BloodreadRogueCharacter.h"
#include "Engine/Engine.h"
#include "Animation/AnimBlueprint.h"
#include "Engine/StreamableManager.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/CapsuleComponent.h"
#include "TimerManager.h"
#include "Engine/DamageEvents.h"
#include "PracticeDummy.h"

ABloodreadRogueCharacter::ABloodreadRogueCharacter()
{
    // Set rogue-specific defaults
    UE_LOG(LogTemp, Warning, TEXT("RogueCharacter constructor - initializing rogue data"));
    InitializeRogueData();
    UE_LOG(LogTemp, Warning, TEXT("RogueCharacter constructor complete - CurrentClass=%d"), (int32)CurrentCharacterClass);
    
    // Rogues are faster and more agile
    GetCharacterMovement()->MaxWalkSpeed = 600.f;
    GetCharacterMovement()->JumpZVelocity = 800.f;
}

void ABloodreadRogueCharacter::BeginPlay()
{
    Super::BeginPlay();
    
    // Apply rogue-specific initialization
    InitializeRogueData();  
    InitializeFromClassData(CharacterClassData);
}

void ABloodreadRogueCharacter::OnCharacterClassChanged()
{
    Super::OnCharacterClassChanged();
    UE_LOG(LogTemp, Warning, TEXT("Rogue character class changed"));
}

bool ABloodreadRogueCharacter::OnAbility1Used()
{
    Super::OnAbility1Used();
    Teleport();
    return true;
}

bool ABloodreadRogueCharacter::OnAbility2Used()
{
    Super::OnAbility2Used();
    ShadowPush();
    return true;
}

void ABloodreadRogueCharacter::Teleport()
{
    // Rogue Ability 1: Teleport - Teleport behind target with same orientation
    UE_LOG(LogTemp, Warning, TEXT("Rogue uses Teleport!"));
    
    // Note: Mana consumption and cooldown are already handled by BaseCharacter::UseAbility1()
    // This function is called from OnAbility1Used() after the base checks pass
    
    // USE UNIVERSAL TARGETING SYSTEM (like BaseCharacter does)
    FTargetableActor Target = GetCrosshairTargetActor();
    
    if (!Target.Actor)
    {
        UE_LOG(LogTemp, Warning, TEXT("Teleport: No target found with universal targeting system"));
        return;
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Teleport: Found target %s using universal targeting"), *Target.Actor->GetName());
    
    // Add detailed debugging for target type detection
    UE_LOG(LogTemp, Warning, TEXT("Teleport: Target.bIsPlayer = %s, Target.bIsDummy = %s"), 
           Target.bIsPlayer ? TEXT("TRUE") : TEXT("FALSE"),
           Target.bIsDummy ? TEXT("TRUE") : TEXT("FALSE"));
    UE_LOG(LogTemp, Warning, TEXT("Teleport: Target actor class: %s"), *Target.Actor->GetClass()->GetName());
    
    // Only teleport to other characters (players or dummies), not random objects
    if (Target.bIsPlayer || Target.bIsDummy)
    {
        UE_LOG(LogTemp, Warning, TEXT("Teleport: Target is valid player or dummy, attempting cast..."));
        
        // Handle both BloodreadBaseCharacter and PracticeDummy targets
        ABloodreadBaseCharacter* CharacterTarget = Cast<ABloodreadBaseCharacter>(Target.Actor);
        APracticeDummy* DummyTarget = Cast<APracticeDummy>(Target.Actor);
        
        UE_LOG(LogTemp, Warning, TEXT("Teleport: Cast results - CharacterTarget = %s, DummyTarget = %s"), 
               CharacterTarget ? TEXT("SUCCESS") : TEXT("FAILED"),
               DummyTarget ? TEXT("SUCCESS") : TEXT("FAILED"));
        
        if (CharacterTarget || DummyTarget)
        {
            // STEP 1: Get target's current position and orientation
            FVector TargetLocation = Target.Actor->GetActorLocation();
            FRotator TargetRotation = Target.Actor->GetActorRotation();
            FVector TargetForward = Target.Actor->GetActorForwardVector();
            
            UE_LOG(LogTemp, Warning, TEXT("Teleport: Target %s at location %s"), *Target.Actor->GetName(), *TargetLocation.ToString());
            UE_LOG(LogTemp, Warning, TEXT("Teleport: Target rotation: %s"), *TargetRotation.ToString());
            UE_LOG(LogTemp, Warning, TEXT("Teleport: Target facing direction (forward): %s"), *TargetForward.ToString());
            
            // STEP 2: Calculate position BEHIND target based on their orientation
            // Behind = opposite of their forward direction
            float BehindDistance = 100.0f; // 1 meter behind for good attack range
            FVector BehindOffset = -TargetForward * BehindDistance;
            FVector TeleportLocation = TargetLocation + BehindOffset;
            
            UE_LOG(LogTemp, Warning, TEXT("Teleport: Behind offset vector: %s"), *BehindOffset.ToString());
            UE_LOG(LogTemp, Warning, TEXT("Teleport: Calculated teleport position: %s"), *TeleportLocation.ToString());
            
            // STEP 3: Adjust for ground level (find valid ground position)
            FHitResult GroundHit;
            FVector TraceStart = TeleportLocation + FVector(0, 0, 200.0f);
            FVector TraceEnd = TeleportLocation - FVector(0, 0, 500.0f);
            
            if (GetWorld()->LineTraceSingleByChannel(GroundHit, TraceStart, TraceEnd, ECC_WorldStatic))
            {
                TeleportLocation = GroundHit.Location + FVector(0, 0, GetCapsuleComponent()->GetScaledCapsuleHalfHeight());
                UE_LOG(LogTemp, Warning, TEXT("Teleport: Adjusted to ground level: %s"), *TeleportLocation.ToString());
            }
            else
            {
                UE_LOG(LogTemp, Warning, TEXT("Teleport: No ground found, using original height"));
            }
            
            // STEP 4: Set rotation to match target's orientation
            // If we're behind them facing their back, we face the same direction as them
            FRotator MatchTargetRotation = TargetRotation;
            
            UE_LOG(LogTemp, Warning, TEXT("Teleport: Matching target's rotation: %s"), *MatchTargetRotation.ToString());
            UE_LOG(LogTemp, Warning, TEXT("Teleport: Behind target, facing same direction (toward their back)"));
            
            // STEP 5: Execute the teleport behind target, facing the target
            FVector CurrentLocation = GetActorLocation();
            FRotator CurrentRotation = GetActorRotation();
            
            UE_LOG(LogTemp, Warning, TEXT("Teleport: === TELEPORT EXECUTION ==="));
            UE_LOG(LogTemp, Warning, TEXT("Teleport: FROM - Position: %s, Rotation: %s"), 
                   *CurrentLocation.ToString(), *CurrentRotation.ToString());
            UE_LOG(LogTemp, Warning, TEXT("Teleport: TO - Position: %s, Rotation: %s"), 
                   *TeleportLocation.ToString(), *MatchTargetRotation.ToString());
            
            // Try multiple teleport methods for reliability
            bool bTeleportSuccess = false;
            
            // Method 1: Use TeleportTo (Unreal's built-in safe teleport)
            bTeleportSuccess = TeleportTo(TeleportLocation, MatchTargetRotation);
            UE_LOG(LogTemp, Warning, TEXT("TeleportTo result: %s"), bTeleportSuccess ? TEXT("SUCCESS") : TEXT("FAILED"));
            
            if (!bTeleportSuccess)
            {
                // Method 2: Try with elevated position
                FVector ElevatedLocation = TeleportLocation + FVector(0, 0, 100.0f);
                bTeleportSuccess = TeleportTo(ElevatedLocation, MatchTargetRotation);
                UE_LOG(LogTemp, Warning, TEXT("Elevated TeleportTo result: %s"), bTeleportSuccess ? TEXT("SUCCESS") : TEXT("FAILED"));
                
                if (bTeleportSuccess)
                {
                    TeleportLocation = ElevatedLocation;
                }
            }
            
            if (!bTeleportSuccess)
            {
                // Method 3: Force teleport using SetActorLocation and SetActorRotation
                UE_LOG(LogTemp, Warning, TEXT("Forcing teleport with SetActorLocation"));
                bTeleportSuccess = SetActorLocation(TeleportLocation, false, nullptr, ETeleportType::TeleportPhysics);
                if (bTeleportSuccess)
                {
                    // Force rotation update - try multiple methods for reliability
                    SetActorRotation(MatchTargetRotation);
                    
                    // Also set control rotation if we have a controller (for players)
                    if (GetController())
                    {
                        GetController()->SetControlRotation(MatchTargetRotation);
                        UE_LOG(LogTemp, Warning, TEXT("Force teleport SUCCESS - Set both Actor and Control rotation"));
                    }
                    else
                    {
                        UE_LOG(LogTemp, Warning, TEXT("Force teleport SUCCESS - Set Actor rotation only"));
                    }
                }
                else
                {
                    UE_LOG(LogTemp, Error, TEXT("Force teleport FAILED"));
                }
            }
            
            // Ensure rotation is properly set regardless of teleport method
            if (bTeleportSuccess)
            {
                // Double-check rotation setting after successful teleport
                SetActorRotation(MatchTargetRotation);
                if (GetController())
                {
                    GetController()->SetControlRotation(MatchTargetRotation);
                }
                
                UE_LOG(LogTemp, Warning, TEXT("Teleport: Final rotation verification - Actor: %s, Control: %s"), 
                       *GetActorRotation().ToString(), 
                       GetController() ? *GetController()->GetControlRotation().ToString() : TEXT("No Controller"));
            }
            
            // Log final result
            if (bTeleportSuccess)
            {
                FVector FinalPosition = GetActorLocation();
                FRotator FinalRotation = GetActorRotation();
                UE_LOG(LogTemp, Error, TEXT("*** TELEPORT SUCCESS *** Final position: %s, rotation: %s"), 
                       *FinalPosition.ToString(), *FinalRotation.ToString());
                UE_LOG(LogTemp, Error, TEXT("*** STEALTH POSITION *** Behind target, same orientation"));
                UE_LOG(LogTemp, Warning, TEXT("Target was facing: %s"), *TargetRotation.ToString());
                UE_LOG(LogTemp, Warning, TEXT("Rogue now facing: %s"), *FinalRotation.ToString());
                UE_LOG(LogTemp, Warning, TEXT("Rotation match: %s"), 
                       FinalRotation.Equals(TargetRotation, 10.0f) ? TEXT("SUCCESS - Same direction") : TEXT("MISMATCH - Rotation issue"));
                
                // Ensure rotation sticks by setting it one more time after a tiny delay
                GetWorld()->GetTimerManager().SetTimerForNextTick([this, MatchTargetRotation]()
                {
                    SetActorRotation(MatchTargetRotation);
                    if (GetController())
                    {
                        GetController()->SetControlRotation(MatchTargetRotation);
                    }
                    UE_LOG(LogTemp, Warning, TEXT("Teleport: Post-teleport rotation enforcement - Final: %s"), 
                           *GetActorRotation().ToString());
                });
            }
            else
            {
                UE_LOG(LogTemp, Error, TEXT("*** TELEPORT COMPLETELY FAILED *** All methods failed"));
            }
            
            // Gain 30% movement speed for 5 seconds
            float CurrentSpeed = GetCharacterMovement()->MaxWalkSpeed;
            GetCharacterMovement()->MaxWalkSpeed = CurrentSpeed * (1.0f + MovementSpeedBonus);
            
            // Clear any existing speed boost timer
            GetWorldTimerManager().ClearTimer(SpeedBoostTimerHandle);
            
            // Set timer to restore normal speed
            GetWorldTimerManager().SetTimer(SpeedBoostTimerHandle, [this, CurrentSpeed]()
            {
                GetCharacterMovement()->MaxWalkSpeed = CurrentSpeed;
                UE_LOG(LogTemp, Warning, TEXT("Teleport speed boost ended"));
            }, SpeedBoostDuration, false);
            
            UE_LOG(LogTemp, Warning, TEXT("Teleported behind %s with speed boost"), *Target.Actor->GetName());
        }
        else
        {
            UE_LOG(LogTemp, Error, TEXT("Teleport: Cast to ABloodreadBaseCharacter FAILED for target %s (class: %s)"), 
                   *Target.Actor->GetName(), *Target.Actor->GetClass()->GetName());
        }
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("Teleport: Target found but not a character - teleporting forward"));
        // Target found but not a character - teleport forward instead
        FVector CameraLocation;
        FRotator CameraRotation;
        GetActorEyesViewPoint(CameraLocation, CameraRotation);
        FVector ForwardVector = CameraRotation.Vector();
        
        FVector TeleportLocation = GetActorLocation() + (ForwardVector * 400.0f);
        bool bTeleportSuccess = TeleportTo(TeleportLocation, GetActorRotation());
        if (!bTeleportSuccess)
        {
            // If teleport failed, try with current ground level
            TeleportLocation.Z = GetActorLocation().Z;
            bTeleportSuccess = TeleportTo(TeleportLocation, GetActorRotation());
        }
        
        if (bTeleportSuccess)
        {
            UE_LOG(LogTemp, Warning, TEXT("No target found - teleported forward successfully"));
        }
        else
        {
            UE_LOG(LogTemp, Warning, TEXT("Forward teleport failed - invalid location"));
        }
    }
}

void ABloodreadRogueCharacter::ShadowPush()
{
    // Rogue Ability 2: Shadow Push - Directional push with shield
    UE_LOG(LogTemp, Warning, TEXT("Rogue uses Shadow Push!"));
    
    // Check if ability is on cooldown or insufficient mana
    if (!CanUseAbility2())
    {
        return;
    }
    
    // Consume mana
    CurrentMana = FMath::Max(0, CurrentMana - 25);
    
    // Start ability cooldown
    CharacterClassData.Ability2.CooldownRemaining = CharacterClassData.Ability2.Cooldown;
    
    // Get camera direction for targeting
    FVector CameraLocation;
    FRotator CameraRotation;
    GetActorEyesViewPoint(CameraLocation, CameraRotation);
    FVector ForwardVector = CameraRotation.Vector();
    
    // Line trace to find target in crosshair
    FHitResult HitResult;
    FVector StartLocation = CameraLocation;
    FVector EndLocation = StartLocation + (ForwardVector * 600.0f);
    
    FCollisionQueryParams QueryParams;
    QueryParams.AddIgnoredActor(this);
    
    if (GetWorld()->LineTraceSingleByChannel(HitResult, StartLocation, EndLocation, ECC_Pawn, QueryParams))
    {
        if (ABloodreadBaseCharacter* Target = Cast<ABloodreadBaseCharacter>(HitResult.GetActor()))
        {
            // Push target in the direction of crosshair (NOT omnidirectional)
            FVector PushDirection = ForwardVector;
            PushDirection.Z = 0.0f; // Keep horizontal
            PushDirection.Normalize();
            
            FVector KnockbackForce = PushDirection * ShadowPushKnockback;
            KnockbackForce.Z += 100.0f; // Add small vertical lift
            
            Target->LaunchCharacter(KnockbackForce, true, true);
            
            UE_LOG(LogTemp, Warning, TEXT("Shadow Push hit %s"), *Target->GetName());
        }
    }
    
    // Gain shield of 20 health for 5 seconds
    int32 NewMaxHealth = CurrentStats.MaxHealth + ShieldHealth;
    int32 NewCurrentHealth = CurrentHealth + ShieldHealth;
    
    // Set temporary max health
    CurrentStats.MaxHealth = NewMaxHealth;
    CurrentHealth = NewCurrentHealth;
    
    // Start shield duration timer
    FTimerHandle ShieldTimerHandle;
    GetWorldTimerManager().SetTimer(ShieldTimerHandle, [this]()
    {
        // Remove shield after duration
        CurrentStats.MaxHealth -= ShieldHealth;
        CurrentHealth = FMath::Min(CurrentHealth, CurrentStats.MaxHealth);
        UE_LOG(LogTemp, Warning, TEXT("Shadow Push shield expired"));
    }, ShieldDuration, false);
    
    UE_LOG(LogTemp, Warning, TEXT("Shadow Push activated - Shield: +%d health for %f seconds"), (int32)ShieldHealth, ShieldDuration);
}

void ABloodreadRogueCharacter::Attack()
{
    Super::Attack();
    
    // Rogue-specific attack implementation - deals 6 damage, gains 20 mana per hit
    UE_LOG(LogTemp, Warning, TEXT("Rogue attacks for 6 damage!"));
    
    // Set rogue-specific damage
    float RogueDamage = 6.0f;
    
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
            // Use enhanced damage system with backstab knockback
            FVector KnockbackDirection = (Enemy->GetActorLocation() - GetActorLocation()).GetSafeNormal();
            float KnockbackForce = 450.0f; // Strong knockback for backstab
            Enemy->DealDamageWithKnockback(RogueDamage, KnockbackDirection, KnockbackForce, this);
            
            // Gain 20 mana per hit
            CurrentMana = FMath::Min(CurrentMana + 20, 400); // Cap at reasonable mana limit
            
            UE_LOG(LogTemp, Warning, TEXT("Rogue hit enemy for %f damage, gained 20 mana (current: %d)"), RogueDamage, CurrentMana);
        }
    }
}

void ABloodreadRogueCharacter::InitializeRogueData()
{
    // Set character class
    CurrentCharacterClass = ECharacterClass::Rogue;
    
    // Initialize rogue stats (based on new specification)
    FCharacterStats RogueStats;
    RogueStats.MaxHealth = 100;    // 100 base health
    RogueStats.Strength = 12;      // Medium strength
    RogueStats.Defense = 6;        // Low defense
    RogueStats.Speed = 16;         // Very high speed
    RogueStats.Mana = 120;         // Starting mana
    RogueStats.CriticalChance = 0.3f; // Very high crit chance
    
    // Initialize rogue abilities (new system)
    FCharacterAbilityData TeleportAbility;
    TeleportAbility.Name = "Teleport";
    TeleportAbility.Description = "Teleport behind target with same orientation and gain movement speed";
    TeleportAbility.Type = EAbilityType::Movement;
    TeleportAbility.Cooldown = 8.0f;   // 8 second cooldown
    TeleportAbility.ManaCost = 100;    // Costs 100 mana
    TeleportAbility.Damage = 0.0f;     // No direct damage
    TeleportAbility.Duration = SpeedBoostDuration; // Movement speed bonus duration
    
    FCharacterAbilityData ShadowPushAbility;
    ShadowPushAbility.Name = "Shadow Push";
    ShadowPushAbility.Description = "Push opponent back and gain a protective shield";
    ShadowPushAbility.Type = EAbilityType::Damage;
    ShadowPushAbility.Cooldown = 8.0f;  // 8 second cooldown
    ShadowPushAbility.ManaCost = 25;    // Costs 25 mana
    ShadowPushAbility.Damage = 0.0f;    // No direct damage, just knockback
    ShadowPushAbility.Duration = ShieldDuration; // Shield duration
    
    // Set up character class data
    CharacterClassData.CharacterClass = ECharacterClass::Rogue;
    CharacterClassData.ClassName = "Rogue";
    CharacterClassData.Description = "A swift assassin specializing in teleportation and tactical positioning";
    CharacterClassData.BaseStats = RogueStats;
    CharacterClassData.Ability1 = TeleportAbility;
    CharacterClassData.Ability2 = ShadowPushAbility;
    
    // Set rogue mesh (Shinbi character)
    CharacterClassData.CharacterMesh = TSoftObjectPtr<USkeletalMesh>(FSoftObjectPath(TEXT("/Game/ParagonShinbi/Characters/Heroes/Shinbi/Skins/Tier_1/Shinbi_Dynasty/Meshes/ShinbiDynasty.ShinbiDynasty")));
    
    // Set rogue animation blueprint path (will be loaded later by the system)
    CharacterClassData.AnimationBlueprintPath = TEXT("/Game/ParagonShinbi/Characters/Heroes/Shinbi/Shinbi_AnimBlueprint.Shinbi_AnimBlueprint_C");
    UE_LOG(LogTemp, Warning, TEXT("Set Shinbi animation blueprint path: %s"), *CharacterClassData.AnimationBlueprintPath);
    
    // Set rogue attack animation paths (using actual existing animation files)
    CharacterClassData.BasicAttackAnimationPath = TEXT("/Game/ParagonShinbi/Characters/Heroes/Shinbi/Animations/PrimaryMelee_B_Slow.PrimaryMelee_B_Slow");
    CharacterClassData.Ability1AnimationPath = TEXT("/Game/ParagonShinbi/Characters/Heroes/Shinbi/Animations/Ability_Dash.Ability_Dash");
    CharacterClassData.Ability2AnimationPath = TEXT("/Game/ParagonShinbi/Characters/Heroes/Shinbi/Animations/Ability_CirclingWolves.Ability_CirclingWolves");
    
    // Set camera offset adjusted for Shinbi Dynasty character
    CharacterClassData.CameraOffset = FVector(25.0f, 0.0f, 85.0f);
    
    // Apply stats
    CurrentStats = RogueStats;
    CurrentHealth = CurrentStats.MaxHealth;
    CurrentMana = CurrentStats.Mana;
}


