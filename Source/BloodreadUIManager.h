#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"
#include "BloodreadPlayerCharacter.h"
#include "BloodreadBaseCharacter.h"
#include "BloodreadUIManager.generated.h"

/**
 * UI Manager class that provides helper functions for creating game UI
 * This can be used in Blueprints to simplify UI creation and binding
 */
UCLASS(BlueprintType, Blueprintable)
class BLOODREADGAME_API UBloodreadUIManager : public UObject
{
	GENERATED_BODY()

public:
	// Static helper functions for UI binding
	
	/**
	 * Get health percentage for progress bars (0.0 to 1.0)
	 * @param PlayerCharacter The player character to get health from
	 * @return Health percentage as float between 0.0 and 1.0
	 */
	UFUNCTION(BlueprintPure, Category = "UI Helpers")
	static float GetHealthBarProgress(ABloodreadPlayerCharacter* PlayerCharacter);
	
	/**
	 * Get formatted health text for display
	 * @param PlayerCharacter The player character to get health from
	 * @return Formatted text like "75/100"
	 */
	UFUNCTION(BlueprintPure, Category = "UI Helpers")
	static FText GetHealthText(ABloodreadPlayerCharacter* PlayerCharacter);
	
	/**
	 * Get ability cooldown text for display
	 * @param PlayerCharacter The player character
	 * @param AbilityIndex 1 for Ability1, 2 for Ability2
	 * @return Formatted text like "2.5s" or "READY"
	 */
	UFUNCTION(BlueprintPure, Category = "UI Helpers")
	static FText GetAbilityCooldownText(ABloodreadPlayerCharacter* PlayerCharacter, int32 AbilityIndex);
	
	/**
	 * Get ability button tint color based on cooldown status
	 * @param PlayerCharacter The player character
	 * @param AbilityIndex 1 for Ability1, 2 for Ability2
	 * @return White if ready, Gray if on cooldown
	 */
	UFUNCTION(BlueprintPure, Category = "UI Helpers")
	static FLinearColor GetAbilityButtonTint(ABloodreadPlayerCharacter* PlayerCharacter, int32 AbilityIndex);
	
	/**
	 * Get ability cooldown progress for circular progress bars
	 * @param PlayerCharacter The player character
	 * @param AbilityIndex 1 for Ability1, 2 for Ability2
	 * @return Progress from 0.0 (on cooldown) to 1.0 (ready)
	 */
	UFUNCTION(BlueprintPure, Category = "UI Helpers")
	static float GetAbilityCooldownProgress(ABloodreadPlayerCharacter* PlayerCharacter, int32 AbilityIndex);
	
	/**
	 * Create a formatted damage number text for floating damage indicators
	 * @param DamageAmount The amount of damage dealt
	 * @param bWasCritical Whether this was a critical hit
	 * @return Formatted text with appropriate styling
	 */
	UFUNCTION(BlueprintPure, Category = "UI Helpers")
	static FText FormatDamageText(int32 DamageAmount, bool bWasCritical = false);
	
	/**
	 * Get color for damage text based on damage type
	 * @param DamageAmount The damage amount
	 * @param bWasCritical Whether this was critical damage
	 * @return Appropriate color for the damage text
	 */
	UFUNCTION(BlueprintPure, Category = "UI Helpers")
	static FLinearColor GetDamageTextColor(int32 DamageAmount, bool bWasCritical = false);

	// === NEW CHARACTER SYSTEM SUPPORT ===
	// Overloaded functions for BloodreadBaseCharacter (new modular system)
	
	/**
	 * Get health percentage for progress bars (0.0 to 1.0) - New Character System
	 * @param BaseCharacter The base character to get health from
	 * @return Health percentage as float between 0.0 and 1.0
	 */
	UFUNCTION(BlueprintPure, Category = "UI Helpers - New Characters")
	static float GetHealthBarProgressBase(ABloodreadBaseCharacter* BaseCharacter);
	
	/**
	 * Get formatted health text for display - New Character System
	 * @param BaseCharacter The base character to get health from
	 * @return Formatted text like "75/100"
	 */
	UFUNCTION(BlueprintPure, Category = "UI Helpers - New Characters")
	static FText GetHealthTextBase(ABloodreadBaseCharacter* BaseCharacter);
	
	/**
	 * Get character class name for display - New Character System
	 * @param BaseCharacter The base character to get class from
	 * @return Character class name like "Warrior", "Mage", "Rogue"
	 */
	UFUNCTION(BlueprintPure, Category = "UI Helpers - New Characters")
	static FText GetCharacterClassName(ABloodreadBaseCharacter* BaseCharacter);
};