#pragma once

#include "CoreMinimal.h"
#include "BloodreadBaseCharacter.h"
#include "BloodreadHealerCharacter.generated.h"

UCLASS()
class BLOODREADGAME_API ABloodreadHealerCharacter : public ABloodreadBaseCharacter
{
    GENERATED_BODY()

public:
    ABloodreadHealerCharacter();

protected:
    virtual void BeginPlay() override;
    virtual void OnCharacterClassChanged() override;
    public:
    virtual bool OnAbility1Used() override;
    virtual bool OnAbility2Used() override;

    // Healer-specific properties
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Healer")
    float HealRange = 800.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Healer")
    float TeamRegenDuration = 10.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Healer")
    float TeamRegenRate = 2.0f; // 20 health over 10 seconds = 2 per second

public:
    // Healer-specific abilities
    UFUNCTION(BlueprintCallable, Category = "Healer Abilities")
    void Bond();

    UFUNCTION(BlueprintCallable, Category = "Healer Abilities")
    void Regeneration();

    // Override attack for healer-specific combat
    virtual void Attack() override;

    // Healer data initialization
    UFUNCTION(BlueprintCallable, Category = "Healer")
    void InitializeHealerData();

private:
    // Track active team regeneration
    TArray<FTimerHandle> ActiveRegenTimers;
};
