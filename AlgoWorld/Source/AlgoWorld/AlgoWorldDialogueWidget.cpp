#include "AlgoWorldDialogueWidget.h"
#include "Blueprint/WidgetTree.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/Border.h"
#include "Components/VerticalBox.h"
#include "Components/VerticalBoxSlot.h"
#include "Components/TextBlock.h"

void UAlgoWorldDialogueWidget::NativeOnInitialized()
{
    Super::NativeOnInitialized();
    auto* Canvas=WidgetTree->ConstructWidget<UCanvasPanel>(UCanvasPanel::StaticClass(),TEXT("DialogueCanvas"));
    WidgetTree->RootWidget=Canvas;
    auto* Panel=WidgetTree->ConstructWidget<UBorder>();
    Panel->SetBrushColor(FLinearColor(0.025,0.045,0.04,0.96));
    Panel->SetPadding(FMargin(24,18));
    auto* DialoguePanelSlot=Canvas->AddChildToCanvas(Panel);
    DialoguePanelSlot->SetAnchors(FAnchors(0.12f,0.60f,0.88f,0.94f));
    DialoguePanelSlot->SetOffsets(FMargin(0));
    auto* Column=WidgetTree->ConstructWidget<UVerticalBox>();
    Panel->SetContent(Column);
    Heading=WidgetTree->ConstructWidget<UTextBlock>();
    Body=WidgetTree->ConstructWidget<UTextBlock>();
    Footer=WidgetTree->ConstructWidget<UTextBlock>();
    for(UTextBlock* Text : {Heading.Get(),Body.Get(),Footer.Get()})
    {
        auto Font=Text->GetFont();
        Font.Size=Text==Heading.Get() ? 24 : Text==Body.Get() ? 22 : 16;
        Text->SetFont(Font);
        Text->SetAutoWrapText(true);
        Text->SetColorAndOpacity(FSlateColor(FLinearColor(0.93,0.92,0.85)));
        auto* Row=Column->AddChildToVerticalBox(Text);
        Row->SetPadding(FMargin(0,0,0,12));
        if(Text==Body.Get()) Row->SetSize(FSlateChildSize(ESlateSizeRule::Fill));
    }
    Heading->SetColorAndOpacity(FSlateColor(FLinearColor(0.55,0.85,0.70)));
    SetVisibility(ESlateVisibility::HitTestInvisible);
}

void UAlgoWorldDialogueWidget::ShowLine(const FText& Name,const FText& Role,const FText& Line,int32 Index,int32 Count)
{
    if(!Heading) return;
    Heading->SetText(FText::Format(FText::FromString(TEXT("{0}  |  {1}")),Name,Role));
    Body->SetText(Line);
    Footer->SetText(FText::FromString(FString::Printf(TEXT("%d / %d     E / Enter: %s     B: close"),
        Index+1,Count,Index+1==Count ? TEXT("finish") : TEXT("next"))));
}
