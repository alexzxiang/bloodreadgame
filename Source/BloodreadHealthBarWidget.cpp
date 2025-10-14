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
    
    UE_LOG(LogTemp, Warning, TEXT("BloodreadHealthBarWidget: NativeConstruct called"));
    
    // Validate widget components (all are now optional)
    if (!HealthBar)
    {
        UE_LOG(LogTemp, Warning, TEXT("HealthBar not found - widget binding is optional"));
    }
    
    if (!HealthProgressBar)
    {
        UE_LOG(LogTemp, Warning, TEXT("HealthProgressBar not found - widget binding is optional"));
    }
    
    if (!HealthText)
    {
        UE_LOG(LogTemp, Warning, TEXT("HealthText not found - widget binding is optional"));
    }
    
    if (!CharacterClassText)
    {
        UE_LOG(LogTemp, Warning, TEXT("CharacterClassText not found - widget binding is optional"));
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
        if (HealthBar)
        {
            HealthBar->SetPercent(0.0f);
        }
        
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
        
        return;
    }
    
    float HealthPercent = PlayerCharacterRef->GetHealthPercent();
    
    // Update health progress bars (we have two for flexibility)
    if (HealthBar)
    {
        HealthBar->SetPercent(HealthPercent);
        
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
        
        HealthBar->SetFillColorAndOpacity(BarColor);
    }
    
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
            default:
                ClassName = "Unknown";
                break;
        }
        CharacterClassText->SetText(FText::FromString(ClassName));
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
