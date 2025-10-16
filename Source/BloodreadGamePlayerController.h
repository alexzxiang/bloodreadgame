// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "BloodreadBaseCharacter.h"
#include "Blueprint/UserWidget.h"
#include "BloodreadGamePlayerController.generated.h"

// Forward declarations
class UBloodreadHealthBarWidget;
class UCharacterSelectionManager;

/**
 *  Simple Player Controller for traditional input system
 *  Uses Blueprint GameMode for Enhanced Input setup
 */
UCLASS()
class BLOODREADGAME_API ABloodreadGamePlayerController : public APlayerController
{
	GENERATED_BODY()
	
public:

	/** Constructor */
	ABloodreadGamePlayerController();

	/** Initialize health bar widget */
	UFUNCTION(BlueprintCallable, Category = "UI")
	void InitializeHealthBarWidget(ABloodreadBaseCharacter* PlayerCharacter);

	/** Hide character selection widget (can be called from Blueprint) */
	UFUNCTION(BlueprintCallable, Category = "UI")
	void HideCharacterSelectionWidget();

	/** Request character selection (multiplayer-safe) - call this from UI */
	UFUNCTION(BlueprintCallable, Category = "Character Selection")
	void RequestCharacterSelection(int32 CharacterClassIndex);

	/** Server RPC to handle character selection */
	UFUNCTION(Server, Reliable, Category = "Character Selection")
	void ServerRequestCharacterSelection(int32 CharacterClassIndex);

	/** Client RPC to notify client of successful character possession */
	UFUNCTION(Client, Reliable, Category = "Character Selection")
	void ClientNotifyCharacterPossession(APawn* NewCharacter);

	/** Server RPC to request possession of a specific character */
	UFUNCTION(Server, Reliable, Category = "Character Selection")
	void ServerRequestPossession(APawn* CharacterToPossess);

protected:

	/** Gameplay initialization */
	virtual void BeginPlay() override;

	/** Called when possessing a pawn */
	virtual void OnPossess(APawn* InPawn) override;

	/** Perform possession operations (UI setup, input, etc.) */
	UFUNCTION(BlueprintCallable, Category = "Character")
	void PerformPossessionOperations(APawn* InPawn);

	/** Initialize character mesh and systems */
	UFUNCTION(BlueprintCallable, Category = "Character")
	void InitializeCharacterMesh(ABloodreadBaseCharacter* InCharacter);

	/** Attach camera to character head bone */
	UFUNCTION(BlueprintCallable, Category = "Character")
	void AttachCameraToHeadBone(ABloodreadBaseCharacter* PlayerCharacter, USkeletalMeshComponent* MeshComponent);

	/** Initialize player UI components */
	UFUNCTION(BlueprintCallable, Category = "UI")
	void InitializePlayerUI();

	/** Initialize character selection widget */
	UFUNCTION(BlueprintCallable, Category = "UI")
	void InitializeCharacterSelectionWidget();

	/** Force show character selection widget (can be called from Blueprint) */
	UFUNCTION(BlueprintCallable, Category = "UI")
	void ForceShowCharacterSelection();

	/** Force hide character selection and switch to game mode (called when character is selected) */
	UFUNCTION(BlueprintCallable, Category = "UI")
	void OnCharacterSelected();

protected:
	// Character selection widget reference
	UPROPERTY(BlueprintReadWrite, Category = "UI")
	UUserWidget* CharacterSelectionWidget;

	// Character selection widget class
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI")
	TSubclassOf<UUserWidget> CharacterSelectionWidgetClass;

	// Health bar widget reference
	UPROPERTY(BlueprintReadWrite, Category = "UI")
	UBloodreadHealthBarWidget* HealthBarWidget;

	// Health bar widget class
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI")
	TSubclassOf<UBloodreadHealthBarWidget> HealthBarWidgetClass;

	// Character Selection Manager
	UPROPERTY(BlueprintReadWrite, Category = "Character Selection")
	UCharacterSelectionManager* CharacterSelectionManager;

private:
	// Timer handle for checking possession after RPC
	FTimerHandle CheckPossessionTimerHandle;

};