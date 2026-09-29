#pragma once
#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "AlgoWorldDialogueWidget.generated.h"
class UTextBlock;

UCLASS()
class ALGOWORLD_API UAlgoWorldDialogueWidget : public UUserWidget
{
    GENERATED_BODY()
public:
    void ShowLine(const FText& Name, const FText& Role, const FText& Line, int32 Index, int32 Count);
protected:
    virtual void NativeOnInitialized() override;
private:
    UPROPERTY() TObjectPtr<UTextBlock> Heading;
    UPROPERTY() TObjectPtr<UTextBlock> Body;
    UPROPERTY() TObjectPtr<UTextBlock> Footer;
};
