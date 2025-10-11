#include "BloodreadBaseCharacter.h"
#include "Engine/Engine.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Camera/CameraComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputActionValue.h"
#include "Animation/AnimInstance.h"
#include "Animation/AnimMontage.h"
#include "Animation/AnimSequence.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/WidgetComponent.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"
#include "Blueprint/UserWidget.h"
#include "UObject/ConstructorHelpers.h"
#include "PracticeDummy.h"
#include "BloodreadWarriorCharacter.h"
#include "BloodreadMageCharacter.h"
#include "BloodreadRogueCharacter.h"
#include "BloodreadHealerCharacter.h"
#include "BloodreadHealthBarWidget.h"
#include "BloodreadDragonCharacter.h"
#include "UniversalHealthBarWidget.h"

ABloodreadBaseCharacter::ABloodreadBaseCharacter()
{
    PrimaryActorTick.bCanEverTick = true;
    
    // Enable network replication
    bReplicates = true;
    SetReplicateMovement(true);

    // Set size for collision capsule
    GetCapsuleComponent()->SetCapsuleSize(42.f, 96.0f);
    
    // CRITICAL: Set up collision for targeting system
    GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
    GetCapsuleComponent()->SetCollisionObjectType(ECollisionChannel::ECC_Pawn);
    GetCapsuleComponent()->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Block);
    GetCapsuleComponent()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);
    
    UE_LOG(LogTemp, Warning, TEXT("BloodreadBaseCharacter: Collision setup complete - ObjectType=Pawn, Response=Block"));

    // Configure character movement for smooth first-person controls
    GetCharacterMovement()->bOrientRotationToMovement = false; // FIXED: Don't rotate body to movement direction
    GetCharacterMovement()->RotationRate = FRotator(0.0f, 720.0f, 0.0f); // Smooth rotation rate
    GetCharacterMovement()->JumpZVelocity = 700.f;
    GetCharacterMovement()->AirControl = 0.35f;
    GetCharacterMovement()->MaxWalkSpeed = 500.f;
    GetCharacterMovement()->MinAnalogWalkSpeed = 20.f;
    GetCharacterMovement()->BrakingDecelerationWalking = 2000.f;
    GetCharacterMovement()->MaxAcceleration = 2048.0f; // Faster acceleration for responsive movement
    GetCharacterMovement()->GroundFriction = 8.0f; // Better ground control
    GetCharacterMovement()->BrakingDecelerationFalling = 1500.f; // Smooth air control
    
    // KNOCKBACK SETTINGS: Essential for proper knockback functionality
    GetCharacterMovement()->Mass = 100.0f; // Set reasonable mass for knockback calculations
    GetCharacterMovement()->bCanWalkOffLedges = true; // Allow knockback to push off edges
    GetCharacterMovement()->bCanWalkOffLedgesWhenCrouching = true;
    GetCharacterMovement()->bImpartBaseVelocityX = true; // Allow velocity inheritance
    GetCharacterMovement()->bImpartBaseVelocityY = true;
    GetCharacterMovement()->bImpartBaseVelocityZ = true;
    
    UE_LOG(LogTemp, Warning, TEXT("BloodreadBaseCharacter: Knockback settings configured - Mass: %.1f"), GetCharacterMovement()->Mass);

    // First-person controller rotation settings
    bUseControllerRotationPitch = false;
    bUseControllerRotationYaw = true;  // Allow yaw rotation for camera
    bUseControllerRotationRoll = false;

    // Create first person camera component
    FirstPersonCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FirstPersonCamera"));
    FirstPersonCamera->SetupAttachment(RootComponent);
    FirstPersonCamera->SetRelativeLocation(CustomMeshCameraOffset);
    FirstPersonCamera->bUsePawnControlRotation = true;

    // Initialize default stats
    CurrentStats = FCharacterStats();
    CurrentHealth = CurrentStats.MaxHealth;
    CurrentMana = CurrentStats.Mana;
    
    // Initialize health bar component (exactly like PracticeDummy)
    HealthBarWidgetComponent = CreateDefaultSubobject<UWidgetComponent>(TEXT("HealthBarWidgetComponent"));
    HealthBarWidgetComponent->SetupAttachment(RootComponent); // Attach to root like dummy
    HealthBarWidgetComponent->SetRelativeLocation(FVector(0.0f, 0.0f, 120.0f)); // Above player head
    HealthBarWidgetComponent->SetDrawSize(FVector2D(200.0f, 50.0f));
    HealthBarWidgetComponent->SetWidgetSpace(EWidgetSpace::Screen); // Screen space - always face camera
    HealthBarWidgetComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision); // No collision like dummy
    
    // Note: Widget class should be set in Blueprint - just like PracticeDummy
    // The Blueprint will assign the proper overhead health bar widget class
    
    // Distance-based visibility settings
    MaxHealthBarVisibilityDistance = 2000.0f; // Hide health bars beyond 20 meters
    HealthBarVisibilityCheckInterval = 0.5f; // Check every 0.5 seconds
    
    // Initialize widget state variables
    CurrentHealthBarWidget = nullptr;
    
    // Note: The actual widget class should be set in Blueprint defaults
    // This matches how PracticeDummy works - widget creation handled by Blueprint
}

void ABloodreadBaseCharacter::BeginPlay()
{
    Super::BeginPlay();

    UE_LOG(LogTemp, Warning, TEXT("=== CHARACTER BEGINPLAY START ==="));
    UE_LOG(LogTemp, Warning, TEXT("Character BeginPlay called for %s, CurrentClass=%d"), *GetName(), (int32)CurrentCharacterClass);
    UE_LOG(LogTemp, Warning, TEXT("Character class name: %s"), *GetClass()->GetName());

    // ALWAYS force initialization to debug the mesh issue
    UE_LOG(LogTemp, Warning, TEXT("FORCING character initialization for mesh debugging"));
    ForceInitializeCharacterSystems();

    // Initialize character from class data
    UE_LOG(LogTemp, Warning, TEXT("Calling InitializeFromClassData"));
    InitializeFromClassData(CharacterClassData);
    
    // Initialize health bar UI
    InitializeHealthBar();
    
    // CRITICAL: Force health bar visibility check
    if (HealthBarWidgetComponent)
    {
        UE_LOG(LogTemp, Error, TEXT("*** HEALTH BAR COMPONENT EXISTS: %s ***"), *HealthBarWidgetComponent->GetName());
        UE_LOG(LogTemp, Error, TEXT("*** Widget Class Set: %s ***"), 
               HealthBarWidgetComponent->GetWidgetClass() ? *HealthBarWidgetComponent->GetWidgetClass()->GetName() : TEXT("NONE - THIS IS THE PROBLEM!"));
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("*** CRITICAL: HealthBarWidgetComponent is NULL! ***"));
    }
    
    // Set up input if we have a controller
    SetupInputContext();
    
    // Start health bar visibility timer
    if (HealthBarVisibilityCheckInterval > 0.0f)
    {
        GetWorldTimerManager().SetTimer(HealthBarVisibilityTimerHandle, 
                                       this, &ABloodreadBaseCharacter::UpdateHealthBarVisibility, 
                                       HealthBarVisibilityCheckInterval, true);
    }
    
    UE_LOG(LogTemp, Warning, TEXT("=== CHARACTER BEGINPLAY END ==="));
}

void ABloodreadBaseCharacter::SetupInputContext()
{
    // Input context is now handled by PlayerController in SetupInputComponent
    UE_LOG(LogTemp, Warning, TEXT("SetupInputContext called - PlayerController should handle input mapping contexts"));
}

void ABloodreadBaseCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
    Super::SetupPlayerInputComponent(PlayerInputComponent);

    // Use traditional input system (matches DefaultInput.ini)
    UE_LOG(LogTemp, Warning, TEXT("Setting up traditional input system"));

    // Action Mappings (from DefaultInput.ini)
    PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &ACharacter::Jump);
    PlayerInputComponent->BindAction("Jump", IE_Released, this, &ACharacter::StopJumping);
    PlayerInputComponent->BindAction("Attack", IE_Pressed, this, &ABloodreadBaseCharacter::Attack);
    PlayerInputComponent->BindAction("Ability1", IE_Pressed, this, &ABloodreadBaseCharacter::UseAbility1);
    PlayerInputComponent->BindAction("Ability2", IE_Pressed, this, &ABloodreadBaseCharacter::UseAbility2);
    PlayerInputComponent->BindAction("TestDamage", IE_Pressed, this, &ABloodreadBaseCharacter::TestDamage);
    PlayerInputComponent->BindAction("TestHeal", IE_Pressed, this, &ABloodreadBaseCharacter::TestHeal);

    // Axis Mappings (from DefaultInput.ini)
    PlayerInputComponent->BindAxis("MoveForward", this, &ABloodreadBaseCharacter::MoveForward);
    PlayerInputComponent->BindAxis("MoveRight", this, &ABloodreadBaseCharacter::MoveRight);
    PlayerInputComponent->BindAxis("Turn", this, &ABloodreadBaseCharacter::Turn);
    PlayerInputComponent->BindAxis("LookUp", this, &ABloodreadBaseCharacter::LookUp);

    UE_LOG(LogTemp, Warning, TEXT("✅ Traditional input system setup complete"));
}

void ABloodreadBaseCharacter::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    UpdateAbilityCooldowns(DeltaTime);
    
    // Mana regeneration system
    ManaRegenTimer += DeltaTime;
    if (ManaRegenTimer >= 1.0f) // Regenerate every second
    {
        int32 OldMana = CurrentMana;
        CurrentMana = FMath::Min(CurrentMana + static_cast<int32>(ManaRegenRate), CurrentStats.Mana);
        if (CurrentMana != OldMana)
        {
            OnManaChanged(OldMana, CurrentMana);
        }
        ManaRegenTimer = 0.0f;
    }
}

void ABloodreadBaseCharacter::SetCharacterClass(ECharacterClass NewClass)
{
    CurrentCharacterClass = NewClass;
    OnCharacterClassChanged();
    
    UE_LOG(LogTemp, Warning, TEXT("Character class changed to: %d"), (int32)NewClass);
}

void ABloodreadBaseCharacter::InitializeFromClassData(const FCharacterClassData& ClassData)
{
    CharacterClassData = ClassData;
    CurrentCharacterClass = ClassData.CharacterClass;
    CurrentStats = ClassData.BaseStats;
    CurrentHealth = CurrentStats.MaxHealth;
    CurrentMana = CurrentStats.Mana;

    UE_LOG(LogTemp, Warning, TEXT("InitializeFromClassData: Initializing character as %s"), *ClassData.ClassName);

    // Find all skeletal mesh components and try to apply mesh
    TArray<USkeletalMeshComponent*> MeshComponents;
    GetComponents<USkeletalMeshComponent>(MeshComponents);

    UE_LOG(LogTemp, Warning, TEXT("Found %d skeletal mesh components"), MeshComponents.Num());

    bool bMeshApplied = false;

    // Try to apply mesh based on character class using our new system
    for (USkeletalMeshComponent* MeshComp : MeshComponents)
    {
        if (MeshComp)
        {
            UE_LOG(LogTemp, Warning, TEXT("Attempting to apply mesh to component: %s"), *MeshComp->GetName());
            if (ApplyClassDataToMeshComponent(MeshComp, CurrentCharacterClass))
            {
                bMeshApplied = true;
                UE_LOG(LogTemp, Warning, TEXT("SUCCESS: Mesh applied to component %s"), *MeshComp->GetName());
                
                // Adjust mesh positioning to fix floating issues and align with camera
                // Position mesh so character stands on ground and camera aligns with head
                MeshComp->SetRelativeLocation(FVector(0.0f, 0.0f, -88.0f)); // Adjusted for better camera alignment
                MeshComp->SetRelativeRotation(FRotator(0.0f, -90.0f, 0.0f)); // Rotate to face forward (Paragon characters face sideways by default)
                
                UE_LOG(LogTemp, Warning, TEXT("Applied mesh positioning: Location(0,0,-88), Rotation(0,-90,0)"));
                
                // Also try to set animation blueprint if we have one
                if (!ClassData.AnimationBlueprintPath.IsEmpty())
                {
                    bool bAnimBPApplied = SetAnimationBlueprintOnComponent(MeshComp, ClassData.AnimationBlueprintPath);
                    if (bAnimBPApplied)
                    {
                        UE_LOG(LogTemp, Warning, TEXT("Applied animation blueprint from class data to %s"), *MeshComp->GetName());
                    }
                }
                else
                {
                    UE_LOG(LogTemp, Error, TEXT("No animation blueprint path found for character class %d"), (int32)CurrentCharacterClass);
                    
                    // Try to load animation blueprint based on character class as fallback
                    FString FallbackAnimBPPath;
                    switch (CurrentCharacterClass)
                    {
                        case ECharacterClass::Warrior:
                            FallbackAnimBPPath = TEXT("/Game/ParagonGideon/Characters/Heroes/Gideon/Animations/Gideon_AnimBlueprint.Gideon_AnimBlueprint_C");
                            break;
                        case ECharacterClass::Mage:
                            FallbackAnimBPPath = TEXT("/Game/ParagonAurora/Characters/Heroes/Aurora/Animations/Aurora_AnimBlueprint.Aurora_AnimBlueprint_C");
                            break;
                        case ECharacterClass::Rogue:
                            FallbackAnimBPPath = TEXT("/Game/ParagonShinbi/Characters/Heroes/Shinbi/Shinbi_AnimBlueprint.Shinbi_AnimBlueprint_C");
                            break;
                        case ECharacterClass::Healer:
                            FallbackAnimBPPath = TEXT("/Game/ParagonFey/Characters/Heroes/Fey/Fey_AnimBlueprint.Fey_AnimBlueprint_C");
                            break;
                        case ECharacterClass::Dragon:
                            FallbackAnimBPPath = TEXT("/Game/ParagonYin/Characters/Heroes/Yin/Yin_AnimBlueprint.Yin_AnimBlueprint_C");
                            break;
                        default:
                            break;
                    }
                    
                    if (!FallbackAnimBPPath.IsEmpty())
                    {
                        bool bFallbackApplied = SetAnimationBlueprintOnComponent(MeshComp, FallbackAnimBPPath);
                        if (bFallbackApplied)
                        {
                            UE_LOG(LogTemp, Warning, TEXT("Applied fallback animation blueprint for class %d"), (int32)CurrentCharacterClass);
                        }
                        else
                        {
                            UE_LOG(LogTemp, Error, TEXT("Failed to load fallback animation blueprint for class %d"), (int32)CurrentCharacterClass);
                        }
                    }
                }
            }
        }
    }

    if (!bMeshApplied)
    {
        UE_LOG(LogTemp, Error, TEXT("FAILED: Could not apply mesh to any skeletal mesh component"));
    }
    else
    {
        // After mesh is applied, reactivate animation blueprints to ensure they're working
        UE_LOG(LogTemp, Warning, TEXT("Mesh application complete, reactivating animation blueprints..."));
    }

    // Update camera offset based on character class
    if (FirstPersonCamera)
    {
        FirstPersonCamera->SetRelativeLocation(ClassData.CameraOffset);
        UE_LOG(LogTemp, Warning, TEXT("Updated camera offset to: %s"), *ClassData.CameraOffset.ToString());
    }

    OnCharacterClassChanged();
    UE_LOG(LogTemp, Warning, TEXT("Character initialized as: %s"), *ClassData.ClassName);
}

bool ABloodreadBaseCharacter::SetMeshOnComponent(USkeletalMeshComponent* MeshComponent, const FString& MeshPath)
{
    if (!MeshComponent)
    {
        UE_LOG(LogTemp, Error, TEXT("SetMeshOnComponent: MeshComponent is null"));
        return false;
    }

    if (MeshPath.IsEmpty())
    {
        UE_LOG(LogTemp, Error, TEXT("SetMeshOnComponent: MeshPath is empty"));
        return false;
    }

    UE_LOG(LogTemp, Warning, TEXT("SetMeshOnComponent: Attempting to load mesh from path: %s"), *MeshPath);

    // Try multiple loading methods for better reliability
    USkeletalMesh* LoadedMesh = nullptr;

    // Method 1: Direct asset loading using LoadObject
    LoadedMesh = LoadObject<USkeletalMesh>(nullptr, *MeshPath);
    if (LoadedMesh)
    {
        UE_LOG(LogTemp, Warning, TEXT("SUCCESS: LoadObject found mesh at path: %s"), *MeshPath);
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("LoadObject failed, trying TSoftObjectPtr method"));
        
        // Method 2: TSoftObjectPtr loading (our previous method)
        FSoftObjectPath MeshSoftObjectPath(MeshPath);
        TSoftObjectPtr<USkeletalMesh> MeshSoftPtr(MeshSoftObjectPath);
        LoadedMesh = MeshSoftPtr.LoadSynchronous();
        
        if (LoadedMesh)
        {
            UE_LOG(LogTemp, Warning, TEXT("SUCCESS: TSoftObjectPtr found mesh at path: %s"), *MeshPath);
        }
        else
        {
            UE_LOG(LogTemp, Error, TEXT("Both loading methods failed for path: %s"), *MeshPath);
        }
    }

    // Apply the mesh if we found one
    if (LoadedMesh)
    {
        MeshComponent->SetSkeletalMesh(LoadedMesh);
        
        // Force refresh the component
        MeshComponent->RecreateRenderState_Concurrent();
        
        // Log detailed mesh information
        UE_LOG(LogTemp, Warning, TEXT("SUCCESS: Applied mesh '%s' to component '%s'"), 
               *LoadedMesh->GetName(), *MeshComponent->GetName());
        UE_LOG(LogTemp, Warning, TEXT("Mesh Skeleton: %s"), 
               LoadedMesh->GetSkeleton() ? *LoadedMesh->GetSkeleton()->GetName() : TEXT("None"));
        
        return true;
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("FAILED: Could not load any mesh from path: %s"), *MeshPath);
        return false;
    }
}

bool ABloodreadBaseCharacter::ApplyClassDataToMeshComponent(USkeletalMeshComponent* MeshComponent, ECharacterClass CharacterClass)
{
    if (!MeshComponent)
    {
        UE_LOG(LogTemp, Error, TEXT("ApplyClassDataToMeshComponent: MeshComponent is null"));
        return false;
    }

    // Debug current character state
    UE_LOG(LogTemp, Warning, TEXT("ApplyClassDataToMeshComponent: Character=%s, RequestedClass=%d, CurrentClass=%d"), 
           *GetName(), (int32)CharacterClass, (int32)CurrentCharacterClass);

    FString MeshPath;
    FString ClassName;

    // Get the appropriate mesh path based on character class
    switch (CharacterClass)
    {
        case ECharacterClass::Warrior:
            MeshPath = "/Game/ParagonGideon/Characters/Heroes/Gideon/Meshes/Gideon.Gideon";
            ClassName = "Warrior";
            break;
        case ECharacterClass::Mage:
            MeshPath = "/Game/ParagonAurora/Characters/Heroes/Aurora/Meshes/Aurora.Aurora";
            ClassName = "Mage";
            break;
        case ECharacterClass::Rogue:
            MeshPath = "/Game/ParagonShinbi/Characters/Heroes/Shinbi/Skins/Tier_1/Shinbi_Dynasty/Meshes/ShinbiDynasty.ShinbiDynasty";
            ClassName = "Rogue";
            break;
        case ECharacterClass::Healer:
            MeshPath = "/Game/ParagonFey/Characters/Heroes/Fey/Meshes/Fey.Fey";
            ClassName = "Healer";
            break;
        case ECharacterClass::Dragon:
            MeshPath = "/Game/ParagonYin/Characters/Heroes/Yin/Meshes/Yin.Yin";
            ClassName = "Dragon";
            break;
        case ECharacterClass::None:
            UE_LOG(LogTemp, Error, TEXT("ApplyClassDataToMeshComponent: Character class is None - character not initialized properly"));
            return false;
        default:
            UE_LOG(LogTemp, Error, TEXT("ApplyClassDataToMeshComponent: Unsupported character class: %d"), (int32)CharacterClass);
            return false;
    }

    UE_LOG(LogTemp, Warning, TEXT("ApplyClassDataToMeshComponent: Setting %s mesh (%s) on component %s"), 
           *ClassName, *MeshPath, *MeshComponent->GetName());
    return SetMeshOnComponent(MeshComponent, MeshPath);
}

bool ABloodreadBaseCharacter::SetAnimationBlueprintOnComponent(USkeletalMeshComponent* MeshComponent, const FString& AnimBPPath)
{
    if (!MeshComponent)
    {
        UE_LOG(LogTemp, Error, TEXT("SetAnimationBlueprintOnComponent: MeshComponent is null"));
        return false;
    }

    if (AnimBPPath.IsEmpty())
    {
        UE_LOG(LogTemp, Error, TEXT("SetAnimationBlueprintOnComponent: AnimBPPath is empty"));
        return false;
    }

    UE_LOG(LogTemp, Warning, TEXT("SetAnimationBlueprintOnComponent: Attempting to load animation blueprint from path: %s"), *AnimBPPath);

    // Try to load the animation blueprint using LoadObject<UClass>
    UClass* AnimBPClass = LoadObject<UClass>(nullptr, *AnimBPPath);
    
    if (AnimBPClass)
    {
        // First, ensure the animation mode is set to use animation blueprint
        MeshComponent->SetAnimationMode(EAnimationMode::AnimationBlueprint);
        
        // Set the animation instance class
        MeshComponent->SetAnimInstanceClass(AnimBPClass);
        
        // Force the animation instance to initialize
        if (MeshComponent->GetAnimInstance())
        {
            UE_LOG(LogTemp, Warning, TEXT("SUCCESS: Animation blueprint '%s' applied and activated on component '%s'"), 
                   *AnimBPClass->GetName(), *MeshComponent->GetName());
        }
        else
        {
            // Try to force initialization
            MeshComponent->InitializeAnimScriptInstance(true);
            UE_LOG(LogTemp, Warning, TEXT("SUCCESS: Animation blueprint '%s' applied to component '%s' - forced initialization"), 
                   *AnimBPClass->GetName(), *MeshComponent->GetName());
        }
        return true;
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("FAILED: Could not load animation blueprint from path: %s"), *AnimBPPath);
        return false;
    }
}

void ABloodreadBaseCharacter::ReactivateAnimationBlueprints()
{
    UE_LOG(LogTemp, Warning, TEXT("ReactivateAnimationBlueprints: Reactivating animation blueprints for %s"), *GetName());
    
    // Get all skeletal mesh components
    TArray<USkeletalMeshComponent*> MeshComponents;
    GetComponents<USkeletalMeshComponent>(MeshComponents);
    
    for (USkeletalMeshComponent* MeshComp : MeshComponents)
    {
        if (MeshComp && MeshComp->GetAnimClass())
        {
            UE_LOG(LogTemp, Warning, TEXT("Reactivating animation blueprint on component: %s"), *MeshComp->GetName());
            
            // Store the current animation class
            UClass* AnimClass = MeshComp->GetAnimClass();
            
            // Reset and reapply the animation blueprint
            MeshComp->SetAnimationMode(EAnimationMode::AnimationBlueprint);
            MeshComp->SetAnimInstanceClass(AnimClass);
            MeshComp->InitializeAnimScriptInstance(true);
            
            // Verify it's working
            if (UAnimInstance* AnimInstance = MeshComp->GetAnimInstance())
            {
                UE_LOG(LogTemp, Warning, TEXT("SUCCESS: Animation instance reactivated for %s"), *MeshComp->GetName());
            }
            else
            {
                UE_LOG(LogTemp, Error, TEXT("FAILED: Animation instance not created for %s"), *MeshComp->GetName());
            }
        }
    }
}

void ABloodreadBaseCharacter::ForceInitializeCharacterSystems()
{
    UE_LOG(LogTemp, Warning, TEXT("ForceInitializeCharacterSystems: Starting initialization for %s (Class: %s), CurrentClass=%d"), 
           *GetName(), *GetClass()->GetName(), (int32)CurrentCharacterClass);

    // Debug: Show the actual class hierarchy
    UClass* CurrentClass = GetClass();
    while (CurrentClass)
    {
        UE_LOG(LogTemp, Warning, TEXT("Class hierarchy: %s"), *CurrentClass->GetName());
        CurrentClass = CurrentClass->GetSuperClass();
        if (CurrentClass == ABloodreadBaseCharacter::StaticClass() || CurrentClass == ACharacter::StaticClass())
        {
            break;
        }
    }

    // Force character-specific initialization based on actual class type, not CurrentCharacterClass
    if (ABloodreadWarriorCharacter* WarriorChar = Cast<ABloodreadWarriorCharacter>(this))
    {
        UE_LOG(LogTemp, Warning, TEXT("✅ Detected WarriorCharacter - calling InitializeWarriorData"));
        WarriorChar->InitializeWarriorData();
        CurrentCharacterClass = ECharacterClass::Warrior;
    }
    else if (ABloodreadMageCharacter* MageChar = Cast<ABloodreadMageCharacter>(this))
    {
        UE_LOG(LogTemp, Warning, TEXT("✅ Detected MageCharacter - calling InitializeMageData"));
        MageChar->InitializeMageData();
        CurrentCharacterClass = ECharacterClass::Mage;
    }
    else if (ABloodreadRogueCharacter* RogueChar = Cast<ABloodreadRogueCharacter>(this))
    {
        UE_LOG(LogTemp, Warning, TEXT("✅ Detected RogueCharacter - calling InitializeRogueData"));
        RogueChar->InitializeRogueData();
        CurrentCharacterClass = ECharacterClass::Rogue;
    }
    else if (ABloodreadHealerCharacter* HealerChar = Cast<ABloodreadHealerCharacter>(this))
    {
        UE_LOG(LogTemp, Warning, TEXT("✅ Detected HealerCharacter - calling InitializeHealerData"));
        HealerChar->InitializeHealerData();
        CurrentCharacterClass = ECharacterClass::Healer;
    }
    else if (ABloodreadDragonCharacter* DragonChar = Cast<ABloodreadDragonCharacter>(this))
    {
        UE_LOG(LogTemp, Warning, TEXT("✅ Detected DragonCharacter - calling InitializeDragonData"));
        DragonChar->InitializeDragonData();
        CurrentCharacterClass = ECharacterClass::Dragon;
    }
    else
    {
        // If we can't cast to specific types, try to infer from the class name
        FString ClassName = GetClass()->GetName();
        UE_LOG(LogTemp, Warning, TEXT("❌ Direct cast failed, trying name-based detection for class: %s"), *ClassName);
        
        if (ClassName.Contains(TEXT("Warrior")))
        {
            UE_LOG(LogTemp, Warning, TEXT("🔧 Detected Warrior by name - setting class manually"));
            CurrentCharacterClass = ECharacterClass::Warrior;
            // Manually initialize warrior data since we can't cast
            InitializeWarriorDataManually();
        }
        else if (ClassName.Contains(TEXT("Mage")))
        {
            UE_LOG(LogTemp, Warning, TEXT("🔧 Detected Mage by name - setting class manually"));
            CurrentCharacterClass = ECharacterClass::Mage;
            // Manually initialize mage data since we can't cast
            InitializeMageDataManually();
        }
        else if (ClassName.Contains(TEXT("Rogue")))
        {
            UE_LOG(LogTemp, Warning, TEXT("🔧 Detected Rogue by name - setting class manually"));
            CurrentCharacterClass = ECharacterClass::Rogue;
            // Manually initialize rogue data since we can't cast
            InitializeRogueDataManually();
        }
        else if (ClassName.Contains(TEXT("Healer")))
        {
            UE_LOG(LogTemp, Warning, TEXT("🔧 Detected Healer by name - setting class manually"));
            CurrentCharacterClass = ECharacterClass::Healer;
            // Manually initialize healer data since we can't cast
            InitializeHealerDataManually();
        }
        else if (ClassName.Contains(TEXT("Dragon")))
        {
            UE_LOG(LogTemp, Warning, TEXT("🔧 Detected Dragon by name - setting class manually"));
            CurrentCharacterClass = ECharacterClass::Dragon;
            // Manually initialize dragon data since we can't cast
            InitializeDragonDataManually();
        }
        else if (ClassName.Contains(TEXT("BloodreadBaseCharacter")) || ClassName.Contains(TEXT("BloodreadBase")) || ClassName == TEXT("BloodreadBaseCharacter_C"))
        {
            // This is a base character spawned directly - default to Warrior for now
            UE_LOG(LogTemp, Warning, TEXT("🔧 Detected BloodreadBaseCharacter - defaulting to Warrior class"));
            CurrentCharacterClass = ECharacterClass::Warrior;
            InitializeWarriorDataManually();
        }
        else
        {
            UE_LOG(LogTemp, Error, TEXT("❌ Unknown character type - cannot determine class from name: %s"), *ClassName);
            UE_LOG(LogTemp, Warning, TEXT("Defaulting to Warrior as fallback"));
            CurrentCharacterClass = ECharacterClass::Warrior;
            InitializeWarriorDataManually();
        }
    }

    UE_LOG(LogTemp, Warning, TEXT("ForceInitializeCharacterSystems: Character class set to %d, proceeding with mesh application"), 
           (int32)CurrentCharacterClass);

    // Now that the character class is properly set, re-initialize from class data
    InitializeFromClassData(CharacterClassData);

    UE_LOG(LogTemp, Warning, TEXT("ForceInitializeCharacterSystems: Character systems initialization complete"));
}

void ABloodreadBaseCharacter::TestAllMeshPaths()
{
    UE_LOG(LogTemp, Warning, TEXT("=== TESTING ALL MESH PATHS ==="));
    
    // Test various possible mesh paths for each character
    TArray<FString> TestPaths = {
        // Kwang paths
        "/Game/ParagonKwang/Characters/Heroes/Kwang/Meshes/KwangSunrise.KwangSunrise",
        "/Game/ParagonKwang/Characters/Heroes/Kwang/Meshes/KwangSunrise",
        "/Game/ParagonKwang/Characters/Heroes/Kwang/KwangPlayerCharacter",
        
        // Shinbi paths
        "/Game/ParagonShinbi/Characters/Heroes/Shinbi/Meshes/Shinbi.Shinbi",
        "/Game/ParagonShinbi/Characters/Heroes/Shinbi/Meshes/Shinbi",
        "/Game/ParagonShinbi/Characters/Heroes/Shinbi/Skins/Tier_1/Shinbi_Dynasty/Meshes/ShinbiDynasty.ShinbiDynasty",
        "/Game/ParagonShinbi/Characters/Heroes/Shinbi/Skins/Tier_1/Shinbi_Dynasty/Meshes/ShinbiDynasty",
        "/Game/ParagonShinbi/Characters/Heroes/Shinbi/ShinbiPlayerCharacter",
        
        // Revenant paths
        "/Game/ParagonRevenant/Characters/Heroes/Revenant/Meshes/Revenant.Revenant",
        "/Game/ParagonRevenant/Characters/Heroes/Revenant/Meshes/Revenant",
        "/Game/ParagonRevenant/Characters/Heroes/Revenant/RevenantPlayerCharacter"
    };

    for (const FString& TestPath : TestPaths)
    {
        UE_LOG(LogTemp, Warning, TEXT("Testing path: %s"), *TestPath);
        
        // Try LoadObject first
        USkeletalMesh* TestMesh = LoadObject<USkeletalMesh>(nullptr, *TestPath);
        if (TestMesh)
        {
            UE_LOG(LogTemp, Warning, TEXT("✅ SUCCESS with LoadObject: %s -> %s"), *TestPath, *TestMesh->GetName());
        }
        else
        {
            // Try TSoftObjectPtr
            FSoftObjectPath SoftPath(TestPath);
            TSoftObjectPtr<USkeletalMesh> SoftPtr(SoftPath);
            USkeletalMesh* SoftMesh = SoftPtr.LoadSynchronous();
            
            if (SoftMesh)
            {
                UE_LOG(LogTemp, Warning, TEXT("✅ SUCCESS with TSoftObjectPtr: %s -> %s"), *TestPath, *SoftMesh->GetName());
            }
            else
            {
                UE_LOG(LogTemp, Error, TEXT("❌ FAILED: %s"), *TestPath);
            }
        }
    }
    
    UE_LOG(LogTemp, Warning, TEXT("=== MESH PATH TESTING COMPLETE ==="));
}

void ABloodreadBaseCharacter::InitializeWarriorDataManually()
{
    UE_LOG(LogTemp, Warning, TEXT("InitializeWarriorDataManually: Setting up warrior data"));
    
    // Set character class
    CurrentCharacterClass = ECharacterClass::Warrior;
    
    // Initialize warrior stats
    FCharacterStats WarriorStats;
    WarriorStats.MaxHealth = 150;  // High health
    WarriorStats.Strength = 18;    // High strength
    WarriorStats.Defense = 12;     // High defense
    WarriorStats.Speed = 8;        // Lower speed
    WarriorStats.Mana = 30;        // Low mana
    WarriorStats.CriticalChance = 0.15f;
    
    // Initialize warrior abilities
    FCharacterAbilityData PowerStrikeAbility;
    PowerStrikeAbility.Name = "Power Strike";
    PowerStrikeAbility.Description = "A devastating melee attack that deals massive damage and knockback";
    PowerStrikeAbility.Type = EAbilityType::Damage;
    PowerStrikeAbility.Cooldown = 8.0f;
    PowerStrikeAbility.ManaCost = 15;
    PowerStrikeAbility.Damage = 60.0f;
    PowerStrikeAbility.Duration = 0.0f;
    
    FCharacterAbilityData ChargeAbility;
    ChargeAbility.Name = "Charge Attack";
    ChargeAbility.Description = "Rush forward, dealing damage to all enemies in your path";
    ChargeAbility.Type = EAbilityType::Movement;
    ChargeAbility.Cooldown = 12.0f;
    ChargeAbility.ManaCost = 20;
    ChargeAbility.Damage = 40.0f;
    ChargeAbility.Duration = 1.0f;
    
    // Set up character class data
    CharacterClassData.CharacterClass = ECharacterClass::Warrior;
    CharacterClassData.ClassName = "Warrior";
    CharacterClassData.Description = "A mighty melee fighter with high health and devastating close-combat abilities";
    CharacterClassData.BaseStats = WarriorStats;
    CharacterClassData.Ability1 = PowerStrikeAbility;
    CharacterClassData.Ability2 = ChargeAbility;
    
    // Apply stats
    CurrentStats = WarriorStats;
    CurrentHealth = CurrentStats.MaxHealth;
    CurrentMana = CurrentStats.Mana;
    
    UE_LOG(LogTemp, Warning, TEXT("InitializeWarriorDataManually: Warrior data setup complete"));
}

void ABloodreadBaseCharacter::InitializeMageDataManually()
{
    UE_LOG(LogTemp, Warning, TEXT("InitializeMageDataManually: Setting up mage data"));
    
    // Set character class
    CurrentCharacterClass = ECharacterClass::Mage;
    
    // Initialize mage stats
    FCharacterStats MageStats;
    MageStats.MaxHealth = 80;      // Lower health
    MageStats.Strength = 8;        // Low strength
    MageStats.Defense = 6;         // Low defense
    MageStats.Speed = 12;          // Higher speed
    MageStats.Mana = 80;           // High mana
    MageStats.CriticalChance = 0.25f; // Higher crit chance
    
    // Initialize mage abilities
    FCharacterAbilityData FireballAbility;
    FireballAbility.Name = "Fireball";
    FireballAbility.Description = "Launch a fiery projectile that explodes on impact";
    FireballAbility.Type = EAbilityType::Damage;
    FireballAbility.Cooldown = 5.0f;
    FireballAbility.ManaCost = 25;
    FireballAbility.Damage = 45.0f;
    FireballAbility.Duration = 0.0f;
    
    FCharacterAbilityData TeleportAbility;
    TeleportAbility.Name = "Teleport";
    TeleportAbility.Description = "Instantly teleport to a target location";
    TeleportAbility.Type = EAbilityType::Movement;
    TeleportAbility.Cooldown = 8.0f;
    TeleportAbility.ManaCost = 30;
    TeleportAbility.Damage = 0.0f;
    TeleportAbility.Duration = 0.0f;
    
    // Set up character class data
    CharacterClassData.CharacterClass = ECharacterClass::Mage;
    CharacterClassData.ClassName = "Mage";
    CharacterClassData.Description = "A powerful spellcaster with high magical damage and mobility";
    CharacterClassData.BaseStats = MageStats;
    CharacterClassData.Ability1 = FireballAbility;
    CharacterClassData.Ability2 = TeleportAbility;
    
    // Apply stats
    CurrentStats = MageStats;
    CurrentHealth = CurrentStats.MaxHealth;
    CurrentMana = CurrentStats.Mana;
    
    UE_LOG(LogTemp, Warning, TEXT("InitializeMageDataManually: Mage data setup complete"));
}

void ABloodreadBaseCharacter::InitializeRogueDataManually()
{
    UE_LOG(LogTemp, Warning, TEXT("InitializeRogueDataManually: Setting up rogue data"));
    
    // Set character class
    CurrentCharacterClass = ECharacterClass::Rogue;
    
    // Initialize rogue stats
    FCharacterStats RogueStats;
    RogueStats.MaxHealth = 100;    // Medium health
    RogueStats.Strength = 14;      // Good strength
    RogueStats.Defense = 8;        // Medium defense
    RogueStats.Speed = 16;         // High speed
    RogueStats.Mana = 50;          // Medium mana
    RogueStats.CriticalChance = 0.35f; // Highest crit chance
    
    // Initialize rogue abilities
    FCharacterAbilityData ShadowStrikeAbility;
    ShadowStrikeAbility.Name = "Shadow Strike";
    ShadowStrikeAbility.Description = "A quick strike from the shadows with increased critical chance";
    ShadowStrikeAbility.Type = EAbilityType::Damage;
    ShadowStrikeAbility.Cooldown = 6.0f;
    ShadowStrikeAbility.ManaCost = 20;
    ShadowStrikeAbility.Damage = 50.0f;
    ShadowStrikeAbility.Duration = 0.0f;
    
    FCharacterAbilityData StealthAbility;
    StealthAbility.Name = "Stealth";
    StealthAbility.Description = "Become invisible for a short duration, increasing damage and movement speed";
    StealthAbility.Type = EAbilityType::Buff;
    StealthAbility.Cooldown = 15.0f;
    StealthAbility.ManaCost = 25;
    StealthAbility.Damage = 0.0f;
    StealthAbility.Duration = 5.0f;
    
    // Set up character class data
    CharacterClassData.CharacterClass = ECharacterClass::Rogue;
    CharacterClassData.ClassName = "Rogue";
    CharacterClassData.Description = "A swift assassin with high critical chance and stealth abilities";
    CharacterClassData.BaseStats = RogueStats;
    CharacterClassData.Ability1 = ShadowStrikeAbility;
    CharacterClassData.Ability2 = StealthAbility;
    
    // Apply stats
    CurrentStats = RogueStats;
    CurrentHealth = CurrentStats.MaxHealth;
    CurrentMana = CurrentStats.Mana;
    
    UE_LOG(LogTemp, Warning, TEXT("InitializeRogueDataManually: Rogue data setup complete"));
}

void ABloodreadBaseCharacter::InitializeHealerDataManually()
{
    UE_LOG(LogTemp, Warning, TEXT("InitializeHealerDataManually: Setting up healer data"));
    
    // Set character class
    CurrentCharacterClass = ECharacterClass::Healer;
    
    // Initialize healer stats
    FCharacterStats HealerStats;
    HealerStats.MaxHealth = 120;    // Good health
    HealerStats.Strength = 6;       // Low strength
    HealerStats.Defense = 8;        // Medium defense
    HealerStats.Speed = 10;         // Medium speed
    HealerStats.Mana = 100;         // High mana
    HealerStats.CriticalChance = 0.1f; // Low crit chance
    
    // Initialize healer abilities
    FCharacterAbilityData BondAbility;
    BondAbility.Name = "Bond";
    BondAbility.Description = "Heal a teammate for 50% of their missing health";
    BondAbility.Type = EAbilityType::Heal;
    BondAbility.Cooldown = 30.0f;
    BondAbility.ManaCost = 100;
    BondAbility.Damage = 0.0f;
    BondAbility.Duration = 0.0f;
    
    FCharacterAbilityData RegenerationAbility;
    RegenerationAbility.Name = "Regeneration";
    RegenerationAbility.Description = "Grant team-wide health regeneration for a duration";
    RegenerationAbility.Type = EAbilityType::Heal;
    RegenerationAbility.Cooldown = 15.0f;
    RegenerationAbility.ManaCost = 50;
    RegenerationAbility.Damage = 0.0f;
    RegenerationAbility.Duration = 10.0f;
    
    // Set up character class data
    CharacterClassData.CharacterClass = ECharacterClass::Healer;
    CharacterClassData.ClassName = "Healer";
    CharacterClassData.Description = "A support character specializing in healing and team regeneration";
    CharacterClassData.BaseStats = HealerStats;
    CharacterClassData.Ability1 = BondAbility;
    CharacterClassData.Ability2 = RegenerationAbility;
    
    // Apply stats
    CurrentStats = HealerStats;
    CurrentHealth = CurrentStats.MaxHealth;
    CurrentMana = CurrentStats.Mana;
    
    UE_LOG(LogTemp, Warning, TEXT("InitializeHealerDataManually: Healer data setup complete"));
}

void ABloodreadBaseCharacter::InitializeDragonDataManually()
{
    UE_LOG(LogTemp, Warning, TEXT("InitializeDragonDataManually: Setting up dragon data"));
    
    // Set character class
    CurrentCharacterClass = ECharacterClass::Dragon;
    
    // Initialize dragon stats
    FCharacterStats DragonStats;
    DragonStats.MaxHealth = 100;    // Medium health
    DragonStats.Strength = 6;       // Base strength (scalable)
    DragonStats.Defense = 6;        // Medium defense
    DragonStats.Speed = 12;         // Good speed
    DragonStats.Mana = 80;          // Good mana
    DragonStats.CriticalChance = 0.15f; // Medium crit chance
    
    // Initialize dragon abilities
    FCharacterAbilityData AscentAbility;
    AscentAbility.Name = "Ascent";
    AscentAbility.Description = "Two-part leap attack - first press to leap up, second to blitz down with damage";
    AscentAbility.Type = EAbilityType::Movement;
    AscentAbility.Cooldown = 30.0f;
    AscentAbility.ManaCost = 100;
    AscentAbility.Damage = 50.0f;
    AscentAbility.Duration = 0.0f;
    
    FCharacterAbilityData KingsGreedAbility;
    KingsGreedAbility.Name = "Kings Greed";
    KingsGreedAbility.Description = "Gain movement speed and damage scaling based on successful hits";
    KingsGreedAbility.Type = EAbilityType::Buff;
    KingsGreedAbility.Cooldown = 30.0f;
    KingsGreedAbility.ManaCost = 50;
    KingsGreedAbility.Damage = 0.0f;
    KingsGreedAbility.Duration = 15.0f;
    
    // Set up character class data
    CharacterClassData.CharacterClass = ECharacterClass::Dragon;
    CharacterClassData.ClassName = "Dragon";
    CharacterClassData.Description = "A powerful aerial combatant with leap attacks and scalable damage abilities";
    CharacterClassData.BaseStats = DragonStats;
    CharacterClassData.Ability1 = AscentAbility;
    CharacterClassData.Ability2 = KingsGreedAbility;
    
    // Apply stats
    CurrentStats = DragonStats;
    CurrentHealth = CurrentStats.MaxHealth;
    CurrentMana = CurrentStats.Mana;
    
    UE_LOG(LogTemp, Warning, TEXT("InitializeDragonDataManually: Dragon data setup complete"));
}

// Health system helper functions
void ABloodreadBaseCharacter::DealDamage(float DamageAmount)
{
    UE_LOG(LogTemp, Warning, TEXT("*** WARNING: DealDamage called directly - no knockback applied! Consider using TakeCustomDamage instead ***"));
    
    int32 OldHealth = CurrentHealth;
    int32 IntDamage = FMath::RoundToInt(DamageAmount);
    CurrentHealth = FMath::Max(0, CurrentHealth - IntDamage);
    
    OnHealthChanged(OldHealth, CurrentHealth);
    
    if (CurrentHealth <= 0)
    {
        UE_LOG(LogTemp, Warning, TEXT("Character died!"));
        // Handle death logic here
    }
    else
    {
        // Flash red on damage
        FlashRed();
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Character took %.1f damage. Health: %d/%d"), DamageAmount, CurrentHealth, CurrentStats.MaxHealth);
}

void ABloodreadBaseCharacter::HealCharacter(float HealAmount)
{
    int32 OldHealth = CurrentHealth;
    int32 IntHeal = FMath::RoundToInt(HealAmount);
    CurrentHealth = FMath::Min(CurrentStats.MaxHealth, CurrentHealth + IntHeal);
    
    OnHealthChanged(OldHealth, CurrentHealth);
    
    // Update health bar display
    UpdateHealthDisplay();
    
    UE_LOG(LogTemp, Warning, TEXT("Character healed for %.1f. Health: %d/%d"), HealAmount, CurrentHealth, CurrentStats.MaxHealth);
}

// Combined damage and knockback function - USE THIS for attacks with knockback
void ABloodreadBaseCharacter::DealDamageWithKnockback(float DamageAmount, FVector KnockbackDirection, float KnockbackForce, ABloodreadBaseCharacter* Attacker)
{
    UE_LOG(LogTemp, Error, TEXT("🚨 DealDamageWithKnockback called on %s - Damage: %.1f, Knockback: %.1f, Attacker: %s"), 
           *GetName(), DamageAmount, KnockbackForce, Attacker ? *Attacker->GetName() : TEXT("None"));
    UE_LOG(LogTemp, Error, TEXT("🚨 Target character class: %s"), *GetClass()->GetName());
    
    // Apply damage first
    bool bDamageTaken = TakeCustomDamage(FMath::RoundToInt(DamageAmount), Attacker);
    
    if (bDamageTaken)
    {
        // Apply knockback after damage
        UE_LOG(LogTemp, Error, TEXT("🚨 Damage taken successfully, now applying knockback..."));
        ApplyKnockback(KnockbackDirection, KnockbackForce);
        UE_LOG(LogTemp, Error, TEXT("🚨 Both damage and knockback should be applied now"));
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("Damage was not applied, skipping knockback"));
    }
}

float ABloodreadBaseCharacter::GetHealthPercent() const
{
    if (CurrentStats.MaxHealth <= 0) return 0.0f;
    return (float)CurrentHealth / (float)CurrentStats.MaxHealth;
}

FString ABloodreadBaseCharacter::GetHealthDisplayText() const
{
    return FString::Printf(TEXT("%d/%d"), CurrentHealth, CurrentStats.MaxHealth);
}

bool ABloodreadBaseCharacter::UseMana(int32 ManaAmount)
{
    if (CurrentMana >= ManaAmount)
    {
        int32 OldMana = CurrentMana;
        CurrentMana -= ManaAmount;
        OnManaChanged(OldMana, CurrentMana);
        return true;
    }
    return false;
}

void ABloodreadBaseCharacter::RestoreMana(int32 ManaAmount)
{
    int32 OldMana = CurrentMana;
    CurrentMana = FMath::Min(CurrentStats.Mana, CurrentMana + ManaAmount);
    OnManaChanged(OldMana, CurrentMana);
}

float ABloodreadBaseCharacter::GetManaPercentage() const
{
    if (CurrentStats.Mana <= 0) return 0.0f;
    return (float)CurrentMana / (float)CurrentStats.Mana;
}

void ABloodreadBaseCharacter::UseAbility1()
{
    if (CanUseAbility1())
    {
        if (UseMana(CharacterClassData.Ability1.ManaCost))
        {
            CharacterClassData.Ability1.CooldownRemaining = CharacterClassData.Ability1.Cooldown;
            PlayAbility1Animation(); // Play animation first
            OnAbility1Used();
            UE_LOG(LogTemp, Warning, TEXT("Used Ability 1: %s"), *CharacterClassData.Ability1.Name);
        }
        else
        {
            UE_LOG(LogTemp, Warning, TEXT("Not enough mana for Ability 1"));
        }
    }
}

void ABloodreadBaseCharacter::UseAbility2()
{
    if (CanUseAbility2())
    {
        if (UseMana(CharacterClassData.Ability2.ManaCost))
        {
            CharacterClassData.Ability2.CooldownRemaining = CharacterClassData.Ability2.Cooldown;
            PlayAbility2Animation(); // Play animation first
            OnAbility2Used();
            UE_LOG(LogTemp, Warning, TEXT("Used Ability 2: %s"), *CharacterClassData.Ability2.Name);
        }
        else
        {
            UE_LOG(LogTemp, Warning, TEXT("Not enough mana for Ability 2"));
        }
    }
}

bool ABloodreadBaseCharacter::CanUseAbility1() const
{
    return CharacterClassData.Ability1.CooldownRemaining <= 0.0f && CurrentMana >= CharacterClassData.Ability1.ManaCost;
}

bool ABloodreadBaseCharacter::CanUseAbility2() const
{
    return CharacterClassData.Ability2.CooldownRemaining <= 0.0f && CurrentMana >= CharacterClassData.Ability2.ManaCost;
}

float ABloodreadBaseCharacter::GetAbility1CooldownPercentage() const
{
    if (CharacterClassData.Ability1.Cooldown <= 0.0f) return 0.0f;
    return FMath::Clamp(CharacterClassData.Ability1.CooldownRemaining / CharacterClassData.Ability1.Cooldown, 0.0f, 1.0f);
}

float ABloodreadBaseCharacter::GetAbility2CooldownPercentage() const
{
    if (CharacterClassData.Ability2.Cooldown <= 0.0f) return 0.0f;
    return FMath::Clamp(CharacterClassData.Ability2.CooldownRemaining / CharacterClassData.Ability2.Cooldown, 0.0f, 1.0f);
}

void ABloodreadBaseCharacter::SetCameraPosition(FVector NewRelativeLocation)
{
    if (FirstPersonCamera)
    {
        FirstPersonCamera->SetRelativeLocation(NewRelativeLocation);
        CustomMeshCameraOffset = NewRelativeLocation;
        UE_LOG(LogTemp, Warning, TEXT("Camera position set to: %s"), *NewRelativeLocation.ToString());
    }
}

void ABloodreadBaseCharacter::AdjustCameraForMesh(float HeightOffset, float ForwardOffset, float RightOffset)
{
    FVector BaseOffset = FVector(25.0f, 1.75f, 64.0f); // Default position - just in front of face
    FVector AdjustedOffset = BaseOffset + FVector(ForwardOffset, RightOffset, HeightOffset);
    
    // Apply mesh scale multiplier
    AdjustedOffset *= MeshScaleMultiplier;
    
    SetCameraPosition(AdjustedOffset);
    
    UE_LOG(LogTemp, Warning, TEXT("Camera adjusted for custom mesh - Height: %.1f, Forward: %.1f, Right: %.1f"), 
           HeightOffset, ForwardOffset, RightOffset);
}

FVector ABloodreadBaseCharacter::GetCurrentCameraPosition() const
{
    return FirstPersonCamera ? FirstPersonCamera->GetRelativeLocation() : FVector::ZeroVector;
}

void ABloodreadBaseCharacter::Move(const FInputActionValue& Value)
{
    FVector2D MovementVector = Value.Get<FVector2D>();

    if (Controller != nullptr)
    {
        // Add movement input
        AddMovementInput(GetActorForwardVector(), MovementVector.Y);
        AddMovementInput(GetActorRightVector(), MovementVector.X);
    }
}

void ABloodreadBaseCharacter::Look(const FInputActionValue& Value)
{
    FVector2D LookAxisVector = Value.Get<FVector2D>();

    if (Controller != nullptr)
    {
        // Add yaw and pitch input to controller
        AddControllerYawInput(LookAxisVector.X);
        AddControllerPitchInput(LookAxisVector.Y);
    }
}

void ABloodreadBaseCharacter::Attack()
{
    // Use the new universal targeting attack system
    AttackTarget();
}

APracticeDummy* ABloodreadBaseCharacter::GetCrosshairTarget()
{
    // Get camera location and forward direction
    FVector CameraLocation;
    FRotator CameraRotation;
    GetActorEyesViewPoint(CameraLocation, CameraRotation);
    FVector CameraForward = CameraRotation.Vector();
    
    // Perform raycast from center of screen
    FVector TraceStart = CameraLocation;
    FVector TraceEnd = TraceStart + (CameraForward * 1000.0f); // 10 meter range
    
    FHitResult HitResult;
    FCollisionQueryParams CollisionParams;
    CollisionParams.AddIgnoredActor(this);
    
    bool bHit = GetWorld()->LineTraceSingleByChannel(
        HitResult,
        TraceStart,
        TraceEnd,
        ECollisionChannel::ECC_Visibility,
        CollisionParams
    );
    
    if (bHit && HitResult.GetActor())
    {
        // Check if hit actor is a practice dummy
        APracticeDummy* HitDummy = Cast<APracticeDummy>(HitResult.GetActor());
        if (HitDummy && HitDummy->IsAlive())
        {
            UE_LOG(LogTemp, Log, TEXT("GetCrosshairTarget: Found dummy %s at distance %.1f"), 
                   *HitDummy->GetName(), HitResult.Distance);
            return HitDummy;
        }
    }
    
    return nullptr;
}

FTargetableActor ABloodreadBaseCharacter::GetCrosshairTargetActor()
{
    // Get camera location and forward direction
    FVector CameraLocation;
    FRotator CameraRotation;
    GetActorEyesViewPoint(CameraLocation, CameraRotation);
    FVector CameraForward = CameraRotation.Vector();
    
    UE_LOG(LogTemp, Warning, TEXT("GetCrosshairTargetActor: Camera at %s, looking %s"), 
           *CameraLocation.ToString(), *CameraForward.ToString());
    
    // Perform raycast from center of screen
    FVector TraceStart = CameraLocation;
    FVector TraceEnd = TraceStart + (CameraForward * 1000.0f); // 10 meter range
    
    FHitResult HitResult;
    FCollisionQueryParams CollisionParams;
    CollisionParams.AddIgnoredActor(this);
    
    UE_LOG(LogTemp, Warning, TEXT("GetCrosshairTargetActor: Tracing from %s to %s"), 
           *TraceStart.ToString(), *TraceEnd.ToString());
    
    // Try multiple collision channels to see what works
    bool bHit = GetWorld()->LineTraceSingleByChannel(
        HitResult,
        TraceStart,
        TraceEnd,
        ECollisionChannel::ECC_Visibility,
        CollisionParams
    );
    
    UE_LOG(LogTemp, Warning, TEXT("GetCrosshairTargetActor: Visibility trace hit = %s"), 
           bHit ? TEXT("TRUE") : TEXT("FALSE"));
    
    if (!bHit)
    {
        // Try with Pawn collision channel
        bHit = GetWorld()->LineTraceSingleByChannel(
            HitResult,
            TraceStart,
            TraceEnd,
            ECollisionChannel::ECC_Pawn,
            CollisionParams
        );
        UE_LOG(LogTemp, Warning, TEXT("GetCrosshairTargetActor: Pawn trace hit = %s"), 
               bHit ? TEXT("TRUE") : TEXT("FALSE"));
    }
    
    if (!bHit)
    {
        // Try with WorldDynamic collision channel
        bHit = GetWorld()->LineTraceSingleByChannel(
            HitResult,
            TraceStart,
            TraceEnd,
            ECollisionChannel::ECC_WorldDynamic,
            CollisionParams
        );
        UE_LOG(LogTemp, Warning, TEXT("GetCrosshairTargetActor: WorldDynamic trace hit = %s"), 
               bHit ? TEXT("TRUE") : TEXT("FALSE"));
    }
    
    if (bHit && HitResult.GetActor())
    {
        AActor* HitActor = HitResult.GetActor();
        
        UE_LOG(LogTemp, Warning, TEXT("GetCrosshairTargetActor: Hit actor %s of class %s at distance %.1f"), 
               *HitActor->GetName(), *HitActor->GetClass()->GetName(), HitResult.Distance);
        
        // Check if it's a practice dummy
        APracticeDummy* HitDummy = Cast<APracticeDummy>(HitActor);
        if (HitDummy && HitDummy->IsAlive())
        {
            UE_LOG(LogTemp, Log, TEXT("GetCrosshairTargetActor: Found dummy %s at distance %.1f"), 
                   *HitDummy->GetName(), HitResult.Distance);
            FTargetableActor Target(HitActor);
            Target.bIsDummy = true;
            Target.bIsPlayer = false;
            return Target;
        }
        
        // Check if it's another player character (including Blueprint-derived characters)
        ABloodreadBaseCharacter* HitCharacter = Cast<ABloodreadBaseCharacter>(HitActor);
        
        UE_LOG(LogTemp, Warning, TEXT("GetCrosshairTargetActor: Trying to cast %s to ABloodreadBaseCharacter - Result: %s"), 
               *HitActor->GetClass()->GetName(), HitCharacter ? TEXT("SUCCESS") : TEXT("FAILED"));
        
        // Additional check: Is it derived from ABloodreadBaseCharacter but not casting?
        if (!HitCharacter)
        {
            bool bIsChildOfBaseCharacter = HitActor->IsA<ABloodreadBaseCharacter>();
            UE_LOG(LogTemp, Warning, TEXT("GetCrosshairTargetActor: IsA<ABloodreadBaseCharacter> check: %s"), 
                   bIsChildOfBaseCharacter ? TEXT("TRUE") : TEXT("FALSE"));
                   
            // Try direct inheritance check
            UClass* ActorClass = HitActor->GetClass();
            UClass* BaseCharacterClass = ABloodreadBaseCharacter::StaticClass();
            bool bInheritsFromBase = ActorClass->IsChildOf(BaseCharacterClass);
            UE_LOG(LogTemp, Warning, TEXT("GetCrosshairTargetActor: IsChildOf ABloodreadBaseCharacter: %s"), 
                   bInheritsFromBase ? TEXT("TRUE") : TEXT("FALSE"));
        }
        
        if (HitCharacter && HitCharacter != this && HitCharacter->GetIsAlive())
        {
            UE_LOG(LogTemp, Log, TEXT("GetCrosshairTargetActor: Found player character %s at distance %.1f"), 
                   *HitCharacter->GetName(), HitResult.Distance);
            FTargetableActor Target(HitActor);
            Target.bIsPlayer = true;
            Target.bIsDummy = false;
            return Target;
        }
        
        // FALLBACK: Check if it's a Blueprint-derived BloodreadBaseCharacter using inheritance
        if (!HitCharacter && HitActor->IsA<ABloodreadBaseCharacter>())
        {
            UE_LOG(LogTemp, Warning, TEXT("GetCrosshairTargetActor: Found Blueprint-derived BloodreadBaseCharacter %s using IsA check"), 
                   *HitActor->GetName());
            FTargetableActor Target(HitActor);
            Target.bIsPlayer = true;
            Target.bIsDummy = false;
            return Target;
        }
        
        // If we hit something but it's not a targetable type, log it
        UE_LOG(LogTemp, Warning, TEXT("GetCrosshairTargetActor: Hit %s but it's not targetable (not BloodreadBaseCharacter or PracticeDummy)"), 
               *HitActor->GetClass()->GetName());
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("GetCrosshairTargetActor: No hit detected with any collision channel"));
    }
    
    return FTargetableActor();
}

void ABloodreadBaseCharacter::AttackTarget()
{
    if (!GetIsAlive()) return;

    UE_LOG(LogTemp, Warning, TEXT("AttackTarget called - using universal crosshair targeting"));

    // Use new universal targeting system
    FTargetableActor Target = GetCrosshairTargetActor();
    
    if (!Target.Actor)
    {
        UE_LOG(LogTemp, Warning, TEXT("AttackTarget: No target found in crosshair"));
        return;
    }

    // Check if target is within attack range
    float Distance = FVector::Dist(GetActorLocation(), Target.Actor->GetActorLocation());
    if (Distance > BasicAttackRange)
    {
        UE_LOG(LogTemp, Warning, TEXT("AttackTarget: Target too far (%.1f > %.1f)"), Distance, BasicAttackRange);
        return;
    }

    UE_LOG(LogTemp, Warning, TEXT("AttackTarget: Attacking target at distance %.1f"), Distance);
    
    // Calculate damage with strength bonus
    int32 TotalDamage = BasicAttackDamage + CurrentStats.Strength;
    
    // Calculate knockback direction
    FVector KnockbackDirection = (Target.Actor->GetActorLocation() - GetActorLocation()).GetSafeNormal();
    KnockbackDirection.Z = 0.2f; // Add slight upward force

    if (Target.bIsDummy)
    {
        // Target is a practice dummy
        APracticeDummy* TargetDummy = Cast<APracticeDummy>(Target.Actor);
        if (TargetDummy && TargetDummy->IsAlive())
        {
            TargetDummy->TakeCustomDamage(TotalDamage, nullptr);
            TargetDummy->ApplyKnockback(KnockbackDirection, BasicAttackKnockbackForce);
            UE_LOG(LogTemp, Log, TEXT("Character dealt %d damage to practice dummy"), TotalDamage);
        }
    }
    else if (Target.bIsPlayer)
    {
        // Target is another player character
        ABloodreadBaseCharacter* TargetPlayer = Cast<ABloodreadBaseCharacter>(Target.Actor);
        if (TargetPlayer && TargetPlayer->GetIsAlive())
        {
            // Apply damage to player
            TargetPlayer->TakeCustomDamage(TotalDamage, this);
            
            // Apply knockback to player
            TargetPlayer->ApplyKnockback(KnockbackDirection, BasicAttackKnockbackForce);
            
            UE_LOG(LogTemp, Log, TEXT("Character dealt %d damage to player character %s"), TotalDamage, *TargetPlayer->GetName());
        }
    }
    
    // Restore mana on successful attack
    RestoreMana(10);
    UE_LOG(LogTemp, Log, TEXT("Character restored 10 mana from attack - Current mana: %d/%d"), CurrentMana, CurrentStats.Mana);
    
    // Play basic attack animation
    PlayBasicAttackAnimation();
    
    // Call Blueprint event for animation/effects
    OnBasicAttack();
    
    // Also call the new OnAttackHit event for animation system
    OnAttackHit();
}

void ABloodreadBaseCharacter::ApplyKnockback(FVector KnockbackDirection, float Force)
{
    if (!GetIsAlive()) 
    {
        UE_LOG(LogTemp, Warning, TEXT("Knockback: Character is not alive, ignoring knockback"));
        return;
    }

    UE_LOG(LogTemp, Error, TEXT("🚨 === KNOCKBACK DEBUG START === Target: %s"), *GetName());
    UE_LOG(LogTemp, Error, TEXT("🚨 Knockback: Original direction: %s, Force: %.2f"), *KnockbackDirection.ToString(), Force);
    UE_LOG(LogTemp, Error, TEXT("🚨 Character Controller: %s, Class: %s"), 
           GetController() ? *GetController()->GetName() : TEXT("NONE"), 
           *GetClass()->GetName());
    UE_LOG(LogTemp, Error, TEXT("🚨 Controller Type: %s"), 
           GetController() ? *GetController()->GetClass()->GetName() : TEXT("NONE"));
    UE_LOG(LogTemp, Error, TEXT("🚨 Is Possessed: %s, Has Player Controller: %s"), 
           GetController() ? TEXT("YES") : TEXT("NO"),
           Cast<APlayerController>(GetController()) ? TEXT("YES") : TEXT("NO"));
    
    // Add upward component to knockback
    KnockbackDirection.Z += 0.3f;
    KnockbackDirection = KnockbackDirection.GetSafeNormal();
    
    UE_LOG(LogTemp, Warning, TEXT("Knockback: Normalized direction: %s"), *KnockbackDirection.ToString());
    
    // Calculate impulse
    FVector Impulse = KnockbackDirection * Force;
    UE_LOG(LogTemp, Warning, TEXT("Knockback: Calculated impulse: %s"), *Impulse.ToString());
    
    // Get character movement component info
    UCharacterMovementComponent* MovementComp = GetCharacterMovement();
    if (!MovementComp)
    {
        UE_LOG(LogTemp, Error, TEXT("Knockback: CharacterMovementComponent is NULL!"));
        return;
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Knockback: Movement comp - Mass: %.2f, MaxWalkSpeed: %.2f, GroundFriction: %.2f"), 
           MovementComp->Mass, MovementComp->MaxWalkSpeed, MovementComp->GroundFriction);
    UE_LOG(LogTemp, Warning, TEXT("Knockback: Movement mode: %d, IsOnGround: %s"), 
           (int32)MovementComp->MovementMode, MovementComp->IsMovingOnGround() ? TEXT("true") : TEXT("false"));
    
    // Try multiple knockback methods for reliability
    bool bKnockbackApplied = false;
    
    // CRITICAL: Disable AI input temporarily to prevent interference
    if (AController* CurrentController = GetController())
    {
        // Disable input for AI controllers during knockback
        if (!Cast<APlayerController>(CurrentController))
        {
            UE_LOG(LogTemp, Error, TEXT("🚨 DISABLING AI INPUT - Detected AI Controller: %s"), *CurrentController->GetName());
            CurrentController->SetIgnoreMoveInput(true);
            
            // Re-enable input after knockback duration
            GetWorld()->GetTimerManager().SetTimer(KnockbackRecoveryTimerHandle, [this]()
            {
                if (AController* Controller = GetController())
                {
                    Controller->SetIgnoreMoveInput(false);
                    UE_LOG(LogTemp, Warning, TEXT("🚨 AI INPUT RE-ENABLED after knockback"));
                }
            }, 1.0f, false);
        }
    }

    // Method 1: AddImpulse with mass override
    UE_LOG(LogTemp, Warning, TEXT("Knockback: Trying Method 1 - AddImpulse with mass override"));
    MovementComp->AddImpulse(Impulse, true); // true = ignore mass
    bKnockbackApplied = true;
    
    // Method 2: If character is on ground, also try Launch
    if (MovementComp->IsMovingOnGround())
    {
        UE_LOG(LogTemp, Warning, TEXT("Knockback: Trying Method 2 - LaunchCharacter (on ground)"));
        FVector LaunchVelocity = KnockbackDirection * Force * 0.8f; // Slightly reduced for Launch
        LaunchCharacter(LaunchVelocity, false, false);
        bKnockbackApplied = true;
    }
    
    // Method 3: Direct velocity manipulation as backup
    FVector CurrentVelocity = MovementComp->Velocity;
    FVector NewVelocity = CurrentVelocity + (KnockbackDirection * Force * 0.5f);
    UE_LOG(LogTemp, Warning, TEXT("Knockback: Current velocity: %s, Adding: %s"), 
           *CurrentVelocity.ToString(), *(KnockbackDirection * Force * 0.5f).ToString());
    
    // Method 4: Force velocity directly
    UE_LOG(LogTemp, Error, TEXT("🚨 Method 4: FORCING velocity directly"));
    MovementComp->Velocity = KnockbackDirection * Force;
    UE_LOG(LogTemp, Error, TEXT("🚨 Forced velocity to: %s"), *MovementComp->Velocity.ToString());
    
    // Method 5: Try stopping movement first, then applying force
    UE_LOG(LogTemp, Error, TEXT("🚨 Method 5: Stopping movement then applying force"));
    MovementComp->StopMovementImmediately();
    MovementComp->AddImpulse(Impulse * 2.0f, true); // Double the force
    
    // Method 6: Use SetActorLocation for instant displacement (as last resort)
    FVector DisplacementVector = KnockbackDirection * 50.0f; // 50 units displacement
    FVector NewLocation = GetActorLocation() + DisplacementVector;
    UE_LOG(LogTemp, Error, TEXT("🚨 Method 6: Attempting displacement from %s to %s"), 
           *GetActorLocation().ToString(), *NewLocation.ToString());
    SetActorLocation(NewLocation, true);
    
    // Check if position actually changed
    FVector ActualNewLocation = GetActorLocation();
    float DisplacementDistance = FVector::Dist(GetActorLocation(), NewLocation);
    UE_LOG(LogTemp, Error, TEXT("🚨 Displacement result: Target=%s, Actual=%s, Distance=%.2f"), 
           *NewLocation.ToString(), *ActualNewLocation.ToString(), DisplacementDistance);
    
    // Store velocity to check if it changes
    GetWorld()->GetTimerManager().SetTimerForNextTick([this, MovementComp, Impulse]()
    {
        UE_LOG(LogTemp, Warning, TEXT("Knockback: Post-impulse velocity: %s"), *MovementComp->Velocity.ToString());
        UE_LOG(LogTemp, Warning, TEXT("=== KNOCKBACK DEBUG END ==="));
    });
    
    OnKnockbackApplied(KnockbackDirection, Force);
    UE_LOG(LogTemp, Warning, TEXT("Knockback: Applied with force %.2f - Success: %s"), Force, bKnockbackApplied ? TEXT("YES") : TEXT("NO"));
}

bool ABloodreadBaseCharacter::TakeCustomDamage(int32 Damage, ABloodreadBaseCharacter* Attacker)
{
    if (!GetIsAlive() || Damage <= 0) return false;

    int32 PreviousHealth = CurrentHealth;
    
    // Apply damage
    CurrentHealth = FMath::Max(0, CurrentHealth - Damage);
    
    // Call Blueprint event
    OnTakeDamage(Damage, Attacker);
    
    // Flash red effect
    FlashRed();
    
    UE_LOG(LogTemp, Warning, TEXT("Base Character took %d damage! Health: %d -> %d"), 
           Damage, PreviousHealth, CurrentHealth);
    
    // Call virtual health changed callback
    OnHealthChanged(PreviousHealth, CurrentHealth);
    
    // Update health bar display
    UpdateHealthDisplay();
    
    return true;
}

void ABloodreadBaseCharacter::UpdateAbilityCooldowns(float DeltaTime)
{
    if (CharacterClassData.Ability1.CooldownRemaining > 0.0f)
    {
        CharacterClassData.Ability1.CooldownRemaining -= DeltaTime;
        CharacterClassData.Ability1.CooldownRemaining = FMath::Max(0.0f, CharacterClassData.Ability1.CooldownRemaining);
    }

    if (CharacterClassData.Ability2.CooldownRemaining > 0.0f)
    {
        CharacterClassData.Ability2.CooldownRemaining -= DeltaTime;
        CharacterClassData.Ability2.CooldownRemaining = FMath::Max(0.0f, CharacterClassData.Ability2.CooldownRemaining);
    }
}

// Traditional Input System Functions
void ABloodreadBaseCharacter::MoveForward(float Value)
{
    if ((Controller != nullptr) && (Value != 0.0f))
    {
        // Find out which way is forward based on camera/controller rotation
        const FRotator Rotation = Controller->GetControlRotation();
        const FRotator YawRotation(0, Rotation.Yaw, 0);

        // Get forward vector (ignore pitch for ground movement)
        const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
        AddMovementInput(Direction, Value);
    }
}

void ABloodreadBaseCharacter::MoveRight(float Value)
{
    if ((Controller != nullptr) && (Value != 0.0f))
    {
        // Find out which way is right based on camera/controller rotation
        const FRotator Rotation = Controller->GetControlRotation();
        const FRotator YawRotation(0, Rotation.Yaw, 0);

        // Get right vector (ignore pitch for ground movement)
        const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);
        AddMovementInput(Direction, Value);
    }
}

void ABloodreadBaseCharacter::Turn(float Value)
{
    AddControllerYawInput(Value);
}

void ABloodreadBaseCharacter::LookUp(float Value)
{
    AddControllerPitchInput(Value);
}

void ABloodreadBaseCharacter::TestDamage()
{
    UE_LOG(LogTemp, Warning, TEXT("Test Damage called!"));
    DealDamage(10.0f);
}

void ABloodreadBaseCharacter::TestHeal()
{
    UE_LOG(LogTemp, Warning, TEXT("Test Heal called!"));
    HealCharacter(15.0f);
}

FString ABloodreadBaseCharacter::GetHealthText() const
{
    return FString::Printf(TEXT("%d/%d"), CurrentHealth, CurrentStats.MaxHealth);
}

FString ABloodreadBaseCharacter::GetAbility1Name() const
{
    return CharacterClassData.Ability1.Name;
}

FString ABloodreadBaseCharacter::GetAbility2Name() const
{
    return CharacterClassData.Ability2.Name;
}

float ABloodreadBaseCharacter::GetAbility1RemainingCooldown() const
{
    return CharacterClassData.Ability1.CooldownRemaining;
}

float ABloodreadBaseCharacter::GetAbility2RemainingCooldown() const
{
    return CharacterClassData.Ability2.CooldownRemaining;
}

void ABloodreadBaseCharacter::SetHealthBarWidget(UUserWidget* Widget)
{
    if (!Widget)
    {
        UE_LOG(LogTemp, Error, TEXT("*** SetHealthBarWidget: Widget parameter is NULL! ***"));
        CurrentHealthBarWidget = nullptr;
        return;
    }
    
    // Validate widget is still valid before assigning
    if (IsValid(Widget))
    {
        CurrentHealthBarWidget = Widget;
        UE_LOG(LogTemp, Warning, TEXT("Player Character health bar widget set successfully: %s"), 
               *Widget->GetClass()->GetName());
               
        // Try to cast to UniversalHealthBarWidget for enhanced functionality
        UUniversalHealthBarWidget* UniversalWidget = Cast<UUniversalHealthBarWidget>(Widget);
        if (UniversalWidget)
        {
            UE_LOG(LogTemp, Warning, TEXT("*** SUCCESS: Widget is UniversalHealthBarWidget! Initializing with character data ***"));
            UniversalWidget->InitializeWithCharacter(this);
        }
        else
        {
            UE_LOG(LogTemp, Warning, TEXT("Widget is not UniversalHealthBarWidget, using legacy approach"));
        }
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("*** SetHealthBarWidget: Widget is invalid! Cast may have failed ***"));
        CurrentHealthBarWidget = nullptr;
    }
}

void ABloodreadBaseCharacter::UpdateHealthDisplay()
{
    UE_LOG(LogTemp, Error, TEXT("=== UpdateHealthDisplay CALLED === Health: %d/%d"), CurrentHealth, CurrentStats.MaxHealth);
    
    if (CurrentHealthBarWidget)
    {
        UE_LOG(LogTemp, Warning, TEXT("Widget exists: %s"), *CurrentHealthBarWidget->GetClass()->GetName());
        
        // Make sure the widget is visible
        CurrentHealthBarWidget->SetVisibility(ESlateVisibility::Visible);
        
        // Calculate health percentage 
        float HealthPercent = (CurrentStats.MaxHealth > 0) ? (float)CurrentHealth / (float)CurrentStats.MaxHealth : 0.0f;
        UE_LOG(LogTemp, Warning, TEXT("Health percentage calculated: %.2f (from %d/%d)"), HealthPercent, CurrentHealth, CurrentStats.MaxHealth);
        
        // Use PracticeDummy approach - direct widget component access
        bool bUpdatedSuccessfully = false;
        
        UE_LOG(LogTemp, Error, TEXT("*** USING DIRECT WIDGET COMPONENT ACCESS (PracticeDummy approach) ***"));
        
        // Try multiple common progress bar names with extensive logging
        TArray<FString> ProgressBarNames = {
            TEXT("HealthProgressBar"), TEXT("ProgressBar"), TEXT("HealthBar"), 
            TEXT("HP_ProgressBar"), TEXT("MainProgressBar"), TEXT("Bar"),
            TEXT("ProgressBar_0"), TEXT("ProgressBar_1"), TEXT("Health_Bar")
        };
        
        for (const FString& BarName : ProgressBarNames)
        {
            UE_LOG(LogTemp, Warning, TEXT("Searching for progress bar with name: '%s'"), *BarName);
            if (UWidget* FoundWidget = CurrentHealthBarWidget->GetWidgetFromName(*BarName))
            {
                UE_LOG(LogTemp, Warning, TEXT("Found widget '%s' of class: %s"), *BarName, *FoundWidget->GetClass()->GetName());
                if (UProgressBar* ProgressBar = Cast<UProgressBar>(FoundWidget))
                {
                    ProgressBar->SetPercent(HealthPercent);
                    bUpdatedSuccessfully = true;
                    UE_LOG(LogTemp, Error, TEXT("*** SUCCESS: Updated progress bar '%s' directly to %.1f%% ***"), *BarName, HealthPercent * 100.0f);
                    break;
                }
            }
        }
        
        // Try multiple common text block names for health text
        TArray<FString> TextNames = {
            TEXT("HealthText"), TEXT("HealthLabel"), TEXT("HPText"), TEXT("HP_Text"),
            TEXT("TextBlock"), TEXT("HealthDisplay"), TEXT("Text"), TEXT("HealthNumbers")
        };
        
        for (const FString& TextName : TextNames)
        {
            if (UWidget* FoundWidget = CurrentHealthBarWidget->GetWidgetFromName(*TextName))
            {
                if (UTextBlock* TextBlock = Cast<UTextBlock>(FoundWidget))
                {
                    FText HealthText = FText::FromString(FString::Printf(TEXT("%d/%d"), CurrentHealth, CurrentStats.MaxHealth));
                    TextBlock->SetText(HealthText);
                    UE_LOG(LogTemp, Error, TEXT("*** SUCCESS: Updated text block '%s' to '%s' ***"), *TextName, *HealthText.ToString());
                    bUpdatedSuccessfully = true;
                    break;
                }
            }
        }
        
        if (bUpdatedSuccessfully)
        {
            UE_LOG(LogTemp, Error, TEXT("*** PLAYER CHARACTER HEALTH BAR UPDATED SUCCESSFULLY ***"));
        }
        else
        {
            UE_LOG(LogTemp, Warning, TEXT("*** Could not find appropriate progress bar or text components ***"));
        }
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("No health bar widget found for %s"), *GetName());
    }
}

void ABloodreadBaseCharacter::InitializeHealthBar()
{
    // Initialize health bar widget - EXACT copy from PracticeDummy approach
    UWidgetComponent* WorkingWidgetComponent = nullptr;
    
    // Approach 1: Use C++ component if it exists
    if (HealthBarWidgetComponent)
    {
        WorkingWidgetComponent = HealthBarWidgetComponent;
        UE_LOG(LogTemp, Warning, TEXT("Player Character using C++ HealthBarWidgetComponent: %s"), 
               *HealthBarWidgetComponent->GetName());
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("Player Character C++ HealthBarWidgetComponent is null!"));
        
        // Approach 2: Find any widget component by name (Blueprint components)
        TArray<UWidgetComponent*> WidgetComponents;
        GetComponents<UWidgetComponent>(WidgetComponents);
        
        UE_LOG(LogTemp, Warning, TEXT("Found %d widget components total"), WidgetComponents.Num());
        
        for (UWidgetComponent* WidgetComp : WidgetComponents)
        {
            UE_LOG(LogTemp, Warning, TEXT("Found widget component: %s"), *WidgetComp->GetName());
            if (WidgetComp->GetName().Contains(TEXT("Health")) || WidgetComp->GetName().Contains(TEXT("health")))
            {
                WorkingWidgetComponent = WidgetComp;
                HealthBarWidgetComponent = WidgetComp; // Store reference
                UE_LOG(LogTemp, Warning, TEXT("Using Blueprint health widget component: %s"), *WidgetComp->GetName());
                break;
            }
        }
        
        // Approach 3: Use first widget component if we have any
        if (!WorkingWidgetComponent && WidgetComponents.Num() > 0)
        {
            WorkingWidgetComponent = WidgetComponents[0];
            HealthBarWidgetComponent = WidgetComponents[0];
            UE_LOG(LogTemp, Warning, TEXT("Using first available widget component: %s"), *WidgetComponents[0]->GetName());
        }
    }
    
    // Now try to initialize the widget (exactly like PracticeDummy)
    if (WorkingWidgetComponent)
    {
        WorkingWidgetComponent->SetVisibility(true);
        WorkingWidgetComponent->SetHiddenInGame(false);
        
        UE_LOG(LogTemp, Warning, TEXT("Widget component setup - Class: %s"), 
               WorkingWidgetComponent->GetWidgetClass() ? *WorkingWidgetComponent->GetWidgetClass()->GetName() : TEXT("None"));
        
        // Force widget creation if it doesn't exist
        if (!WorkingWidgetComponent->GetUserWidgetObject())
        {
            UE_LOG(LogTemp, Warning, TEXT("Widget object null, attempting to create..."));
            WorkingWidgetComponent->InitWidget();
        }
        
        // Get the widget and initialize it (exactly like PracticeDummy)
        if (WorkingWidgetComponent->GetUserWidgetObject())
        {
            SetHealthBarWidget(WorkingWidgetComponent->GetUserWidgetObject());
            UpdateHealthDisplay();  // Initialize the display like PracticeDummy does
            UE_LOG(LogTemp, Error, TEXT("*** SUCCESS: Player Character health bar widget initialized! ***"));
        }
        else
        {
            UE_LOG(LogTemp, Error, TEXT("*** FAILED: Could not create widget object! Check Widget Class assignment ***"));
        }
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("*** FAILED: No widget components found at all! ***"));
    }
}

void ABloodreadBaseCharacter::UpdateHealthBarVisibility()
{
    if (!HealthBarWidgetComponent)
    {
        return;
    }

    // Get the local player's camera location
    APawn* LocalPlayerPawn = GetWorld()->GetFirstPlayerController()->GetPawn();
    if (!LocalPlayerPawn)
    {
        return;
    }

    // Calculate distance between this character and the local player
    float Distance = FVector::Dist(GetActorLocation(), LocalPlayerPawn->GetActorLocation());
    
    // Show/hide health bar based on distance
    bool bShouldBeVisible = Distance <= MaxHealthBarVisibilityDistance;
    
    if (HealthBarWidgetComponent->IsVisible() != bShouldBeVisible)
    {
        HealthBarWidgetComponent->SetVisibility(bShouldBeVisible);
        HealthBarWidgetComponent->SetHiddenInGame(!bShouldBeVisible);
        
        // Optional: Log visibility changes for debugging
        UE_LOG(LogTemp, Log, TEXT("Health bar visibility for %s: %s (Distance: %.1f)"), 
               *GetName(), 
               bShouldBeVisible ? TEXT("VISIBLE") : TEXT("HIDDEN"), 
               Distance);
    }
}

void ABloodreadBaseCharacter::InitializeEditorSpawnedCharacter()
{
    UE_LOG(LogTemp, Warning, TEXT("=== INITIALIZING EDITOR-SPAWNED CHARACTER ==="));
    UE_LOG(LogTemp, Warning, TEXT("Character: %s, Class: %s"), *GetName(), *GetClass()->GetName());
    
    // Force the character to initialize based on its actual C++ class type
    if (ABloodreadWarriorCharacter* WarriorChar = Cast<ABloodreadWarriorCharacter>(this))
    {
        UE_LOG(LogTemp, Warning, TEXT("Detected Warrior character, initializing warrior data..."));
        SetCharacterClass(ECharacterClass::Warrior);
        WarriorChar->InitializeWarriorData();
    }
    else if (ABloodreadMageCharacter* MageChar = Cast<ABloodreadMageCharacter>(this))
    {
        UE_LOG(LogTemp, Warning, TEXT("Detected Mage character, initializing mage data..."));
        SetCharacterClass(ECharacterClass::Mage);
        MageChar->InitializeMageData();
    }
    else if (ABloodreadRogueCharacter* RogueChar = Cast<ABloodreadRogueCharacter>(this))
    {
        UE_LOG(LogTemp, Warning, TEXT("Detected Rogue character, initializing rogue data..."));
        SetCharacterClass(ECharacterClass::Rogue);
        RogueChar->InitializeRogueData();
    }
    else if (ABloodreadHealerCharacter* HealerChar = Cast<ABloodreadHealerCharacter>(this))
    {
        UE_LOG(LogTemp, Warning, TEXT("Detected Healer character, initializing healer data..."));
        SetCharacterClass(ECharacterClass::Healer);
        HealerChar->InitializeHealerData();
    }
    else if (ABloodreadDragonCharacter* DragonChar = Cast<ABloodreadDragonCharacter>(this))
    {
        UE_LOG(LogTemp, Warning, TEXT("Detected Dragon character, initializing dragon data..."));
        SetCharacterClass(ECharacterClass::Dragon);
        DragonChar->InitializeDragonData();
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("Base character type, setting to Warrior as default"));
        SetCharacterClass(ECharacterClass::Warrior);
    }
    
    // Force health bar initialization
    InitializeHealthBar();
    
    // Update health display
    UpdateHealthDisplay();
    
    UE_LOG(LogTemp, Warning, TEXT("=== EDITOR CHARACTER INITIALIZATION COMPLETE ==="));
}

void ABloodreadBaseCharacter::TestTakeDamage(int32 DamageAmount)
{
    UE_LOG(LogTemp, Warning, TEXT("=== TESTING DAMAGE ON %s ==="), *GetName());
    UE_LOG(LogTemp, Warning, TEXT("Before damage - Health: %d/%d"), CurrentHealth, CurrentStats.MaxHealth);
    
    bool bResult = TakeCustomDamage(DamageAmount, nullptr);
    
    UE_LOG(LogTemp, Warning, TEXT("After damage - Health: %d/%d, Damage applied: %s"), 
           CurrentHealth, CurrentStats.MaxHealth, bResult ? TEXT("YES") : TEXT("NO"));
}

void ABloodreadBaseCharacter::TestKnockback(FVector Direction, float Force)
{
    UE_LOG(LogTemp, Warning, TEXT("=== TESTING KNOCKBACK ON %s ==="), *GetName());
    UE_LOG(LogTemp, Warning, TEXT("Applying knockback - Direction: %s, Force: %.1f"), *Direction.ToString(), Force);
    
    ApplyKnockback(Direction, Force);
    
    UE_LOG(LogTemp, Warning, TEXT("Knockback applied"));
}

FString ABloodreadBaseCharacter::GetOwnerClassName() const
{
    // Get the character class enum and convert to readable string
    ECharacterClass CharClass = GetCharacterClass();
    
    switch (CharClass)
    {
    case ECharacterClass::Warrior:
        return TEXT("Warrior");
    case ECharacterClass::Mage:
        return TEXT("Mage");
    case ECharacterClass::Healer:
        return TEXT("Healer");
    case ECharacterClass::Rogue:
        return TEXT("Rogue");
    case ECharacterClass::Dragon:
        return TEXT("Dragon");
    default:
        return TEXT("Unknown Class");
    }
}

void ABloodreadBaseCharacter::ForceEnableKnockbackPhysics()
{
    UE_LOG(LogTemp, Error, TEXT("🚨 ForceEnableKnockbackPhysics called for %s"), *GetName());
    
    // Enable physics interaction settings
    UCharacterMovementComponent* MovementComp = GetCharacterMovement();
    if (MovementComp)
    {
        UE_LOG(LogTemp, Error, TEXT("🚨 Forcing physics interaction settings"));
        MovementComp->bEnablePhysicsInteraction = true;
        MovementComp->PushForceFactor = 750000.0f; // High push force
        // PushForcePointFactor doesn't exist, removing typo
        MovementComp->TouchForceFactor = 750000.0f;
        MovementComp->MinTouchForce = 1000.0f;
        MovementComp->MaxTouchForce = 100000.0f;
        
        // Force mass and velocity settings
        MovementComp->Mass = 100.0f;
        MovementComp->bImpartBaseVelocityX = true;
        MovementComp->bImpartBaseVelocityY = true;
        MovementComp->bImpartBaseVelocityZ = true;
        
        UE_LOG(LogTemp, Error, TEXT("🚨 Physics settings applied - PushForceFactor: %.1f"), MovementComp->PushForceFactor);
    }
    
    // Ensure collision is set up properly
    if (UCapsuleComponent* Capsule = GetCapsuleComponent())
    {
        UE_LOG(LogTemp, Error, TEXT("🚨 Configuring collision settings"));
        Capsule->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
        Capsule->SetCollisionObjectType(ECollisionChannel::ECC_Pawn);
        Capsule->SetCollisionResponseToAllChannels(ECR_Block);
        Capsule->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECR_Ignore);
        
        UE_LOG(LogTemp, Error, TEXT("🚨 Collision settings configured"));
    }
}

void ABloodreadBaseCharacter::PlayBasicAttackAnimation()
{
    UE_LOG(LogTemp, Warning, TEXT("🎭 PlayBasicAttackAnimation called"));
    
    if (!CharacterClassData.BasicAttackAnimationPath.IsEmpty())
    {
        bool bPlayed = PlayAnimationFromPath(CharacterClassData.BasicAttackAnimationPath);
        UE_LOG(LogTemp, Warning, TEXT("🎭 Basic attack animation result: %s"), bPlayed ? TEXT("SUCCESS") : TEXT("FAILED"));
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("🎭 No basic attack animation path set for character class"));
    }
}

void ABloodreadBaseCharacter::PlayAbility1Animation()
{
    UE_LOG(LogTemp, Warning, TEXT("🎭 PlayAbility1Animation called"));
    
    if (!CharacterClassData.Ability1AnimationPath.IsEmpty())
    {
        bool bPlayed = PlayAnimationFromPath(CharacterClassData.Ability1AnimationPath);
        UE_LOG(LogTemp, Warning, TEXT("🎭 Ability 1 animation result: %s"), bPlayed ? TEXT("SUCCESS") : TEXT("FAILED"));
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("🎭 No ability 1 animation path set for character class"));
    }
}

void ABloodreadBaseCharacter::PlayAbility2Animation()
{
    UE_LOG(LogTemp, Warning, TEXT("🎭 PlayAbility2Animation called"));
    
    if (!CharacterClassData.Ability2AnimationPath.IsEmpty())
    {
        bool bPlayed = PlayAnimationFromPath(CharacterClassData.Ability2AnimationPath);
        UE_LOG(LogTemp, Warning, TEXT("🎭 Ability 2 animation result: %s"), bPlayed ? TEXT("SUCCESS") : TEXT("FAILED"));
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("🎭 No ability 2 animation path set for character class"));
    }
}

bool ABloodreadBaseCharacter::PlayAnimationFromPath(const FString& AnimationPath)
{
    UE_LOG(LogTemp, Warning, TEXT("🎭 PlayAnimationFromPath: %s"), *AnimationPath);
    
    if (AnimationPath.IsEmpty())
    {
        UE_LOG(LogTemp, Error, TEXT("🎭 Animation path is empty"));
        return false;
    }
    
    USkeletalMeshComponent* MeshComp = GetMesh();
    if (!MeshComp)
    {
        UE_LOG(LogTemp, Error, TEXT("🎭 No skeletal mesh component found"));
        return false;
    }
    
    // Try to load the animation sequence
    UAnimSequence* AnimSequence = LoadObject<UAnimSequence>(nullptr, *AnimationPath);
    if (!AnimSequence)
    {
        UE_LOG(LogTemp, Error, TEXT("🎭 Failed to load animation sequence from path: %s"), *AnimationPath);
        return false;
    }
    
    UE_LOG(LogTemp, Warning, TEXT("🎭 Successfully loaded animation sequence: %s"), *AnimSequence->GetName());
    
    // Clear any existing timer
    GetWorld()->GetTimerManager().ClearTimer(AnimationBlendBackTimer);
    
    // Store current animation mode to restore later
    EAnimationMode::Type OriginalAnimMode = MeshComp->GetAnimationMode();
    
    // Play the animation directly (this ensures it actually plays)
    MeshComp->PlayAnimation(AnimSequence, false); // false = don't loop
    
    UE_LOG(LogTemp, Warning, TEXT("🎭 Animation playback started with PlayAnimation"));
    
    // Calculate when to restore animation blueprint mode
    float AnimationDuration = AnimSequence->GetPlayLength();
    float RestoreDelay = AnimationDuration + 0.2f; // Small buffer time
    
    UE_LOG(LogTemp, Warning, TEXT("🎭 Animation duration: %.2f seconds, will restore animation blueprint in %.2f seconds"), 
           AnimationDuration, RestoreDelay);
    
    // Set timer to restore animation blueprint mode after animation completes
    GetWorld()->GetTimerManager().SetTimer(
        AnimationBlendBackTimer,
        [this, MeshComp]()
        {
            UE_LOG(LogTemp, Warning, TEXT("🎭 Restoring animation blueprint mode"));
            if (MeshComp && IsValid(MeshComp))
            {
                // Force back to animation blueprint mode
                MeshComp->SetAnimationMode(EAnimationMode::AnimationBlueprint);
                
                // If we have an animation blueprint class, reinitialize it
                if (UClass* AnimBPClass = MeshComp->GetAnimClass())
                {
                    MeshComp->SetAnimInstanceClass(AnimBPClass);
                    MeshComp->InitializeAnimScriptInstance(true);
                    UE_LOG(LogTemp, Warning, TEXT("🎭 Animation blueprint reinitialized"));
                }
                
                UE_LOG(LogTemp, Warning, TEXT("🎭 Successfully restored to animation blueprint mode"));
            }
        },
        RestoreDelay,
        false // Don't loop
    );
    
    return true;
}

// Network replication functions
void ABloodreadBaseCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    
    DOREPLIFETIME(ABloodreadBaseCharacter, CurrentHealth);
    DOREPLIFETIME(ABloodreadBaseCharacter, CurrentMana);
}

void ABloodreadBaseCharacter::OnRep_Health()
{
    UE_LOG(LogTemp, Warning, TEXT("Health replicated: %d"), CurrentHealth);
    
    // Update UI and visual effects
    Multicast_OnHealthChanged(CurrentHealth, CurrentStats.MaxHealth);
}

// Multiplayer RPC implementations
void ABloodreadBaseCharacter::Server_UseAbility_Implementation(int32 AbilityIndex, FVector TargetLocation)
{
    UE_LOG(LogTemp, Warning, TEXT("Server: Using ability %d at location %s"), AbilityIndex, *TargetLocation.ToString());
    
    // Validate ability usage on server
    if (AbilityIndex >= 0 && AbilityIndex <= 1)
    {
        // Use the ability (call existing logic)
        if (AbilityIndex == 0)
        {
            UseAbility1();
        }
        else if (AbilityIndex == 1)
        {
            UseAbility2();
        }
        
        // Replicate animation to all clients
        Multicast_PlayAbilityAnimation(AbilityIndex);
    }
}

void ABloodreadBaseCharacter::Multicast_PlayAbilityAnimation_Implementation(int32 AbilityIndex)
{
    UE_LOG(LogTemp, Warning, TEXT("Multicast: Playing ability animation %d"), AbilityIndex);
    
    // Play animation for ability (call existing animation logic)
    if (AbilityIndex == 0)
    {
        PlayAnimationFromPath(CharacterClassData.Ability1AnimationPath);
    }
    else if (AbilityIndex == 1)
    {
        PlayAnimationFromPath(CharacterClassData.Ability2AnimationPath);
    }
}

void ABloodreadBaseCharacter::Server_TakeDamage_Implementation(float DamageAmount, ABloodreadBaseCharacter* DamageSource)
{
    UE_LOG(LogTemp, Warning, TEXT("Server: Taking %f damage from %s"), DamageAmount, DamageSource ? *DamageSource->GetName() : TEXT("Unknown"));
    
    // Apply damage on server
    DealDamage(DamageAmount);
    
    // Replicate hit animation
    Multicast_PlayHitAnimation();
}

void ABloodreadBaseCharacter::Multicast_OnHealthChanged_Implementation(int32 NewHealth, int32 MaxHealth)
{
    UE_LOG(LogTemp, Warning, TEXT("Multicast: Health changed to %d/%d"), NewHealth, MaxHealth);
    
    // Update health bar UI for all clients
    if (HealthBarWidgetComponent)
    {
        if (UUniversalHealthBarWidget* HealthWidget = Cast<UUniversalHealthBarWidget>(HealthBarWidgetComponent->GetUserWidgetObject()))
        {
            HealthWidget->UpdateHealthDisplay();
        }
    }
}

void ABloodreadBaseCharacter::Server_BasicAttack_Implementation(FVector TargetLocation)
{
    UE_LOG(LogTemp, Warning, TEXT("Server: Basic attack at %s"), *TargetLocation.ToString());
    
    // Perform attack logic on server
    PerformAttack();
}

void ABloodreadBaseCharacter::Multicast_PlayHitAnimation_Implementation()
{
    UE_LOG(LogTemp, Warning, TEXT("Multicast: Playing hit animation"));
    
    // Play hit animation on all clients - using basic attack animation as placeholder
    PlayAnimationFromPath(CharacterClassData.BasicAttackAnimationPath);
}
