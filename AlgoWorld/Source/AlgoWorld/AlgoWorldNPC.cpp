#include "AlgoWorldNPC.h"
#include "AlgoWorldConversationState.h"
#include "AlgoWorldDialogueWidget.h"
#include "Components/SceneComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/TextRenderComponent.h"
#include "Engine/StaticMesh.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "UObject/ConstructorHelpers.h"
#include "UObject/UnrealType.h"
#include "InputCoreTypes.h"

AAlgoWorldNPC::AAlgoWorldNPC()
{
    PrimaryActorTick.bCanEverTick=true;
    RootComponent=CreateDefaultSubobject<USceneComponent>(TEXT("TutorRoot"));
    auto* Capsule=CreateDefaultSubobject<UCapsuleComponent>(TEXT("TutorCollision"));
    Capsule->SetupAttachment(RootComponent);
    Capsule->SetRelativeLocation(FVector(0,0,100));
    Capsule->InitCapsuleSize(35,100);
    Capsule->SetCollisionProfileName(TEXT("BlockAllDynamic"));
    VisualRoot=CreateDefaultSubobject<USceneComponent>(TEXT("TutorBody"));
    VisualRoot->SetupAttachment(RootComponent);
    static ConstructorHelpers::FObjectFinder<UStaticMesh> Cube(TEXT("/Engine/BasicShapes/Cube.Cube"));
    static ConstructorHelpers::FObjectFinder<UStaticMesh> Sphere(TEXT("/Engine/BasicShapes/Sphere.Sphere"));
    Part(TEXT("Jacket"),Cube.Object,FVector(0,0,117),FVector(.46,.62,.62),TEXT("Coat"));
    Part(TEXT("Shirt"),Cube.Object,FVector(25,0,133),FVector(.025,.18,.25),TEXT("Paper"));
    Part(TEXT("Badge"),Cube.Object,FVector(25,-18,123),FVector(.035,.12,.08),TEXT("Gold"));
    Part(TEXT("Neck"),Sphere.Object,FVector(0,0,151),FVector(.22),TEXT("Skin"));
    Part(TEXT("Head"),Sphere.Object,FVector(0,0,173),FVector(.58,.58,.65),TEXT("Skin"));
    Part(TEXT("Hair"),Sphere.Object,FVector(-4,0,199),FVector(.56,.60,.20),TEXT("Dark"));
    Part(TEXT("EyeL"),Sphere.Object,FVector(28,-12,178),FVector(.09,.085,.10),TEXT("Paper"));
    Part(TEXT("EyeR"),Sphere.Object,FVector(28,12,178),FVector(.09,.085,.10),TEXT("Paper"));
    Part(TEXT("PupilL"),Sphere.Object,FVector(32,-12,178),FVector(.035,.04,.055),TEXT("Dark"));
    Part(TEXT("PupilR"),Sphere.Object,FVector(32,12,178),FVector(.035,.04,.055),TEXT("Dark"));
    Part(TEXT("Smile"),Cube.Object,FVector(27,0,162),FVector(.04,.15,.025),TEXT("Dark"));
    Part(TEXT("LegL"),Cube.Object,FVector(0,-18,51),FVector(.22,.23,.70),TEXT("Trousers"));
    Part(TEXT("LegR"),Cube.Object,FVector(0,18,51),FVector(.22,.23,.70),TEXT("Trousers"));
    Part(TEXT("ShoeL"),Cube.Object,FVector(7,-18,10),FVector(.39,.27,.20),TEXT("Dark"));
    Part(TEXT("ShoeR"),Cube.Object,FVector(7,18,10),FVector(.39,.27,.20),TEXT("Dark"));
    WaveArm=CreateDefaultSubobject<USceneComponent>(TEXT("GreetingArm"));
    WaveArm->SetupAttachment(VisualRoot);
    WaveArm->SetRelativeLocation(FVector(0,-42,142));
    Part(TEXT("SleeveL"),Cube.Object,FVector(0,0,-23),FVector(.20,.20,.46),TEXT("Coat"),WaveArm);
    Part(TEXT("HandL"),Sphere.Object,FVector(0,0,-50),FVector(.22),TEXT("Skin"),WaveArm);
    Part(TEXT("SleeveR"),Cube.Object,FVector(0,42,118),FVector(.20,.20,.48),TEXT("Coat"));
    Part(TEXT("HandR"),Sphere.Object,FVector(0,42,91),FVector(.22),TEXT("Skin"));
    Part(TEXT("Notebook"),Cube.Object,FVector(16,43,93),FVector(.14,.28,.38),TEXT("Gold"));
    for(auto Pair : {TPair<const TCHAR*,float>(TEXT("TutorName"),240),TPair<const TCHAR*,float>(TEXT("TalkPrompt"),215)})
    {
        auto* Label=CreateDefaultSubobject<UTextRenderComponent>(Pair.Key);
        Label->SetupAttachment(RootComponent);
        Label->SetRelativeLocation(FVector(0,0,Pair.Value));
        Label->SetHorizontalAlignment(EHTA_Center);
        Label->SetVerticalAlignment(EVRTA_TextCenter);
        Label->SetWorldSize(Pair.Value==240 ? 19 : 15);
        Label->SetTextRenderColor(Pair.Value==240 ? FColor(245,230,185) : FColor::White);
        Label->SetCollisionEnabled(ECollisionEnabled::NoCollision);
        if(Pair.Value==240) NameLabel=Label; else Prompt=Label;
    }
}

UStaticMeshComponent* AAlgoWorldNPC::Part(const TCHAR* Name,UStaticMesh* Mesh,FVector Position,FVector Scale,FName Colour,USceneComponent* Parent)
{
    auto* C=CreateDefaultSubobject<UStaticMeshComponent>(Name);
    C->SetMobility(EComponentMobility::Movable);
    C->SetupAttachment(Parent ? Parent : VisualRoot.Get());
    C->SetStaticMesh(Mesh); C->SetRelativeLocation(Position); C->SetRelativeScale3D(Scale);
    C->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    C->ComponentTags.Add(Colour);
    Parts.Add(C);
    return C;
}

FText AAlgoWorldNPC::TutorName() const
{
    if(!DisplayName.IsEmpty()) return DisplayName;
    const TCHAR* Names[]={TEXT("Mira"),TEXT("Arin"),TEXT("Sam"),TEXT("Tara")};
    return FText::FromString(Names[FMath::Clamp(Profile,0,3)]);
}

FText AAlgoWorldNPC::TutorRole() const
{
    if(!RoleTitle.IsEmpty()) return RoleTitle;
    const TCHAR* Roles[]={TEXT("Campus guide"),TEXT("Array tutor"),TEXT("Stack tutor"),TEXT("Binary Tree tutor")};
    return FText::FromString(Roles[FMath::Clamp(Profile,0,3)]);
}

TArray<FText> AAlgoWorldNPC::DefaultLines() const
{
    TArray<FString> Text;
    switch(FMath::Clamp(Profile,0,3))
    {
    case 0: Text={
        TEXT("Welcome to AlgoWorld! Explore the campus and talk to the tutors. Each classroom lets you try a data structure for yourself."),
        TEXT("Look for the Array, Stack and Binary Tree entrances. Walk close and press E to enter. Inside, walk to the learning console and press E again."),
        TEXT("Start in practice mode. Try one operation, predict the result, and watch what changes. Then use the quiz to check your understanding."),
        TEXT("An algorithm is a sequence of steps. A data structure organizes the information those steps work on. A good choice can make the same task much faster."),
        TEXT("The classroom exit returns you to where you entered from the Hub. Come back and talk to us whenever you want another explanation!")}; break;
    case 1: Text={
        TEXT("I am the Array tutor. An array keeps elements in indexed positions. Our classroom starts counting at zero: the first element is A[0]."),
        TEXT("Size and capacity are different. If three slots are used in a six-slot array, the size is 3 and the capacity is 6. A[3] is not yet a valid element to read."),
        TEXT("Reading a valid index is O(1): you can go straight to its position. You do not have to search through all the earlier elements."),
        TEXT("Inserting in the middle shifts later elements to the right. Deleting shifts them left. That is why these operations can take O(n) time."),
        TEXT("Linear search compares values one at a time. With duplicate values, our room returns the first match. Try searching for a value that is absent too.")}; break;
    case 2: Text={
        TEXT("Think of a pile of plates. The last plate placed on top is usually the first one removed. A stack follows Last In, First Out: LIFO."),
        TEXT("Push adds to the top. Pop removes and returns the top value. Peek reads the top without removing it."),
        TEXT("An empty stack has nothing to pop: that is underflow. Our six-slot classroom stack rejects a push when full: that is overflow."),
        TEXT("Push 12, then 27, then 8. The next pop returns 8, not 12. Try this sequence in practice and watch the orange TOP marker."),
        TEXT("Function calls use a call stack to track where execution should return. Undo features can also use stacks to reverse recent actions.")}; break;
    default: Text={
        TEXT("A binary tree gives each node at most two children, called left and right. A leaf has no children. The root is the starting node."),
        TEXT("A binary tree is not automatically a binary search tree. A search tree has an ordering rule; our Binary Tree room focuses on traversal without that rule."),
        TEXT("Preorder visits node, left, right. Inorder visits left, node, right. Postorder visits left, right, node. Watch how changing the rule changes the output."),
        TEXT("Level-order traversal visits one level at a time, using a queue. It is a breadth-first traversal."),
        TEXT("We measure height in edges in this room. A root with two levels below it has height 2, but there are 3 levels. Always check which convention a question uses.")}; break;
    }
    TArray<FText> Result;
    for(const FString& Line:Text) Result.Add(FText::FromString(Line));
    return Result;
}

void AAlgoWorldNPC::RefreshAppearance()
{
    for(const auto& Entry : Parts)
    {
        auto* C=Entry.Get();
        if(!C || !SurfaceMaterial) continue;
        FLinearColor Colour(.06,.075,.08);
        if(C->ComponentHasTag(TEXT("Coat"))) Colour=OutfitColor;
        else if(C->ComponentHasTag(TEXT("Skin"))) Colour=SkinColor;
        else if(C->ComponentHasTag(TEXT("Paper"))) Colour=FLinearColor(.85,.82,.72);
        else if(C->ComponentHasTag(TEXT("Gold"))) Colour=FLinearColor(.55,.35,.12);
        else if(C->ComponentHasTag(TEXT("Trousers"))) Colour=FLinearColor(.11,.13,.16);
        if(auto* Mat=C->CreateDynamicMaterialInstance(0,SurfaceMaterial)) Mat->SetVectorParameterValue(TEXT("Tint"),Colour);
    }
    NameLabel->SetText(FText::Format(FText::FromString(TEXT("{0} | {1}")),TutorName(),TutorRole()));
    Prompt->SetText(FText::FromString(TEXT("E - Talk")));
}

void AAlgoWorldNPC::OnConstruction(const FTransform& Transform)
{
    Super::OnConstruction(Transform); RefreshAppearance();
}

void AAlgoWorldNPC::BeginPlay()
{
    Super::BeginPlay(); RefreshAppearance();
    Lines=CustomDialogue.IsEmpty() ? DefaultLines() : CustomDialogue;
    Age=(GetUniqueID()%100)*.07f;
    Prompt->SetVisibility(false);
}

bool AAlgoWorldNPC::NearEntrance(APawn* Pawn) const
{
    for(TActorIterator<AActor> It(GetWorld());It;++It)
    {
        bool Entrance=false;
        for(UClass* C=It->GetClass();C;C=C->GetSuperClass())
        {
            const FName Name=C->GetFName();
            if(Name==TEXT("ArrayRoomEntrance") || Name==TEXT("StackRoomEntrance") || Name==TEXT("BinaryTreeRoomEntrance")) { Entrance=true; break; }
        }
        if(!Entrance) continue;
        float Radius=250;
        if(auto* Property=FindFProperty<FFloatProperty>(It->GetClass(),TEXT("InteractionRadius"))) Radius=Property->GetPropertyValue_InContainer(*It);
        if(FVector::DistSquared(Pawn->GetActorLocation(),It->GetActorLocation())<=FMath::Square(Radius+60)) return true;
    }
    return false;
}

bool AAlgoWorldNPC::CanTalk(APlayerController* PC) const
{
    APawn* Pawn=PC ? PC->GetPawn() : nullptr;
    if(!Pawn || FVector::Dist2D(Pawn->GetActorLocation(),GetActorLocation())>TalkRadius ||
        FMath::Abs(Pawn->GetActorLocation().Z-GetActorLocation().Z)>220 || NearEntrance(Pawn)) return false;
    FVector Eye; FRotator View; PC->GetPlayerViewPoint(Eye,View);
    const FVector Target=GetActorLocation()+FVector(0,0,165);
    if(FVector::DotProduct(View.Vector(),(Target-Eye).GetSafeNormal())<0.1f) return false;
    FCollisionQueryParams Params(SCENE_QUERY_STAT(NPCTalk),false,this); Params.AddIgnoredActor(Pawn);
    FHitResult Hit;
    return !GetWorld()->LineTraceSingleByChannel(Hit,Eye,Target,ECC_Visibility,Params);
}

AAlgoWorldNPC* AAlgoWorldNPC::FindNearest(APlayerController* PC,UWorld* World)
{
    AAlgoWorldNPC* Best=nullptr; double Distance=TNumericLimits<double>::Max();
    for(TActorIterator<AAlgoWorldNPC> It(World);It;++It)
    {
        if(!It->CanTalk(PC)) continue;
        const double D=FVector::DistSquared(PC->GetPawn()->GetActorLocation(),It->GetActorLocation());
        if(D<Distance || (D==Distance && Best && It->GetUniqueID()<Best->GetUniqueID())) { Distance=D; Best=*It; }
    }
    return Best;
}

void AAlgoWorldNPC::StartTalk(APlayerController* PC)
{
    auto* State=GetWorld()->GetSubsystem<UAlgoWorldConversationState>();
    if(!State || State->IsBlocking() || Lines.IsEmpty()) return;
    Dialogue=CreateWidget<UAlgoWorldDialogueWidget>(PC,UAlgoWorldDialogueWidget::StaticClass());
    if(!Dialogue) return;
    State->Speaker=this;
    TalkingController=PC; TalkingPawn=PC->GetPawn(); bTalking=true; LineIndex=0;
    if(auto* Character=Cast<ACharacter>(PC->GetPawn()))
    {
        Character->StopJumping(); Character->GetCharacterMovement()->StopMovementImmediately();
    }
    PC->SetIgnoreMoveInput(true); PC->SetIgnoreLookInput(true); bLocked=true;
    Dialogue->AddToViewport(50);
    Dialogue->ShowLine(TutorName(),TutorRole(),Lines[0],0,Lines.Num());
    Prompt->SetVisibility(false); GreetingTime=1.6f;
}

void AAlgoWorldNPC::StopTalk()
{
    if(Dialogue) { Dialogue->RemoveFromParent(); Dialogue=nullptr; }
    if(IsValid(TalkingController) && bLocked)
    {
        TalkingController->SetIgnoreMoveInput(false); TalkingController->SetIgnoreLookInput(false);
    }
    bLocked=false; bTalking=false; TalkingController=nullptr; TalkingPawn.Reset();
    if(GetWorld()) if(auto* State=GetWorld()->GetSubsystem<UAlgoWorldConversationState>()) State->Finish(this);
}

void AAlgoWorldNPC::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds); Age+=DeltaSeconds;
    GreetingCooldown=FMath::Max(0.f,GreetingCooldown-DeltaSeconds);
    GreetingTime=FMath::Max(0.f,GreetingTime-DeltaSeconds);
    WaveArm->SetRelativeRotation(FRotator(0,0,GreetingTime>0 ? 150+12*FMath::Sin(Age*9) : 3*FMath::Sin(Age*1.5f)));
    VisualRoot->SetRelativeLocation(FVector(0,0,.7f*FMath::Sin(Age*1.8f)));
    auto* PC=UGameplayStatics::GetPlayerController(this,0);
    if(!PC || !PC->GetPawn()) { if(bTalking) StopTalk(); return; }
    FVector Eye; FRotator View; PC->GetPlayerViewPoint(Eye,View);
    NameLabel->SetWorldRotation((Eye-NameLabel->GetComponentLocation()).Rotation());
    Prompt->SetWorldRotation((Eye-Prompt->GetComponentLocation()).Rotation());
    const double Distance=FVector::Dist2D(PC->GetPawn()->GetActorLocation(),GetActorLocation());
    NameLabel->SetVisibility(Distance<1200);
    if(Distance<500)
    {
        FRotator Aim=(PC->GetPawn()->GetActorLocation()-GetActorLocation()).Rotation(); Aim.Pitch=0; Aim.Roll=0;
        VisualRoot->SetWorldRotation(FMath::RInterpTo(VisualRoot->GetComponentRotation(),Aim,DeltaSeconds,2));
    }
    if(bTalking)
    {
        if(TalkingPawn.Get()!=PC->GetPawn() || Distance>TalkRadius+120) { StopTalk(); return; }
        if(PC->WasInputKeyJustPressed(EKeys::B)) { StopTalk(); return; }
        if(PC->WasInputKeyJustPressed(EKeys::E) || PC->WasInputKeyJustPressed(EKeys::Enter))
        {
            if(++LineIndex>=Lines.Num()) { StopTalk(); return; }
            Dialogue->ShowLine(TutorName(),TutorRole(),Lines[LineIndex],LineIndex,Lines.Num());
        }
        return;
    }
    auto* State=GetWorld()->GetSubsystem<UAlgoWorldConversationState>();
    const bool Focused=State && !State->IsBlocking() && CanTalk(PC) && FindNearest(PC,GetWorld())==this;
    Prompt->SetVisibility(Focused);
    if(Focused && GreetingCooldown<=0) { GreetingTime=1.6f; GreetingCooldown=15; }
    if(Focused && PC->WasInputKeyJustPressed(EKeys::E)) StartTalk(PC);
}

void AAlgoWorldNPC::EndPlay(const EEndPlayReason::Type Reason)
{
    StopTalk(); Super::EndPlay(Reason);
}
