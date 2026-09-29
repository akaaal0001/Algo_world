#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "BinaryTreeRoomEntrance.generated.h"
class UTextRenderComponent;
UCLASS()
class ALGOWORLD_API ABinaryTreeRoomEntrance : public AActor
{
    GENERATED_BODY()
public:
    ABinaryTreeRoomEntrance();
    virtual void Tick(float DeltaSeconds) override;
    UPROPERTY(EditAnywhere, Category="AlgoWorld", meta=(ClampMin="50", ClampMax="1000"))
    float InteractionRadius = 250;
private:
    UPROPERTY() TObjectPtr<UTextRenderComponent> Prompt;
    bool bTravelRequested = false;
    float EntryCooldown = 0;
    float RestoreWait = 0;
};
