#pragma once
#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "AlgoWorldArraySession.generated.h"

// Per-play-session state survives OpenLevel without replacing the GameInstance.
UCLASS()
class ALGOWORLD_API UAlgoWorldArraySession : public UGameInstanceSubsystem
{
    GENERATED_BODY()
public:
    UPROPERTY() FTransform HubReturnTransform;
    UPROPERTY() FRotator HubReturnView;
    UPROPERTY() bool bHasHubReturn = false;
    UPROPERTY() bool bRestoreAtEntrance = false;
};
