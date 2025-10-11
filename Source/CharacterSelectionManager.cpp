#include "CharacterSelectionManager.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "BloodreadHealerCharacter.h"
#include "BloodreadDragonCharacter.h"

UCharacterSelectionManager::UCharacterSelectionManager()
{
    InitializeCharacterClasses();
}

TArray<FCharacterClassData> UCharacterSelectionManager::GetAvailableCharacterClasses() const
{
    TArray<FCharacterClassData> AvailableClasses;
    
    for (const auto& ClassPair : CharacterClassesData)
    {
        AvailableClasses.Add(ClassPair.Value);
    }
    
    return AvailableClasses;
}

FCharacterClassData UCharacterSelectionManager::GetCharacterClassData(ECharacterClass CharacterClass) const
{
    const FCharacterClassData* FoundData = CharacterClassesData.Find(CharacterClass);
    return FoundData ? *FoundData : FCharacterClassData();
}

TSubclassOf<ABloodreadBaseCharacter> UCharacterSelectionManager::GetCharacterClassBlueprint(ECharacterClass CharacterClass) const
{
    const TSubclassOf<ABloodreadBaseCharacter>* FoundBlueprint = CharacterClassBlueprints.Find(CharacterClass);
    return FoundBlueprint ? *FoundBlueprint : nullptr;
}

ABloodreadBaseCharacter* UCharacterSelectionManager::SpawnCharacterOfClass(UWorld* World, ECharacterClass CharacterClass, FVector Location, FRotator Rotation) const
{
    if (!World)
    {
        UE_LOG(LogTemp, Error, TEXT("World is null in SpawnCharacterOfClass"));
        return nullptr;
    }

    // Get the character class to spawn
    TSubclassOf<ABloodreadBaseCharacter> CharacterBlueprint = GetCharacterClassBlueprint(CharacterClass);
    
    if (!CharacterBlueprint)
    {
        // Fallback to C++ classes if no blueprint is set
        switch (CharacterClass)
        {
            case ECharacterClass::Warrior:
                CharacterBlueprint = ABloodreadWarriorCharacter::StaticClass();
                break;
            case ECharacterClass::Mage:
                CharacterBlueprint = ABloodreadMageCharacter::StaticClass();
                break;
            case ECharacterClass::Rogue:
                CharacterBlueprint = ABloodreadRogueCharacter::StaticClass();
                break;
            case ECharacterClass::Healer:
                CharacterBlueprint = ABloodreadHealerCharacter::StaticClass();
                break;
            case ECharacterClass::Dragon:
                CharacterBlueprint = ABloodreadDragonCharacter::StaticClass();
                break;
            default:
                CharacterBlueprint = ABloodreadBaseCharacter::StaticClass();
                break;
        }
    }

    if (CharacterBlueprint)
    {
        FActorSpawnParameters SpawnParams;
        SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
        
        ABloodreadBaseCharacter* SpawnedCharacter = World->SpawnActor<ABloodreadBaseCharacter>(CharacterBlueprint, Location, Rotation, SpawnParams);
        
        if (SpawnedCharacter)
        {
            // Initialize the character with class data
            FCharacterClassData ClassData = GetCharacterClassData(CharacterClass);
            SpawnedCharacter->InitializeFromClassData(ClassData);
            
            // CRITICAL: Verify physics and movement settings for knockback
            UCharacterMovementComponent* MovementComp = SpawnedCharacter->GetCharacterMovement();
            if (MovementComp)
            {
                UE_LOG(LogTemp, Error, TEXT("🚨 SPAWNED CHARACTER PHYSICS CHECK: %s"), *SpawnedCharacter->GetName());
                UE_LOG(LogTemp, Error, TEXT("🚨 Mass: %.2f, CanWalkOffLedges: %s"), 
                       MovementComp->Mass, MovementComp->bCanWalkOffLedges ? TEXT("YES") : TEXT("NO"));
                UE_LOG(LogTemp, Error, TEXT("🚨 ImpartVelocity X/Y/Z: %s/%s/%s"), 
                       MovementComp->bImpartBaseVelocityX ? TEXT("YES") : TEXT("NO"),
                       MovementComp->bImpartBaseVelocityY ? TEXT("YES") : TEXT("NO"),
                       MovementComp->bImpartBaseVelocityZ ? TEXT("YES") : TEXT("NO"));
                UE_LOG(LogTemp, Error, TEXT("🚨 Controller: %s"), 
                       SpawnedCharacter->GetController() ? *SpawnedCharacter->GetController()->GetName() : TEXT("NONE"));
            }
            
            // FORCE enable knockback physics on spawned character
            SpawnedCharacter->ForceEnableKnockbackPhysics();
            
            UE_LOG(LogTemp, Warning, TEXT("Spawned character of class: %s"), *ClassData.ClassName);
            return SpawnedCharacter;
        }
    }

    UE_LOG(LogTemp, Error, TEXT("Failed to spawn character of class: %d"), (int32)CharacterClass);
    return nullptr;
}

void UCharacterSelectionManager::InitializeCharacterClasses()
{
    SetupDefaultCharacterData();
    
    // Set default C++ class blueprints
    CharacterClassBlueprints.Add(ECharacterClass::Warrior, ABloodreadWarriorCharacter::StaticClass());
    CharacterClassBlueprints.Add(ECharacterClass::Mage, ABloodreadMageCharacter::StaticClass());
    CharacterClassBlueprints.Add(ECharacterClass::Rogue, ABloodreadRogueCharacter::StaticClass());
}

void UCharacterSelectionManager::SetupDefaultCharacterData()
{
    // Warrior data
    FCharacterClassData WarriorData;
    WarriorData.CharacterClass = ECharacterClass::Warrior;
    WarriorData.ClassName = "Warrior";
    WarriorData.Description = "A mighty melee fighter with high health and devastating close-combat abilities";
    
    WarriorData.BaseStats.MaxHealth = 150;
    WarriorData.BaseStats.Strength = 18;
    WarriorData.BaseStats.Defense = 12;
    WarriorData.BaseStats.Speed = 8;
    WarriorData.BaseStats.Mana = 30;
    WarriorData.BaseStats.CriticalChance = 0.15f;
    
    WarriorData.Ability1.Name = "Power Strike";
    WarriorData.Ability1.Description = "A devastating melee attack that deals massive damage and knockback";
    WarriorData.Ability1.Type = EAbilityType::Damage;
    WarriorData.Ability1.Cooldown = 8.0f;
    WarriorData.Ability1.ManaCost = 15;
    WarriorData.Ability1.Damage = 60.0f;
    
    WarriorData.Ability2.Name = "Charge Attack";
    WarriorData.Ability2.Description = "Rush forward, dealing damage to all enemies in your path";
    WarriorData.Ability2.Type = EAbilityType::Movement;
    WarriorData.Ability2.Cooldown = 12.0f;
    WarriorData.Ability2.ManaCost = 20;
    WarriorData.Ability2.Damage = 40.0f;
    
    CharacterClassesData.Add(ECharacterClass::Warrior, WarriorData);

    // Mage data
    FCharacterClassData MageData;
    MageData.CharacterClass = ECharacterClass::Mage;
    MageData.ClassName = "Mage";
    MageData.Description = "A powerful spellcaster with high mana and devastating ranged magical abilities";
    
    MageData.BaseStats.MaxHealth = 80;
    MageData.BaseStats.Strength = 6;
    MageData.BaseStats.Defense = 4;
    MageData.BaseStats.Speed = 12;
    MageData.BaseStats.Mana = 120;
    MageData.BaseStats.CriticalChance = 0.2f;
    
    MageData.Ability1.Name = "Fireball";
    MageData.Ability1.Description = "Launch a fiery projectile that explodes on impact, dealing area damage";
    MageData.Ability1.Type = EAbilityType::Damage;
    MageData.Ability1.Cooldown = 5.0f;
    MageData.Ability1.ManaCost = 25;
    MageData.Ability1.Damage = 45.0f;
    
    MageData.Ability2.Name = "Teleport";
    MageData.Ability2.Description = "Instantly teleport a short distance forward";
    MageData.Ability2.Type = EAbilityType::Movement;
    MageData.Ability2.Cooldown = 10.0f;
    MageData.Ability2.ManaCost = 30;
    MageData.Ability2.Damage = 0.0f;
    
    CharacterClassesData.Add(ECharacterClass::Mage, MageData);

    // Rogue data
    FCharacterClassData RogueData;
    RogueData.CharacterClass = ECharacterClass::Rogue;
    RogueData.ClassName = "Rogue";
    RogueData.Description = "A swift assassin specializing in stealth, critical strikes, and hit-and-run tactics";
    
    RogueData.BaseStats.MaxHealth = 90;
    RogueData.BaseStats.Strength = 12;
    RogueData.BaseStats.Defense = 6;
    RogueData.BaseStats.Speed = 16;
    RogueData.BaseStats.Mana = 60;
    RogueData.BaseStats.CriticalChance = 0.3f;
    
    RogueData.Ability1.Name = "Shadow Strike";
    RogueData.Ability1.Description = "Dash forward and strike with deadly precision, dealing extra damage from stealth";
    RogueData.Ability1.Type = EAbilityType::Damage;
    RogueData.Ability1.Cooldown = 6.0f;
    RogueData.Ability1.ManaCost = 20;
    RogueData.Ability1.Damage = 35.0f;
    
    RogueData.Ability2.Name = "Stealth";
    RogueData.Ability2.Description = "Become invisible and gain increased movement speed for a short duration";
    RogueData.Ability2.Type = EAbilityType::Buff;
    RogueData.Ability2.Cooldown = 15.0f;
    RogueData.Ability2.ManaCost = 25;
    RogueData.Ability2.Damage = 0.0f;
    
    CharacterClassesData.Add(ECharacterClass::Rogue, RogueData);
}
