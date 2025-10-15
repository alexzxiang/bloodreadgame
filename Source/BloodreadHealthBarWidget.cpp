#include "BloodreadHealthBarWidget.h"
#include "BloodreadBaseCharacter.h"
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
    
    UE_LOG(LogTemp, Warning, TEXT("=== BloodreadHealthBarWidget::NativeConstruct START ==="));
    
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
    if (!PlayerCharacterRef)
    {
        // No character reference - show default/empty state
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
    
    // Update health progress bar
    if (HealthProgressBar)
    {
        HealthProgressBar->SetPercent(HealthPercent);
        
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
        // For now, assume abilities are always ready (you can add cooldown logic later)
        float Ability1Progress = 1.0f; // Replace with actual cooldown progress
        Ability1ProgressBar->SetPercent(Ability1Progress);
        
        // Set ability bar color (green when ready, red when on cooldown)
        FLinearColor AbilityColor = Ability1Progress >= 1.0f ? FLinearColor::Green : FLinearColor::Red;
        Ability1ProgressBar->SetFillColorAndOpacity(AbilityColor);
    }
    
    if (Ability2ProgressBar)
    {
        // For now, assume abilities are always ready (you can add cooldown logic later)
        float Ability2Progress = 1.0f; // Replace with actual cooldown progress
        Ability2ProgressBar->SetPercent(Ability2Progress);
        
        // Set ability bar color (green when ready, red when on cooldown)
        FLinearColor AbilityColor = Ability2Progress >= 1.0f ? FLinearColor::Green : FLinearColor::Red;
        Ability2ProgressBar->SetFillColorAndOpacity(AbilityColor);
    }
}

void UBloodreadHealthBarWidget::SetUpdateFrequency(float NewUpdateFrequency)
{
    UpdateFrequency = FMath::Max(0.1f, NewUpdateFrequency); // Minimum 0.1 seconds
    
    if (GetWorld())
    {
        // Clear existing timer
        GetWorld()->GetTimerManager().ClearTimer(UpdateTimerHandle);
        
        // Set new timer
        GetWorld()->GetTimerManager().SetTimer(
            UpdateTimerHandle,
            this,
            &UBloodreadHealthBarWidget::UpdateHealthDisplay,
            1.0f / UpdateFrequency,
            true // Loop
        );
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
        return PlayerCharacterRef->GetHealthText();
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
        return PlayerCharacterRef->GetManaText();
    }
    
    return FString(TEXT("0/0"));
}