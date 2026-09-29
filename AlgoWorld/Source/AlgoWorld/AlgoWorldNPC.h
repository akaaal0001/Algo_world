#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "AlgoWorldNPC.generated.h"
class UStaticMesh;
class USceneComponent;
class UStaticMeshComponent;
class UTextRenderComponent;
class UMaterialInterface;
class UAlgoWorldDialogueWidget;
class APlayerController;
class APawn;

UCLASS()
class ALGOWORLD_API AAlgoWorldNPC : public AActor
{
    GENERATED_BODY()
public:
    AAlgoWorldNPC();
    virtual void Tick(float DeltaSeconds) override;
    virtual void OnConstruction(const FTransform& Transform) override;
    // 0 campus guide, 1 Array tutor, 2 Stack tutor, 3 Binary Tree tutor.
    UPROPERTY(EditAnywhere,Category="Tutor",meta=(ClampMin="0",ClampMax="3")) int32 Profile=0;
    UPROPERTY(EditAnywhere,Category="Tutor") FName NPCId;
    UPROPERTY(EditAnywhere,Category="Tutor") FText DisplayName;
    UPROPERTY(EditAnywhere,Category="Tutor") FText RoleTitle;
    UPROPERTY(EditAnywhere,Category="Tutor",meta=(MultiLine="true")) TArray<FText> CustomDialogue;
    UPROPERTY(EditAnywhere,Category="Tutor",meta=(ClampMin="100",ClampMax="500")) float TalkRadius=280;
    UPROPERTY(EditAnywhere,Category="Appearance") FLinearColor OutfitColor=FLinearColor(0.15,0.32,0.23);
    UPROPERTY(EditAnywhere,Category="Appearance") FLinearColor SkinColor=FLinearColor(0.55,0.33,0.20);
    UPROPERTY(EditAnywhere,Category="Appearance") TObjectPtr<UMaterialInterface> SurfaceMaterial;
    UFUNCTION(BlueprintCallable,CallInEditor,Category="Appearance") void RefreshAppearance();
protected:
    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type Reason) override;
private:
    UPROPERTY() TObjectPtr<USceneComponent> VisualRoot;
    UPROPERTY() TObjectPtr<USceneComponent> WaveArm;
    UPROPERTY() TObjectPtr<UTextRenderComponent> NameLabel;
    UPROPERTY() TObjectPtr<UTextRenderComponent> Prompt;
    UPROPERTY() TArray<TObjectPtr<UStaticMeshComponent>> Parts;
    UPROPERTY() TObjectPtr<UAlgoWorldDialogueWidget> Dialogue;
    UPROPERTY() TObjectPtr<APlayerController> TalkingController;
    TWeakObjectPtr<APawn> TalkingPawn;
    TArray<FText> Lines;
    int32 LineIndex=0;
    bool bTalking=false;
    bool bLocked=false;
    float Age=0;
    float GreetingTime=0;
    float GreetingCooldown=0;
    UStaticMeshComponent* Part(const TCHAR* Name,UStaticMesh* Mesh,FVector Position,FVector Scale,FName Colour,USceneComponent* Parent=nullptr);
    FText TutorName() const;
    FText TutorRole() const;
    TArray<FText> DefaultLines() const;
    bool CanTalk(APlayerController* PC) const;
    bool NearEntrance(APawn* Pawn) const;
    static AAlgoWorldNPC* FindNearest(APlayerController* PC,UWorld* World);
    void StartTalk(APlayerController* PC);
    void StopTalk();
};
