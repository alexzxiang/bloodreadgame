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
   UE_LOG(LogTemp, Error, TEXT("🚨 PLAYERCONTROLLER CONSTRUCTOR 🚨"));
   UE_LOG(LogTemp, Warning, TEXT("BloodreadGamePlayerController constructor - initializing for Blueprint-based Steam multiplayer"));
   UE_LOG(LogTemp, Warning, TEXT("CONSTRUCTOR: PlayerController Name (might be temp): %s"), *GetName());
   UE_LOG(LogTemp, Warning, TEXT("CONSTRUCTOR: HasAuthority: %s"), HasAuthority() ? TEXT("TRUE") : TEXT("FALSE"));
   UE_LOG(LogTemp, Warning, TEXT("CONSTRUCTOR: NetMode: %d"), (int32)GetNetMode());
   UE_LOG(LogTemp, Warning, TEXT("CONSTRUCTOR: IsLocalController: %s"), IsLocalController() ? TEXT("TRUE") : TEXT("FALSE"));

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

   UE_LOG(LogTemp, Error, TEXT("🚨 PLAYERCONTROLLER BEGINPLAY 🚨"));
   UE_LOG(LogTemp, Warning, TEXT("BloodreadGamePlayerController BeginPlay - Setting up Blueprint-based Steam multiplayer UI"));
   UE_LOG(LogTemp, Warning, TEXT("BEGINPLAY: PlayerController Name: %s"), *GetName());
   UE_LOG(LogTemp, Warning, TEXT("BEGINPLAY: HasAuthority: %s"), HasAuthority() ? TEXT("TRUE") : TEXT("FALSE"));
   UE_LOG(LogTemp, Warning, TEXT("BEGINPLAY: NetMode: %d"), (int32)GetNetMode());
   UE_LOG(LogTemp, Warning, TEXT("BEGINPLAY: IsLocalController: %s"), IsLocalController() ? TEXT("TRUE") : TEXT("FALSE"));
   
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

   UE_LOG(LogTemp, Error, TEXT("🚨 CLIENT OnPossess CALLED 🚨"));
   UE_LOG(LogTemp, Warning, TEXT("=== PlayerController::OnPossess START ==="));
   UE_LOG(LogTemp, Warning, TEXT("PlayerController possessing pawn: %s"), InPawn ? *InPawn->GetName() : TEXT("NULL"));
   UE_LOG(LogTemp, Warning, TEXT("CLIENT OnPossess: THIS PlayerController Name: %s"), *GetName());
   UE_LOG(LogTemp, Warning, TEXT("CLIENT OnPossess: IsLocalController: %s"), IsLocalController() ? TEXT("TRUE") : TEXT("FALSE"));
   UE_LOG(LogTemp, Warning, TEXT("CLIENT OnPossess: HasAuthority: %s"), HasAuthority() ? TEXT("TRUE") : TEXT("FALSE"));
   UE_LOG(LogTemp, Warning, TEXT("CLIENT OnPossess: NetMode: %d"), (int32)GetNetMode());
   
   // Check if this is being called on the correct machine
   if (IsLocalController())
   {
       UE_LOG(LogTemp, Warning, TEXT("CLIENT OnPossess: This IS the local controller - possession working correctly!"));
       // This is the proper local possession, proceed with all operations
       PerformPossessionOperations(InPawn);
   }
   else
   {
       UE_LOG(LogTemp, Error, TEXT("CLIENT OnPossess: This is NOT the local controller - this is server-side possession"));
       UE_LOG(LogTemp, Warning, TEXT("CLIENT OnPossess: Server-side possession for remote client, performing server-only operations"));
       
       // This is server-side possession for a remote client (PC_1)
       // We still need to do some setup, but skip client-specific UI operations
       if (HasAuthority())
       {
           UE_LOG(LogTemp, Warning, TEXT("SERVER OnPossess: Setting up character for remote client"));
           
           if (InPawn)
           {
               // Enable input for the remote client
               InPawn->EnableInput(this);
               
               // Try to cast to our base character and ensure it's properly set up
               if (ABloodreadBaseCharacter* BloodreadCharacter = Cast<ABloodreadBaseCharacter>(InPawn))
               {
                   UE_LOG(LogTemp, Warning, TEXT("SERVER OnPossess: Setting up BloodreadBaseCharacter for remote client"));
                   
                   // CRITICAL: Ensure physics and collision are properly set up
                   BloodreadCharacter->ForceEnableKnockbackPhysics();
                   
                   // Ensure character is on the ground
                   if (UCharacterMovementComponent* MovementComp = BloodreadCharacter->GetCharacterMovement())
                   {
                       // Force the character to check for ground
                       MovementComp->SetMovementMode(MOVE_Walking);
                       MovementComp->bIgnoreBaseRotation = false;
                       UE_LOG(LogTemp, Warning, TEXT("SERVER OnPossess: Forced movement mode to Walking"));
                   }
                   
                   // Set up input context for the remote client
                   BloodreadCharacter->SetupInputContext();
                   
                   // Force character initialization
                   BloodreadCharacter->ForceInitializeCharacterSystems();
               }
           }
           
           UE_LOG(LogTemp, Warning, TEXT("SERVER OnPossess: Server-side setup complete, Client RPC should handle UI"));
       }
       
       // Don't return early - let the possession complete
   }
  
   UE_LOG(LogTemp, Warning, TEXT("=== PlayerController::OnPossess END ==="));
}

void ABloodreadGamePlayerController::PerformPossessionOperations(APawn* InPawn)
{
   UE_LOG(LogTemp, Warning, TEXT("=== PerformPossessionOperations START ==="));
   
   // Hide character selection widget immediately when possessing any pawn
   if (CharacterSelectionWidget && CharacterSelectionWidget->IsInViewport())
   {
       UE_LOG(LogTemp, Warning, TEXT("PerformPossessionOperations: Hiding character selection widget immediately"));
       HideCharacterSelectionWidget();
   }
   
   if (InPawn)
   {
       UE_LOG(LogTemp, Warning, TEXT("PerformPossessionOperations: Pawn class: %s"), *InPawn->GetClass()->GetName());
      
       // Try to cast to our base character and initialize mesh
       if (ABloodreadBaseCharacter* BloodreadCharacter = Cast<ABloodreadBaseCharacter>(InPawn))
       {
           UE_LOG(LogTemp, Warning, TEXT("PerformPossessionOperations: SUCCESS - Cast to BloodreadBaseCharacter %s - Current Class: %d"),
                  *BloodreadCharacter->GetName(), (int32)BloodreadCharacter->GetCharacterClass());
           
           // CRITICAL: Fix physics and collision issues
           UE_LOG(LogTemp, Warning, TEXT("PerformPossessionOperations: Fixing physics and collision"));
           BloodreadCharacter->ForceEnableKnockbackPhysics();
           
           // CRITICAL: Ensure proper movement setup
           if (UCharacterMovementComponent* MovementComp = BloodreadCharacter->GetCharacterMovement())
           {
               MovementComp->SetMovementMode(MOVE_Walking);
               MovementComp->bIgnoreBaseRotation = false;
               // Force ground check
               MovementComp->FindFloor(BloodreadCharacter->GetActorLocation(), MovementComp->CurrentFloor, true);
               UE_LOG(LogTemp, Warning, TEXT("PerformPossessionOperations: Fixed movement and forced ground check"));
           }
           
           // CRITICAL: Reset character rotation to face forward properly
           FRotator CurrentRotation = BloodreadCharacter->GetActorRotation();
           FRotator CorrectedRotation = FRotator(0.0f, CurrentRotation.Yaw, 0.0f); // Keep only Yaw, reset Pitch/Roll
           BloodreadCharacter->SetActorRotation(CorrectedRotation);
           UE_LOG(LogTemp, Warning, TEXT("PerformPossessionOperations: Corrected character rotation from %s to %s"), 
                  *CurrentRotation.ToString(), *CorrectedRotation.ToString());
           
           InitializeCharacterMesh(BloodreadCharacter);
           
           // Initialize health bar widget for the possessed character (client-side UI)
           InitializeHealthBarWidget(BloodreadCharacter);
           
           // Set input mode to game after UI is ready
           SetInputMode(FInputModeGameOnly());
           SetShowMouseCursor(false);
           UE_LOG(LogTemp, Warning, TEXT("PerformPossessionOperations: Input mode set to game, mouse cursor hidden"));
           
           UE_LOG(LogTemp, Error, TEXT("🚨 MULTIPLAYER POSSESSION COMPLETE - CHARACTER SHOULD BE CONTROLLABLE! 🚨"));
       }
       else
       {
           UE_LOG(LogTemp, Error, TEXT("PerformPossessionOperations: FAILED - Possessed pawn is not a BloodreadBaseCharacter - Class: %s"), *InPawn->GetClass()->GetName());
       }
   }
   else
   {
       UE_LOG(LogTemp, Error, TEXT("PerformPossessionOperations: InPawn is NULL!"));
   }
  
   UE_LOG(LogTemp, Warning, TEXT("=== PerformPossessionOperations END ==="));
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
           CameraComponent->SetRelativeLocation(FVector(0.0f, 10.0f, 10.0f)); // Reduced Y from 10.0 to 5.0 to shift camera slightly right
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
           
           UE_LOG(LogTemp, Warning, TEXT("InitializeHealthBarWidget: Health bar widget setup complete"));
       }
       else
       {
           UE_LOG(LogTemp, Error, TEXT("InitializeHealthBarWidget: Failed to create health bar widget"));
       }
   }
   else
   {
       UE_LOG(LogTemp, Warning, TEXT("InitializeHealthBarWidget: HealthBarWidgetClass not set"));
   }
}

void ABloodreadGamePlayerController::RequestCharacterSelection(int32 CharacterClassIndex)
{
    UE_LOG(LogTemp, Warning, TEXT("RequestCharacterSelection called by player: %s with index: %d"), 
           *GetName(), CharacterClassIndex);
    
    // Enhanced logging to trace the request
    UE_LOG(LogTemp, Warning, TEXT("CLIENT REQUEST: PlayerController Name: %s"), *GetName());
    UE_LOG(LogTemp, Warning, TEXT("CLIENT REQUEST: PlayerController Class: %s"), *GetClass()->GetName());
    UE_LOG(LogTemp, Warning, TEXT("CLIENT REQUEST: IsLocalController: %s"), IsLocalController() ? TEXT("TRUE") : TEXT("FALSE"));
    UE_LOG(LogTemp, Warning, TEXT("CLIENT REQUEST: HasAuthority: %s"), HasAuthority() ? TEXT("TRUE") : TEXT("FALSE"));
    UE_LOG(LogTemp, Warning, TEXT("CLIENT REQUEST: NetMode: %d"), (int32)GetNetMode());
    
    // Check current pawn BEFORE character selection
    APawn* CurrentPawn = GetPawn();
    if (CurrentPawn)
    {
        UE_LOG(LogTemp, Warning, TEXT("CLIENT REQUEST: Current pawn BEFORE selection: %s"), *CurrentPawn->GetName());
        UE_LOG(LogTemp, Warning, TEXT("CLIENT REQUEST: Current pawn class: %s"), *CurrentPawn->GetClass()->GetName());
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("CLIENT REQUEST: No current pawn BEFORE selection"));
    }
    
    // Check if we're the server (host) or client
    if (HasAuthority())
    {
        UE_LOG(LogTemp, Warning, TEXT("Client: We are the HOST - sending RPC to server"));
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("Client: We are a CLIENT - sending RPC to server"));
    }
    
    UE_LOG(LogTemp, Warning, TEXT("CLIENT REQUEST: About to call ServerRequestCharacterSelection RPC"));
    
    // Send RPC to server (works whether we're host or client)
    ServerRequestCharacterSelection(CharacterClassIndex);
    
    UE_LOG(LogTemp, Warning, TEXT("CLIENT REQUEST: ServerRequestCharacterSelection RPC call completed"));
    
    // Add a timer to check if possession happened on the client side
    GetWorld()->GetTimerManager().SetTimer(CheckPossessionTimerHandle, [this]()
    {
        APawn* NewPawn = GetPawn();
        if (NewPawn)
        {
            UE_LOG(LogTemp, Warning, TEXT("CLIENT CHECK: Pawn after RPC: %s"), *NewPawn->GetName());
            UE_LOG(LogTemp, Warning, TEXT("CLIENT CHECK: Pawn class after RPC: %s"), *NewPawn->GetClass()->GetName());
        }
        else
        {
            UE_LOG(LogTemp, Error, TEXT("CLIENT CHECK: Still no pawn after RPC"));
        }
    }, 2.0f, false);
}

void ABloodreadGamePlayerController::ServerRequestCharacterSelection_Implementation(int32 CharacterClassIndex)
{
    UE_LOG(LogTemp, Warning, TEXT("ServerRequestCharacterSelection_Implementation called for player: %s with index: %d"), 
           *GetName(), CharacterClassIndex);
    
    // Enhanced logging to trace the PlayerController
    UE_LOG(LogTemp, Error, TEXT("🚨 SERVER RPC: THIS PlayerController Name: %s 🚨"), *GetName());
    UE_LOG(LogTemp, Warning, TEXT("SERVER RPC: PlayerController Class: %s"), *GetClass()->GetName());
    UE_LOG(LogTemp, Warning, TEXT("SERVER RPC: IsLocalController: %s"), IsLocalController() ? TEXT("TRUE") : TEXT("FALSE"));
    UE_LOG(LogTemp, Warning, TEXT("SERVER RPC: HasAuthority: %s"), HasAuthority() ? TEXT("TRUE") : TEXT("FALSE"));
    UE_LOG(LogTemp, Warning, TEXT("SERVER RPC: NetMode: %d"), (int32)GetNetMode());
    
    // Check all PlayerControllers in the world to see what we have
    UWorld* World = GetWorld();
    if (World)
    {
        UE_LOG(LogTemp, Warning, TEXT("SERVER RPC: Listing all PlayerControllers:"));
        int32 PlayerIndex = 0;
        for (FConstPlayerControllerIterator Iterator = World->GetPlayerControllerIterator(); Iterator; ++Iterator)
        {
            APlayerController* PC = Iterator->Get();
            if (PC)
            {
                UE_LOG(LogTemp, Warning, TEXT("SERVER RPC: PlayerController[%d]: %s (IsLocal: %s, HasAuth: %s)"), 
                       PlayerIndex, *PC->GetName(), 
                       PC->IsLocalController() ? TEXT("TRUE") : TEXT("FALSE"),
                       PC->HasAuthority() ? TEXT("TRUE") : TEXT("FALSE"));
                PlayerIndex++;
            }
        }
    }
    
    // Verify we're on the server (this should always be true for Server RPCs)
    if (!HasAuthority())
    {
        UE_LOG(LogTemp, Error, TEXT("ServerRequestCharacterSelection_Implementation called without authority!"));
        return;
    }
    
    // Verify the world exists (we already got it above)
    if (!World)
    {
        UE_LOG(LogTemp, Error, TEXT("Server: No valid world found"));
        return;
    }
    
    // GetAuthGameMode() only returns valid GameMode on the server
    AGameModeBase* BaseGameMode = World->GetAuthGameMode();
    if (!BaseGameMode)
    {
        UE_LOG(LogTemp, Error, TEXT("Server: No GameMode found - this should not happen on server"));
        return;
    }
    
    // Cast to our specific GameMode
    ABloodreadGameMode* GameMode = Cast<ABloodreadGameMode>(BaseGameMode);
    if (!GameMode)
    {
        UE_LOG(LogTemp, Error, TEXT("Server: GameMode is not BloodreadGameMode type"));
        return;
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Server: Successfully found BloodreadGameMode, forwarding character selection request"));
    UE_LOG(LogTemp, Warning, TEXT("Server: About to call HandleCharacterSelection with PlayerController: %s"), *GetName());
    UE_LOG(LogTemp, Error, TEXT("🚨 RPC: Calling GameMode->HandleCharacterSelection with THIS (%s) 🚨"), *GetName());
    UE_LOG(LogTemp, Warning, TEXT("RPC: PlayerController IsValid: %s"), IsValid(this) ? TEXT("TRUE") : TEXT("FALSE"));
    UE_LOG(LogTemp, Warning, TEXT("RPC: PlayerController pointer address: %p"), this);
    
    // Forward to GameMode with this PlayerController
    GameMode->HandleCharacterSelection(CharacterClassIndex, this);
}

void ABloodreadGamePlayerController::ClientNotifyCharacterPossession_Implementation(APawn* NewCharacter)
{
    UE_LOG(LogTemp, Error, TEXT("🚨 CLIENT RPC: ClientNotifyCharacterPossession called! 🚨"));
    UE_LOG(LogTemp, Warning, TEXT("CLIENT RPC: NewCharacter: %s"), NewCharacter ? *NewCharacter->GetName() : TEXT("NULL"));
    UE_LOG(LogTemp, Warning, TEXT("CLIENT RPC: This PlayerController: %s"), *GetName());
    UE_LOG(LogTemp, Warning, TEXT("CLIENT RPC: IsLocalController: %s"), IsLocalController() ? TEXT("TRUE") : TEXT("FALSE"));
    UE_LOG(LogTemp, Warning, TEXT("CLIENT RPC: HasAuthority: %s"), HasAuthority() ? TEXT("TRUE") : TEXT("FALSE"));
    UE_LOG(LogTemp, Warning, TEXT("CLIENT RPC: NetMode: %d"), (int32)GetNetMode());
    
    // Client RPCs are only sent to the owning client, so if we receive this, we should execute it
    // regardless of IsLocalController() value
    if (NewCharacter)
    {
        UE_LOG(LogTemp, Warning, TEXT("CLIENT RPC: Character received, manually performing OnPossess operations..."));
        UE_LOG(LogTemp, Warning, TEXT("CLIENT RPC: Proceeding without IsLocalController check since this is a Client RPC"));
        
        // Check current pawn state
        APawn* CurrentPawn = GetPawn();
        UE_LOG(LogTemp, Warning, TEXT("CLIENT RPC: Current pawn: %s, Target pawn: %s"), 
               CurrentPawn ? *CurrentPawn->GetName() : TEXT("NULL"), *NewCharacter->GetName());
        
        // Manually perform the OnPossess operations that should have happened automatically
        UE_LOG(LogTemp, Warning, TEXT("CLIENT RPC: Calling PerformPossessionOperations to simulate proper OnPossess"));
        
        // Use our centralized possession operations function
        PerformPossessionOperations(NewCharacter);
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("CLIENT RPC: NewCharacter is NULL!"));
    }
}

void ABloodreadGamePlayerController::ServerRequestPossession_Implementation(APawn* CharacterToPossess)
{
    UE_LOG(LogTemp, Error, TEXT("🚨 SERVER: ServerRequestPossession called! 🚨"));
    UE_LOG(LogTemp, Warning, TEXT("SERVER POSSESS: Character: %s"), CharacterToPossess ? *CharacterToPossess->GetName() : TEXT("NULL"));
    UE_LOG(LogTemp, Warning, TEXT("SERVER POSSESS: PlayerController: %s"), *GetName());
    
    if (!HasAuthority())
    {
        UE_LOG(LogTemp, Error, TEXT("ServerRequestPossession called without authority!"));
        return;
    }
    
    if (!CharacterToPossess)
    {
        UE_LOG(LogTemp, Error, TEXT("ServerRequestPossession: CharacterToPossess is NULL!"));
        return;
    }
    
    // Check if we're already possessing this character
    if (GetPawn() == CharacterToPossess)
    {
        UE_LOG(LogTemp, Warning, TEXT("SERVER POSSESS: Already possessing this character"));
        return;
    }
    
    UE_LOG(LogTemp, Warning, TEXT("SERVER POSSESS: Attempting to possess character"));
    
    // Try to possess the character
    Possess(CharacterToPossess);
    
    // Verify possession worked
    if (GetPawn() == CharacterToPossess)
    {
        UE_LOG(LogTemp, Warning, TEXT("SERVER POSSESS: SUCCESS - Possession completed"));
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("SERVER POSSESS: FAILED - Possession did not work"));
    }
}