#include "PracticeDummy.h"
#include "BloodreadPlayerCharacter.h"
#include "BloodreadGameMode.h"
#include "Components/StaticMeshComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/WidgetComponent.h"
#include "Blueprint/UserWidget.h"
#include "Blueprint/WidgetTree.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/Engine.h"
#include "TimerManager.h"

APracticeDummy::APracticeDummy()
{
    PrimaryActorTick.bCanEverTick = true;

    // Create collision capsule as root
    CapsuleComponent = CreateDefaultSubobject<UCapsuleComponent>(TEXT("CapsuleComponent"));
    RootComponent = CapsuleComponent;
    CapsuleComponent->SetCapsuleSize(34.0f, 88.0f);
    CapsuleComponent->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
    CapsuleComponent->SetCollisionResponseToAllChannels(ECR_Block);
    
    // For stable spawning - no physics on capsule, we'll handle movement manually
    CapsuleComponent->SetSimulatePhysics(false);

    // Create mesh component
    DummyMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("DummyMesh"));
    DummyMesh->SetupAttachment(RootComponent);
    
    // Disable physics on mesh - we'll use manual knockback
    DummyMesh->SetSimulatePhysics(false);
    DummyMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision); // Let capsule handle collision
    
    // Create health bar widget component
    HealthBarWidgetComponent = CreateDefaultSubobject<UWidgetComponent>(TEXT("HealthBarWidgetComponent"));
    HealthBarWidgetComponent->SetupAttachment(RootComponent);
    HealthBarWidgetComponent->SetRelativeLocation(FVector(0.0f, 0.0f, 120.0f)); // Above dummy head
    HealthBarWidgetComponent->SetDrawSize(FVector2D(200.0f, 50.0f));
    HealthBarWidgetComponent->SetWidgetSpace(EWidgetSpace::Screen); // Always face camera
    
    // Try to set widget class - this may be overridden by Blueprint
    // Note: The actual widget class should be set in Blueprint defaults

    // Initialize stats
    CurrentHealth = MaxHealth;
    bCanTakeDamage = true;
    DamageImmunityTicksRemaining = 0;
    CurrentKnockbackVelocity = FVector::ZeroVector;
}

void APracticeDummy::BeginPlay()
{
    Super::BeginPlay();
    
    // Store initial location for reset purposes
    InitialLocation = GetActorLocation();
    
    // Initialize health bar widget - try multiple approaches
    UWidgetComponent* WorkingWidgetComponent = nullptr;
    
    // Approach 1: Use C++ component if it exists
    if (HealthBarWidgetComponent)
    {
        WorkingWidgetComponent = HealthBarWidgetComponent;
        UE_LOG(LogTemp, Warning, TEXT("Practice Dummy using C++ HealthBarWidgetComponent: %s"), 
               *HealthBarWidgetComponent->GetName());
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("Practice Dummy C++ HealthBarWidgetComponent is null!"));
        
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
    
    // Now try to initialize the widget
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
        
        if (WorkingWidgetComponent->GetUserWidgetObject())
        {
            SetHealthBarWidget(WorkingWidgetComponent->GetUserWidgetObject());
            UpdateHealthDisplay();
            UE_LOG(LogTemp, Error, TEXT("*** SUCCESS: Practice Dummy health bar widget initialized! ***"));
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
    
    UE_LOG(LogTemp, Warning, TEXT("Practice Dummy spawned with %d/%d health at location %s"), 
           CurrentHealth, MaxHealth, *InitialLocation.ToString());
}

void APracticeDummy::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    FVector CurrentLocation = GetActorLocation();
    bool bShouldApplyGravity = false;
    
    // Apply knockback velocity if any
    if (!CurrentKnockbackVelocity.IsNearlyZero())
    {
        UE_LOG(LogTemp, Warning, TEXT("Practice Dummy applying knockback velocity: %s"), *CurrentKnockbackVelocity.ToString());
        
        // Apply gravity to Z component if dummy is above ground
        if (CurrentLocation.Z > InitialLocation.Z + 5.0f) // 5 unit tolerance
        {
            CurrentKnockbackVelocity.Z -= 980.0f * DeltaTime; // Apply gravity (980 cm/s^2)
            bShouldApplyGravity = true;
        }
        else if (CurrentKnockbackVelocity.Z < 0.0f)
        {
            // Stop downward velocity when near ground
            CurrentKnockbackVelocity.Z = 0.0f;
        }
        
        // Apply movement with collision detection
        FVector MovementDelta = CurrentKnockbackVelocity * DeltaTime;
        FVector NewLocation = CurrentLocation + MovementDelta;
        
        // Clamp to ground level
        if (NewLocation.Z < InitialLocation.Z)
        {
            NewLocation.Z = InitialLocation.Z;
            CurrentKnockbackVelocity.Z = 0.0f;
        }
        
        // Use sweep to detect collisions during movement
        FHitResult HitResult;
        bool bHit = SetActorLocation(NewLocation, true, &HitResult);
        
        if (HitResult.bBlockingHit)
        {
            // Stop movement in the direction of collision
            FVector HitNormal = HitResult.ImpactNormal;
            
            // Remove velocity component in direction of collision
            float DotProduct = FVector::DotProduct(CurrentKnockbackVelocity.GetSafeNormal(), HitNormal);
            if (DotProduct < 0.0f) // Moving towards the wall
            {
                CurrentKnockbackVelocity = CurrentKnockbackVelocity - (HitNormal * FVector::DotProduct(CurrentKnockbackVelocity, HitNormal));
                UE_LOG(LogTemp, Warning, TEXT("Practice Dummy hit wall, adjusted velocity: %s"), *CurrentKnockbackVelocity.ToString());
            }
        }
        
        // Decay horizontal velocity
        FVector HorizontalVelocity = FVector(CurrentKnockbackVelocity.X, CurrentKnockbackVelocity.Y, 0.0f);
        HorizontalVelocity = FMath::VInterpTo(HorizontalVelocity, FVector::ZeroVector, DeltaTime, KnockbackDecayRate);
        CurrentKnockbackVelocity.X = HorizontalVelocity.X;
        CurrentKnockbackVelocity.Y = HorizontalVelocity.Y;
        
        UE_LOG(LogTemp, VeryVerbose, TEXT("Practice Dummy velocity after decay: %s"), *CurrentKnockbackVelocity.ToString());
        
        // Stop when velocity is very small
        if (CurrentKnockbackVelocity.Size() < 10.0f)
        {
            CurrentKnockbackVelocity = FVector::ZeroVector;
            UE_LOG(LogTemp, Log, TEXT("Practice Dummy knockback stopped"));
        }
    }
    else if (CurrentLocation.Z > InitialLocation.Z + 2.0f)
    {
        // Apply settling gravity even when no knockback velocity
        FVector SettleVelocity = FVector(0.0f, 0.0f, -500.0f); // Gentle settling
        FVector MovementDelta = SettleVelocity * DeltaTime;
        FVector NewLocation = CurrentLocation + MovementDelta;
        
        if (NewLocation.Z <= InitialLocation.Z)
        {
            NewLocation.Z = InitialLocation.Z;
        }
        
        // Use sweep for settling as well
        FHitResult HitResult;
        SetActorLocation(NewLocation, true, &HitResult);
        UE_LOG(LogTemp, VeryVerbose, TEXT("Practice Dummy settling to ground"));
    }
    
    // Always ensure dummy stays upright
    FRotator CurrentRotation = GetActorRotation();
    FRotator TargetRotation = FRotator(0.0f, CurrentRotation.Yaw, 0.0f); // Only keep Yaw rotation
    
    if (!CurrentRotation.Equals(TargetRotation, 1.0f))
    {
        FRotator NewRotation = FMath::RInterpTo(CurrentRotation, TargetRotation, DeltaTime, 5.0f);
        SetActorRotation(NewRotation);
    }
}

void APracticeDummy::TakeCustomDamage(int32 Damage, ABloodreadPlayerCharacter* Attacker)
{
    UE_LOG(LogTemp, Error, TEXT("=== DUMMY DAMAGE CALLED === Damage: %d, Current Health: %d, CanTakeDamage: %s"), 
           Damage, CurrentHealth, bCanTakeDamage ? TEXT("true") : TEXT("false"));
    
    if (!bCanTakeDamage || DamageImmunityTicksRemaining > 0) 
    {
        UE_LOG(LogTemp, Warning, TEXT("Practice Dummy damage blocked - CanTakeDamage: %s, ImmunityTicks: %d"), 
               bCanTakeDamage ? TEXT("true") : TEXT("false"), DamageImmunityTicksRemaining);
        return;
    }

    int32 PreviousHealth = CurrentHealth;
    CurrentHealth = FMath::Max(0, CurrentHealth - Damage);
    
    // Set damage immunity following game tick system
    ABloodreadGameMode* GameMode = Cast<ABloodreadGameMode>(UGameplayStatics::GetGameMode(this));
    if (GameMode)
    {
        bCanTakeDamage = false;
    }
    
    // Update health bar display
    UpdateHealthDisplay();
    
    // Flash red when taking damage
    FlashRed();
    
    // Blueprint event for additional effects
    OnTakeDamage(Damage, Attacker);
    
    UE_LOG(LogTemp, Warning, TEXT("Practice Dummy took %d damage! Health: %d -> %d (Max: %d) [%.1f%%]"), 
           Damage, PreviousHealth, CurrentHealth, MaxHealth, GetHealthPercentage() * 100.0f);
    
    // Auto-reset if health gets too low (for practice mode)
    if (CurrentHealth <= 0)
    {
        // Reset after a delay
        FTimerHandle ResetTimer;
        GetWorld()->GetTimerManager().SetTimer(ResetTimer, this, &APracticeDummy::ResetDummy, 3.0f, false);
    }
}

void APracticeDummy::ApplyKnockback(FVector KnockbackDirection, float Force)
{
    if (bUseStablePhysics)
    {
        // Apply knockback resistance
        float ActualForce = Force * KnockbackResistance;
        
        // Minecraft-style knockback: mostly horizontal with slight vertical lift
        KnockbackDirection.Z = FMath::Max(KnockbackDirection.Z, 0.3f); // Minimum upward component
        KnockbackDirection = KnockbackDirection.GetSafeNormal();
        
        // Convert force to velocity (Minecraft-style)
        FVector KnockbackVelocity = KnockbackDirection * ActualForce * 10.0f; // Scale for good feel
        
        // Limit vertical component so it doesn't fly too high
        KnockbackVelocity.Z = FMath::Min(KnockbackVelocity.Z, 300.0f);
        
        // Set the knockback velocity for Tick to apply
        CurrentKnockbackVelocity = KnockbackVelocity;
        
        OnKnockbackApplied(KnockbackDirection, ActualForce);
        
        UE_LOG(LogTemp, Log, TEXT("Practice Dummy stable knockback applied: Direction=%s, Force=%.2f"), 
               *KnockbackDirection.ToString(), ActualForce);
    }
    else
    {
        // Fallback to old physics system if disabled
        if (DummyMesh && DummyMesh->IsSimulatingPhysics())
        {
            float ActualForce = Force * KnockbackResistance;
            KnockbackDirection.Z += 0.2f;
            KnockbackDirection = KnockbackDirection.GetSafeNormal();
            
            FVector Impulse = KnockbackDirection * ActualForce;
            DummyMesh->AddImpulse(Impulse, NAME_None, true);
            
            OnKnockbackApplied(KnockbackDirection, ActualForce);
        }
    }
}

void APracticeDummy::ResetDummy()
{
    CurrentHealth = MaxHealth;
    bCanTakeDamage = true;
    DamageImmunityTicksRemaining = 0;
    
    // Reset position and rotation to initial state
    SetActorLocation(InitialLocation);
    SetActorRotation(FRotator::ZeroRotator);
    
    // Reset knockback velocity
    CurrentKnockbackVelocity = FVector::ZeroVector;
    
    // Reset physics velocity if using old system
    if (DummyMesh && DummyMesh->IsSimulatingPhysics())
    {
        DummyMesh->SetPhysicsLinearVelocity(FVector::ZeroVector);
        DummyMesh->SetPhysicsAngularVelocityInDegrees(FVector::ZeroVector);
    }
    
    // Update health bar
    UpdateHealthDisplay();
    
    // Stop red flash
    StopFlashRed();
    
    UE_LOG(LogTemp, Log, TEXT("Practice Dummy reset to full health at %s"), *InitialLocation.ToString());
}

void APracticeDummy::ProcessGameTick()
{
    // Decrease damage immunity
    if (DamageImmunityTicksRemaining > 0)
    {
        DamageImmunityTicksRemaining--;
        if (DamageImmunityTicksRemaining == 0)
        {
            bCanTakeDamage = true;
        }
    }
}

float APracticeDummy::GetHealthPercentage() const
{
    if (MaxHealth == 0) return 0.0f;
    return (float)CurrentHealth / (float)MaxHealth;
}

void APracticeDummy::SetHealthBarWidget(UUserWidget* Widget)
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
        UE_LOG(LogTemp, Warning, TEXT("Practice Dummy health bar widget set successfully: %s"), 
               *Widget->GetClass()->GetName());
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("*** SetHealthBarWidget: Widget is invalid! Cast may have failed ***"));
        CurrentHealthBarWidget = nullptr;
    }
}

void APracticeDummy::UpdateHealthDisplay()
{
    UE_LOG(LogTemp, Error, TEXT("=== UpdateHealthDisplay CALLED === Health: %d/%d"), CurrentHealth, MaxHealth);
    
    if (CurrentHealthBarWidget)
    {
        UE_LOG(LogTemp, Warning, TEXT("Widget exists: %s"), *CurrentHealthBarWidget->GetClass()->GetName());
        
        // Make sure the widget is visible
        CurrentHealthBarWidget->SetVisibility(ESlateVisibility::Visible);
        
        // SKIP ALL BLUEPRINT FUNCTION CALLS - GO STRAIGHT TO DIRECT COMPONENT ACCESS
        bool bUpdatedSuccessfully = false;
        
        UE_LOG(LogTemp, Error, TEXT("*** SKIPPING BLUEPRINT FUNCTIONS - USING DIRECT COMPONENT ACCESS ONLY ***"));
        
        // COMPLETELY SKIP BLUEPRINT FUNCTION CALLS TO AVOID CAST FAILURES
        // This eliminates the root cause by not calling Blueprint functions at all
        UE_LOG(LogTemp, Warning, TEXT("Bypassing all Blueprint function calls to prevent cast failures"));
        
        // PRIMARY METHOD: Direct widget component access (completely avoids Blueprint function calls)
        UE_LOG(LogTemp, Error, TEXT("=== ATTEMPTING DIRECT WIDGET COMPONENT ACCESS ==="));
        
        // Calculate health percentage
        float HealthPercent = (MaxHealth > 0) ? (float)CurrentHealth / (float)MaxHealth : 0.0f;
        UE_LOG(LogTemp, Warning, TEXT("Health percentage calculated: %.2f (from %d/%d)"), HealthPercent, CurrentHealth, MaxHealth);
        
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
                else
                {
                    UE_LOG(LogTemp, Warning, TEXT("Widget '%s' found but is not a UProgressBar"), *BarName);
                }
            }
            else
            {
                UE_LOG(LogTemp, Warning, TEXT("No widget found with name: '%s'"), *BarName);
            }
        }
        
        // Try multiple common text widget names with extensive logging
        TArray<FString> TextNames = {
            TEXT("HealthText"), TEXT("TextBlock"), TEXT("HealthLabel"), 
            TEXT("HP_Text"), TEXT("MainText"), TEXT("Text"),
            TEXT("TextBlock_0"), TEXT("TextBlock_1"), TEXT("Health_Text")
        };
        
        for (const FString& TextName : TextNames)
        {
            UE_LOG(LogTemp, Warning, TEXT("Searching for text widget with name: '%s'"), *TextName);
            if (UWidget* TextWidget = CurrentHealthBarWidget->GetWidgetFromName(*TextName))
            {
                UE_LOG(LogTemp, Warning, TEXT("Found widget '%s' of class: %s"), *TextName, *TextWidget->GetClass()->GetName());
                if (UTextBlock* TextBlock = Cast<UTextBlock>(TextWidget))
                {
                    FString HealthText = FString::Printf(TEXT("%d/%d"), CurrentHealth, MaxHealth);
                    TextBlock->SetText(FText::FromString(HealthText));
                    UE_LOG(LogTemp, Error, TEXT("*** SUCCESS: Set health text '%s' to: %s ***"), *TextName, *HealthText);
                    break;
                }
                else
                {
                    UE_LOG(LogTemp, Warning, TEXT("Widget '%s' found but is not a UTextBlock"), *TextName);
                }
            }
            else
            {
                UE_LOG(LogTemp, Warning, TEXT("No widget found with name: '%s'"), *TextName);
            }
        }
        
        // FALLBACK METHOD: Try to set health properties directly on the widget
        if (!bUpdatedSuccessfully)
        {
            UE_LOG(LogTemp, Warning, TEXT("=== TRYING DIRECT PROPERTY ACCESS ==="));
            
            // List all properties available on the widget
            UE_LOG(LogTemp, Warning, TEXT("Available properties on widget:"));
            for (FProperty* Property = CurrentHealthBarWidget->GetClass()->PropertyLink; Property; Property = Property->PropertyLinkNext)
            {
                UE_LOG(LogTemp, Warning, TEXT("  - Property: %s (Type: %s)"), *Property->GetName(), *Property->GetClass()->GetName());
            }
            
            // Try to find and set health-related properties
            TArray<FString> HealthPropertyNames = {
                TEXT("CurrentHealth"), TEXT("Health"), TEXT("HP"), TEXT("HealthValue"),
                TEXT("MaxHealth"), TEXT("MaxHP"), TEXT("HealthPercent"), TEXT("Percentage")
            };
            
            for (const FString& PropertyName : HealthPropertyNames)
            {
                if (FProperty* HealthProp = CurrentHealthBarWidget->GetClass()->FindPropertyByName(*PropertyName))
                {
                    UE_LOG(LogTemp, Warning, TEXT("Found property '%s' of type: %s"), *PropertyName, *HealthProp->GetClass()->GetName());
                    
                    if (FIntProperty* IntProp = CastField<FIntProperty>(HealthProp))
                    {
                        int32 ValueToSet = PropertyName.Contains(TEXT("Max")) ? MaxHealth : CurrentHealth;
                        IntProp->SetPropertyValue_InContainer(CurrentHealthBarWidget, ValueToSet);
                        bUpdatedSuccessfully = true;
                        UE_LOG(LogTemp, Error, TEXT("*** SUCCESS: Set %s property directly to %d ***"), *PropertyName, ValueToSet);
                    }
                    else if (FFloatProperty* FloatProp = CastField<FFloatProperty>(HealthProp))
                    {
                        float ValueToSet = PropertyName.Contains(TEXT("Percent")) ? HealthPercent : (PropertyName.Contains(TEXT("Max")) ? (float)MaxHealth : (float)CurrentHealth);
                        FloatProp->SetPropertyValue_InContainer(CurrentHealthBarWidget, ValueToSet);
                        bUpdatedSuccessfully = true;
                        UE_LOG(LogTemp, Error, TEXT("*** SUCCESS: Set %s property directly to %.2f ***"), *PropertyName, ValueToSet);
                    }
                }
                else
                {
                    UE_LOG(LogTemp, Warning, TEXT("Property '%s' not found"), *PropertyName);
                }
            }
        }
        
        // FINAL STEPS: Force widget refresh and summary
        UE_LOG(LogTemp, Warning, TEXT("=== FORCING WIDGET REFRESH ==="));
        CurrentHealthBarWidget->ForceLayoutPrepass();
        CurrentHealthBarWidget->InvalidateLayoutAndVolatility();
        
        // Force a complete widget reconstruction if nothing else worked
        if (!bUpdatedSuccessfully)
        {
            UE_LOG(LogTemp, Error, TEXT("*** ALL DIRECT UPDATE METHODS FAILED - WIDGET NEEDS BLUEPRINT REDESIGN ***"));
            UE_LOG(LogTemp, Warning, TEXT("Widget class: %s"), 
                   CurrentHealthBarWidget->GetClass() ? *CurrentHealthBarWidget->GetClass()->GetName() : TEXT("Unknown"));
            UE_LOG(LogTemp, Warning, TEXT("Consider creating a simpler health bar widget or exposing properties for C++ access"));
        }
        else
        {
            UE_LOG(LogTemp, Error, TEXT("*** WIDGET UPDATE COMPLETED SUCCESSFULLY ***"));
        }
        
        UE_LOG(LogTemp, Error, TEXT("=== HEALTH DISPLAY UPDATE SUMMARY ==="));
        UE_LOG(LogTemp, Error, TEXT("Health: %d/%d (%.1f%%) - Widget Visible: %s - Update Success: %s"), 
               CurrentHealth, MaxHealth, GetHealthPercentage() * 100.0f,
               CurrentHealthBarWidget->GetVisibility() == ESlateVisibility::Visible ? TEXT("Yes") : TEXT("No"),
               bUpdatedSuccessfully ? TEXT("YES") : TEXT("NO"));
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("Practice Dummy health bar widget is null, cannot update display"));
        
        // Try to reinitialize if widget component exists
        if (HealthBarWidgetComponent && HealthBarWidgetComponent->GetUserWidgetObject())
        {
            SetHealthBarWidget(HealthBarWidgetComponent->GetUserWidgetObject());
            UpdateHealthDisplay(); // Recursive call after reinitializing
            UE_LOG(LogTemp, Warning, TEXT("Reinitialized and retried dummy health bar widget update"));
        }
    }
}

void APracticeDummy::FlashRed()
{
    if (bIsFlashingRed) return;
    
    bIsFlashingRed = true;
    
    // Set material parameter for red tint (Blueprint implementable)
    if (DummyMesh)
    {
        // This will be handled in Blueprint with material parameters
        // For now, just log and set timer to stop flash
        UE_LOG(LogTemp, Log, TEXT("Practice Dummy flashing red"));
    }
    
    // Stop flashing after 0.5 seconds
    GetWorld()->GetTimerManager().SetTimer(RedFlashTimer, this, &APracticeDummy::StopFlashRed, 0.5f, false);
}

void APracticeDummy::StopFlashRed()
{
    if (!bIsFlashingRed) return;
    
    bIsFlashingRed = false;
    
    // Reset material parameter (Blueprint implementable)
    if (DummyMesh)
    {
        UE_LOG(LogTemp, Log, TEXT("Practice Dummy stopped flashing red"));
    }
    
    // Clear timer
    GetWorld()->GetTimerManager().ClearTimer(RedFlashTimer);
}

void APracticeDummy::DebugUpdateHealthBar()
{
    UE_LOG(LogTemp, Error, TEXT("*** DEBUG: Manual health bar update called! Health: %d/%d ***"), CurrentHealth, MaxHealth);
    
    // Debug widget information
    if (CurrentHealthBarWidget)
    {
        UE_LOG(LogTemp, Warning, TEXT("Widget Class: %s"), *CurrentHealthBarWidget->GetClass()->GetName());
        UE_LOG(LogTemp, Warning, TEXT("Widget Valid: %s"), IsValid(CurrentHealthBarWidget) ? TEXT("Yes") : TEXT("No"));
        
        // List all available functions
        UE_LOG(LogTemp, Warning, TEXT("Available functions in widget:"));
        for (TFieldIterator<UFunction> FuncIt(CurrentHealthBarWidget->GetClass()); FuncIt; ++FuncIt)
        {
            UFunction* Function = *FuncIt;
            UE_LOG(LogTemp, Warning, TEXT("  - %s (Params: %d)"), *Function->GetName(), Function->NumParms);
        }
        
        // List all child widgets using WidgetTree
        UE_LOG(LogTemp, Warning, TEXT("Child widgets:"));
        if (CurrentHealthBarWidget->WidgetTree)
        {
            TArray<UWidget*> AllWidgets;
            CurrentHealthBarWidget->WidgetTree->GetAllWidgets(AllWidgets);
            for (UWidget* Child : AllWidgets)
            {
                if (Child)
                {
                    UE_LOG(LogTemp, Warning, TEXT("  - %s (%s)"), *Child->GetName(), *Child->GetClass()->GetName());
                }
            }
        }
        else
        {
            UE_LOG(LogTemp, Warning, TEXT("Widget tree is null - cannot enumerate children"));
        }
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("CurrentHealthBarWidget is NULL!"));
    }
    
    UpdateHealthDisplay();
}

void APracticeDummy::FixHealthBarWidget()
{
    UE_LOG(LogTemp, Error, TEXT("*** MANUAL WIDGET FIX CALLED ***"));
    
    // Reset widget reference
    CurrentHealthBarWidget = nullptr;
    
    // Find all widget components and try to connect
    TArray<UWidgetComponent*> WidgetComponents;
    GetComponents<UWidgetComponent>(WidgetComponents);
    
    UE_LOG(LogTemp, Warning, TEXT("Found %d widget components total"), WidgetComponents.Num());
    
    for (UWidgetComponent* WidgetComp : WidgetComponents)
    {
        UE_LOG(LogTemp, Warning, TEXT("Checking widget component: %s"), *WidgetComp->GetName());
        
        // Check if this is the health bar widget component
        if (WidgetComp->GetName().Contains(TEXT("HealthBar")))
        {
            UE_LOG(LogTemp, Warning, TEXT("Found HealthBar component: %s"), *WidgetComp->GetName());
            
            // Check widget class
            UClass* WidgetClass = WidgetComp->GetWidgetClass();
            if (WidgetClass)
            {
                UE_LOG(LogTemp, Warning, TEXT("Widget class: %s"), *WidgetClass->GetName());
                
                // Try to initialize the existing widget instead of destroying
                WidgetComp->InitWidget();
                
                UUserWidget* CreatedWidget = WidgetComp->GetUserWidgetObject();
                if (CreatedWidget)
                {
                    HealthBarWidgetComponent = WidgetComp;
                    SetHealthBarWidget(CreatedWidget);
                    UpdateHealthDisplay();
                    UE_LOG(LogTemp, Error, TEXT("*** WIDGET FIXED! Using existing component ***"));
                    return;
                }
                else
                {
                    UE_LOG(LogTemp, Error, TEXT("*** InitWidget failed - trying widget recreation ***"));
                    
                    // Try recreating widget without destroying component
                    WidgetComp->SetWidget(nullptr); // Clear current widget
                    WidgetComp->InitWidget(); // Recreate
                    
                    CreatedWidget = WidgetComp->GetUserWidgetObject();
                    if (CreatedWidget)
                    {
                        HealthBarWidgetComponent = WidgetComp;
                        SetHealthBarWidget(CreatedWidget);
                        UpdateHealthDisplay();
                        UE_LOG(LogTemp, Error, TEXT("*** WIDGET RECREATED AND FIXED! ***"));
                        return;
                    }
                    else
                    {
                        UE_LOG(LogTemp, Error, TEXT("*** WIDGET RECREATION FAILED - GetUserWidgetObject still null ***"));
                    }
                }
            }
            else
            {
                UE_LOG(LogTemp, Error, TEXT("*** WIDGET CLASS IS NULL! Check Blueprint widget class assignment ***"));
            }
        }
    }
    
    UE_LOG(LogTemp, Error, TEXT("*** WIDGET FIX FAILED - No HealthBar widget component found ***"));
}

void APracticeDummy::DebugWidgetInfo()
{
    UE_LOG(LogTemp, Error, TEXT("=== WIDGET DEBUG INFO ==="));
    
    // Check HealthBarWidgetComponent
    if (HealthBarWidgetComponent)
    {
        UE_LOG(LogTemp, Warning, TEXT("HealthBarWidgetComponent exists: %s"), *HealthBarWidgetComponent->GetName());
        
        UClass* WidgetClass = HealthBarWidgetComponent->GetWidgetClass();
        if (WidgetClass)
        {
            UE_LOG(LogTemp, Warning, TEXT("Widget Class: %s"), *WidgetClass->GetName());
        }
        else
        {
            UE_LOG(LogTemp, Error, TEXT("Widget Class is NULL! Set Widget Class in Blueprint!"));
        }
        
        UUserWidget* UserWidget = HealthBarWidgetComponent->GetUserWidgetObject();
        if (UserWidget)
        {
            UE_LOG(LogTemp, Warning, TEXT("UserWidget exists: %s"), *UserWidget->GetClass()->GetName());
        }
        else
        {
            UE_LOG(LogTemp, Error, TEXT("UserWidget is NULL! Widget not initialized!"));
        }
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("HealthBarWidgetComponent is NULL!"));
    }
    
    // Check CurrentHealthBarWidget
    if (CurrentHealthBarWidget)
    {
        UE_LOG(LogTemp, Warning, TEXT("CurrentHealthBarWidget exists: %s"), *CurrentHealthBarWidget->GetClass()->GetName());
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("CurrentHealthBarWidget is NULL!"));
    }
    
    UE_LOG(LogTemp, Error, TEXT("=== END WIDGET DEBUG INFO ==="));
}
