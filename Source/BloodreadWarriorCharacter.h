#pragma once

#include "CoreMinimal.h"
#include "BloodreadBaseCharacter.h"
#include "BloodreadWarriorCharacter.generated.h"

UCLASS()
class BLOODREADGAME_API ABloodreadWarriorCharacter : public ABloodreadBaseCharacter
{
    GENERATED_BODY()

public:
    ABloodreadWarriorCharacter();

protected:
    virtual void BeginPlay() override;
    virtual void OnCharacterClassChanged() override;
    virtual bool OnAbility1Used() override;
    virtual bool OnAbility2Used() override;

    // Warrior-specific properties
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Warrior")
    float ChargeDistance = 500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Warrior")
    float ShieldBlockReduction = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Warrior")
    float HexPunchRange = 200.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Warrior")
    float ShieldHealthBonus = 50.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Warrior")
    float ShieldDuration = 8.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Warrior")
    float ShieldRegenRate = 5.0f;

public:
    // Warrior-specific abilities
    UFUNCTION(BlueprintCallable, Category = "Warrior Abilities")
    void HexPunch();

    UFUNCTION(BlueprintCallable, Category = "Warrior Abilities")
    void PowerShield();

    // Override attack for warrior-specific combat
    virtual void Attack() override;

    // Warrior data initialization
    UFUNCTION(BlueprintCallable, Category = "Warrior")
    void InitializeWarriorData();
};
