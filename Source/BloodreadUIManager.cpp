#include "BloodreadUIManager.h"
#include "Engine/Engine.h"

float UBloodreadUIManager::GetHealthBarProgress(ABloodreadPlayerCharacter* PlayerCharacter)
{
    if (!PlayerCharacter)
    {
        return 0.0f;
    }
    
    return PlayerCharacter->GetHealthPercentage();
}

FText UBloodreadUIManager::GetHealthText(ABloodreadPlayerCharacter* PlayerCharacter)
{
    if (!PlayerCharacter)
    {
        return FText::FromString("0/0");
    }
    
    int32 CurrentHealth = PlayerCharacter->GetCurrentHealth();
    int32 MaxHealth = PlayerCharacter->GetMaxHealth();
    
    return FText::FromString(FString::Printf(TEXT("%d/%d"), CurrentHealth, MaxHealth));
}

FText UBloodreadUIManager::GetAbilityCooldownText(ABloodreadPlayerCharacter* PlayerCharacter, int32 AbilityIndex)
{
    if (!PlayerCharacter)
    {
        return FText::FromString("N/A");
    }
    
    float CooldownRemaining = 0.0f;
    
    switch (AbilityIndex)
    {
        case 1:
            if (PlayerCharacter->CanUseAbility1())
            {
                return FText::FromString("READY");
            }
            CooldownRemaining = PlayerCharacter->GetAbility1CooldownRemaining();
            break;
            
        case 2:
            if (PlayerCharacter->CanUseAbility2())
            {
                return FText::FromString("READY");
            }
            CooldownRemaining = PlayerCharacter->GetAbility2CooldownRemaining();
            break;
            
        default:
            return FText::FromString("N/A");
    }
    
    if (CooldownRemaining <= 0.0f)
    {
        return FText::FromString("READY");
    }
    
    // Format to 1 decimal place
    return FText::FromString(FString::Printf(TEXT("%.1fs"), CooldownRemaining));
}

FLinearColor UBloodreadUIManager::GetAbilityButtonTint(ABloodreadPlayerCharacter* PlayerCharacter, int32 AbilityIndex)
{
    if (!PlayerCharacter)
    {
        return FLinearColor::Gray;
    }
    
    bool bCanUse = false;
    
    switch (AbilityIndex)
    {
        case 1:
            bCanUse = PlayerCharacter->CanUseAbility1();
            break;
        case 2:
            bCanUse = PlayerCharacter->CanUseAbility2();
            break;
        default:
            return FLinearColor::Gray;
    }
    
    return bCanUse ? FLinearColor::White : FLinearColor(0.5f, 0.5f, 0.5f, 0.7f);
}

float UBloodreadUIManager::GetAbilityCooldownProgress(ABloodreadPlayerCharacter* PlayerCharacter, int32 AbilityIndex)
{
    if (!PlayerCharacter)
    {
        return 0.0f;
    }
    
    float CooldownPercentage = 0.0f;
    
    switch (AbilityIndex)
    {
        case 1:
            CooldownPercentage = PlayerCharacter->GetAbility1CooldownPercentage();
            break;
        case 2:
            CooldownPercentage = PlayerCharacter->GetAbility2CooldownPercentage();
            break;
        default:
            return 0.0f;
    }
    
    // Return inverted percentage so bar fills up as cooldown finishes
    return 1.0f - CooldownPercentage;
}

FText UBloodreadUIManager::FormatDamageText(int32 DamageAmount, bool bWasCritical)
{
    if (bWasCritical)
    {
        return FText::FromString(FString::Printf(TEXT("-%d!"), DamageAmount));
    }
    else
    {
        return FText::FromString(FString::Printf(TEXT("-%d"), DamageAmount));
    }
}

FLinearColor UBloodreadUIManager::GetDamageTextColor(int32 DamageAmount, bool bWasCritical)
{
    if (bWasCritical)
    {
        // Bright yellow for critical hits
        return FLinearColor(1.0f, 1.0f, 0.0f, 1.0f);
    }
    else if (DamageAmount >= 50)
    {
        // Red for high damage
        return FLinearColor(1.0f, 0.2f, 0.2f, 1.0f);
    }
    else if (DamageAmount >= 25)
    {
        // Orange for medium damage
        return FLinearColor(1.0f, 0.6f, 0.2f, 1.0f);
    }
    else
    {
        // White for low damage
        return FLinearColor(1.0f, 1.0f, 1.0f, 1.0f);
    }
}

// === NEW CHARACTER SYSTEM IMPLEMENTATIONS ===

float UBloodreadUIManager::GetHealthBarProgressBase(ABloodreadBaseCharacter* BaseCharacter)
{
    if (!BaseCharacter)
    {
        return 0.0f;
    }
    
    return BaseCharacter->GetHealthPercent();
}

FText UBloodreadUIManager::GetHealthTextBase(ABloodreadBaseCharacter* BaseCharacter)
{
    if (!BaseCharacter)
    {
        return FText::FromString("0/0");
    }
    
    float CurrentHealth = BaseCharacter->GetCurrentHealthFloat();
    float MaxHealth = BaseCharacter->GetMaxHealthFloat();
    
    return FText::FromString(FString::Printf(TEXT("%.0f/%.0f"), CurrentHealth, MaxHealth));
}

FText UBloodreadUIManager::GetCharacterClassName(ABloodreadBaseCharacter* BaseCharacter)
{
    if (!BaseCharacter)
    {
        return FText::FromString("Unknown");
    }
    
    // Get the character class from the current stats
    switch (BaseCharacter->GetCharacterClass())
    {
        case ECharacterClass::Warrior:
            return FText::FromString("Warrior");
        case ECharacterClass::Mage:
            return FText::FromString("Mage");
        case ECharacterClass::Rogue:
            return FText::FromString("Rogue");
        default:
            return FText::FromString("Unknown");
    }
}