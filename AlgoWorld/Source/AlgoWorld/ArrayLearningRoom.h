#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ArrayLearningRoom.generated.h"

class UTextRenderComponent;
class UStaticMeshComponent;
class UStaticMesh;
class UCameraComponent;
class APlayerController;
class UMaterialInterface;
class UMaterialInstanceDynamic;

// Place ONE instance in a dedicated learning map. Builds only during Play.
UCLASS()
class ALGOWORLD_API AArrayLearningRoom : public AActor
{
    GENERATED_BODY()
public:
    AArrayLearningRoom();
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
    UPROPERTY() TObjectPtr<UTextRenderComponent> Status;
    UPROPERTY() TObjectPtr<UTextRenderComponent> Output;
    UPROPERTY() TObjectPtr<UTextRenderComponent> ConsolePrompt;
    UPROPERTY() TObjectPtr<UTextRenderComponent> ExitPrompt;
    UPROPERTY() TObjectPtr<APlayerController> Controller;
    UPROPERTY() TArray<TObjectPtr<UMaterialInstanceDynamic>> Palette;
    UPROPERTY() TArray<TObjectPtr<UMaterialInstanceDynamic>> NodePaint;
    UPROPERTY() TArray<TObjectPtr<UStaticMeshComponent>> Confetti;
    UPROPERTY() TObjectPtr<UTextRenderComponent> CelebrationText;
    TArray<FVector> ConfettiVelocity;
    float CelebrationTime = 0;
    bool bCelebrating = false;
    FRandomStream Random;
    TWeakObjectPtr<AActor> PreviousView;
    TArray<int32> Values;
    int32 Cursor = 0;
    int32 Selected = 0;
    int32 Correct = 0;
    int32 Mistakes = 0;
    bool bQuiz = false;
    bool bInputLocked = false;
    bool bPlayerPlaced = false;
    bool bTravelRequested = false;
    static constexpr int32 Capacity = 6;
    int32 Candidate = 1;
    int32 ActiveIndex = 0;
    int32 SearchIndex = -1;
    int32 SearchValue = 0;
    float SearchElapsed = 0;
    bool bSearching = false;
    void SearchStep();
    FVector Slot(int32 Index) const;
    void SnapCells();
    float BusyTime = 0;
    bool bAnswered = false;
    bool bFinished = false;
    FString Question, CorrectAnswer, Explanation;
    TArray<FString> Answers, History;
    UPROPERTY() TObjectPtr<UTextRenderComponent> TopMarker;
    UPROPERTY() TObjectPtr<UTextRenderComponent> ChoiceText;
    UPROPERTY() TObjectPtr<UTextRenderComponent> HistoryText;
    UPROPERTY() TObjectPtr<UStaticMeshComponent> Ghost;
    UPROPERTY() TObjectPtr<UTextRenderComponent> GhostLabel;
    void Operate(int32 Operation);
    void MakeQuestion();
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
