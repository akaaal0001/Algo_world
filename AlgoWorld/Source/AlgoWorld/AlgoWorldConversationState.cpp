#include "AlgoWorldConversationState.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"

bool UAlgoWorldConversationState::IsBlocking() const
{
    return Speaker.IsValid() || (GetWorld() && GetWorld()->GetTimeSeconds() < BlockUntil);
}

void UAlgoWorldConversationState::Finish(AActor* Actor)
{
    if(Speaker.Get()==Actor)
    {
        Speaker.Reset();
        BlockUntil=GetWorld()->GetTimeSeconds()+0.35f;
    }
}
