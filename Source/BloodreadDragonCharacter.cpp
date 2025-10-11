#include "BloodreadDragonCharacter.h"
#include "Engine/Engine.h"
#include "Animation/AnimBlueprint.h"
#include "Engine/StreamableManager.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "TimerManager.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/DamageEvents.h"
#include "PracticeDummy.h"

ABloodreadDragonCharacter::ABloodreadDragonCharacter()
{
    // Set dragon-specific defaults
    UE_LOG(LogTemp, Warning, TEXT("DragonCharacter constructor - initializing dragon data"));
    InitializeDragonData();
    UE_LOG(LogTemp, Warning, TEXT("DragonCharacter constructor complete - CurrentClass=%d"), (int32)CurrentCharacterClass);
}

void ABloodreadDragonCharacter::BeginPlay()
{
    Super::BeginPlay();
    
    // Apply dragon-specific initialization
    InitializeDragonData();
    InitializeFromClassData(CharacterClassData);
}

void ABloodreadDragonCharacter::OnCharacterClassChanged()
{
    Super::OnCharacterClassChanged();
    UE_LOG(LogTemp, Warning, TEXT("Dragon character class changed"));
}

void ABloodreadDragonCharacter::UseAbility1()
{
    // Dragon's Ascent ability - special handling for two-part ability
    if (!bAscentFirstPress)
    {
        // First press: Normal ability consumption (mana + cooldown)
        if (CanUseAbility1())
        {
            if (UseMana(CharacterClassData.Ability1.ManaCost))
            {
                CharacterClassData.Ability1.CooldownRemaining = CharacterClassData.Ability1.Cooldown;
                OnAbility1Used();
                UE_LOG(LogTemp, Warning, TEXT("Dragon Ascent First Press - Used Ability 1: %s"), *CharacterClassData.Ability1.Name);
            }
            else
            {
                UE_LOG(LogTemp, Warning, TEXT("Not enough mana for Ascent first press"));
            }
        }
        else
        {
            UE_LOG(LogTemp, Warning, TEXT("Ascent first press on cooldown"));
        }
    }
    else
    {
        // Second press: No mana/cooldown consumption, just execute if in air
        if (bIsInAir)
        {
            UE_LOG(LogTemp, Warning, TEXT("Dragon Ascent Second Press - No mana/cooldown consumed"));
            Ascent(); // Call ascent directly for second press
        }
        else
        {
            UE_LOG(LogTemp, Warning, TEXT("Dragon not in air - cannot use second press"));
        }
    }
}

bool ABloodreadDragonCharacter::OnAbility1Used()
{
    Super::OnAbility1Used();
    Ascent();
    return true;
}

bool ABloodreadDragonCharacter::OnAbility2Used()
{
    Super::OnAbility2Used();
    KingsGreed();
    return true;
}

void ABloodreadDragonCharacter::Ascent()
{
    // Dragon Ability 1: Ascent - Two-part leap and blitz ability
    if (!bAscentFirstPress)
    {
        // First press: Leap into the air
        UE_LOG(LogTemp, Warning, TEXT("Dragon uses Ascent - First Press (Leap)"));
        
        // Note: Mana consumption and cooldown are already handled by BaseCharacter::UseAbility1()
        // This function is called from OnAbility1Used() after the base checks pass
        
        PerformAscentLeap();
        bAscentFirstPress = true;
        bIsInAir = true;
    }
    else
    {
        // Second press: Straight-line ground slam if dragon is airborne
        // Check both our state flag AND actual character movement state for responsiveness
        bool bActuallyInAir = GetCharacterMovement()->IsFalling();
        
        if (bIsInAir && bActuallyInAir)
        {
            UE_LOG(LogTemp, Warning, TEXT("Dragon uses Ascent - Second Press (GRAVITY-BIASED GROUND SLAM)"));
            PerformBlitzDown();
            bAscentFirstPress = false;
            bIsInAir = false;
        }
        else if (!bActuallyInAir)
        {
            UE_LOG(LogTemp, Warning, TEXT("Dragon cannot ground slam - not airborne! bIsInAir=%s, IsFalling=%s"), 
                   bIsInAir ? TEXT("true") : TEXT("false"), 
                   bActuallyInAir ? TEXT("true") : TEXT("false"));
            // Reset state if we're somehow on ground but still have first press active
            bAscentFirstPress = false;
            bIsInAir = false;
        }
    }
}

void ABloodreadDragonCharacter::PerformAscentLeap()
{
    // Get camera direction for mega jump calculation
    FVector CameraLocation;
    FRotator CameraRotation;
    GetActorEyesViewPoint(CameraLocation, CameraRotation);
    
    // Convert pitch to radians - pitch is negative when looking up, positive when looking down
    float PitchRadians = FMath::DegreesToRadians(CameraRotation.Pitch);
    float YawRadians = FMath::DegreesToRadians(CameraRotation.Yaw);
    
    // Height calculation: Looking straight up (-90°) = max height, looking straight down (+90°) = min height
    // Looking at horizon (0°) = medium height
    float NormalizedPitch = FMath::Clamp(PitchRadians / (PI / 2.0f), -1.0f, 1.0f); // -1 to 1 range
    float HeightMultiplier = FMath::Clamp(1.0f - NormalizedPitch, 0.3f, 2.0f); // Higher when looking up
    
    // Forward momentum: Maximum when looking at horizon (pitch = 0), minimum when looking straight up/down
    float ForwardMultiplier = FMath::Abs(FMath::Cos(PitchRadians)); // 1.0 at horizon, 0.0 at straight up/down
    ForwardMultiplier = FMath::Clamp(ForwardMultiplier * 1.5f, 0.2f, 1.5f); // Scale and clamp for better feel
    
    // Calculate mega jump velocity - much stronger forces for dramatic effect
    FVector MegaJumpVelocity;
    MegaJumpVelocity.X = FMath::Cos(YawRadians) * AscentLaunchForce * ForwardMultiplier;
    MegaJumpVelocity.Y = FMath::Sin(YawRadians) * AscentLaunchForce * ForwardMultiplier;
    MegaJumpVelocity.Z = AscentLaunchForce * HeightMultiplier * 1.8f; // Extra vertical boost for mega jump
    
    // Launch the character with dramatic force
    LaunchCharacter(MegaJumpVelocity, true, true);
    
    // Set a timer to automatically reset ability state if player doesn't use second press
    GetWorldTimerManager().ClearTimer(AscentResetTimerHandle);
    GetWorldTimerManager().SetTimer(AscentResetTimerHandle, [this]()
    {
        if (bAscentFirstPress && bIsInAir)
        {
            UE_LOG(LogTemp, Warning, TEXT("Dragon Ascent auto-reset - player didn't use second press"));
            bAscentFirstPress = false;
            bIsInAir = false;
        }
    }, 5.0f, false); // 5 second timeout
    
    UE_LOG(LogTemp, Warning, TEXT("Dragon MEGA JUMP! Pitch: %.1f°, Height Multi: %.2f, Forward Multi: %.2f"), 
           FMath::RadiansToDegrees(PitchRadians), HeightMultiplier, ForwardMultiplier);
    UE_LOG(LogTemp, Warning, TEXT("Mega jump velocity: %s"), *MegaJumpVelocity.ToString());
}

void ABloodreadDragonCharacter::PerformBlitzDown()
{
    // Get current camera direction for gravity-biased ground slam
    FVector CameraLocation;
    FRotator CameraRotation;
    GetActorEyesViewPoint(CameraLocation, CameraRotation);
    
    // Get crosshair direction but apply strong downward bias
    FVector CrosshairDirection = CameraRotation.Vector();
    CrosshairDirection = CrosshairDirection.GetSafeNormal();
    
    // Apply intense gravity bias while preserving some crosshair control
    // The dragon should ALWAYS go downward, but can adjust trajectory based on crosshair
    
    // Extract horizontal (XY) and vertical (Z) components
    FVector HorizontalComponent = FVector(CrosshairDirection.X, CrosshairDirection.Y, 0.0f);
    float VerticalComponent = CrosshairDirection.Z;
    
    // Apply strong downward bias - even if looking up, still go down significantly
    // But allow some upward movement if looking very far up
    float BiasedVerticalComponent = FMath::Clamp(VerticalComponent, -1.0f, -0.3f); // Between 30% and 100% downward
    
    // Scale horizontal component based on crosshair verticality
    // Looking straight down = less horizontal, looking more horizontal = more horizontal movement
    float VerticalIntensity = FMath::Abs(VerticalComponent);
    float HorizontalScale = FMath::Lerp(1.0f, 0.4f, VerticalIntensity); // Linear blend based on vertical look
    
    // Construct gravity-biased slam direction
    FVector GravityBiasedDirection = (HorizontalComponent * HorizontalScale) + FVector(0.0f, 0.0f, BiasedVerticalComponent);
    GravityBiasedDirection = GravityBiasedDirection.GetSafeNormal();
    
    // Calculate ground slam velocity with intense gravitational pull
    FVector GroundSlamVelocity = GravityBiasedDirection * BlitzDownwardForce * 2.0f;
    
    // Apply the gravity-biased ground slam movement
    LaunchCharacter(GroundSlamVelocity, true, true);
    
    // Clear the auto-reset timer since player used second press
    GetWorldTimerManager().ClearTimer(AscentResetTimerHandle);
    
    UE_LOG(LogTemp, Warning, TEXT("Dragon GRAVITY-BIASED GROUND SLAM! Original: %s, Biased: %s, Velocity: %s"), 
           *CrosshairDirection.ToString(), *GravityBiasedDirection.ToString(), *GroundSlamVelocity.ToString());
    UE_LOG(LogTemp, Warning, TEXT("Crosshair Pitch: %.1f°, Yaw: %.1f° | Vertical Bias: %.2f, Horizontal Scale: %.2f"), 
           CameraRotation.Pitch, CameraRotation.Yaw, BiasedVerticalComponent, HorizontalScale);
    
    // The actual landing damage will be handled in the Landed() override function
}

void ABloodreadDragonCharacter::Landed(const FHitResult& Hit)
{
    Super::Landed(Hit);
    
    // Check if we're landing from an ascent ability
    if (bAscentFirstPress && bIsInAir)
    {
        UE_LOG(LogTemp, Warning, TEXT("Dragon landed from Ascent ability!"));
        
        // Reset ascent state
        bAscentFirstPress = false;
        bIsInAir = false;
        
        // Clear any remaining timers
        GetWorldTimerManager().ClearTimer(AscentResetTimerHandle);
        
        // Check for ground slam damage (only if this was a blitz down, indicated by high downward velocity)
        FVector Velocity = GetCharacterMovement()->Velocity;
        bool bWasGroundSlam = Velocity.Z < -2000.0f; // High downward velocity indicates ground slam
        
        if (bWasGroundSlam)
        {
            UE_LOG(LogTemp, Warning, TEXT("Ground slam impact detected! Checking for enemy damage..."));
            
            // Check for enemies within blitz radius when landing
            FVector LandingLocation = GetActorLocation();
            
            TArray<FHitResult> HitResults;
            FCollisionShape Sphere = FCollisionShape::MakeSphere(BlitzRadius);
            FCollisionQueryParams QueryParams;
            QueryParams.AddIgnoredActor(this);
            
            bool bFoundTargets = GetWorld()->SweepMultiByChannel(
                HitResults,
                LandingLocation,
                LandingLocation + FVector(0, 0, 1),
                FQuat::Identity,
                ECC_Pawn,
                Sphere,
                QueryParams
            );
            
            if (bFoundTargets)
            {
                for (const FHitResult& TargetHit : HitResults)
                {
                    if (ABloodreadBaseCharacter* Enemy = Cast<ABloodreadBaseCharacter>(TargetHit.GetActor()))
                    {
                        if (Enemy != this && Enemy->GetIsAlive())
                        {
                            // Deal damage to enemy using our custom damage system
                            Enemy->TakeCustomDamage(static_cast<int32>(BlitzDamage), this);
                            UE_LOG(LogTemp, Warning, TEXT("Dragon ground slam hit %s for %d damage"), *Enemy->GetName(), static_cast<int32>(BlitzDamage));
                        }
                    }
                    else if (APracticeDummy* Dummy = Cast<APracticeDummy>(TargetHit.GetActor()))
                    {
                        if (Dummy->IsAlive())
                        {
                            Dummy->TakeCustomDamage(static_cast<int32>(BlitzDamage), nullptr);
                            UE_LOG(LogTemp, Warning, TEXT("Dragon ground slam hit practice dummy for %d damage"), static_cast<int32>(BlitzDamage));
                        }
                    }
                }
            }
            
            // Take recoil damage from ground slam
            int32 RecoilDamage = static_cast<int32>(BlitzRecoilDamage);
            CurrentHealth = FMath::Max(1, CurrentHealth - RecoilDamage); // Don't let it kill the dragon
            UE_LOG(LogTemp, Warning, TEXT("Dragon took %d recoil damage from ground slam"), RecoilDamage);
        }
    }
}

void ABloodreadDragonCharacter::KingsGreed()
{
    // Dragon Ability 2: King's Greed - Movement speed and potential damage boost
    UE_LOG(LogTemp, Warning, TEXT("Dragon uses King's Greed!"));
    
    // Note: Mana consumption and cooldown are already handled by BaseCharacter::UseAbility2()
    // This function is called from OnAbility2Used() after the base checks pass
    
    // Activate King's Greed
    bKingsGreedActive = true;
    KingsGreedHitCount = 0;
    
    // Apply 30% movement speed immediately
    float CurrentSpeed = GetCharacterMovement()->MaxWalkSpeed;
    GetCharacterMovement()->MaxWalkSpeed = CurrentSpeed * (1.0f + MovementSpeedBonus);
    
    // Set timer for base duration
    GetWorldTimerManager().SetTimer(GreedTimerHandle, [this, CurrentSpeed]()
    {
        // End King's Greed
        bKingsGreedActive = false;
        KingsGreedHitCount = 0;
        GetCharacterMovement()->MaxWalkSpeed = CurrentSpeed;
        
        UE_LOG(LogTemp, Warning, TEXT("King's Greed ended"));
    }, GreedDuration, false);
    
    UE_LOG(LogTemp, Warning, TEXT("King's Greed activated - 30%% movement speed for %f seconds"), GreedDuration);
}

void ABloodreadDragonCharacter::OnKingsGreedHit()
{
    if (!bKingsGreedActive)
        return;
    
    KingsGreedHitCount++;
    
    if (KingsGreedHitCount >= HitsRequiredForBonus && !bDamageBoostActive)
    {
        // Activate damage boost and extend duration
        bDamageBoostActive = true;
        
        // Extend both movement speed and add damage boost for 5 more seconds
        float CurrentSpeed = GetCharacterMovement()->MaxWalkSpeed / (1.0f + MovementSpeedBonus); // Get base speed
        
        // Clear existing timer and set new extended timer
        GetWorldTimerManager().ClearTimer(GreedTimerHandle);
        GetWorldTimerManager().SetTimer(GreedTimerHandle, [this, CurrentSpeed]()
        {
            // End all effects
            bKingsGreedActive = false;
            bDamageBoostActive = false;
            KingsGreedHitCount = 0;
            GetCharacterMovement()->MaxWalkSpeed = CurrentSpeed;
            
            UE_LOG(LogTemp, Warning, TEXT("King's Greed (enhanced) ended"));
        }, 5.0f, false); // 5 more seconds
        
        // Reset hit count for potential stacking
        KingsGreedHitCount = 0;
        
        UE_LOG(LogTemp, Warning, TEXT("King's Greed enhanced! +33%% damage and 5 more seconds"));
    }
}

void ABloodreadDragonCharacter::Attack()
{
    Super::Attack();
    
    // Dragon-specific attack implementation - deals 6 damage (or enhanced), gains 10 mana per hit
    float BaseDamage = 6.0f;
    float FinalDamage = bDamageBoostActive ? BaseDamage * (1.0f + DamageBonus) : BaseDamage;
    
    UE_LOG(LogTemp, Warning, TEXT("Dragon attacks for %f damage!"), FinalDamage);
    
    // Get camera direction for targeting
    FVector CameraLocation;
    FRotator CameraRotation;
    GetActorEyesViewPoint(CameraLocation, CameraRotation);
    FVector ForwardVector = CameraRotation.Vector();
    
    // Line trace for melee attack
    FVector StartLocation = CameraLocation;
    FVector EndLocation = StartLocation + (ForwardVector * 150.0f); // Melee range
    
    FHitResult HitResult;
    FCollisionQueryParams QueryParams;
    QueryParams.AddIgnoredActor(this);
    
    if (GetWorld()->LineTraceSingleByChannel(HitResult, StartLocation, EndLocation, ECC_Pawn, QueryParams))
    {
        if (ABloodreadBaseCharacter* Enemy = Cast<ABloodreadBaseCharacter>(HitResult.GetActor()))
        {
            // Use enhanced damage system with dragon knockback
            FVector KnockbackDirection = (Enemy->GetActorLocation() - GetActorLocation()).GetSafeNormal();
            float KnockbackForce = 500.0f; // Heavy knockback for dragon attack
            Enemy->DealDamageWithKnockback(FinalDamage, KnockbackDirection, KnockbackForce, this);
            
            // Gain 10 mana per hit
            CurrentMana = FMath::Min(CurrentMana + 10, 300); // Cap at reasonable mana limit
            
            // Track hit for King's Greed
            OnKingsGreedHit();
            
            UE_LOG(LogTemp, Warning, TEXT("Dragon hit enemy for %f damage, gained 10 mana (current: %d)"), FinalDamage, CurrentMana);
        }
    }
}

void ABloodreadDragonCharacter::InitializeDragonData()
{
    // Set character class
    CurrentCharacterClass = ECharacterClass::Dragon;
    
    // Initialize dragon stats (based on new specification)
    FCharacterStats DragonStats;
    DragonStats.MaxHealth = 100;   // 100 base health
    DragonStats.Strength = 12;     // Medium strength
    DragonStats.Defense = 8;       // Medium defense
    DragonStats.Speed = 12;        // Medium speed
    DragonStats.Mana = 120;        // Starting mana
    DragonStats.CriticalChance = 0.15f; // Medium crit chance
    
    // Initialize dragon abilities (new system)
    FCharacterAbilityData AscentAbility;
    AscentAbility.Name = "Ascent";
    AscentAbility.Description = "Two-part leap and blitz ability for aerial combat";
    AscentAbility.Type = EAbilityType::Movement;
    AscentAbility.Cooldown = 30.0f;  // 30 second cooldown
    AscentAbility.ManaCost = 100;    // Costs 100 mana
    AscentAbility.Damage = BlitzDamage; // Damage on blitz landing
    AscentAbility.Duration = 0.0f;   // Variable duration based on usage
    
    FCharacterAbilityData KingsGreedAbility;
    KingsGreedAbility.Name = "King's Greed";
    KingsGreedAbility.Description = "Gain movement speed, potential damage boost from consecutive hits";
    KingsGreedAbility.Type = EAbilityType::Buff;
    KingsGreedAbility.Cooldown = 30.0f;  // 30 second cooldown
    KingsGreedAbility.ManaCost = 50;     // Costs 50 mana
    KingsGreedAbility.Damage = 0.0f;     // No direct damage
    KingsGreedAbility.Duration = GreedDuration; // Base 10 seconds
    
    // Set up character class data
    CharacterClassData.CharacterClass = ECharacterClass::Dragon;
    CharacterClassData.ClassName = "Dragon";
    CharacterClassData.Description = "A powerful aerial combatant with leap attacks and greed-based power scaling";
    CharacterClassData.BaseStats = DragonStats;
    CharacterClassData.Ability1 = AscentAbility;
    CharacterClassData.Ability2 = KingsGreedAbility;
    
    // Set dragon mesh (Yin character - mystical dragon-like figure)
    CharacterClassData.CharacterMesh = TSoftObjectPtr<USkeletalMesh>(FSoftObjectPath(TEXT("/Game/ParagonYin/Characters/Heroes/Yin/Meshes/Yin.Yin")));
    
    // Set dragon animation blueprint path (will be loaded later by the system)
    CharacterClassData.AnimationBlueprintPath = TEXT("/Game/ParagonYin/Characters/Heroes/Yin/Yin_AnimBlueprint.Yin_AnimBlueprint_C");
    UE_LOG(LogTemp, Warning, TEXT("Set Yin animation blueprint path: %s"), *CharacterClassData.AnimationBlueprintPath);
    
    // Set dragon attack animation paths (using actual existing animation files)
    CharacterClassData.BasicAttackAnimationPath = TEXT("/Game/ParagonYin/Characters/Heroes/Yin/Animations/Primary_Attack_A_Slow.Primary_Attack_A_Slow");
    CharacterClassData.Ability1AnimationPath = TEXT("/Game/ParagonYin/Characters/Heroes/Yin/Animations/Q_Pull_Kick.Q_Pull_Kick");
    CharacterClassData.Ability2AnimationPath = TEXT("/Game/ParagonYin/Characters/Heroes/Yin/Animations/E_Ability_Attack_A.E_Ability_Attack_A");
    
    // Set camera offset adjusted for Yin character
    CharacterClassData.CameraOffset = FVector(25.0f, 0.0f, 85.0f);
    
    // Apply stats
    CurrentStats = DragonStats;
    CurrentHealth = CurrentStats.MaxHealth;
    CurrentMana = CurrentStats.Mana;
}
