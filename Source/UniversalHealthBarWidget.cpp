#include "UniversalHealthBarWidget.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"
#include "BloodreadBaseCharacter.h"
#include "PracticeDummy.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "TimerManager.h"

void UUniversalHealthBarWidget::InitializeWithCharacter(ABloodreadBaseCharacter* Character)
{
    if (!Character)
    {
        UE_LOG(LogTemp, Error, TEXT("UniversalHealthBar: InitializeWithCharacter - Character is NULL"));
        return;
    }

    OwnerCharacter = Character;
    OwnerDummy = nullptr;

    UE_LOG(LogTemp, Warning, TEXT("UniversalHealthBar: Initialized with BloodreadBaseCharacter %s"), 
           *Character->GetName());

    // Initial health data update
    RefreshHealthData();
    UpdateHealthDisplay();
}

void UUniversalHealthBarWidget::InitializeWithDummy(APracticeDummy* Dummy)
{
    if (!Dummy)
    {
        UE_LOG(LogTemp, Error, TEXT("UniversalHealthBar: InitializeWithDummy - Dummy is NULL"));
        return;
    }

    OwnerDummy = Dummy;
    OwnerCharacter = nullptr;

    UE_LOG(LogTemp, Warning, TEXT("UniversalHealthBar: Initialized with PracticeDummy %s"), 
           *Dummy->GetName());

    // Initial health data update
    RefreshHealthData();
    UpdateHealthDisplay();
}

void UUniversalHealthBarWidget::UpdateHealthDisplay()
{
    // Refresh health data from owner
    RefreshHealthData();

    UE_LOG(LogTemp, Log, TEXT("UniversalHealthBar: UpdateHealthDisplay - Health: %d/%d (%.1f%%)"), 
           CurrentHealth, MaxHealth, HealthPercentage * 100.0f);

    // Update progress bar
    if (HealthProgressBar)
    {
        HealthProgressBar->SetPercent(HealthPercentage);
        
        // Set color based on health percentage
        FLinearColor HealthColor;
        if (HealthPercentage > 0.6f)
        {
            HealthColor = FLinearColor::Green;
        }
        else if (HealthPercentage > 0.3f)
        {
            HealthColor = FLinearColor::Yellow;
        }
        else
        {
            HealthColor = FLinearColor::Red;
        }
        
        HealthProgressBar->SetFillColorAndOpacity(HealthColor);
    }

    // Update health text
    if (HealthText)
    {
        FText HealthTextValue = FText::FromString(FString::Printf(TEXT("%d/%d"), CurrentHealth, MaxHealth));
        HealthText->SetText(HealthTextValue);
    }

    // Update character name text
    if (CharacterNameText)
    {
        FText NameText = FText::FromString(GetOwnerDisplayName());
        CharacterNameText->SetText(NameText);
    }
}

float UUniversalHealthBarWidget::GetHealthPercentage() const
{
    return HealthPercentage;
}

FText UUniversalHealthBarWidget::GetHealthText() const
{
    return FText::FromString(FString::Printf(TEXT("%d/%d"), CurrentHealth, MaxHealth));
}

FText UUniversalHealthBarWidget::GetCharacterName() const
{
    return FText::FromString(GetOwnerDisplayName());
}

FText UUniversalHealthBarWidget::GetCharacterClass() const
{
    return FText::FromString(GetOwnerClassName());
}

bool UUniversalHealthBarWidget::IsOwnerAlive() const
{
    if (OwnerCharacter)
    {
        return OwnerCharacter->GetIsAlive();
    }
    else if (OwnerDummy)
    {
        return OwnerDummy->IsAlive();
    }
    return false;
}

void UUniversalHealthBarWidget::RefreshHealthData()
{
    if (OwnerCharacter)
    {
        // Get health data from BloodreadBaseCharacter
        CurrentHealth = OwnerCharacter->GetCurrentHealth();
        MaxHealth = OwnerCharacter->GetMaxHealth();
        
        UE_LOG(LogTemp, VeryVerbose, TEXT("UniversalHealthBar: Character health - %d/%d"), 
               CurrentHealth, MaxHealth);
    }
    else if (OwnerDummy)
    {
        // Get health data from PracticeDummy
        CurrentHealth = OwnerDummy->GetCurrentHealth();
        MaxHealth = OwnerDummy->GetMaxHealth();
        
        UE_LOG(LogTemp, VeryVerbose, TEXT("UniversalHealthBar: Dummy health - %d/%d"), 
               CurrentHealth, MaxHealth);
    }
    else
    {
        // No valid owner
        CurrentHealth = 0;
        MaxHealth = 100;
        UE_LOG(LogTemp, Warning, TEXT("UniversalHealthBar: No valid owner for health data"));
    }

    // Calculate health percentage
    if (MaxHealth > 0)
    {
        HealthPercentage = (float)CurrentHealth / (float)MaxHealth;
    }
    else
    {
        HealthPercentage = 0.0f;
    }
}

FString UUniversalHealthBarWidget::GetOwnerDisplayName() const
{
    if (OwnerCharacter)
    {
        // For player characters, show a more descriptive name
        FString ActorName = OwnerCharacter->GetName();
        
        // Clean up the name (remove Blueprint suffixes)
        ActorName = ActorName.Replace(TEXT("_C_"), TEXT(" "));
        ActorName = ActorName.Replace(TEXT("BP_"), TEXT(""));
        ActorName = ActorName.Replace(TEXT("Character_"), TEXT(""));
        
        return ActorName;
    }
    else if (OwnerDummy)
    {
        return TEXT("Practice Dummy");
    }
    
    return TEXT("Unknown");
}

FString UUniversalHealthBarWidget::GetOwnerClassName() const
{
    if (OwnerCharacter)
    {
        // Get the character class enum and convert to readable string
        ECharacterClass CharClass = OwnerCharacter->GetCharacterClass();
        
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
    else if (OwnerDummy)
    {
        return TEXT("Training Dummy");
    }
    
    return TEXT("Unknown");
}

// Auto-update system implementation
void UUniversalHealthBarWidget::NativeConstruct()
{
    Super::NativeConstruct();
    
    UE_LOG(LogTemp, Warning, TEXT("UniversalHealthBar: NativeConstruct called"));
    
    // Start auto-update if enabled
    if (bAutoStartUpdating)
    {
        StartAutoUpdate();
    }
}

void UUniversalHealthBarWidget::NativeDestruct()
{
    // Clean up timer
    StopAutoUpdate();
    
    Super::NativeDestruct();
}

void UUniversalHealthBarWidget::StartAutoUpdate()
{
    if (GetWorld())
    {
        UE_LOG(LogTemp, Warning, TEXT("UniversalHealthBar: Starting auto-update with interval %.2f seconds"), UpdateInterval);
        
        GetWorld()->GetTimerManager().SetTimer(
            UpdateTimerHandle,
            this,
            &UUniversalHealthBarWidget::UpdateDisplay,
            UpdateInterval,
            true  // Looping
        );
    }
}

void UUniversalHealthBarWidget::StopAutoUpdate()
{
    if (GetWorld() && UpdateTimerHandle.IsValid())
    {
        UE_LOG(LogTemp, Warning, TEXT("UniversalHealthBar: Stopping auto-update"));
        GetWorld()->GetTimerManager().ClearTimer(UpdateTimerHandle);
    }
}

void UUniversalHealthBarWidget::UpdateDisplay()
{
    // Refresh data from owner
    RefreshHealthData();
    
    // Apply visual updates to widget components
    ApplyHealthToWidgets();
}

void UUniversalHealthBarWidget::ApplyHealthToWidgets()
{
    // Update progress bar
    if (HealthProgressBar)
    {
        HealthProgressBar->SetPercent(HealthPercentage);
        
        // Optional: Change color based on health
        FLinearColor HealthColor = FLinearColor::Green;
        if (HealthPercentage < 0.3f)
        {
            HealthColor = FLinearColor::Red;
        }
        else if (HealthPercentage < 0.6f)
        {
            HealthColor = FLinearColor::Yellow;
        }
        
        HealthProgressBar->SetFillColorAndOpacity(HealthColor);
    }
    
    // Update health text
    if (HealthText)
    {
        FText HealthDisplayText = FText::Format(
            FText::FromString(TEXT("{0}/{1}")),
            FText::AsNumber(CurrentHealth),
            FText::AsNumber(MaxHealth)
        );
        HealthText->SetText(HealthDisplayText);
    }
    
    // Update character name/class text
    if (CharacterNameText)
    {
        FString DisplayName = GetOwnerClassName();
        CharacterNameText->SetText(FText::FromString(DisplayName));
    }
}
