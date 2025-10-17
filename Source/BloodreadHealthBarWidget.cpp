#include "BloodreadHealthBarWidget.h"
#include "BloodreadBaseCharacter.h"
#include "BloodreadPlayerCharacter.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "Kismet/GameplayStatics.h"

UBloodreadHealthBarWidget::UBloodreadHealthBarWidget(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
{
    // Set default values
    PlayerCharacterRef = nullptr;
    UpdateFrequency = 10.0f;
    
    // Set default colors
    HealthyColor = FLinearColor::Green;
    DamagedColor = FLinearColor::Yellow;
    CriticalColor = FLinearColor::Red;
}

void UBloodreadHealthBarWidget::NativeConstruct()
{
    Super::NativeConstruct();
    
    UWorld* World = GetWorld();
    bool bIsServer = World ? World->GetNetMode() == NM_DedicatedServer || World->GetNetMode() == NM_ListenServer : false;
    bool bIsClient = World ? World->GetNetMode() == NM_Client : false;
    
    UE_LOG(LogTemp, Warning, TEXT("=== BloodreadHealthBarWidget::NativeConstruct START - Server: %s, Client: %s ==="),
           bIsServer ? TEXT("YES") : TEXT("NO"), 
           bIsClient ? TEXT("YES") : TEXT("NO"));
    
    // Count bound widgets
    int32 BoundWidgetCount = 0;
    int32 TotalRequiredWidgets = 7; // HealthProgressBar, HealthText, CharacterClassText, ManaProgressBar, ManaText, Ability1ProgressBar, Ability2ProgressBar
    
    // Validate widget components (all are now required)
    if (!HealthProgressBar)
    {
        UE_LOG(LogTemp, Error, TEXT("REQUIRED: HealthProgressBar not found - add a Progress Bar named 'HealthProgressBar' to your Blueprint widget"));
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("HealthProgressBar FOUND and bound successfully"));
        BoundWidgetCount++;
    }
    
    if (!HealthText)
    {
        UE_LOG(LogTemp, Error, TEXT("REQUIRED: HealthText not found - add a Text Block named 'HealthText' to your Blueprint widget"));
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("HealthText FOUND and bound successfully"));
        BoundWidgetCount++;
    }
    
    if (!CharacterClassText)
    {
        UE_LOG(LogTemp, Error, TEXT("REQUIRED: CharacterClassText not found - add a Text Block named 'CharacterClassText' to your Blueprint widget"));
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("CharacterClassText FOUND and bound successfully"));
        BoundWidgetCount++;
    }
    
    if (!ManaProgressBar)
    {
        UE_LOG(LogTemp, Error, TEXT("REQUIRED: ManaProgressBar not found - add a Progress Bar named 'ManaProgressBar' to your Blueprint widget"));
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("ManaProgressBar FOUND and bound successfully"));
        BoundWidgetCount++;
    }
    
    if (!ManaText)
    {
        UE_LOG(LogTemp, Error, TEXT("REQUIRED: ManaText not found - add a Text Block named 'ManaText' to your Blueprint widget"));
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("ManaText FOUND and bound successfully"));
        BoundWidgetCount++;
    }
    
    if (!Ability1ProgressBar)
    {
        UE_LOG(LogTemp, Error, TEXT("REQUIRED: Ability1ProgressBar not found - add a Progress Bar named 'Ability1ProgressBar' to your Blueprint widget"));
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("Ability1ProgressBar FOUND and bound successfully"));
        BoundWidgetCount++;
    }
    
    if (!Ability2ProgressBar)
    {
        UE_LOG(LogTemp, Error, TEXT("REQUIRED: Ability2ProgressBar not found - add a Progress Bar named 'Ability2ProgressBar' to your Blueprint widget"));
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("Ability2ProgressBar FOUND and bound successfully"));
        BoundWidgetCount++;
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Total bound widgets: %d / %d"), BoundWidgetCount, TotalRequiredWidgets);
    
    if (BoundWidgetCount < TotalRequiredWidgets)
    {
        UE_LOG(LogTemp, Error, TEXT("MISSING REQUIRED WIDGETS! Your Blueprint widget must have UI elements with these exact names:"));
        UE_LOG(LogTemp, Error, TEXT("- Progress Bar named 'HealthProgressBar'"));
        UE_LOG(LogTemp, Error, TEXT("- Progress Bar named 'ManaProgressBar'"));
        UE_LOG(LogTemp, Error, TEXT("- Progress Bar named 'Ability1ProgressBar'"));
        UE_LOG(LogTemp, Error, TEXT("- Progress Bar named 'Ability2ProgressBar'"));
        UE_LOG(LogTemp, Error, TEXT("- Text Block named 'HealthText'"));
        UE_LOG(LogTemp, Error, TEXT("- Text Block named 'ManaText'"));
        UE_LOG(LogTemp, Error, TEXT("- Text Block named 'CharacterClassText'"));
    }
    
    // Only initialize UI on client side (even for listen server host)
    // Reuse existing World variable from above
    bool bIsListenServer = World && World->GetNetMode() == NM_ListenServer;
    // Update bIsClient to include standalone mode
    bIsClient = World && (World->GetNetMode() == NM_Client || World->GetNetMode() == NM_Standalone);
    
    if (!bIsClient && !bIsListenServer)
    {
        UE_LOG(LogTemp, Warning, TEXT("BloodreadHealthBarWidget: Skipping UI initialization on dedicated server"));
        return;
    }
    
    // Try to automatically find the player character
    if (!PlayerCharacterRef)
    {
        if (APlayerController* PC = UGameplayStatics::GetPlayerController(this, 0))
        {
            if (ABloodreadBaseCharacter* Character = Cast<ABloodreadBaseCharacter>(PC->GetPawn()))
            {
                InitializeHealthBar(Character);
                UE_LOG(LogTemp, Warning, TEXT("BloodreadHealthBarWidget: Auto-found player character"));
            }
        }
    }
    
    // Start the update timer
    SetUpdateFrequency(UpdateFrequency);
}

void UBloodreadHealthBarWidget::NativeDestruct()
{
    // Clean up timer and event bindings
    if (GetWorld())
    {
        GetWorld()->GetTimerManager().ClearTimer(UpdateTimerHandle);
    }
    
    UnbindHealthEvents();
    
    Super::NativeDestruct();
    
    UE_LOG(LogTemp, Warning, TEXT("BloodreadHealthBarWidget: NativeDestruct called"));
}

void UBloodreadHealthBarWidget::InitializeHealthBar(ABloodreadBaseCharacter* Character)
{
    if (!Character)
    {
        UE_LOG(LogTemp, Warning, TEXT("BloodreadHealthBarWidget: Cannot initialize with null character"));
        return;
    }
    
    // Unbind from previous character
    UnbindHealthEvents();
    
    // Set new character reference
    PlayerCharacterRef = Character;
    
    // Bind to new character's health events
    BindHealthEvents(Character);
    
    // Update display immediately
    UpdateHealthDisplay();
    
    UE_LOG(LogTemp, Warning, TEXT("BloodreadHealthBarWidget: Initialized with character: %s"), 
           *Character->GetClass()->GetName());
}

void UBloodreadHealthBarWidget::UpdateHealthDisplay()
{
    // Log network mode details
    UWorld* World = GetWorld();
    bool bIsClient = World && (World->GetNetMode() == NM_Client || World->GetNetMode() == NM_Standalone);
    bool bIsListenServer = World && World->GetNetMode() == NM_ListenServer;
    bool bIsDedicatedServer = World && World->GetNetMode() == NM_DedicatedServer;
    
    UE_LOG(LogTemp, Warning, TEXT("UpdateHealthDisplay called - Client: %s, ListenServer: %s, DedicatedServer: %s"), 
           bIsClient ? TEXT("YES") : TEXT("NO"), 
           bIsListenServer ? TEXT("YES") : TEXT("NO"), 
           bIsDedicatedServer ? TEXT("YES") : TEXT("NO"));

    // Don't update UI on dedicated server only - listen server needs UI for host player
    if (bIsDedicatedServer)
    {
        UE_LOG(LogTemp, Warning, TEXT("UpdateHealthDisplay: Skipping UI updates on dedicated server"));
        return;
    }

    // Try to find a valid character reference if we don't have one
    if (!PlayerCharacterRef)
    {
        UE_LOG(LogTemp, Warning, TEXT("UpdateHealthDisplay: No PlayerCharacterRef, trying to find one..."));
        
        // First try to get the character from the PlayerController
        if (APlayerController* PC = UGameplayStatics::GetPlayerController(World, 0))
        {
            UE_LOG(LogTemp, Warning, TEXT("UpdateHealthDisplay: Found PlayerController: %s"), *PC->GetName());
            
            if (APawn* Pawn = PC->GetPawn())
            {
                UE_LOG(LogTemp, Warning, TEXT("UpdateHealthDisplay: PlayerController has pawn: %s"), *Pawn->GetName());
                
                if (ABloodreadBaseCharacter* Character = Cast<ABloodreadBaseCharacter>(Pawn))
                {
                    PlayerCharacterRef = Character;
                    UE_LOG(LogTemp, Warning, TEXT("UpdateHealthDisplay: Found character via PlayerController: %s"), *Character->GetName());
                }
                else
                {
                    UE_LOG(LogTemp, Warning, TEXT("UpdateHealthDisplay: Pawn is not a BloodreadBaseCharacter"));
                }
            }
            else
            {
                UE_LOG(LogTemp, Warning, TEXT("UpdateHealthDisplay: PlayerController has no pawn"));
            }
        }
        else
        {
            UE_LOG(LogTemp, Warning, TEXT("UpdateHealthDisplay: No PlayerController found"));
        }
        
        // If we still don't have a character, try TargetCharacter
        if (!PlayerCharacterRef && TargetCharacter)
        {
            PlayerCharacterRef = TargetCharacter;
            UE_LOG(LogTemp, Warning, TEXT("UpdateHealthDisplay: Using TargetCharacter: %s"), *TargetCharacter->GetName());
        }
        else if (!PlayerCharacterRef)
        {
            UE_LOG(LogTemp, Warning, TEXT("UpdateHealthDisplay: TargetCharacter is also null"));
        }
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("UpdateHealthDisplay: Using existing PlayerCharacterRef: %s"), *PlayerCharacterRef->GetName());
    }
    
    if (!PlayerCharacterRef)
    {
        // No character reference - show default/empty state
        UE_LOG(LogTemp, Error, TEXT("UpdateHealthDisplay: No character reference available - showing default state"));
        
        if (HealthProgressBar)
        {
            HealthProgressBar->SetPercent(0.0f);
        }
        
        if (HealthText)
        {
            HealthText->SetText(FText::FromString("0/0"));
        }
        
        if (CharacterClassText)
        {
            CharacterClassText->SetText(FText::FromString("No Character"));
        }
        
        if (ManaProgressBar)
        {
            ManaProgressBar->SetPercent(0.0f);
        }
        
        if (ManaText)
        {
            ManaText->SetText(FText::FromString("0/0"));
        }
        
        if (Ability1ProgressBar)
        {
            Ability1ProgressBar->SetPercent(1.0f); // Abilities start ready (full bar)
        }
        
        if (Ability2ProgressBar)
        {
            Ability2ProgressBar->SetPercent(1.0f); // Abilities start ready (full bar)
        }
        
        return;
    }
    
    float HealthPercent = PlayerCharacterRef->GetHealthPercent();
    
    UE_LOG(LogTemp, Warning, TEXT("UpdateHealthDisplay: Character %s has health percent: %f"), 
           *PlayerCharacterRef->GetName(), HealthPercent);
    
    // Update health progress bar
    if (HealthProgressBar)
    {
        HealthProgressBar->SetPercent(HealthPercent);
        UE_LOG(LogTemp, Warning, TEXT("UpdateHealthDisplay: Set HealthProgressBar to %f"), HealthPercent);
        
        // Update color based on health percentage
        FLinearColor BarColor;
        if (HealthPercent > 0.6f)
        {
            BarColor = HealthyColor;
        }
        else if (HealthPercent > 0.3f)
        {
            BarColor = DamagedColor;
        }
        else
        {
            BarColor = CriticalColor;
        }
        
        HealthProgressBar->SetFillColorAndOpacity(BarColor);
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("UpdateHealthDisplay: HealthProgressBar is null!"));
    }
    
    // Update health text
    if (HealthText)
    {
        int32 CurrentHealth = static_cast<int32>(PlayerCharacterRef->GetCurrentHealthFloat());
        int32 MaxHealth = static_cast<int32>(PlayerCharacterRef->GetMaxHealthFloat());
        FString HealthString = FString::Printf(TEXT("%d/%d"), CurrentHealth, MaxHealth);
        HealthText->SetText(FText::FromString(HealthString));
    }
    
    // Update character class text
    if (CharacterClassText)
    {
        FString ClassName;
        switch (PlayerCharacterRef->GetCharacterClass())
        {
            case ECharacterClass::Warrior:
                ClassName = "Warrior";
                break;
            case ECharacterClass::Mage:
                ClassName = "Mage";
                break;
            case ECharacterClass::Rogue:
                ClassName = "Rogue";
                break;
            case ECharacterClass::Healer:
                ClassName = "Healer";
                break;
            case ECharacterClass::Dragon:
                ClassName = "Dragon";
                break;
            default:
                ClassName = "Unknown";
                break;
        }
        CharacterClassText->SetText(FText::FromString(ClassName));
    }
    
    // Update mana progress bar
    if (ManaProgressBar)
    {
        float ManaPercent = PlayerCharacterRef->GetManaPercentage();
        ManaProgressBar->SetPercent(ManaPercent);
        
        // Set mana bar color (you can customize these colors)
        FLinearColor ManaColor = FLinearColor::Blue;
        if (ManaPercent < 0.3f)
        {
            ManaColor = FLinearColor(0.4f, 0.4f, 1.0f, 1.0f); // Darker blue when low
        }
        ManaProgressBar->SetFillColorAndOpacity(ManaColor);
    }
    
    // Update mana text
    if (ManaText)
    {
        int32 CurrentMana = PlayerCharacterRef->GetCurrentMana();
        int32 MaxMana = PlayerCharacterRef->GetMaxMana();
        FString ManaString = FString::Printf(TEXT("%d/%d"), CurrentMana, MaxMana);
        ManaText->SetText(FText::FromString(ManaString));
    }
    
    // Update ability progress bars (1.0 = ready, 0.0 = on cooldown)
    if (Ability1ProgressBar)
    {
        // Get actual cooldown progress (inverted so 1.0 = ready, 0.0 = on cooldown)
        float Ability1CooldownPercent = PlayerCharacterRef->GetAbility1CooldownPercentage();
        float Ability1Progress = 1.0f - Ability1CooldownPercent; // Invert: 1.0 = ready, 0.0 = on cooldown
        Ability1ProgressBar->SetPercent(Ability1Progress);
        
        // Set ability bar color (green when ready, red when on cooldown)
        FLinearColor AbilityColor = Ability1Progress >= 1.0f ? FLinearColor::Green : FLinearColor::Red;
        Ability1ProgressBar->SetFillColorAndOpacity(AbilityColor);
    }
    
    if (Ability2ProgressBar)
    {
        // Get actual cooldown progress (inverted so 1.0 = ready, 0.0 = on cooldown)
        float Ability2CooldownPercent = PlayerCharacterRef->GetAbility2CooldownPercentage();
        float Ability2Progress = 1.0f - Ability2CooldownPercent; // Invert: 1.0 = ready, 0.0 = on cooldown
        Ability2ProgressBar->SetPercent(Ability2Progress);
        
        // Set ability bar color (green when ready, red when on cooldown)
        FLinearColor AbilityColor = Ability2Progress >= 1.0f ? FLinearColor::Green : FLinearColor::Red;
        Ability2ProgressBar->SetFillColorAndOpacity(AbilityColor);
    }
}

void UBloodreadHealthBarWidget::SetUpdateFrequency(float NewUpdateFrequency)
{
    UpdateFrequency = FMath::Max(0.1f, NewUpdateFrequency); // Minimum 0.1 seconds
    
    UWorld* World = GetWorld();
    bool bIsClient = World && (World->GetNetMode() == NM_Client || World->GetNetMode() == NM_Standalone);
    bool bIsListenServer = World && World->GetNetMode() == NM_ListenServer;
    bool bIsDedicatedServer = World && World->GetNetMode() == NM_DedicatedServer;
    
    UE_LOG(LogTemp, Warning, TEXT("SetUpdateFrequency called with %f, Client: %s, ListenServer: %s, DedicatedServer: %s"), 
           NewUpdateFrequency, bIsClient ? TEXT("YES") : TEXT("NO"), 
           bIsListenServer ? TEXT("YES") : TEXT("NO"), 
           bIsDedicatedServer ? TEXT("YES") : TEXT("NO"));
    
    // Don't set up UI update timers on dedicated server only
    if (bIsDedicatedServer)
    {
        UE_LOG(LogTemp, Warning, TEXT("SetUpdateFrequency: Skipping timer setup on dedicated server (UI not needed)"));
        return;
    }
    
    if (World)
    {
        // Clear existing timer
        World->GetTimerManager().ClearTimer(UpdateTimerHandle);
        
        // Set new timer
        World->GetTimerManager().SetTimer(
            UpdateTimerHandle,
            this,
            &UBloodreadHealthBarWidget::UpdateHealthDisplay,
            1.0f / UpdateFrequency,
            true // Loop
        );
        
        UE_LOG(LogTemp, Warning, TEXT("SetUpdateFrequency: Timer set with frequency %f seconds"), 1.0f / UpdateFrequency);
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("SetUpdateFrequency: No world available!"));
    }
}

void UBloodreadHealthBarWidget::BindHealthEvents(ABloodreadBaseCharacter* Character)
{
    if (!Character)
    {
        return;
    }
    
    // Note: If your BloodreadBaseCharacter has health change events/delegates,
    // you would bind to them here. For now, we rely on the timer-based updates.
    
    UE_LOG(LogTemp, Warning, TEXT("BloodreadHealthBarWidget: Bound to character health events"));
}

void UBloodreadHealthBarWidget::UnbindHealthEvents()
{
    // Unbind any health change event delegates here if they exist
    
    UE_LOG(LogTemp, Warning, TEXT("BloodreadHealthBarWidget: Unbound from health events"));
}

void UBloodreadHealthBarWidget::OnCharacterHealthChanged(int32 OldHealth, int32 NewHealth)
{
    // This function would be called by health change delegates
    // For now, the timer-based updates handle health changes
    
    UE_LOG(LogTemp, Warning, TEXT("BloodreadHealthBarWidget: Health changed from %d to %d"), OldHealth, NewHealth);
    
    // Update display immediately when health changes
    UpdateHealthDisplay();
}

FString UBloodreadHealthBarWidget::GetHealthText() const
{
    if (TargetCharacter)
    {
        return TargetCharacter->GetHealthText();
    }
    else if (PlayerCharacterRef)
    {
        // BloodreadPlayerCharacter doesn't have GetHealthText(), so build it from individual values
        return FString::Printf(TEXT("%d/%d"), PlayerCharacterRef->GetCurrentHealth(), PlayerCharacterRef->GetMaxHealth());
    }
    
    // Also try to find a BloodreadPlayerCharacter if we don't have a BaseCharacter reference
    if (APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0))
    {
        if (ABloodreadPlayerCharacter* PlayerChar = Cast<ABloodreadPlayerCharacter>(PC->GetPawn()))
        {
            return FString::Printf(TEXT("%d/%d"), PlayerChar->GetCurrentHealth(), PlayerChar->GetMaxHealth());
        }
    }
    
    return FString(TEXT("0/0"));
}

FString UBloodreadHealthBarWidget::GetManaText() const
{
    if (TargetCharacter)
    {
        return TargetCharacter->GetManaText();
    }
    else if (PlayerCharacterRef)
    {
        // BloodreadPlayerCharacter doesn't have GetManaText(), so we need to check if it has mana properties
        // For now, return a placeholder since PlayerCharacter might not have mana system yet
        return FString(TEXT("N/A"));
    }
    
    // Also try to find a BloodreadPlayerCharacter if we don't have a BaseCharacter reference
    if (APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0))
    {
        if (ABloodreadPlayerCharacter* PlayerChar = Cast<ABloodreadPlayerCharacter>(PC->GetPawn()))
        {
            // BloodreadPlayerCharacter doesn't have mana system, return placeholder
            return FString(TEXT("N/A"));
        }
    }
    
    return FString(TEXT("0/0"));
}

float UBloodreadHealthBarWidget::GetAbility1CooldownPercentage() const
{
    if (TargetCharacter)
    {
        return TargetCharacter->GetAbility1CooldownPercentage();
    }
    else if (PlayerCharacterRef)
    {
        return PlayerCharacterRef->GetAbility1CooldownPercentage();
    }
    
    // Also try to find a BloodreadPlayerCharacter if we don't have a BaseCharacter reference
    if (APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0))
    {
        if (ABloodreadPlayerCharacter* PlayerChar = Cast<ABloodreadPlayerCharacter>(PC->GetPawn()))
        {
            return PlayerChar->GetAbility1CooldownPercentage();
        }
    }
    
    return 1.0f; // Default to ready (100%)
}

float UBloodreadHealthBarWidget::GetAbility2CooldownPercentage() const
{
    if (TargetCharacter)
    {
        return TargetCharacter->GetAbility2CooldownPercentage();
    }
    else if (PlayerCharacterRef)
    {
        return PlayerCharacterRef->GetAbility2CooldownPercentage();
    }
    
    // Also try to find a BloodreadPlayerCharacter if we don't have a BaseCharacter reference
    if (APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0))
    {
        if (ABloodreadPlayerCharacter* PlayerChar = Cast<ABloodreadPlayerCharacter>(PC->GetPawn()))
        {
            return PlayerChar->GetAbility2CooldownPercentage();
        }
    }
    
    return 1.0f; // Default to ready (100%)
}

FString UBloodreadHealthBarWidget::GetAbility1Name() const
{
    if (TargetCharacter)
    {
        return TargetCharacter->GetAbility1Name();
    }
    else if (PlayerCharacterRef)
    {
        return PlayerCharacterRef->GetAbility1Name();
    }
    
    // Also try to find a BloodreadPlayerCharacter if we don't have a BaseCharacter reference
    if (APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0))
    {
        if (ABloodreadBaseCharacter* Character = Cast<ABloodreadBaseCharacter>(PC->GetPawn()))
        {
            return Character->GetAbility1Name();
        }
    }
    
    return FString(TEXT("Ability 1"));
}

FString UBloodreadHealthBarWidget::GetAbility2Name() const
{
    if (TargetCharacter)
    {
        return TargetCharacter->GetAbility2Name();
    }
    else if (PlayerCharacterRef)
    {
        return PlayerCharacterRef->GetAbility2Name();
    }
    
    // Also try to find a BloodreadPlayerCharacter if we don't have a BaseCharacter reference
    if (APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0))
    {
        if (ABloodreadBaseCharacter* Character = Cast<ABloodreadBaseCharacter>(PC->GetPawn()))
        {
            return Character->GetAbility2Name();
        }
    }
    
    return FString(TEXT("Ability 2"));
}