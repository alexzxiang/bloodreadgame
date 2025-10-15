#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "BloodreadBaseCharacter.h"
#include "CharacterSelectionManager.h"
#include "Blueprint/UserWidget.h"
#include "Components/Widget.h"
#include "BloodreadGameMode.generated.h"

UCLASS()
class BLOODREADGAME_API ABloodreadGameMode : public AGameModeBase
{
    GENERATED_BODY()

public:
    ABloodreadGameMode();

    virtual void BeginPlay() override;
    
    // Override spawn to use character selection system
    virtual APawn* SpawnDefaultPawnFor_Implementation(AController* NewPlayer, AActor* StartSpot) override;


    // Character Selection System
    UFUNCTION(BlueprintCallable, Category="Character Selection")
    void ShowCharacterSelectionWidget();

    UFUNCTION(BlueprintCallable, Category="Character Selection")
    void HideCharacterSelectionWidget();

    UFUNCTION(BlueprintCallable, Category="HUD Management")
    void ShowPlayerHUD();

    UFUNCTION(BlueprintCallable, Category="HUD Management")
    void HidePlayerHUD();

    UFUNCTION(BlueprintCallable, Category="Character Selection")
    void HandleCharacterSelection(int32 CharacterClassIndex, APlayerController* PlayerController = nullptr);
    
    UFUNCTION(BlueprintCallable, Category="Character Selection") 
    void OnCharacterSelected(int32 CharacterClassIndex);
    
    UFUNCTION(BlueprintCallable, Category="HUD Management")
    void InitializePlayerUI(APlayerController* PlayerController);
    
    UFUNCTION(BlueprintCallable, Category="Character Selection")
    void SpawnSelectedCharacter(ECharacterClass SelectedClass, APlayerController* PlayerController);
    
    // New function for spawning character after UI selection
    UFUNCTION(BlueprintCallable, Category="Character Selection")
    void SpawnCharacterForPlayer(int32 CharacterClassIndex, APlayerController* PlayerController = nullptr);

    UFUNCTION(BlueprintCallable, Category="Character Selection")
    ABloodreadBaseCharacter* CreateCharacterOfClass(ECharacterClass CharacterClass, FVector SpawnLocation, FRotator SpawnRotation);

    UPROPERTY(BlueprintReadWrite, Category="Character Selection")
    class UUserWidget* CharacterSelectionWidget;

    // HUD widget reference for management
    UPROPERTY(BlueprintReadWrite, Category="HUD Management")
    class UUserWidget* PlayerHUDWidget;

protected:
    // Character Selection Manager
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Character Selection")
    UCharacterSelectionManager* CharacterSelectionManager;

    // Widget class reference for character selection
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Character Selection")
    TSubclassOf<UUserWidget> CharacterSelectionWidgetClass;

    // Timer handle for character possession
    FTimerHandle PossessionTimerHandle;

    // Helper function for spawning characters with proper timing
    void SpawnNewCharacter(ECharacterClass SelectedClass, APlayerController* PlayerController, FVector SpawnLocation, FRotator SpawnRotation);
};