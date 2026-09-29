#include "BinaryTreeLearningRoom.h"
#include "BinaryTreeLessonLogic.h"
#include "Camera/CameraComponent.h"
#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/TextRenderComponent.h"
#include "Components/PointLightComponent.h"
#include "Engine/StaticMesh.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "InputCoreTypes.h"
#include "Kismet/GameplayStatics.h"
#include "Misc/PackageName.h"
#include "UObject/ConstructorHelpers.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Materials/MaterialInterface.h"
#include "AlgoWorldRoomSession.h"
#include "Engine/GameInstance.h"
#include "Misc/Guid.h"

ABinaryTreeLearningRoom::ABinaryTreeLearningRoom()
{
    PrimaryActorTick.bCanEverTick = true;
    RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("LessonRoot"));
    LessonCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("LessonCamera"));
    LessonCamera->SetupAttachment(RootComponent);
    LessonCamera->SetRelativeLocation(FVector(-2300, 0, 700));
    LessonCamera->SetRelativeRotation(FRotator::ZeroRotator);
    LessonCamera->FieldOfView = 65;
    LessonCamera->bConstrainAspectRatio = true;
    LessonCamera->AspectRatio = 16.0f / 9.0f;
    LessonCamera->PostProcessSettings.bOverride_BloomIntensity = true;
    LessonCamera->PostProcessSettings.BloomIntensity = 0.5f;
    LessonCamera->PostProcessSettings.bOverride_AutoExposureMethod = true;
    LessonCamera->PostProcessSettings.AutoExposureMethod = EAutoExposureMethod::AEM_Manual;
    LessonCamera->PostProcessSettings.bOverride_AutoExposureBias = true;
    LessonCamera->PostProcessSettings.AutoExposureBias = 0;
    LessonCamera->PostProcessSettings.bOverride_AutoExposureApplyPhysicalCameraExposure = true;
    LessonCamera->PostProcessSettings.AutoExposureApplyPhysicalCameraExposure = false;
    static ConstructorHelpers::FObjectFinder<UStaticMesh> C(TEXT("/Engine/BasicShapes/Cube.Cube"));
    static ConstructorHelpers::FObjectFinder<UStaticMesh> S(TEXT("/Engine/BasicShapes/Sphere.Sphere"));
    Cube = C.Object;
    Sphere = S.Object;
}

UStaticMeshComponent* ABinaryTreeLearningRoom::Mesh(UStaticMesh* Asset, FVector Position, FVector Scale, UMaterialInterface* Material)
{
    auto* Part = NewObject<UStaticMeshComponent>(this);
    AddInstanceComponent(Part);
    Part->SetupAttachment(RootComponent);
    Part->SetStaticMesh(Asset);
    if (Material) Part->SetMaterial(0, Material);
    else if (Palette.Num() > 0 && Palette[0]) Part->SetMaterial(0, Palette[0]);
    Part->SetRelativeLocation(Position);
    Part->SetRelativeScale3D(Scale);
    Part->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    Part->SetCastShadow(false);
    Part->RegisterComponent();
    return Part;
}

UTextRenderComponent* ABinaryTreeLearningRoom::Text(const FString& Value, FVector Position, float Size, FColor Color)
{
    auto* Part = NewObject<UTextRenderComponent>(this);
    AddInstanceComponent(Part);
    Part->SetupAttachment(RootComponent);
    Part->SetRelativeLocation(Position);
    Part->SetRelativeRotation(FRotator(0, 180, 0));
    Part->SetHorizontalAlignment(EHTA_Center);
    Part->SetVerticalAlignment(EVRTA_TextCenter);
    Part->SetWorldSize(Size);
    Part->SetHorizSpacingAdjust(0.5f);
    Part->SetTextRenderColor(Color);
    Part->SetText(FText::FromString(Value));
    Part->RegisterComponent();
    return Part;
}

UMaterialInstanceDynamic* ABinaryTreeLearningRoom::Paint(FLinearColor Color, bool bGlow)
{
    UMaterialInterface* Parent = bGlow ? GlowMaterial.Get() : SurfaceMaterial.Get();
    if (!Parent) return nullptr;
    auto* Material = UMaterialInstanceDynamic::Create(Parent, this);
    Material->SetVectorParameterValue(TEXT("Tint"), Color);
    return Material;
}

void ABinaryTreeLearningRoom::BuildClassroom()
{
    // Dedicated colours keep furniture distinct from the room shell.
    Palette = {Paint(FLinearColor(0.34,0.30,0.24)), Paint(FLinearColor(0.43,0.26,0.12)),
        Paint(FLinearColor(0.065,0.105,0.10)), Paint(FLinearColor(0.025,0.055,0.045),true),
        Paint(FLinearColor(0.10,0.65,0.8),true), Paint(FLinearColor(0.28,0.22,0.14)),
        Paint(FLinearColor(0.83,0.79,0.65)), Paint(FLinearColor(1.0,0.86,0.65),true),
        Paint(FLinearColor(0.38,0.49,0.54),true), Paint(FLinearColor(0.035,0.065,0.06),true),
        Paint(FLinearColor(0.56,0.52,0.43),true), Paint(FLinearColor(0.72,0.70,0.63),true),
        Paint(FLinearColor(0.10,0.24,0.20)), Paint(FLinearColor(0.08,0.09,0.085)),
        Paint(FLinearColor(0.28,0.35,0.29),true), Paint(FLinearColor(0.38,0.16,0.10)),
        Paint(FLinearColor(0.20,0.28,0.34))};
    const auto Solid = [this](FVector P, FVector S, int32 Color)
    {
        auto* Part = Mesh(Cube, P, S, Palette[Color]);
        Part->SetCollisionProfileName(TEXT("BlockAll"));
        return Part;
    };
    // Full room shell, with the teaching camera safely below the ceiling.
    Solid(FVector(-1200,0,1480), FVector(32,26,0.4),11);
    Solid(FVector(-2780,0,740), FVector(0.4,26,14.4),10);
    Solid(FVector(380,0,740), FVector(0.4,26,14.4),10);
    for (float Y : {-1280.0f,1280.0f})
    {
        Solid(FVector(-1200,Y,740), FVector(31.2,0.4,14.4),10);
        // Decorative illuminated window panels: not openings to the outside.
        for (float X : {-650.0f,-1550.0f,-2400.0f})
        {
            Mesh(Cube,FVector(X,FMath::Sign(Y)*1250.0f,820),FVector(5.0,0.08,6.0),Palette[5]);
            Mesh(Cube,FVector(X,FMath::Sign(Y)*1238.0f,820),FVector(4.7,0.08,5.7),Palette[8]);
            Mesh(Cube,FVector(X,FMath::Sign(Y)*1225.0f,820),FVector(0.12,0.1,5.7),Palette[5]);
            Mesh(Cube,FVector(X,FMath::Sign(Y)*1225.0f,820),FVector(4.7,0.1,0.12),Palette[5]);
        }
    }
    // Muted central aisle; no obstacles between arrival and console.
    Mesh(Cube,FVector(-1650,0,22),FVector(20,4.5,0.02),Palette[2]);
    for (float Y : {-225.0f,225.0f}) Mesh(Cube,FVector(-1650,Y,24),FVector(20,0.045,0.02),Palette[5]);
    // Six study desks and chairs, outside the central walking aisle.
    for (float X : {-1450.0f,-1950.0f,-2450.0f})
    for (float Y : {-650.0f,650.0f})
    {
        Solid(FVector(X,Y,130),FVector(2.1,4.0,0.14),1);
        for (float DX : {-85.0f,85.0f}) for (float DY : {-175.0f,175.0f})
            Solid(FVector(X+DX,Y+DY,77),FVector(0.09,0.09,1.0),13);
        Solid(FVector(X-180,Y,75),FVector(1.0,1.2,0.14),12);
        Solid(FVector(X-230,Y,130),FVector(0.10,1.2,1.15),12);
        for (float DX : {-35.0f,35.0f}) for (float DY : {-45.0f,45.0f})
            Solid(FVector(X-180+DX,Y+DY,45),FVector(0.065,0.065,0.5),13);
        // Closed book and a contrasting page block.
        Mesh(Cube,FVector(X,Y-90,145),FVector(0.65,0.85,0.1),Palette[16]);
        Mesh(Cube,FVector(X,Y-90,152),FVector(0.60,0.80,0.04),Palette[6]);
    }
    // Sage lower wall panels and cream skirting stand clear of the shell.
    for(float Y : {-1240.0f,1240.0f})
    {
        Mesh(Cube,FVector(-1200,Y,220),FVector(30.8,0.12,3.6),Palette[14]);
        Mesh(Cube,FVector(-1200,FMath::Sign(Y)*1227,410),FVector(30.8,0.10,0.10),Palette[1]);
        Mesh(Cube,FVector(-1200,FMath::Sign(Y)*1225,55),FVector(30.8,0.12,0.35),Palette[11]);
    }
    // Window sills, solid wood shelves, and separate book spines.
    for(float Y : {-1200.0f,1200.0f})
        for(float X : {-650.0f,-1550.0f,-2400.0f})
            Mesh(Cube,FVector(X,Y,520),FVector(5.1,0.7,0.12),Palette[11]);
    for(float Y : {-1200.0f,-900.0f})
        Solid(FVector(-2650,Y,210),FVector(1.6,0.14,3.8),1);
    for(float Z : {30.0f,150.0f,270.0f,390.0f})
        Solid(FVector(-2650,-1050,Z),FVector(1.6,3.1,0.12),1);
    for(int32 Row=0;Row<3;++Row) for(int32 Book=0;Book<8;++Book)
        Mesh(Cube,FVector(-2620,-1165+Book*32,82+Row*120),
            FVector(0.65,0.23,0.75+(Book%3)*0.07),Palette[Book%2 ? 15 : 16]);
    // Back-wall noticeboard faces into the classroom.
    Mesh(Cube,FVector(-2740,-420,860),FVector(0.12,9.0,4.6),Palette[1]);
    Mesh(Cube,FVector(-2729,-420,860),FVector(0.06,8.65,4.25),Palette[14]);
    Text(TEXT("LEARNING CORNER"),FVector(-2718,-420,1010),34,FColor(245,240,220))
        ->SetRelativeRotation(FRotator::ZeroRotator);
    for(int32 I=0;I<3;++I)
    {
        Mesh(Cube,FVector(-2718,-690+I*270,830),FVector(0.035,2.3,2.25),Palette[6]);
        Text(I==0 ? TEXT("OBSERVE") : I==1 ? TEXT("PREDICT") : TEXT("PRACTISE"),
            FVector(-2712,-690+I*270,835),23,FColor(40,65,55))->SetRelativeRotation(FRotator::ZeroRotator);
    }
    // Teaching stage frame and shallow display plinth (not across the aisle).
    Solid(FVector(230,0,55),FVector(2.0,24,0.7),1);
    for (float Y : {-1200.0f,1200.0f})
    {
        Mesh(Cube,FVector(140,Y,700),FVector(0.6,0.3,14),Palette[5]);
        Mesh(Cube,FVector(110,Y,700),FVector(0.06,0.08,13.6),Palette[5]);
    }
    for (float Z : {30.0f,1370.0f}) Mesh(Cube,FVector(140,0,Z),FVector(0.6,24.3,0.3),Palette[5]);
    // Ceiling beams and warm pendant lights.
    for (float X : {-850.0f,-1850.0f})
    {
        Mesh(Cube,FVector(X,0,1430),FVector(0.22,26,0.32),Palette[1]);
        for (float Y : {-650.0f,650.0f})
        {
            Mesh(Cube,FVector(X,Y,1350),FVector(1.6,1.6,0.14),Palette[5]);
            Mesh(Cube,FVector(X,Y,1335),FVector(1.35,1.35,0.08),Palette[7]);
            auto* Light = NewObject<UPointLightComponent>(this);
            AddInstanceComponent(Light);
            Light->SetupAttachment(RootComponent);
            Light->SetRelativeLocation(FVector(X,Y,1250));
            Light->SetIntensity(18000);
            Light->SetAttenuationRadius(2200);
            Light->SetLightColor(FLinearColor(1.0,0.92,0.80));
            Light->SetCastShadows(false);
            Light->RegisterComponent();
        }
    }
    // Exit frame is placed behind the existing interaction marker.
    for (float Y : {720.0f,1080.0f}) Solid(FVector(-2690,Y,240),FVector(0.25,0.25,4.3),5);
    Mesh(Cube,FVector(-2690,900,455),FVector(0.25,3.85,0.25),Palette[4]);
    Text(TEXT("HUBWORLD"),FVector(-2650,900,510),38,FColor::Cyan)->SetRelativeRotation(FRotator(0,0,0));
}

void ABinaryTreeLearningRoom::BuildRoom()
{
    BuildClassroom();
    RandomizeValues();
    const TArray<FVector> Positions = {
        {0, 0, 990}, {0, -470, 760}, {0, 470, 760},
        {0, -705, 530}, {0, -235, 530}, {0, 235, 530}, {0, 705, 530}};
    Mesh(Cube, FVector(180, 0, 700), FVector(0.3, 24, 13.4), Palette[3]);
    // Floating title and control panels, behind text and outside walking paths.
    Mesh(Cube,FVector(-45,0,1290),FVector(0.1,19,1.05),Palette[9]);
    for (float Z : {1236.0f,1344.0f}) Mesh(Cube,FVector(-52,0,Z),FVector(0.04,19,0.045),Palette[4]);
    Mesh(Cube,FVector(-45,0,180),FVector(0.1,22,2.55),Palette[9]);
    for (float Y : {-1100.0f,1100.0f}) Mesh(Cube,FVector(-52,Y,180),FVector(0.04,0.04,2.55),Palette[4]);
    Mesh(Cube,FVector(-45,0,365),FVector(0.1,21,0.95),Palette[9]);
    // Walkable platform and perimeter: runtime-only, no map assets replaced.
    auto* Floor = Mesh(Cube, FVector(-1200, 0, 0), FVector(32, 26, 0.4));
    Floor->SetCollisionProfileName(TEXT("BlockAll"));
    Mesh(Cube, FVector(-1000, 0, 65), FVector(1.3, 2.6, 0.9),Palette[1])->SetCollisionProfileName(TEXT("BlockAll"));
    Mesh(Cube, FVector(-1000, 0, 115), FVector(1.6, 3.0, 0.12),Palette[2]);
    Mesh(Cube, FVector(-1000, 0, 123), FVector(0.9, 2.0, 0.02),Palette[4]);
    ConsolePrompt = Text(TEXT("LEARNING CONSOLE"), FVector(-1000, 0, 230), 30, FColor::Cyan);
    Mesh(Cube, FVector(-2200, 900, 30), FVector(3.2, 3.2, 0.15),Palette[4]);
    ExitPrompt = Text(TEXT("EXIT TO HUB"), FVector(-2200, 900, 220), 32, FColor::Cyan);
    for (int32 I = 1; I < Positions.Num(); ++I)
    {
        const FVector Start = Positions[(I - 1) / 2];
        const FVector End = Positions[I];
        auto* Edge = Mesh(Cube, (Start + End) / 2, FVector((End - Start).Size() / 100, 0.16, 0.16),Palette[4]);
        Edge->SetRelativeRotation((End - Start).Rotation());
        Text(I % 2 == 1 ? TEXT("L") : TEXT("R"), (Start + End) / 2 + FVector(-30, 0, 20), 34, FColor::Cyan);
    }
    for (int32 I = 0; I < Values.Num(); ++I)
    {
        auto* NodeMID = NodeMaterial ? UMaterialInstanceDynamic::Create(NodeMaterial,this) : Paint(FLinearColor(0.025,0.19,0.24));
        if (NodeMID) NodeMID->SetVectorParameterValue(TEXT("Tint"),FLinearColor(0.03,0.55,1.0));
        NodePaint.Add(NodeMID);
        NodeMeshes.Add(Mesh(Sphere, Positions[I], FVector(0.95),NodePaint.Last()));
        RimPaint.Add(Paint(FLinearColor(0.03,1.2,2.4),true));
        Mesh(Sphere,Positions[I]+FVector(24,0,0),FVector(0.06,1.27,1.27),RimPaint.Last());
        TargetScales.Add(0.95f);
        // A small dark duplicate behind the face text gives a readable drop shadow.
        LabelShadows.Add(Text(FString::FromInt(Values[I]),Positions[I]+FVector(-68,2,-2),60,FColor::Black));
        Labels.Add(Text(FString::FromInt(Values[I]), Positions[I] + FVector(-69, 0, 0), 60, FColor::White));
    }
    Text(TEXT("BINARY TREE CHALLENGE"), FVector(-80, 0, 1290), 56, FColor::White);
    Text(TEXT("7 nodes | 4 leaves | height: 2 edges / 3 levels"), FVector(-80, 0, 1215), 36, FColor::White);
    Status = Text(TEXT(""), FVector(-80, 0, 1130), 34, FColor::White);
    Output = Text(TEXT(""), FVector(-80, 0, 365), 38, FColor::Green);
    Text(TEXT("1 Preorder   2 Inorder   3 Postorder   4 Level order"), FVector(-80, 0, 270), 36, FColor::Cyan);
    Text(TEXT("P Play/Pause   N Step   R Reset   Q Quiz   T New tree"), FVector(-80, 0, 210), 34, FColor::White);
    Text(TEXT("Quiz: arrows + Enter | E: leave console"), FVector(-80, 0, 150), 34, FColor::White);
    Text(TEXT("Cyan: new | Green: visited | Orange: selected | -/+ speed"), FVector(-80, 0, 90), 30, FColor::White);
    CelebrationText = Text(TEXT(""), FVector(-200,0,1110),48,FColor(255,215,100));
    CelebrationText->SetVisibility(false);
    for (float Y : {-700.0f, 700.0f})
    {
        auto* Light = NewObject<UPointLightComponent>(this);
        AddInstanceComponent(Light);
        Light->SetupAttachment(RootComponent);
        Light->SetRelativeLocation(FVector(-450, Y, 1000));
        Light->SetIntensity(15000);
        Light->SetAttenuationRadius(3500);
        Light->SetLightColor(FLinearColor(0.6, 0.85, 1.0));
        Light->SetCastShadows(false);
        Light->RegisterComponent();
    }
}

void ABinaryTreeLearningRoom::RandomizeValues()
{
    Values.Reset();
    while (Values.Num() < 7)
    {
        const int32 Value = Random.RandRange(1,99);
        if (!Values.Contains(Value)) Values.Add(Value);
    }
    // Preserve the lesson's general-binary-tree semantics: left child > root.
    if (Values[1] < Values[0]) Swap(Values[1],Values[0]);
    if (auto* Session = GetGameInstance()->GetSubsystem<UAlgoWorldRoomSession>())
    {
        if (Session->LastTreeValues == Values) Swap(Values[3],Values[4]);
        Session->LastTreeValues = Values;
    }
    for (int32 I=0; I<Labels.Num(); ++I)
    {
        Labels[I]->SetText(FText::AsNumber(Values[I]));
        LabelShadows[I]->SetText(FText::AsNumber(Values[I]));
    }
}

void ABinaryTreeLearningRoom::StartCelebration()
{
    const int32 Stars = Mistakes == 0 ? 3 : Mistakes <= 2 ? 2 : 1;
    Feedback = FString::Printf(TEXT("QUIZ COMPLETE! %d stars | R retry | T new tree"),Stars);
    CelebrationText->SetText(FText::FromString(FString::Printf(TEXT("QUIZ COMPLETE!\n%d STARS  |  %d correct  |  %d mistakes"),Stars,Correct,Mistakes)));
    CelebrationText->SetVisibility(true);
    Status->SetVisibility(false);
    if (Confetti.Num() == 0)
    {
        for (int32 I=0; I<72; ++I)
        {
            Confetti.Add(Mesh(Cube,FVector::ZeroVector,FVector(0.04,0.12,0.20),Palette[I%2 ? 4 : 7]));
            ConfettiVelocity.Add(FVector::ZeroVector);
        }
    }
    for (int32 I=0; I<Confetti.Num(); ++I)
    {
        Confetti[I]->SetVisibility(true);
        Confetti[I]->SetRelativeLocation(FVector(-220,Random.FRandRange(-650,650),Random.FRandRange(480,850)));
        ConfettiVelocity[I] = FVector(0,Random.FRandRange(-160,160),Random.FRandRange(220,480));
    }
    CelebrationTime = 0;
    bCelebrating = true;
}

void ABinaryTreeLearningRoom::StopCelebration()
{
    bCelebrating = false;
    CelebrationTime = 0;
    for (const auto& Piece : Confetti) if (Piece) Piece->SetVisibility(false);
    if (CelebrationText) CelebrationText->SetVisibility(false);
    if (Status) Status->SetVisibility(true);
}

void ABinaryTreeLearningRoom::AnimateCelebration(float DeltaSeconds)
{
    if (!bCelebrating) return;
    CelebrationTime += DeltaSeconds;
    if (CelebrationTime > 4.5f) { StopCelebration(); return; }
    const float Dt = FMath::Min(DeltaSeconds,0.05f);
    for (int32 I=0; I<Confetti.Num(); ++I)
    {
        ConfettiVelocity[I].Z -= 240 * Dt;
        Confetti[I]->SetRelativeLocation(Confetti[I]->GetRelativeLocation()+ConfettiVelocity[I]*Dt);
        Confetti[I]->AddLocalRotation(FRotator(60*Dt,90*Dt,120*Dt));
    }
}

void ABinaryTreeLearningRoom::BeginPlay()
{
    Super::BeginPlay();
    Random.Initialize(static_cast<int32>(FGuid::NewGuid().A));
    BuildRoom();
    ResetLesson();
}

void ABinaryTreeLearningRoom::ResetLesson()
{
    StopCelebration();
    bPlaying = false;
    Elapsed = 0;
    Cursor = 0;
    Selected = 0;
    Correct = Mistakes = 0;
    VisitOrder.Reset();
    std::vector<int> Data;
    for (int32 Value : Values) Data.push_back(Value);
    for (int Index : AlgoTreeLesson::Order(Data, Mode)) VisitOrder.Add(Index);
    Feedback = bQuiz ? TEXT("Predict the next visited node.") : TEXT("Press N to step or P to play.");
    Refresh();
}

void ABinaryTreeLearningRoom::Step()
{
    if (Cursor >= VisitOrder.Num()) { bPlaying = false; return; }
    ++Cursor;
    if (Cursor == VisitOrder.Num())
    {
        bPlaying = false;
        Feedback = TEXT("Traversal complete. R retries; T creates a new tree.");
        if (bQuiz) StartCelebration();
    }
    Refresh();
}

void ABinaryTreeLearningRoom::Refresh()
{
    const TCHAR* Names[] = {TEXT("PREORDER: Node > Left > Right"), TEXT("INORDER: Left > Node > Right"),
        TEXT("POSTORDER: Left > Right > Node"), TEXT("LEVEL ORDER: breadth first, using a queue")};
    for (int32 I = 0; I < Labels.Num(); ++I)
    {
        bool Visited = false;
        for (int32 J = 0; J < Cursor; ++J) if (VisitOrder[J] == I) Visited = true;
        const bool Active = bQuiz ? I == Selected : Cursor > 0 && VisitOrder[Cursor - 1] == I;
        Labels[I]->SetTextRenderColor(FColor::White);
        TargetScales[I] = Active ? 1.12f : 0.95f;
        if (NodePaint[I]) NodePaint[I]->SetVectorParameterValue(TEXT("Tint"),
            Active ? FLinearColor(1.0,0.19,0.008) : Visited ? FLinearColor(0.025,0.65,0.21) : FLinearColor(0.025,0.38,0.85));
        if (RimPaint[I]) RimPaint[I]->SetVectorParameterValue(TEXT("Tint"),
            Active ? FLinearColor(4.5,0.8,0.015) : Visited ? FLinearColor(0.05,1.5,0.45) : FLinearColor(0.03,1.2,2.4));
    }
    FString Sequence = TEXT("Output: ");
    for (int32 J = 0; J < Cursor; ++J) Sequence += FString::Printf(TEXT("%s%d"), J ? TEXT(" > ") : TEXT(""), Values[VisitOrder[J]]);
    Sequence += TEXT("\n") + Feedback;
    Output->SetText(FText::FromString(Sequence));
    Status->SetText(FText::FromString(FString::Printf(TEXT("%s\n%s | Step %d/7 | %.2fs per step | Correct %d, mistakes %d"),
        Names[Mode], !bInputLocked ? TEXT("EXPLORE") : bQuiz ? TEXT("QUIZ") : bPlaying ? TEXT("PLAYING") : TEXT("PAUSED"), Cursor, Interval, Correct, Mistakes)));
}

void ABinaryTreeLearningRoom::EnterConsole()
{
    if (!IsValid(Controller) || bInputLocked) return;
    PreviousView = Controller->GetViewTarget();
    if (auto* Character = Cast<ACharacter>(Controller->GetPawn()))
    {
        Character->StopJumping();
        Character->GetCharacterMovement()->StopMovementImmediately();
    }
    Controller->SetIgnoreMoveInput(true);
    Controller->SetIgnoreLookInput(true);
    Controller->SetViewTargetWithBlend(this, 0.4f);
    bInputLocked = true;
    ConsolePrompt->SetVisibility(false);
    ExitPrompt->SetVisibility(false);
    Feedback = TEXT("E returns to exploration. Your progress is kept.");
    Refresh();
}

void ABinaryTreeLearningRoom::LeaveConsole()
{
    StopCelebration();
    bPlaying = false;
    Elapsed = 0;
    if (IsValid(Controller) && bInputLocked)
    {
        Controller->SetIgnoreMoveInput(false);
        Controller->SetIgnoreLookInput(false);
        AActor* View = PreviousView.IsValid() ? PreviousView.Get() : Controller->GetPawn();
        if (View) Controller->SetViewTargetWithBlend(View, 0.4f);
    }
    bInputLocked = false;
    ConsolePrompt->SetVisibility(true);
    ExitPrompt->SetVisibility(true);
    Feedback = TEXT("Walk to the console and press E to continue.");
    Refresh();
}

void ABinaryTreeLearningRoom::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);
    AnimateCelebration(DeltaSeconds);
    AnimationTime += DeltaSeconds;
    for (int32 I = 0; I < NodeMeshes.Num(); ++I)
    {
        const float Target = TargetScales[I] + (TargetScales[I] > 1.0f ? 0.025f * FMath::Sin(AnimationTime * 4.0f) : 0.0f);
        const float Scale = FMath::FInterpTo(NodeMeshes[I]->GetRelativeScale3D().X,Target,DeltaSeconds,8.0f);
        NodeMeshes[I]->SetRelativeScale3D(FVector(Scale));
    }
    if (!Controller)
    {
        Controller = UGameplayStatics::GetPlayerController(this, 0);
        if (!Controller) return;
    }
    APawn* Pawn = Controller->GetPawn();
    if (!IsValid(Pawn) || bTravelRequested) return;
    // Relocate the existing pawn ONCE, after it exists and the runtime floor is built.
    // Do not spawn a duplicate pawn or change the user's GameMode.
    if (!bPlayerPlaced)
    {
        const FVector Arrival = GetActorTransform().TransformPosition(FVector(-1800, 0, 150));
        if (!Pawn->TeleportTo(Arrival, GetActorRotation())) return;
        if (auto* Character = Cast<ACharacter>(Pawn)) Character->GetCharacterMovement()->StopMovementImmediately();
        Controller->SetControlRotation(GetActorRotation());
        Controller->SetViewTarget(Pawn);
        bPlayerPlaced = true;
    }
    const auto Pressed = [this](FKey Key) { return Controller->WasInputKeyJustPressed(Key); };
    if (!bInputLocked)
    {
        const FVector Local = GetActorTransform().InverseTransformPosition(Pawn->GetActorLocation());
        const bool bNearConsole = FVector::Dist2D(Local, FVector(-1000, 0, 0)) < 280 && FMath::Abs(Local.Z - 120) < 200;
        const bool bNearExit = FVector::Dist2D(Local, FVector(-2200, 900, 0)) < 280 && FMath::Abs(Local.Z - 120) < 200;
        const bool bHubAvailable = FPackageName::DoesPackageExist(TEXT("/Game/Maps/HubWorld"));
        ConsolePrompt->SetText(FText::FromString(bNearConsole ? TEXT("E - START / RESUME LESSON") : TEXT("LEARNING CONSOLE")));
        ExitPrompt->SetText(FText::FromString(bNearExit ? bHubAvailable ? TEXT("E - RETURN TO HUB") : TEXT("HUB MAP NOT AVAILABLE") : TEXT("EXIT TO HUB")));
        FVector CameraPosition; FRotator CameraRotation;
        Controller->GetPlayerViewPoint(CameraPosition, CameraRotation);
        ConsolePrompt->SetWorldRotation((CameraPosition - ConsolePrompt->GetComponentLocation()).Rotation());
        ExitPrompt->SetWorldRotation((CameraPosition - ExitPrompt->GetComponentLocation()).Rotation());
        if (Pressed(EKeys::E))
        {
            if (bNearConsole) EnterConsole();
            else if (bNearExit && bHubAvailable)
            {
                bTravelRequested = true;
                if (auto* Session = GetGameInstance()->GetSubsystem<UAlgoWorldRoomSession>())
                    Session->bRestoreAtEntrance = true;
                UGameplayStatics::OpenLevel(this, FName(TEXT("/Game/Maps/HubWorld")));
            }
        }
        return;
    }
    if (Pressed(EKeys::E)) { LeaveConsole(); return; }
    if (Pressed(EKeys::T)) { RandomizeValues(); ResetLesson(); return; }
    const FKey Modes[] = {EKeys::One, EKeys::Two, EKeys::Three, EKeys::Four};
    for (int32 I = 0; I < 4; ++I) if (Pressed(Modes[I])) { Mode = I; ResetLesson(); return; }
    if (Pressed(EKeys::Q)) { bQuiz = !bQuiz; ResetLesson(); return; }
    if (Pressed(EKeys::R)) { ResetLesson(); return; }
    if (Pressed(EKeys::Equals) || Pressed(EKeys::Add)) { Interval = FMath::Max(0.25f, Interval - 0.25f); Refresh(); }
    if (Pressed(EKeys::Hyphen) || Pressed(EKeys::Subtract)) { Interval = FMath::Min(3.0f, Interval + 0.25f); Refresh(); }
    if (bQuiz)
    {
        if (Pressed(EKeys::Left)) { Selected = (Selected + 6) % 7; Refresh(); }
        if (Pressed(EKeys::Right)) { Selected = (Selected + 1) % 7; Refresh(); }
        if (Pressed(EKeys::Enter) && Cursor < VisitOrder.Num())
        {
            if (Selected == VisitOrder[Cursor]) { ++Correct; Feedback = TEXT("Correct! Choose the next node."); Step(); }
            else { ++Mistakes; Feedback = TEXT("Try again: follow the traversal rule above."); Refresh(); }
        }
        return;
    }
    if (Pressed(EKeys::N)) { bPlaying = false; Elapsed = 0; Step(); return; }
    if (Pressed(EKeys::P))
    {
        if (Cursor == VisitOrder.Num()) ResetLesson();
        bPlaying = !bPlaying; Elapsed = 0; Refresh(); return;
    }
    if (bPlaying) { Elapsed += DeltaSeconds; if (Elapsed >= Interval) { Elapsed = 0; Step(); } }
}

void ABinaryTreeLearningRoom::EndPlay(const EEndPlayReason::Type Reason)
{
    if (IsValid(Controller) && bInputLocked)
    {
        Controller->SetIgnoreMoveInput(false);
        Controller->SetIgnoreLookInput(false);
        if (PreviousView.IsValid()) Controller->SetViewTarget(PreviousView.Get());
    }
    Super::EndPlay(Reason);
}
