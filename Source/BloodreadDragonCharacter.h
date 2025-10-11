#pragma once

#include "CoreMinimal.h"
#include "BloodreadBaseCharacter.h"
#include "BloodreadDragonCharacter.generated.h"

UCLASS()
class BLOODREADGAME_API ABloodreadDragonCharacter : public ABloodreadBaseCharacter
{
    GENERATED_BODY()

public:
    ABloodreadDragonCharacter();

protected:
    virtual void BeginPlay() override;
    virtual void OnCharacterClassChanged() override;
    virtual bool OnAbility1Used() override;
    virtual bool OnAbility2Used() override;

public:
    virtual void UseAbility1() override;

    // Dragon-specific properties
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dragon")
    float AscentLaunchForce = 2200.0f; // Much stronger for mega jump that lasts 2-3 seconds

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dragon")
    float BlitzDownwardForce = 4000.0f; // Very fast gravity-biased ground slam

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dragon")
    float BlitzDamage = 7.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dragon")
    float BlitzRecoilDamage = 7.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dragon")
    float BlitzRadius = 200.0f; // 2 meters

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dragon")
    float MovementSpeedBonus = 0.3f; // 30% movement speed

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dragon")
    float DamageBonus = 0.33f; // 33% damage increase

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dragon")
    float GreedDuration = 10.0f; // Base duration

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dragon")
    int32 HitsRequiredForBonus = 5;

    // Ascent ability state tracking
    UPROPERTY()
    bool bAscentFirstPress = false;

    UPROPERTY()
    bool bIsInAir = false;

    // King's Greed tracking
    UPROPERTY()
    bool bKingsGreedActive = false;

    UPROPERTY()
    int32 KingsGreedHitCount = 0;

    UPROPERTY()
    bool bDamageBoostActive = false;

    UPROPERTY()
    FTimerHandle GreedTimerHandle;

    UPROPERTY()
    FTimerHandle DamageBoostTimerHandle;

    UPROPERTY()
    FTimerHandle AscentResetTimerHandle;

public:
    // Dragon-specific abilities
    UFUNCTION(BlueprintCallable, Category = "Dragon Abilities")
    void Ascent();

    UFUNCTION(BlueprintCallable, Category = "Dragon Abilities")
    void KingsGreed();

    // Override attack for dragon-specific combat
    virtual void Attack() override;

    // Dragon data initialization
    UFUNCTION(BlueprintCallable, Category = "Dragon")
    void InitializeDragonData();

    // Override landed to handle ascent state management
    virtual void Landed(const FHitResult& Hit) override;

private:
    void PerformAscentLeap();
    void PerformBlitzDown();
    void OnKingsGreedHit();
};
