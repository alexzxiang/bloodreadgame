#pragma once

#include "CoreMinimal.h"
#include "BloodreadBaseCharacter.h"
#include "BloodreadRogueCharacter.generated.h"

UCLASS()
class BLOODREADGAME_API ABloodreadRogueCharacter : public ABloodreadBaseCharacter  
{
    GENERATED_BODY()

public:
    ABloodreadRogueCharacter();

protected:
    virtual void BeginPlay() override;
    virtual void OnCharacterClassChanged() override;
    public:
    virtual bool OnAbility1Used() override;
    virtual bool OnAbility2Used() override;

    // Rogue-specific properties
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rogue")
    float TeleportRange = 1000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rogue")
    float MovementSpeedBonus = 0.3f; // 30% movement speed

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rogue")
    float SpeedBoostDuration = 5.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rogue")
    float ShadowPushKnockback = 600.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rogue")
    float ShieldDuration = 5.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rogue")
    float ShieldHealth = 20.0f;

public:
    // Rogue-specific abilities
    UFUNCTION(BlueprintCallable, Category = "Rogue Abilities")
    void Teleport();

    UFUNCTION(BlueprintCallable, Category = "Rogue Abilities")
    void ShadowPush();

    // Override attack for rogue-specific combat
    virtual void Attack() override;

    // Movement speed boost tracking
    UPROPERTY()
    FTimerHandle SpeedBoostTimerHandle;

    // Rogue data initialization
    UFUNCTION(BlueprintCallable, Category = "Rogue")
    void InitializeRogueData();

private:
    void EndStealth();
    
    FTimerHandle StealthTimerHandle;
};
