#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "BinaryTreeLearningRoom.generated.h"

class UTextRenderComponent;
class UStaticMeshComponent;
class UStaticMesh;
class UCameraComponent;
class APlayerController;
class UMaterialInterface;
class UMaterialInstanceDynamic;

// Place ONE instance in a dedicated learning map. Builds only during Play.
UCLASS()
class ALGOWORLD_API ABinaryTreeLearningRoom : public AActor
{
    GENERATED_BODY()
public:
    ABinaryTreeLearningRoom();
    virtual void Tick(float DeltaSeconds) override;
    UPROPERTY(EditAnywhere, Category="Classroom Materials")
    TObjectPtr<UMaterialInterface> SurfaceMaterial;
    UPROPERTY(EditAnywhere, Category="Classroom Materials")
    TObjectPtr<UMaterialInterface> GlowMaterial;
    UPROPERTY(EditAnywhere, Category="Classroom Materials")
    TObjectPtr<UMaterialInterface> NodeMaterial;
protected:
    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
private:
    UPROPERTY() TObjectPtr<UCameraComponent> LessonCamera;
    UPROPERTY() TObjectPtr<UStaticMesh> Cube;
    UPROPERTY() TObjectPtr<UStaticMesh> Sphere;
    UPROPERTY() TArray<TObjectPtr<UStaticMeshComponent>> NodeMeshes;
    UPROPERTY() TArray<TObjectPtr<UTextRenderComponent>> Labels;
    UPROPERTY() TArray<TObjectPtr<UTextRenderComponent>> LabelShadows;
    UPROPERTY() TObjectPtr<UTextRenderComponent> Status;
    UPROPERTY() TObjectPtr<UTextRenderComponent> Output;
    UPROPERTY() TObjectPtr<UTextRenderComponent> ConsolePrompt;
    UPROPERTY() TObjectPtr<UTextRenderComponent> ExitPrompt;
    UPROPERTY() TObjectPtr<APlayerController> Controller;
    UPROPERTY() TArray<TObjectPtr<UMaterialInstanceDynamic>> Palette;
    UPROPERTY() TArray<TObjectPtr<UMaterialInstanceDynamic>> NodePaint;
    UPROPERTY() TArray<TObjectPtr<UMaterialInstanceDynamic>> RimPaint;
    UPROPERTY() TArray<TObjectPtr<UStaticMeshComponent>> Confetti;
    UPROPERTY() TObjectPtr<UTextRenderComponent> CelebrationText;
    TArray<FVector> ConfettiVelocity;
    float CelebrationTime = 0;
    bool bCelebrating = false;
    FRandomStream Random;
    TArray<float> TargetScales;
    float AnimationTime = 0;
    TWeakObjectPtr<AActor> PreviousView;
    TArray<int32> Values;
    TArray<int32> VisitOrder;
    int32 Mode = 0;
    int32 Cursor = 0;
    int32 Selected = 0;
    int32 Correct = 0;
    int32 Mistakes = 0;
    float Elapsed = 0;
    float Interval = 1.0f;
    bool bPlaying = false;
    bool bQuiz = false;
    bool bInputLocked = false;
    bool bPlayerPlaced = false;
    bool bTravelRequested = false;
    FString Feedback;
    UStaticMeshComponent* Mesh(UStaticMesh* Asset, FVector Position, FVector Scale, UMaterialInterface* Material = nullptr);
    UMaterialInstanceDynamic* Paint(FLinearColor Color, bool bGlow = false);
    void BuildClassroom();
    void RandomizeValues();
    void StartCelebration();
    void StopCelebration();
    void AnimateCelebration(float DeltaSeconds);
    UTextRenderComponent* Text(const FString& Value, FVector Position, float Size, FColor Color);
    void BuildRoom();
    void ResetLesson();
    void Step();
    void Refresh();
    void EnterConsole();
    void LeaveConsole();
};
