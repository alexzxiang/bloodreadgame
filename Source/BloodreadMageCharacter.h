#pragma once

#include "CoreMinimal.h"
#include "BloodreadBaseCharacter.h"
#include "BloodreadMageCharacter.generated.h"

UCLASS()
class BLOODREADGAME_API ABloodreadMageCharacter : public ABloodreadBaseCharacter
{
    GENERATED_BODY()

public:
    ABloodreadMageCharacter();

protected:
    virtual void BeginPlay() override;
    virtual void OnCharacterClassChanged() override;
    public:
    virtual bool OnAbility1Used() override;
    virtual bool OnAbility2Used() override;
    virtual void Tick(float DeltaTime) override;

    // Mage-specific properties
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mage")
    float AuraRadius = 500.0f; // 5 meters

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mage")
    float AuraDamageInterval = 0.5f; // Damage every 0.5 seconds

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mage")
    float ExplosionRadius = 300.0f; // 3 meters

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mage")
    float ExplosionKnockback = 800.0f;

    // Active aura tracking
    UPROPERTY()
    TArray<FTimerHandle> ActiveAuraTimers;

public:
    // Mage-specific abilities
    UFUNCTION(BlueprintCallable, Category = "Mage Abilities")
    void FieryAura();

    UFUNCTION(BlueprintCallable, Category = "Mage Abilities")
    void Explosion();

    // Override attack for mage-specific combat
    virtual void Attack() override;

    // Mage data initialization
    UFUNCTION(BlueprintCallable, Category = "Mage")
    void InitializeMageData();

private:
    void RegenerateMana(float DeltaTime);
};
