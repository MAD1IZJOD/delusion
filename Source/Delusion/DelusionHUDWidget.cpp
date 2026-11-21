#include "DelusionHUDWidget.h"

#include "Blueprint/WidgetTree.h"
#include "Components/Border.h"
#include "Components/Button.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"
#include "Components/VerticalBoxSlot.h"
#include "DelusionInteractionComponent.h"
#include "DelusionPlayerController.h"
#include "Input/Reply.h"
#include "InputCoreTypes.h"
#include "Styling/CoreStyle.h"

void UDelusionHUDWidget::NativeOnInitialized()
{
    Super::NativeOnInitialized();
    SetIsFocusable(true);
    UCanvasPanel* Canvas = WidgetTree->ConstructWidget<UCanvasPanel>(UCanvasPanel::StaticClass(), TEXT("HUDCanvas"));
    WidgetTree->RootWidget = Canvas;
    Canvas->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
    auto MakeText = [this](const FString& Label, float FontSize, FLinearColor Color)
    {
        UTextBlock* Text = WidgetTree->ConstructWidget<UTextBlock>();
        Text->SetText(FText::FromString(Label));
        Text->SetFont(FCoreStyle::GetDefaultFontStyle(TEXT("Regular"), FontSize));
        Text->SetColorAndOpacity(FSlateColor(Color));
        Text->SetShadowOffset(FVector2D(1, 1));
        Text->SetShadowColorAndOpacity(FLinearColor::Black);
        Text->SetVisibility(ESlateVisibility::HitTestInvisible);
        return Text;
    };
    auto Place = [Canvas](UWidget* Widget, FVector2D Anchor, FVector2D Alignment, FVector2D Position)
    {
        UCanvasPanelSlot* Slot = Canvas->AddChildToCanvas(Widget);
        Slot->SetAnchors(FAnchors(Anchor.X, Anchor.Y));
        Slot->SetAlignment(Alignment);
        Slot->SetPosition(Position);
        Slot->SetAutoSize(true);
        return Slot;
    };
    Place(MakeText(TEXT("DELUSION"), 28, FLinearColor(0.15f, 0.8f, 1.0f)), FVector2D(0, 0), FVector2D(0, 0), FVector2D(24, 18));
    Place(MakeText(TEXT("FIRST DISTRICT  /  SPRINT 0"), 12, FLinearColor::White), FVector2D(0, 0), FVector2D(0, 0), FVector2D(25, 56));
    Place(MakeText(TEXT("WASD  Move    Mouse  Camera    Shift  Sprint\nSpace  Jump    E  Interact    Esc  Pause"), 14, FLinearColor::White),
        FVector2D(0, 1), FVector2D(0, 1), FVector2D(24, -22));

    PromptPanel = WidgetTree->ConstructWidget<UBorder>();
    PromptPanel->SetBrushColor(FLinearColor(0.015f, 0.025f, 0.04f, 0.92f));
    PromptPanel->SetPadding(FMargin(14, 8));
    PromptText = MakeText(TEXT(""), 20, FLinearColor::White);
    PromptPanel->SetContent(PromptText);
    Place(PromptPanel, FVector2D(0.5f, 0.82f), FVector2D(0.5f, 1), FVector2D::ZeroVector);
    PromptPanel->SetVisibility(ESlateVisibility::Collapsed);
    FeedbackText = MakeText(TEXT(""), 18, FLinearColor(0.25f, 1.0f, 0.55f));
    Place(FeedbackText, FVector2D(0.5f, 0.15f), FVector2D(0.5f, 0), FVector2D::ZeroVector);

    PausePanel = WidgetTree->ConstructWidget<UBorder>();
    PausePanel->SetBrushColor(FLinearColor(0.015f, 0.025f, 0.04f, 0.96f));
    PausePanel->SetPadding(FMargin(28));
    UCanvasPanelSlot* PauseSlot = Place(PausePanel, FVector2D(0.5f), FVector2D(0.5f), FVector2D::ZeroVector);
    PauseSlot->SetAutoSize(false);
    PauseSlot->SetSize(FVector2D(380, 260));
    UVerticalBox* Menu = WidgetTree->ConstructWidget<UVerticalBox>();
    PausePanel->SetContent(Menu);
    Menu->AddChildToVerticalBox(MakeText(TEXT("PAUSED"), 28, FLinearColor::White))->SetPadding(FMargin(0, 0, 0, 20));
    auto AddButton = [this, Menu, &MakeText](const FString& Label)
    {
        UButton* Button = WidgetTree->ConstructWidget<UButton>();
        Button->SetBackgroundColor(FLinearColor(0.08f, 0.35f, 0.5f));
        Button->SetContent(MakeText(Label, 22, FLinearColor::White));
        Menu->AddChildToVerticalBox(Button)->SetPadding(FMargin(0, 6));
        return Button;
    };
    AddButton(TEXT("Resume"))->OnClicked.AddDynamic(this, &UDelusionHUDWidget::ResumeClicked);
    AddButton(TEXT("Quit"))->OnClicked.AddDynamic(this, &UDelusionHUDWidget::QuitClicked);
    Menu->AddChildToVerticalBox(MakeText(TEXT("Escape also resumes the game"), 12, FLinearColor::White))->SetPadding(FMargin(0, 14, 0, 0));
    PausePanel->SetVisibility(ESlateVisibility::Collapsed);
}

void UDelusionHUDWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
    Super::NativeTick(MyGeometry, InDeltaTime);
    const ADelusionPlayerController* Player = Cast<ADelusionPlayerController>(GetOwningPlayer());
    if (Player && PromptText && PromptPanel)
    {
        const FText Prompt = Player->GetInteraction()->GetPrompt();
        if (!PromptText->GetText().EqualTo(Prompt)) { PromptText->SetText(Prompt); }
        PromptPanel->SetVisibility(!Player->IsPaused() && !Prompt.IsEmpty()
            ? ESlateVisibility::HitTestInvisible : ESlateVisibility::Collapsed);
    }
    if (FeedbackTimeRemaining > 0)
    {
        FeedbackTimeRemaining = FMath::Max(0.0f, FeedbackTimeRemaining - InDeltaTime);
        if (FeedbackTimeRemaining == 0 && FeedbackText) { FeedbackText->SetText(FText::GetEmpty()); }
    }
}

void UDelusionHUDWidget::SetPauseMenuVisible(bool bVisible)
{
    if (PausePanel) { PausePanel->SetVisibility(bVisible ? ESlateVisibility::Visible : ESlateVisibility::Collapsed); }
    if (bVisible && PromptPanel) { PromptPanel->SetVisibility(ESlateVisibility::Collapsed); }
}

void UDelusionHUDWidget::ShowInteractionFeedback()
{
    if (FeedbackText)
    {
        FeedbackText->SetText(NSLOCTEXT("Delusion", "InteractionSucceeded", "Terminal updated"));
        FeedbackTimeRemaining = 3.0f;
    }
}

FReply UDelusionHUDWidget::NativeOnKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent)
{
    ADelusionPlayerController* Player = Cast<ADelusionPlayerController>(GetOwningPlayer());
    if (Player && Player->IsPaused() && InKeyEvent.GetKey() == EKeys::Escape)
    {
        Player->ResumeGame();
        return FReply::Handled();
    }
    return Super::NativeOnKeyDown(InGeometry, InKeyEvent);
}

void UDelusionHUDWidget::ResumeClicked()
{
    if (ADelusionPlayerController* Player = Cast<ADelusionPlayerController>(GetOwningPlayer())) { Player->ResumeGame(); }
}

void UDelusionHUDWidget::QuitClicked()
{
    if (ADelusionPlayerController* Player = Cast<ADelusionPlayerController>(GetOwningPlayer())) { Player->QuitGame(); }
}
