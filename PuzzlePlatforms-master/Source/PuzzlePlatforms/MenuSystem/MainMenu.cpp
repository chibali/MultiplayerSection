// Fill out your copyright notice in the Description page of Project Settings.


#include "MainMenu.h"
#include "Components/Button.h"
#include "Components/WidgetSwitcher.h"
#include "Components/EditableTextBox.h"
#include "Components/TextBlock.h"
#include "UObject/ConstructorHelpers.h"
#include "ServerRow.h"

UMainMenu::UMainMenu(const FObjectInitializer& ObjectInitializer)
{
    ConstructorHelpers::FClassFinder<UUserWidget> ServerRowWBPClass(TEXT("/Game/MenuSystem/WBP_ServerRow"));

    if (ServerRowWBPClass.Class != nullptr)
    {
        ServerRowClass = ServerRowWBPClass.Class;
    }
}

bool UMainMenu::Initialize()
{
    bool Success = Super::Initialize();
    if(!Success) return false;

    if (!ensure(HostButton != nullptr)) return false;
    HostButton->OnClicked.AddDynamic(this, &UMainMenu::OpenHostMenu);

    if (!ensure(SetNameButton != nullptr)) return false;
    SetNameButton->OnClicked.AddDynamic(this, &UMainMenu::HostServer);

    if (!ensure(CancelButton != nullptr)) return false;
    CancelButton->OnClicked.AddDynamic(this, &UMainMenu::OpenMainMenu);

    if (!ensure(JoinButton != nullptr)) return false;
    JoinButton->OnClicked.AddDynamic(this, &UMainMenu::OpenJoinMenu);

    if (!ensure(BackButton != nullptr)) return false;
    BackButton->OnClicked.AddDynamic(this, &UMainMenu::OpenMainMenu);

    if (!ensure(CancelButton != nullptr)) return false;
    CancelButton->OnClicked.AddDynamic(this, &UMainMenu::OpenMainMenu);

    if (!ensure(JoinServerButton != nullptr)) return false;
    JoinServerButton->OnClicked.AddDynamic(this, &UMainMenu::JoinServer);

    if (!ensure(QuitButton != nullptr)) return false;
    QuitButton->OnClicked.AddDynamic(this, &UMainMenu::QuitGame);

    return true;
}

void UMainMenu::OnLevelRemovedFromWorld(ULevel* InLevel, UWorld* InWorld)
{
    RemoveFromViewport();

    UWorld* World = GetWorld();
    if(!ensure(World != nullptr)) return;

    APlayerController* PlayerController = World->GetFirstPlayerController();
    if(!ensure (PlayerController != nullptr)) return;

    FInputModeGameOnly InputModeData;
    PlayerController->SetInputMode(InputModeData);
    PlayerController->bShowMouseCursor = false;

    Super::OnLevelRemovedFromWorld(InLevel, InWorld);
}

void UMainMenu::HostServer()
{
    if (MenuInterface != nullptr)
    {
        ServerName = ServerNameField->GetText().ToString();
        MenuInterface->Host();
    }
}

void UMainMenu::SetServerList(TArray<FServerData> ServerNames)
{
    UWorld* World = this->GetWorld();
    if (!ensure(World != nullptr)) return;

    ServerList->ClearChildren();

    uint32 i = 0;
    for (const FServerData& ServerData : ServerNames)
    {
        if (!ensure(ServerRowClass != nullptr)) return;
        ServerRow = CreateWidget<UServerRow>(World, ServerRowClass);

        if (!ensure(ServerRow != nullptr)) return;
        if (!ensure(ServerList != nullptr)) return;

        ServerRow->ServerName->SetText(FText::FromString(ServerData.Name));
        ServerRow->HostUser->SetText(FText::FromString(ServerData.HostUsername));
        FString Connections = (FString::Printf(TEXT("%d/%d"), ServerData.CurrentPlayers, ServerData.MaxPlayers));
        ServerRow->AvailableConnections->SetText((FText::FromString(Connections)));
        ServerRow->Setup(this, i);
        ++i;

        ServerList->AddChild(ServerRow);
    }
}

void UMainMenu::SelectIndex(uint32 Index)
{
    SelectedIndex = Index;
    UpdateChildren();
}

void UMainMenu::UpdateChildren()
{
    for (int32 i = 0; i < ServerList->GetChildrenCount(); ++i)
    {
        UServerRow* Row = Cast<UServerRow>(ServerList->GetChildAt(i));
        if (Row != nullptr)
        {
            Row->Selected = (SelectedIndex.IsSet()) && (SelectedIndex.GetValue() == i);
        }
    }
}

void UMainMenu::JoinServer()
{
    if (SelectedIndex.IsSet() && MenuInterface != nullptr)
    {
        UE_LOG(LogTemp, Warning, TEXT("Selected index %d: "), SelectedIndex.GetValue());
        MenuInterface->Join(SelectedIndex.GetValue());
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("Selected index not set"));
    }
}

void UMainMenu::OpenJoinMenu()
{
    if(!ensure(MenuSwitcher != nullptr)) return;
    if(!ensure(JoinMenu != nullptr)) return;
    if(!ensure(MenuInterface != nullptr)) return;
    MenuInterface->FindSessions();
    MenuSwitcher->SetActiveWidget(JoinMenu);
}

void UMainMenu::OpenMainMenu()
{
    if(!ensure(MenuSwitcher != nullptr)) return;
    if(!ensure(MainMenu != nullptr)) return;
    MenuSwitcher->SetActiveWidget(MainMenu);
}

void UMainMenu::OpenHostMenu()
{
    if (!ensure(MenuSwitcher != nullptr)) return;
    if (!ensure(MainMenu != nullptr)) return;
    MenuSwitcher->SetActiveWidget(HostMenu);
}

void UMainMenu::QuitGame()
{
    UWorld* World = GetWorld();
    if(!ensure(World != nullptr)) return;

    APlayerController* PlayerController = World->GetFirstPlayerController();
    if(!ensure (PlayerController != nullptr)) return;

    PlayerController->ConsoleCommand("Quit");
}
