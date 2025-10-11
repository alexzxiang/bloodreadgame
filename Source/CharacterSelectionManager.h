#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "BloodreadBaseCharacter.h"
#include "BloodreadWarriorCharacter.h"
#include "BloodreadMageCharacter.h"
#include "BloodreadRogueCharacter.h"
#include "CharacterSelectionManager.generated.h"

UCLASS(BlueprintType, Blueprintable)
class BLOODREADGAME_API UCharacterSelectionManager : public UObject
{
    GENERATED_BODY()

public:
    UCharacterSelectionManager();

    // Get all available character classes
    UFUNCTION(BlueprintCallable, Category = "Character Selection")
    TArray<FCharacterClassData> GetAvailableCharacterClasses() const;

    // Get character class data by type
    UFUNCTION(BlueprintCallable, Category = "Character Selection")
    FCharacterClassData GetCharacterClassData(ECharacterClass CharacterClass) const;

    // Get character class blueprint
    UFUNCTION(BlueprintCallable, Category = "Character Selection") 
    TSubclassOf<ABloodreadBaseCharacter> GetCharacterClassBlueprint(ECharacterClass CharacterClass) const;

    // Spawn character of specified class
    UFUNCTION(BlueprintCallable, Category = "Character Selection")
    ABloodreadBaseCharacter* SpawnCharacterOfClass(UWorld* World, ECharacterClass CharacterClass, FVector Location, FRotator Rotation) const;

protected:
    // Available character classes data
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Classes")
    TMap<ECharacterClass, FCharacterClassData> CharacterClassesData;

    // Character class blueprints
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Classes")
    TMap<ECharacterClass, TSubclassOf<ABloodreadBaseCharacter>> CharacterClassBlueprints;

private:
    void InitializeCharacterClasses();
    void SetupDefaultCharacterData();
};
