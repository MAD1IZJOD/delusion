#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "DelusionHUDWidget.generated.h"

class UBorder;
class UTextBlock;

/** Lightweight native UMG interface; no external UI runtime is required. */
UCLASS()
class DELUSION_API UDelusionHUDWidget : public UUserWidget
{
    GENERATED_BODY()

public:
    void SetPauseMenuVisible(bool bVisible);
    void ShowInteractionFeedback();
    void ShowMessage(const FText& Message);

protected:
    virtual void NativeOnInitialized() override;
    virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;
    virtual FReply NativeOnKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent) override;

private:
    UFUNCTION()
    void ResumeClicked();

    UFUNCTION()
    void QuitClicked();

    UPROPERTY(Transient)
    TObjectPtr<UBorder> PausePanel;

    UPROPERTY(Transient)
    TObjectPtr<UBorder> PromptPanel;

    UPROPERTY(Transient)
    TObjectPtr<UTextBlock> PromptText;

    UPROPERTY(Transient)
    TObjectPtr<UTextBlock> FeedbackText;

    UPROPERTY(Transient)
    TObjectPtr<UTextBlock> ControlsText;

    UPROPERTY(Transient)
    TObjectPtr<UTextBlock> VehicleText;

    bool bShowingDrivingControls = false;

    float FeedbackTimeRemaining = 0.0f;
};
