// Copyright Epic Games, Inc. All Rights Reserved.




#include "BloodreadGamePlayerController.h"
#include "Blueprint/UserWidget.h"
#include "Engine/Engine.h"
#include "Engine/AssetManager.h"
#include "UObject/ConstructorHelpers.h"
#include "BloodreadHealthBarWidget.h"
#include "CharacterSelectionManager.h"
#include "BloodreadGameMode.h"
#include "Camera/CameraComponent.h"
#include "Components/SkeletalMeshComponent.h"


ABloodreadGamePlayerController::ABloodreadGamePlayerController()
{
   UE_LOG(LogTemp, Warning, TEXT("BloodreadGamePlayerController constructor - initializing for Blueprint-based Steam multiplayer"));

   // Initialize Character Selection Manager
   CharacterSelectionManager = CreateDefaultSubobject<UCharacterSelectionManager>(TEXT("CharacterSelectionManager"));

   // Set default health bar widget class to prevent "HealthBarWidgetClass not set" error
   HealthBarWidgetClass = UBloodreadHealthBarWidget::StaticClass();
   UE_LOG(LogTemp, Warning, TEXT("BloodreadGamePlayerController: Set default HealthBarWidgetClass to UBloodreadHealthBarWidget"));
   
   // IMPORTANT: You should set this to your Blueprint widget class in the editor!
   // The line above sets it to the C++ class, but you want to use your Blueprint widget
   // Go to your PlayerController Blueprint and set the "Health Bar Widget Class" to your WBP_HealthBar

}


void ABloodreadGamePlayerController::BeginPlay()
{
   Super::BeginPlay();

   UE_LOG(LogTemp, Warning, TEXT("BloodreadGamePlayerController BeginPlay - Setting up Blueprint-based Steam multiplayer UI"));
   
   // Check if this is the local controller
   if (IsLocalController())
   {
       UE_LOG(LogTemp, Warning, TEXT("Local controller detected - initializing character selection UI"));
       
       // Add a small delay to ensure everything is properly initialized
       GetWorld()->GetTimerManager().SetTimerForNextTick([this]()
       {
           InitializeCharacterSelectionWidget();
       });
   }
   else
   {
       UE_LOG(LogTemp, Warning, TEXT("Non-local controller - skipping UI creation"));
   }
}


void ABloodreadGamePlayerController::OnPossess(APawn* InPawn)
{
   Super::OnPossess(InPawn);

   UE_LOG(LogTemp, Warning, TEXT("=== PlayerController::OnPossess START ==="));
   UE_LOG(LogTemp, Warning, TEXT("PlayerController possessing pawn: %s"), InPawn ? *InPawn->GetName() : TEXT("NULL"));
  
   // Hide character selection widget immediately when possessing any pawn
   if (CharacterSelectionWidget && CharacterSelectionWidget->IsInViewport())
   {
       UE_LOG(LogTemp, Warning, TEXT("OnPossess: Hiding character selection widget immediately"));
       HideCharacterSelectionWidget();
   }
   
   if (InPawn)
   {
       UE_LOG(LogTemp, Warning, TEXT("Pawn class: %s"), *InPawn->GetClass()->GetName());
      
       // Try to cast to our base character and initialize mesh
       if (ABloodreadBaseCharacter* BloodreadCharacter = Cast<ABloodreadBaseCharacter>(InPawn))
       {
           UE_LOG(LogTemp, Warning, TEXT("SUCCESS: Cast to BloodreadBaseCharacter %s - Current Class: %d"),
                  *BloodreadCharacter->GetName(), (int32)BloodreadCharacter->GetCharacterClass());
           InitializeCharacterMesh(BloodreadCharacter);
           
           // Initialize health bar widget for the possessed character
           InitializeHealthBarWidget(BloodreadCharacter);
       }
       else
       {
           UE_LOG(LogTemp, Error, TEXT("FAILED: Possessed pawn is not a BloodreadBaseCharacter - Class: %s"), *InPawn->GetClass()->GetName());
       }
   }
   else
   {
       UE_LOG(LogTemp, Error, TEXT("InPawn is NULL!"));
   }
  
   UE_LOG(LogTemp, Warning, TEXT("=== PlayerController::OnPossess END ==="));
}


void ABloodreadGamePlayerController::InitializeCharacterMesh(ABloodreadBaseCharacter* InCharacter)
{
   if (!InCharacter)
   {
       UE_LOG(LogTemp, Error, TEXT("InitializeCharacterMesh: InCharacter is null"));
       return;
   }


   UE_LOG(LogTemp, Warning, TEXT("InitializeCharacterMesh: Starting mesh initialization for %s"), *InCharacter->GetName());


   // Force the character to initialize its class data
   InCharacter->ForceInitializeCharacterSystems();


   // Try to find CharacterMesh0 component (commonly used in Blueprints)
   if (USkeletalMeshComponent* CharacterMesh0 = InCharacter->FindComponentByClass<USkeletalMeshComponent>())
   {
       UE_LOG(LogTemp, Warning, TEXT("Found SkeletalMeshComponent, applying class-based mesh"));
      
       ECharacterClass CharacterClass = InCharacter->GetCharacterClass();
       if (InCharacter->ApplyClassDataToMeshComponent(CharacterMesh0, CharacterClass))
       {
           UE_LOG(LogTemp, Warning, TEXT("SUCCESS: Mesh applied to CharacterMesh0 component"));
       }
       else
       {
           UE_LOG(LogTemp, Error, TEXT("FAILED: Could not apply mesh to CharacterMesh0 component"));
       }
   }
   else
   {
       UE_LOG(LogTemp, Warning, TEXT("No CharacterMesh0 found, using default character mesh"));
      
       // Fallback to the default character mesh
       ECharacterClass CharacterClass = InCharacter->GetCharacterClass();
       if (InCharacter->ApplyClassDataToMeshComponent(InCharacter->GetMesh(), CharacterClass))
       {
           UE_LOG(LogTemp, Warning, TEXT("SUCCESS: Mesh applied to default character mesh"));
       }
       else
       {
           UE_LOG(LogTemp, Error, TEXT("FAILED: Could not apply mesh to default character mesh"));
       }
   }
}

void ABloodreadGamePlayerController::InitializePlayerUI()
{
   UE_LOG(LogTemp, Warning, TEXT("InitializePlayerUI: Setting up player UI components"));
   
   // Only initialize UI for local players
   if (!IsLocalController())
   {
       UE_LOG(LogTemp, Warning, TEXT("InitializePlayerUI: Not a local controller, skipping UI initialization"));
       return;
   }
   
   // Additional UI initialization can be added here
   UE_LOG(LogTemp, Warning, TEXT("InitializePlayerUI: Player UI initialization complete"));
}

void ABloodreadGamePlayerController::InitializeCharacterSelectionWidget()
{
   UE_LOG(LogTemp, Warning, TEXT("=== InitializeCharacterSelectionWidget START ==="));
   
   // Only create UI for local players
   if (!IsLocalController())
   {
       UE_LOG(LogTemp, Warning, TEXT("InitializeCharacterSelectionWidget: Not a local controller, skipping character selection widget creation"));
       return;
   }
   
   UE_LOG(LogTemp, Warning, TEXT("InitializeCharacterSelectionWidget: This is a local controller"));
   UE_LOG(LogTemp, Warning, TEXT("InitializeCharacterSelectionWidget: CharacterSelectionWidgetClass is %s"), 
          CharacterSelectionWidgetClass ? *CharacterSelectionWidgetClass->GetName() : TEXT("NULL"));
   
   // Check if widget already exists
   if (CharacterSelectionWidget && CharacterSelectionWidget->IsInViewport())
   {
       UE_LOG(LogTemp, Warning, TEXT("InitializeCharacterSelectionWidget: Character selection widget already exists and is in viewport"));
       return;
   }
   
   // Create character selection widget if we have a class set
   if (CharacterSelectionWidgetClass)
   {
       UE_LOG(LogTemp, Warning, TEXT("InitializeCharacterSelectionWidget: Creating character selection widget from class: %s"), 
              *CharacterSelectionWidgetClass->GetName());
       
       CharacterSelectionWidget = CreateWidget<UUserWidget>(this, CharacterSelectionWidgetClass);
       if (CharacterSelectionWidget)
       {
           UE_LOG(LogTemp, Warning, TEXT("InitializeCharacterSelectionWidget: Widget created successfully"));
           
           // Add to viewport
           CharacterSelectionWidget->AddToViewport();
           UE_LOG(LogTemp, Warning, TEXT("InitializeCharacterSelectionWidget: Widget added to viewport"));
           
           // Verify it's in viewport
           if (CharacterSelectionWidget->IsInViewport())
           {
               UE_LOG(LogTemp, Warning, TEXT("InitializeCharacterSelectionWidget: CONFIRMED - Widget is in viewport"));
           }
           else
           {
               UE_LOG(LogTemp, Error, TEXT("InitializeCharacterSelectionWidget: ERROR - Widget not in viewport after AddToViewport()"));
           }
           
           // Set input mode to UI only with proper focus
           FInputModeUIOnly InputMode;
           InputMode.SetWidgetToFocus(CharacterSelectionWidget->TakeWidget());
           InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
           SetInputMode(InputMode);
           SetShowMouseCursor(true);
           
           // Make sure we can interact with UI
           bShowMouseCursor = true;
           bEnableClickEvents = true;
           bEnableMouseOverEvents = true;
           
           UE_LOG(LogTemp, Warning, TEXT("InitializeCharacterSelectionWidget: Input mode set to UI only, mouse cursor enabled"));
           UE_LOG(LogTemp, Warning, TEXT("=== Character selection widget setup COMPLETE ==="));
       }
       else
       {
           UE_LOG(LogTemp, Error, TEXT("InitializeCharacterSelectionWidget: FAILED to create character selection widget"));
       }
   }
   else
   {
       UE_LOG(LogTemp, Error, TEXT("InitializeCharacterSelectionWidget: CharacterSelectionWidgetClass is NULL - you need to set this in Blueprint or C++"));
       UE_LOG(LogTemp, Warning, TEXT("InitializeCharacterSelectionWidget: Attempting to call GameMode's ShowCharacterSelectionWidget as fallback"));
       
       // Fallback: try to call the GameMode's character selection widget
       if (ABloodreadGameMode* GameMode = Cast<ABloodreadGameMode>(GetWorld()->GetAuthGameMode()))
       {
           UE_LOG(LogTemp, Warning, TEXT("InitializeCharacterSelectionWidget: Found BloodreadGameMode, calling ShowCharacterSelectionWidget"));
           GameMode->ShowCharacterSelectionWidget();
       }
       else
       {
           UE_LOG(LogTemp, Error, TEXT("InitializeCharacterSelectionWidget: No BloodreadGameMode found, cannot show character selection"));
       }
   }
   
   UE_LOG(LogTemp, Warning, TEXT("=== InitializeCharacterSelectionWidget END ==="));
}

void ABloodreadGamePlayerController::ForceShowCharacterSelection()
{
   UE_LOG(LogTemp, Warning, TEXT("ForceShowCharacterSelection called - forcing character selection display"));
   
   // Clear any existing widget first
   if (CharacterSelectionWidget && CharacterSelectionWidget->IsInViewport())
   {
       CharacterSelectionWidget->RemoveFromParent();
       CharacterSelectionWidget = nullptr;
   }
   
   // Force initialize the character selection widget
   InitializeCharacterSelectionWidget();
}

void ABloodreadGamePlayerController::HideCharacterSelectionWidget()
{
   UE_LOG(LogTemp, Warning, TEXT("HideCharacterSelectionWidget called"));
   
   if (CharacterSelectionWidget)
   {
       if (CharacterSelectionWidget->IsInViewport())
       {
           UE_LOG(LogTemp, Warning, TEXT("HideCharacterSelectionWidget: Removing widget from viewport"));
           CharacterSelectionWidget->RemoveFromParent();
       }
       
       // Clear the reference
       CharacterSelectionWidget = nullptr;
       UE_LOG(LogTemp, Warning, TEXT("HideCharacterSelectionWidget: Widget reference cleared"));
   }
   else
   {
       UE_LOG(LogTemp, Warning, TEXT("HideCharacterSelectionWidget: No character selection widget to hide"));
   }
   
   // Always switch to game input mode when hiding character selection
   SetInputMode(FInputModeGameOnly());
   SetShowMouseCursor(false);
   bShowMouseCursor = false;
   UE_LOG(LogTemp, Warning, TEXT("HideCharacterSelectionWidget: Switched to game input mode, cursor hidden"));
}

void ABloodreadGamePlayerController::OnCharacterSelected()
{
   UE_LOG(LogTemp, Warning, TEXT("OnCharacterSelected called - character selection made"));
   
   // Immediately hide the character selection widget
   HideCharacterSelectionWidget();
   
   // Make sure we're in game input mode
   SetInputMode(FInputModeGameOnly());
   SetShowMouseCursor(false);
   bShowMouseCursor = false;
   
   UE_LOG(LogTemp, Warning, TEXT("OnCharacterSelected: Character selection hidden, ready for character spawn"));
}

void ABloodreadGamePlayerController::AttachCameraToHeadBone(ABloodreadBaseCharacter* PlayerCharacter, USkeletalMeshComponent* MeshComponent)
{
   if (!PlayerCharacter || !MeshComponent)
   {
       UE_LOG(LogTemp, Error, TEXT("AttachCameraToHeadBone: PlayerCharacter or MeshComponent is null"));
       return;
   }
   
   UE_LOG(LogTemp, Warning, TEXT("AttachCameraToHeadBone: Attempting to attach camera to head bone"));
   
   // Try common head bone names
   TArray<FString> HeadBoneNames = {
       TEXT("head"),
       TEXT("Head"),
       TEXT("HEAD"),
       TEXT("head_01"),
       TEXT("Head_01"),
       TEXT("b_head"),
       TEXT("B_Head"),
       TEXT("Bip01_Head"),
       TEXT("spine_03"),
       TEXT("Spine_03")
   };
   
   FName HeadBoneName = NAME_None;
   
   // Find the head bone
   for (const FString& BoneName : HeadBoneNames)
   {
       FName TestBoneName(*BoneName);
       if (MeshComponent->GetBoneIndex(TestBoneName) != INDEX_NONE)
       {
           HeadBoneName = TestBoneName;
           UE_LOG(LogTemp, Warning, TEXT("AttachCameraToHeadBone: Found head bone: %s"), *BoneName);
           break;
       }
   }
   
   if (HeadBoneName == NAME_None)
   {
       UE_LOG(LogTemp, Warning, TEXT("AttachCameraToHeadBone: No head bone found, listing all bones for reference:"));
       
       // List all available bones for debugging
       const FReferenceSkeleton& RefSkeleton = MeshComponent->GetSkeletalMeshAsset()->GetRefSkeleton();
       for (int32 BoneIndex = 0; BoneIndex < RefSkeleton.GetNum(); ++BoneIndex)
       {
           FString BoneName = RefSkeleton.GetBoneName(BoneIndex).ToString();
           UE_LOG(LogTemp, Warning, TEXT("Available bone %d: %s"), BoneIndex, *BoneName);
           
           // If we find any bone with "head" in the name, use it
           if (BoneName.Contains(TEXT("head"), ESearchCase::IgnoreCase) || 
               BoneName.Contains(TEXT("skull"), ESearchCase::IgnoreCase))
           {
               HeadBoneName = RefSkeleton.GetBoneName(BoneIndex);
               UE_LOG(LogTemp, Warning, TEXT("AttachCameraToHeadBone: Using bone with 'head' in name: %s"), *BoneName);
               break;
           }
       }
   }
   
   // Try to find and attach the camera
   if (UCameraComponent* CameraComponent = PlayerCharacter->FindComponentByClass<UCameraComponent>())
   {
       if (HeadBoneName != NAME_None)
       {
           UE_LOG(LogTemp, Warning, TEXT("AttachCameraToHeadBone: Attaching camera to bone: %s"), *HeadBoneName.ToString());
           
           // Detach from current parent first
           CameraComponent->DetachFromComponent(FDetachmentTransformRules::KeepWorldTransform);
           
           // Attach to the head bone
           CameraComponent->AttachToComponent(
               MeshComponent,
               FAttachmentTransformRules::SnapToTargetIncludingScale,
               HeadBoneName
           );
           
           // Set relative transform to position camera in front of head
           // X = Forward, Y = Right, Z = Up in Unreal coordinates
           CameraComponent->SetRelativeLocation(FVector(0.0f, 10.0f, 0.0f)); // Forward 15, center Y, slightly down 2
           CameraComponent->SetRelativeRotation(FRotator::ZeroRotator);
           
           UE_LOG(LogTemp, Warning, TEXT("AttachCameraToHeadBone: Camera attached to head bone at forward position (15, 0, -2)"));
       }
       else
       {
           UE_LOG(LogTemp, Error, TEXT("AttachCameraToHeadBone: Could not find suitable head bone"));
       }
   }
   else
   {
       UE_LOG(LogTemp, Error, TEXT("AttachCameraToHeadBone: No camera component found on character"));
   }
}

void ABloodreadGamePlayerController::InitializeHealthBarWidget(ABloodreadBaseCharacter* PlayerCharacter)
{
   if (!PlayerCharacter)
   {
       UE_LOG(LogTemp, Error, TEXT("InitializeHealthBarWidget: PlayerCharacter is null"));
       return;
   }
   
   // Only create UI for local players
   if (!IsLocalController())
   {
       UE_LOG(LogTemp, Warning, TEXT("InitializeHealthBarWidget: Not a local controller, skipping health bar creation"));
       return;
   }
   
   UE_LOG(LogTemp, Warning, TEXT("InitializeHealthBarWidget: Creating health bar for character: %s"), *PlayerCharacter->GetName());
   
   // Attach camera to head bone now that character is selected and spawned
   USkeletalMeshComponent* MeshComp = PlayerCharacter->FindComponentByClass<USkeletalMeshComponent>();
   if (!MeshComp)
   {
       MeshComp = PlayerCharacter->GetMesh(); // Fallback to default mesh
   }
   if (MeshComp)
   {
       AttachCameraToHeadBone(PlayerCharacter, MeshComp);
   }
   
   // Hide character selection widget when spawning character
   if (CharacterSelectionWidget)
   {
       if (CharacterSelectionWidget->IsInViewport())
       {
           UE_LOG(LogTemp, Warning, TEXT("InitializeHealthBarWidget: Removing character selection widget from viewport"));
           CharacterSelectionWidget->RemoveFromParent();
       }
       else
       {
           UE_LOG(LogTemp, Warning, TEXT("InitializeHealthBarWidget: Character selection widget exists but not in viewport"));
       }
       
       // Clear the reference
       CharacterSelectionWidget = nullptr;
       UE_LOG(LogTemp, Warning, TEXT("InitializeHealthBarWidget: Character selection widget reference cleared"));
   }
   else
   {
       UE_LOG(LogTemp, Warning, TEXT("InitializeHealthBarWidget: No character selection widget to remove"));
   }
   
   // Create health bar widget if we have a class set
   if (HealthBarWidgetClass)
   {
       UE_LOG(LogTemp, Warning, TEXT("InitializeHealthBarWidget: HealthBarWidgetClass is set to: %s"), *HealthBarWidgetClass->GetName());
       
       HealthBarWidget = CreateWidget<UBloodreadHealthBarWidget>(this, HealthBarWidgetClass);
       if (HealthBarWidget)
       {
           UE_LOG(LogTemp, Warning, TEXT("InitializeHealthBarWidget: Widget created successfully"));
           
           // Initialize the health bar with the character
           HealthBarWidget->InitializeHealthBar(PlayerCharacter);
           UE_LOG(LogTemp, Warning, TEXT("InitializeHealthBarWidget: Widget initialized with character"));
           
           // Add to viewport with high Z-order and detailed logging
           HealthBarWidget->AddToViewport(1000);
           UE_LOG(LogTemp, Warning, TEXT("InitializeHealthBarWidget: Widget added to viewport"));
           
           // Verify viewport addition
           if (HealthBarWidget->IsInViewport())
           {
               UE_LOG(LogTemp, Warning, TEXT("InitializeHealthBarWidget: CONFIRMED - Widget is in viewport"));
               
               // Force visibility
               HealthBarWidget->SetVisibility(ESlateVisibility::Visible);
               HealthBarWidget->SetRenderOpacity(1.0f);
               UE_LOG(LogTemp, Warning, TEXT("InitializeHealthBarWidget: Widget visibility and opacity set"));
               
               // Log widget tree info
               UE_LOG(LogTemp, Warning, TEXT("InitializeHealthBarWidget: Widget class: %s"), *HealthBarWidget->GetClass()->GetName());
           }
           else
           {
               UE_LOG(LogTemp, Error, TEXT("InitializeHealthBarWidget: ERROR - Widget not in viewport after AddToViewport()"));
           }
           
           // Switch to game input mode
           SetInputMode(FInputModeGameOnly());
           SetShowMouseCursor(false);
           
           UE_LOG(LogTemp, Warning, TEXT("InitializeHealthBarWidget: Health bar widget setup complete, input mode switched to game"));
       }
       else
       {
           UE_LOG(LogTemp, Error, TEXT("InitializeHealthBarWidget: Failed to create health bar widget"));
       }
   }
   else
   {
       UE_LOG(LogTemp, Warning, TEXT("InitializeHealthBarWidget: HealthBarWidgetClass not set, but still switching input mode"));
       
       // Even if no health bar, switch input mode when character spawns
       SetInputMode(FInputModeGameOnly());
       SetShowMouseCursor(false);
   }
}