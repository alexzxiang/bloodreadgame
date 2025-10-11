// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "BloodreadBaseCharacter.h"
#include "Blueprint/UserWidget.h"
#include "MultiplayerLobbyWidget.h"
#include "BloodreadGamePlayerController.generated.h"

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

	/** Show multiplayer lobby on game start */
	UFUNCTION(BlueprintCallable, Category = "Multiplayer")
	void ShowMultiplayerLobby();

	/** Hide multiplayer lobby */
	UFUNCTION(BlueprintCallable, Category = "Multiplayer")
	void HideMultiplayerLobby();

protected:

	/** Gameplay initialization */
	virtual void BeginPlay() override;

	/** Called when possessing a pawn */
	virtual void OnPossess(APawn* InPawn) override;

	/** Initialize character mesh and systems */
	UFUNCTION(BlueprintCallable, Category = "Character")
	void InitializeCharacterMesh(ABloodreadBaseCharacter* InCharacter);

	/** Create and setup all UI widgets */
	void CreateUIWidgets();

private:
	/** Multiplayer lobby widget reference */
	UPROPERTY()
	UMultiplayerLobbyWidget* MultiplayerLobbyWidget;

	/** Main menu widget reference */
	UPROPERTY()
	TObjectPtr<UUserWidget> WB_MainMenu;

	/** Create server widget reference */
	UPROPERTY()
	TObjectPtr<UUserWidget> WB_CreateServer;

	/** Server browser widget reference */
	UPROPERTY()
	TObjectPtr<UUserWidget> WB_ServerBrowser;

	/** Widget classes loaded from Blueprint assets */
	UPROPERTY()
	TSubclassOf<UUserWidget> MainMenuWidgetClass;

	UPROPERTY()
	TSubclassOf<UUserWidget> CreateServerWidgetClass;

	UPROPERTY()
	TSubclassOf<UUserWidget> ServerBrowserWidgetClass;

	/** Widget class to use for multiplayer lobby (set in Blueprint) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "UI", meta = (AllowPrivateAccess = "true"))
	TSubclassOf<UMultiplayerLobbyWidget> MultiplayerLobbyWidgetClass;

};
