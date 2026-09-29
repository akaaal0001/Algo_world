#include "ArrayLearningRoom.h"
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
#include "AlgoWorldArraySession.h"
#include "Engine/GameInstance.h"
#include "Misc/Guid.h"

AArrayLearningRoom::AArrayLearningRoom()
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

UStaticMeshComponent* AArrayLearningRoom::Mesh(UStaticMesh* Asset, FVector Position, FVector Scale, UMaterialInterface* Material)
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

UTextRenderComponent* AArrayLearningRoom::Text(const FString& Value, FVector Position, float Size, FColor Color)
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

UMaterialInstanceDynamic* AArrayLearningRoom::Paint(FLinearColor Color, bool bGlow)
{
    UMaterialInterface* Parent = bGlow ? GlowMaterial.Get() : SurfaceMaterial.Get();
    if (!Parent) return nullptr;
    auto* Material = UMaterialInstanceDynamic::Create(Parent, this);
    Material->SetVectorParameterValue(TEXT("Tint"), Color);
    return Material;
}

void AArrayLearningRoom::BuildClassroom()
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
    // Classroom detail: desk notebooks, pencils and storage drawers.
    for(float X : {-1450.0f,-1950.0f,-2450.0f}) for(float Y : {-650.0f,650.0f})
    {
        Mesh(Cube,FVector(X+25,Y+70,140),FVector(0.78,1.0,0.035),Palette[6]);
        for(int32 Line=0;Line<5;++Line)
            Mesh(Cube,FVector(X+5+Line*10,Y+70,142.5),FVector(0.012,0.75,0.012),Palette[16]);
        Mesh(Cube,FVector(X-30,Y+140,143),FVector(0.04,0.58,0.04),Palette[1]);
        Mesh(Cube,FVector(X+60,Y,104),FVector(0.72,3.0,0.25),Palette[1]);
    }
    // Analogue wall clock: decorative hands, not a running game timer.
    Mesh(Sphere,FVector(-2740,380,930),FVector(0.14,2.3,2.3),Palette[13]);
    Mesh(Sphere,FVector(-2728,380,930),FVector(0.05,2.12,2.12),Palette[11]);
    for(int32 I=0;I<12;++I)
    {
        const float A=I*PI/6.0f;
        Mesh(Cube,FVector(-2720,380+85*FMath::Sin(A),930+85*FMath::Cos(A)),FVector(0.04,0.07,0.07),Palette[13]);
    }
    Mesh(Cube,FVector(-2715,380,957),FVector(0.04,0.045,0.56),Palette[13]);
    Mesh(Cube,FVector(-2710,405,930),FVector(0.04,0.5,0.055),Palette[13]);
    Text(TEXT("ARRAY LAB"),FVector(-2710,380,765),32,FColor(45,65,55))->SetRelativeRotation(FRotator::ZeroRotator);
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

void AArrayLearningRoom::BuildRoom()
{
    BuildClassroom();
    Mesh(Cube,FVector(180,0,700),FVector(0.3,24,13.4),Palette[3]);
    Mesh(Cube,FVector(-45,0,1290),FVector(0.1,22,1.05),Palette[9]);
    for(float Z:{1232.0f,1348.0f}) Mesh(Cube,FVector(-55,0,Z),FVector(0.04,22,0.08),Palette[4]);
    Mesh(Cube,FVector(-45,0,180),FVector(0.1,22,2.55),Palette[9]);
    Mesh(Cube,FVector(-1200,0,0),FVector(32,26,0.4))->SetCollisionProfileName(TEXT("BlockAll"));
    Mesh(Cube,FVector(-1000,0,65),FVector(1.3,2.6,0.9),Palette[1])->SetCollisionProfileName(TEXT("BlockAll"));
    Mesh(Cube,FVector(-1000,0,115),FVector(1.6,3,0.12),Palette[2]);
    Mesh(Cube,FVector(-1000,0,126),FVector(0.9,2,0.06),Palette[4]);
    ConsolePrompt=Text(TEXT("ARRAY CONSOLE"),FVector(-1000,0,230),30,FColor::Cyan);
    Mesh(Cube,FVector(-2200,900,30),FVector(3.2,3.2,0.15),Palette[4]);
    ExitPrompt=Text(TEXT("EXIT TO HUB"),FVector(-2200,900,220),32,FColor::Cyan);
    for(int32 I=0;I<Capacity;++I)
    {
        FVector P=Slot(I);
        Mesh(Cube,P+FVector(65,0,0),FVector(0.08,3.0,1.45),Palette[9]);
        Text(FString::Printf(TEXT("[%d]"),I),P+FVector(-75,0,-115),36,FColor(205,220,205));
        // A dash means allocated but unused space, never an initialized value.
        Text(TEXT("-"),P+FVector(50,0,0),44,FColor(110,135,120));
        NodePaint.Add(Paint(FLinearColor(0.055,0.22,0.25),true));
        NodeMeshes.Add(Mesh(Cube,P,FVector(1.1,2.7,1.12),NodePaint.Last()));
        for(float DY:{-130.0f,130.0f})
        {
            auto* Rim=Mesh(Cube,P+FVector(-60,DY,0),FVector(0.04,0.05,1.10),Palette[4]);
            Rim->AttachToComponent(NodeMeshes.Last(),FAttachmentTransformRules::KeepWorldTransform);
        }
        Labels.Add(Text(TEXT(""),P+FVector(-72,0,0),52,FColor::White));
    }
    TopMarker=Text(TEXT("SELECTED"),Slot(0)+FVector(-90,0,130),28,FColor(255,190,65));
    Ghost=Mesh(Cube,FVector::ZeroVector,FVector(1.1,2.7,1.12),Palette[7]);
    Ghost->SetVisibility(false);
    GhostLabel=Text(TEXT(""),FVector::ZeroVector,52,FColor::White);
    GhostLabel->SetVisibility(false);
    Text(TEXT("ARRAY EXPLORER"),FVector(-80,0,1290),56,FColor::White);
    Text(TEXT("Indexed elements | contiguous logical slots | zero-based indexing"),FVector(-80,0,1208),30,FColor(205,225,205));
    Status=Text(TEXT(""),FVector(-80,0,1120),32,FColor::White);
    Mesh(Cube,FVector(70,0,500),FVector(0.1,22,3.9),Palette[9]);
    Output=Text(TEXT(""),FVector(-90,-550,555),29,FColor::White);
    ChoiceText=Text(TEXT(""),FVector(-90,550,535),29,FColor(180,220,210));
    HistoryText=Text(TEXT(""),FVector(-90,-550,395),25,FColor(205,220,205));
    Text(TEXT("P Append | I Insert | O Delete | K Read | V Update | S Search"),FVector(-80,0,270),30,FColor::White);
    Text(TEXT("Left/Right: index | Up/Down: value | T Random value | R Clear"),FVector(-80,0,210),29,FColor(205,225,205));
    Text(TEXT("Q Quiz/Practice | Quiz: arrows + Enter, N next | E Leave"),FVector(-80,0,150),29,FColor::White);
    Text(TEXT("Read/Update: O(1) | Insert/Delete/Search: O(n) worst case"),FVector(-80,0,90),29,FColor::White);
    CelebrationText=Text(TEXT(""),FVector(-200,0,1110),44,FColor(255,215,100));
    CelebrationText->SetVisibility(false);
}

void AArrayLearningRoom::RandomizeValues()
{
    Candidate=Random.RandRange(1,99);
}

void AArrayLearningRoom::StartCelebration()
{
    const int32 Stars = Mistakes == 0 ? 3 : Mistakes <= 2 ? 2 : 1;
    Feedback = FString::Printf(TEXT("QUIZ COMPLETE! %d stars\nR: new quiz | Q: practice"),Stars);
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

void AArrayLearningRoom::StopCelebration()
{
    bCelebrating = false;
    CelebrationTime = 0;
    for (const auto& Piece : Confetti) if (Piece) Piece->SetVisibility(false);
    if (CelebrationText) CelebrationText->SetVisibility(false);
    if (Status) Status->SetVisibility(true);
}

void AArrayLearningRoom::AnimateCelebration(float DeltaSeconds)
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

void AArrayLearningRoom::BeginPlay()
{
    Super::BeginPlay();
    Values.Reserve(Capacity);
    Random.Initialize(static_cast<int32>(FGuid::NewGuid().A));
    BuildRoom();
    ResetLesson();
}

void AArrayLearningRoom::ResetLesson()
{
    StopCelebration();
    Ghost->SetVisibility(false); GhostLabel->SetVisibility(false);
    BusyTime=0; bSearching=false; SearchIndex=-1;
    Values.Reset(); History.Reset(); Correct=0; Mistakes=0;
    Cursor=0; Selected=0; ActiveIndex=0; bAnswered=false; bFinished=false;
    RandomizeValues();
    Feedback=TEXT("P appends a value.\nSelect an index with Left/Right.");
    SnapCells();
    if(bQuiz) MakeQuestion(); else Refresh();
}

void AArrayLearningRoom::Step()
{
    if(!bQuiz || !bAnswered || bFinished) return;
    ++Cursor;
    MakeQuestion();
}

void AArrayLearningRoom::Refresh()
{
    for(int32 I=0;I<Capacity;++I)
    {
        bool Used=I<Values.Num();
        NodeMeshes[I]->SetVisibility(Used,true); Labels[I]->SetVisibility(Used);
        if(Used) Labels[I]->SetText(FText::AsNumber(Values[I]));
        if(NodePaint[I]) NodePaint[I]->SetVectorParameterValue(TEXT("Tint"),
            I==ActiveIndex ? FLinearColor(0.85,0.32,0.055) : FLinearColor(0.055,0.22,0.25));
    }
    TopMarker->SetVisibility(!bQuiz);
    TopMarker->SetText(FText::FromString(ActiveIndex==Capacity ? TEXT("OUT OF BOUNDS") :
        ActiveIndex==Values.Num() ? TEXT("INSERT HERE") : ActiveIndex>Values.Num() ? TEXT("UNUSED SLOT") : TEXT("SELECTED")));
    // When full, index 6 is still a useful invalid-index learning example.
    TopMarker->SetRelativeLocation(ActiveIndex<Capacity ? Slot(ActiveIndex)+FVector(-90,0,130) : FVector(-90,1020,1010));
    const FString ModeStatus=bQuiz ? FString::Printf(TEXT("Question %d / 8 | Correct %d | Mistakes %d"),Cursor+1,Correct,Mistakes)
        : FString::Printf(TEXT("Index: %d | Value: %d | %s"),ActiveIndex,Candidate,bSearching ? TEXT("SEARCHING") : TEXT("PRACTICE"));
    Status->SetText(FText::FromString(FString::Printf(TEXT("Size: %d | Capacity: 6 | %s\n%s"),Values.Num(),
        Values.IsEmpty() ? TEXT("No valid element index") : *FString::Printf(TEXT("Valid indices: 0..%d"),Values.Num()-1),*ModeStatus)));
    Output->SetText(FText::FromString(bQuiz ? Question+TEXT("\n\n")+Feedback : Feedback));
    if(bQuiz)
    {
        FString Choices;
        for(int32 I=0;I<Answers.Num();++I)
            Choices+=FString::Printf(TEXT("%s %s\n"),I==Selected ? TEXT(">") : TEXT(" "),*Answers[I]);
        ChoiceText->SetText(FText::FromString(Choices));
        HistoryText->SetText(FText::FromString(bFinished ? TEXT("R: new quiz | Q: practice") : bAnswered ? TEXT("N: next question") : TEXT("Predict before the operation.")));
    }
    else
    {
        ChoiceText->SetText(FText::FromString(TEXT("READ/UPDATE: use an existing index\nINSERT: indices 0 through size\nDELETE: shifts later elements left\nSEARCH: first match, left to right")));
        FString Log;
        for(const FString& Item:History) Log+=Item+TEXT("\n");
        HistoryText->SetText(FText::FromString(Log));
    }
}

void AArrayLearningRoom::EnterConsole()
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
    Refresh();
}

void AArrayLearningRoom::LeaveConsole()
{
    StopCelebration();
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
    Refresh();
}

void AArrayLearningRoom::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);
    AnimateCelebration(DeltaSeconds);
    BusyTime=FMath::Max(0.0f,BusyTime-DeltaSeconds);
    for(int32 I=0;I<Values.Num();++I)
    {
        const FVector P=BusyTime<=0 ? Slot(I) : FMath::VInterpTo(NodeMeshes[I]->GetRelativeLocation(),Slot(I),DeltaSeconds,8);
        NodeMeshes[I]->SetRelativeLocation(P);
        Labels[I]->SetRelativeLocation(P+FVector(-72,0,0));
    }
    if(Ghost->IsVisible())
    {
        Ghost->SetRelativeLocation(Ghost->GetRelativeLocation()+FVector(0,0,-250)*DeltaSeconds);
        GhostLabel->SetRelativeLocation(Ghost->GetRelativeLocation()+FVector(-72,0,0));
        if(BusyTime<=0) { Ghost->SetVisibility(false); GhostLabel->SetVisibility(false); }
    }
    if(bSearching && bInputLocked)
    {
        SearchElapsed+=DeltaSeconds;
        if(SearchElapsed>=0.65f) { SearchElapsed=0; SearchStep(); }
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
                if (auto* Session = GetGameInstance()->GetSubsystem<UAlgoWorldArraySession>())
                    Session->bRestoreAtEntrance = true;
                UGameplayStatics::OpenLevel(this, FName(TEXT("/Game/Maps/HubWorld")));
            }
        }
        return;
    }

    if (Pressed(EKeys::E)) { LeaveConsole(); return; }
    if (Pressed(EKeys::Q)) { bQuiz=!bQuiz; ResetLesson(); return; }
    if (Pressed(EKeys::R)) { ResetLesson(); return; }
    if(bQuiz)
    {
        if(Pressed(EKeys::N)) { Step(); return; }
        if(bAnswered || bFinished) return;
        if(Pressed(EKeys::Left)) { Selected=(Selected+Answers.Num()-1)%Answers.Num(); Refresh(); }
        if(Pressed(EKeys::Right)) { Selected=(Selected+1)%Answers.Num(); Refresh(); }
        if(Pressed(EKeys::Enter))
        {
            if(Answers[Selected]==CorrectAnswer)
            {
                ++Correct; bAnswered=true; Feedback=TEXT("Correct!\n")+Explanation;
                if(Cursor==7) { bFinished=true; StartCelebration(); }
            }
            else { ++Mistakes; Feedback=TEXT("Try again.\nRead the TOP and LIFO rule."); }
            Refresh();
        }
        return;
    }

    if(bSearching || BusyTime>0) return;
    if(Pressed(EKeys::Left)) { ActiveIndex=(ActiveIndex+Capacity)%(Capacity+1); Refresh(); }
    if(Pressed(EKeys::Right)) { ActiveIndex=(ActiveIndex+1)%(Capacity+1); Refresh(); }
    if(Pressed(EKeys::Up)) { Candidate=Candidate%99+1; Refresh(); }
    if(Pressed(EKeys::Down)) { Candidate=(Candidate+97)%99+1; Refresh(); }
    if(Pressed(EKeys::T)) { RandomizeValues(); Refresh(); }
    if(Pressed(EKeys::P)) Operate(0);
    else if(Pressed(EKeys::I)) Operate(1);
    else if(Pressed(EKeys::O)) Operate(2);
    else if(Pressed(EKeys::K)) Operate(3);
    else if(Pressed(EKeys::V)) Operate(4);
    else if(Pressed(EKeys::S)) Operate(5);
}

void AArrayLearningRoom::EndPlay(const EEndPlayReason::Type Reason)
{
    if (IsValid(Controller) && bInputLocked)
    {
        Controller->SetIgnoreMoveInput(false);
        Controller->SetIgnoreLookInput(false);
        if (PreviousView.IsValid()) Controller->SetViewTarget(PreviousView.Get());
    }
    Super::EndPlay(Reason);
}


void AArrayLearningRoom::Operate(int32 Operation)
{
    SearchIndex=-1;
    FString Entry;
    if(Operation==0 || Operation==1)
    {
        const int32 At=Operation==0 ? Values.Num() : ActiveIndex;
        if(Values.Num()==Capacity)
            Feedback=TEXT("FULL: capacity is 6.\nDelete an element before inserting.");
        else if(At<0 || At>Values.Num())
            Feedback=TEXT("Invalid insertion index.\nChoose 0 through the current size.");
        else
        {
            // Preserve old visual positions for shifted values; data stays contiguous.
            for(int32 I=Values.Num();I>At;--I)
                NodeMeshes[I]->SetRelativeLocation(Slot(I-1));
            Values.Insert(Candidate,At);
            NodeMeshes[At]->SetRelativeLocation(Slot(At)+FVector(0,0,200));
            ActiveIndex=At; BusyTime=0.7f;
            Feedback=FString::Printf(TEXT("Inserted %d at index %d.\n%d element(s) shift right."),Candidate,At,Values.Num()-At-1);
            Entry=FString::Printf(TEXT("Insert [%d] = %d"),At,Candidate);
        }
    }
    else if(Operation==5)
    {
        SearchValue=Candidate; SearchIndex=-1; SearchElapsed=0; bSearching=!Values.IsEmpty();
        Feedback=Values.IsEmpty() ? TEXT("Not found: array is empty.\n0 comparisons.") : FString::Printf(TEXT("Searching for %d...\nComparing from index 0."),SearchValue);
        Entry=FString::Printf(TEXT("Linear search: %d"),SearchValue);
    }
    else if(!Values.IsValidIndex(ActiveIndex))
        Feedback=TEXT("Index out of bounds.\nRead/update/delete need an existing element.");
    else if(Operation==2)
    {
        const int32 Old=Values[ActiveIndex];
        Ghost->SetRelativeLocation(Slot(ActiveIndex));
        GhostLabel->SetRelativeLocation(Slot(ActiveIndex)+FVector(-72,0,0));
        GhostLabel->SetText(FText::AsNumber(Old));
        Ghost->SetVisibility(true); GhostLabel->SetVisibility(true);
        Values.RemoveAt(ActiveIndex,1,EAllowShrinking::No);
        for(int32 I=ActiveIndex;I<Values.Num();++I) NodeMeshes[I]->SetRelativeLocation(Slot(I+1));
        BusyTime=0.7f;
        Feedback=FString::Printf(TEXT("Deleted %d at index %d.\n%d element(s) shift left."),Old,ActiveIndex,Values.Num()-ActiveIndex);
        Entry=FString::Printf(TEXT("Delete [%d] -> %d"),ActiveIndex,Old);
    }
    else if(Operation==3)
    {
        Feedback=FString::Printf(TEXT("A[%d] = %d\nDirect indexed access: O(1)."),ActiveIndex,Values[ActiveIndex]);
        Entry=FString::Printf(TEXT("Read [%d] -> %d"),ActiveIndex,Values[ActiveIndex]);
    }
    else if(Operation==4)
    {
        Values[ActiveIndex]=Candidate;
        Feedback=FString::Printf(TEXT("A[%d] updated to %d.\nSize unchanged. O(1)."),ActiveIndex,Candidate);
        Entry=FString::Printf(TEXT("Update [%d] = %d"),ActiveIndex,Candidate);
    }
    if(!Entry.IsEmpty())
    {
        History.Insert(Entry,0);
        if(History.Num()>2) History.SetNum(2);
    }
    Refresh();
}

void AArrayLearningRoom::MakeQuestion()
{
    StopCelebration(); bAnswered=false; Selected=0; Values.Reset();
    const int32 Count=Cursor==5 ? Capacity : Random.RandRange(3,5);
    while(Values.Num()<Count)
    {
        int32 V=Random.RandRange(10,99);
        if(!Values.Contains(V)) Values.Add(V);
    }
    ActiveIndex=Random.RandRange(0,Count-1);
    RandomizeValues();
    Feedback=TEXT("Select your prediction.");
    switch(Cursor)
    {
    case 0:
        Question=TEXT("What is the FIRST valid index?");
        CorrectAnswer=TEXT("0"); Explanation=TEXT("Array indexing starts at zero."); break;
    case 1:
        Question=FString::Printf(TEXT("What is A[%d]?"),ActiveIndex);
        CorrectAnswer=FString::FromInt(Values[ActiveIndex]); Explanation=TEXT("Read the value above that index."); break;
    case 2:
        Question=FString::Printf(TEXT("Insert %d at index %d.\nHow many values shift right?"),Candidate,ActiveIndex);
        CorrectAnswer=FString::FromInt(Count-ActiveIndex); Explanation=TEXT("Shift the suffix starting at the index."); break;
    case 3:
        Question=FString::Printf(TEXT("Delete index %d.\nHow many values shift left?"),ActiveIndex);
        CorrectAnswer=FString::FromInt(Count-ActiveIndex-1); Explanation=TEXT("Only elements after the index shift."); break;
    case 4:
        Question=FString::Printf(TEXT("Size is %d.\nIs reading A[%d] valid?"),Count,Count);
        CorrectAnswer=TEXT("OUT OF BOUNDS"); Explanation=TEXT("Last valid element index is size - 1."); break;
    case 5:
        Question=TEXT("All six slots are used.\nCan this fixed-capacity array append?");
        CorrectAnswer=TEXT("NO: FULL"); Explanation=TEXT("No automatic resize in this lesson."); break;
    case 6:
        Question=FString::Printf(TEXT("Search left to right for %d.\nHow many comparisons to find it?"),Values[ActiveIndex]);
        CorrectAnswer=FString::FromInt(ActiveIndex+1); Explanation=TEXT("Include the comparison at the match."); break;
    default:
        Question=TEXT("What is the time complexity\nof reading a valid A[index]?");
        CorrectAnswer=TEXT("O(1)"); Explanation=TEXT("The index gives direct access."); break;
    }
    Answers={CorrectAnswer};
    if(Cursor==4) Answers={TEXT("OUT OF BOUNDS"),TEXT("VALID"),TEXT("RETURNS ZERO"),TEXT("READS LAST ELEMENT")};
    else if(Cursor==5) Answers={TEXT("NO: FULL"),TEXT("YES: AUTO RESIZE"),TEXT("OVERWRITE INDEX 0"),TEXT("DELETE LAST FIRST")};
    else if(Cursor==7) Answers={TEXT("O(1)"),TEXT("O(n)"),TEXT("O(log n)"),TEXT("O(n squared)")};
    else
    {
        while(Answers.Num()<4)
        {
            const FString S=FString::FromInt(Random.RandRange(Cursor==1 ? 10 : 0,Cursor==1 ? 99 : 6));
            if(!Answers.Contains(S)) Answers.Add(S);
        }
    }
    for(int32 I=Answers.Num()-1;I>0;--I) Answers.Swap(I,Random.RandRange(0,I));
    SnapCells(); Refresh();
}


FVector AArrayLearningRoom::Slot(int32 Index) const
{
    return FVector(0,-850+340*Index,865);
}

void AArrayLearningRoom::SnapCells()
{
    for(int32 I=0;I<Capacity;++I)
    {
        NodeMeshes[I]->SetRelativeLocation(Slot(I));
        Labels[I]->SetRelativeLocation(Slot(I)+FVector(-72,0,0));
    }
}

void AArrayLearningRoom::SearchStep()
{
    ++SearchIndex;
    if(!Values.IsValidIndex(SearchIndex)) { bSearching=false; return; }
    ActiveIndex=SearchIndex;
    if(Values[SearchIndex]==SearchValue)
    {
        bSearching=false;
        Feedback=FString::Printf(TEXT("Found %d at index %d.\n%d comparisons; first match."),SearchValue,SearchIndex,SearchIndex+1);
    }
    else if(SearchIndex==Values.Num()-1)
    {
        bSearching=false;
        Feedback=FString::Printf(TEXT("%d not found.\n%d comparisons."),SearchValue,Values.Num());
    }
    else Feedback=FString::Printf(TEXT("Compare A[%d] = %d with %d.\nNot equal: continue right."),SearchIndex,Values[SearchIndex],SearchValue);
    Refresh();
}
