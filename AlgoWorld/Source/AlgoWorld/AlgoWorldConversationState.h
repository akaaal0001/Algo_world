#pragma once
#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "AlgoWorldConversationState.generated.h"
class AActor;

UCLASS()
class ALGOWORLD_API UAlgoWorldConversationState : public UWorldSubsystem
{
    GENERATED_BODY()
public:
    UPROPERTY() TWeakObjectPtr<AActor> Speaker;
    bool IsBlocking() const;
    void Finish(AActor* Actor);
private:
    float BlockUntil = 0;
};
