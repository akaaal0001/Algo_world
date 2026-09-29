#include "Engine/World.h"
#include "AlgoWorldConversationState.h"
#include "ArrayRoomEntrance.h"
#include "Components/SceneComponent.h"
#include "Components/TextRenderComponent.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/Pawn.h"
#include "Kismet/GameplayStatics.h"
#include "InputCoreTypes.h"
#include "Misc/PackageName.h"
#include "AlgoWorldArraySession.h"
#include "Engine/GameInstance.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"

AArrayRoomEntrance::AArrayRoomEntrance()
{
    PrimaryActorTick.bCanEverTick = true;
    RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("EntranceRoot"));
    Prompt = CreateDefaultSubobject<UTextRenderComponent>(TEXT("EntrancePrompt"));
    Prompt->SetupAttachment(RootComponent);
    Prompt->SetRelativeLocation(FVector(0, 0, 120));
    Prompt->SetHorizontalAlignment(EHTA_Center);
    Prompt->SetWorldSize(28);
    Prompt->SetTextRenderColor(FColor::Cyan);
    Prompt->SetText(FText::FromString(TEXT("Array Lab")));
}

void AArrayRoomEntrance::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);
    // AlgoWorld NPC conversation guard
    if (auto* Conversation = GetWorld()->GetSubsystem<UAlgoWorldConversationState>())
        if (Conversation->IsBlocking()) return;
    auto* PC = UGameplayStatics::GetPlayerController(this, 0);
    if (!PC || bTravelRequested) return;
    APawn* Pawn = PC->GetPawn();
    if (!Pawn) return;
    auto* Session = GetGameInstance()->GetSubsystem<UAlgoWorldArraySession>();
    EntryCooldown = FMath::Max(0.0f, EntryCooldown - DeltaSeconds);
    if (Session && Session->bRestoreAtEntrance)
    {
        RestoreWait += DeltaSeconds;
        const FVector Location = Session->bHasHubReturn ? Session->HubReturnTransform.GetLocation()
            : GetActorLocation() - GetActorForwardVector()*200.0f + FVector(0,0,20);
        const FRotator Rotation = Session->bHasHubReturn ? Session->HubReturnTransform.Rotator() : GetActorRotation();
        // Retry briefly while the destination's collision becomes available.
        const FVector Candidate = Location + FVector(0,0,RestoreWait > 1.0f ? 40.0f : 0.0f);
        if (Pawn->TeleportTo(Candidate, Rotation))
        {
            PC->SetControlRotation(Session->bHasHubReturn ? Session->HubReturnView : Rotation);
            if (auto* Character = Cast<ACharacter>(Pawn)) Character->GetCharacterMovement()->StopMovementImmediately();
            Session->bRestoreAtEntrance = false;
            EntryCooldown = 0.75f;
            RestoreWait = 0;
        }
        else if (RestoreWait > 5.0f)
        {
            Session->bRestoreAtEntrance = false;
            UE_LOG(LogTemp, Warning, TEXT("AlgoWorld: saved entrance position obstructed; kept safe Player Start spawn."));
        }
        return;
    }
    const bool bNear = Pawn && FVector::DistSquared(Pawn->GetActorLocation(), GetActorLocation()) <= FMath::Square(InteractionRadius);
    FVector CameraPosition;
    FRotator CameraRotation;
    PC->GetPlayerViewPoint(CameraPosition, CameraRotation);
    Prompt->SetWorldRotation((CameraPosition - Prompt->GetComponentLocation()).Rotation());
    Prompt->SetText(FText::FromString(bNear ? TEXT("Press E - Array Lab") : TEXT("Array Lab")));
    if (bNear && EntryCooldown <= 0 && PC->WasInputKeyJustPressed(EKeys::E))
    {
        if (FPackageName::DoesPackageExist(TEXT("/Game/Maps/ArrayRoom")))
        {
            if (Session)
            {
                Session->HubReturnTransform = Pawn->GetActorTransform();
                Session->HubReturnView = PC->GetControlRotation();
                Session->bHasHubReturn = true;
                Session->bRestoreAtEntrance = false;
            }
            bTravelRequested = true;
            UGameplayStatics::OpenLevel(this, FName(TEXT("/Game/Maps/ArrayRoom")));
        }
        else Prompt->SetText(FText::FromString(TEXT("ArrayRoom map missing from build")));
    }
}
