// Fill out your copyright notice in the Description page of Project Settings.


#include "PuzzlePlatformsGameInstance.h"
#include "Engine/Engine.h"
#include "OnlineSessionSettings.h"
#include "UObject/ConstructorHelpers.h"
#include "Blueprint/UserWidget.h"
#include "MenuSystem/MainMenu.h"
#include "MenuSystem/MenuWidget.h"
#include "UObject/UnrealNames.h"
#include "PlatformTrigger.h"

const static FName SESSION_NAME = EName::GameSession;
const static FName SERVER_NAME_SETTINGS_KEY = TEXT("ServerName");

UPuzzlePlatformsGameInstance::UPuzzlePlatformsGameInstance(const FObjectInitializer & ObjectInitializer)
{
    ConstructorHelpers::FClassFinder<UUserWidget> MenuSystemWBPClass(TEXT("/Game/MenuSystem/WBP_MainMenu"));

    if (MenuSystemWBPClass.Class != nullptr)
    {
        MenuClass = MenuSystemWBPClass.Class;
    }    

    ConstructorHelpers::FClassFinder<UUserWidget> PauseMenuSystemWBPClass(TEXT("/Game/MenuSystem/WBP_PauseMenu"));

    if (PauseMenuSystemWBPClass.Class != nullptr)
    {
        PauseMenuClass = PauseMenuSystemWBPClass.Class;
    }
}

void UPuzzlePlatformsGameInstance::LoadMenu()
{
    if (MenuClass == nullptr) return;
    
    Menu = CreateWidget<UMainMenu>(this, MenuClass);
    if (Menu == nullptr) return;

    Menu->Setup();   
    Menu->SetMenuInterface(this);
}

void UPuzzlePlatformsGameInstance::LoadPauseMenu()
{
    if (PauseMenuClass == nullptr) return;

    UMenuWidget* PauseMenu = CreateWidget<UMenuWidget>(this, PauseMenuClass);
    if (PauseMenu == nullptr) return;
    PauseMenu->Setup();
    PauseMenu->SetMenuInterface(this);
}

void UPuzzlePlatformsGameInstance::Init()
{
    UE_LOG(LogTemp, Warning, TEXT("Init() Log Message"));
    IOnlineSubsystem* OnlineSubsystem = IOnlineSubsystem::Get();

    if (OnlineSubsystem != nullptr)
    {
        UE_LOG(LogTemp, Warning, TEXT("Found Subsystem: %s"), *OnlineSubsystem->GetSubsystemName().ToString());
        SessionInterface = OnlineSubsystem->GetSessionInterface();
        if (SessionInterface.IsValid())
        {
            SessionInterface->OnCreateSessionCompleteDelegates.AddUObject(this, &UPuzzlePlatformsGameInstance::OnCreateSessionComplete);
            SessionInterface->OnDestroySessionCompleteDelegates.AddUObject(this, &UPuzzlePlatformsGameInstance::OnDestroySessionComplete);
            SessionInterface->OnFindSessionsCompleteDelegates.AddUObject(this, &UPuzzlePlatformsGameInstance::OnFindSessionsComplete);
            SessionInterface->OnJoinSessionCompleteDelegates.AddUObject(this, &UPuzzlePlatformsGameInstance::OnJoinSessionComplete);
        }
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("Found Subsystem"));
    }

    if (GEngine != nullptr)
    {
        GEngine->OnNetworkFailure().AddUObject(this, &UPuzzlePlatformsGameInstance::OnNetworkFailure);
    }
}

void UPuzzlePlatformsGameInstance::Host()
{
    if (SessionInterface.IsValid())
    {
        auto ExistingSession = SessionInterface->GetNamedSession(SESSION_NAME);
        if (ExistingSession != nullptr)
        {
            SessionInterface->DestroySession(SESSION_NAME);
        }
        else
        {
            UE_LOG(LogTemp, Warning, TEXT("FIRST SESSION CREATED"));
            CreateSession();
        }
    }
}

void UPuzzlePlatformsGameInstance::CreateSession()
{
    if (SessionInterface.IsValid())
    {
        FOnlineSessionSettings SessionSettings;
        FString DesiredServerName = Menu->ServerName;

        if (IOnlineSubsystem::Get()->GetSubsystemName() == "NULL")
        {
            SessionSettings.bIsLANMatch = true;
        }
        else
        {
            SessionSettings.bIsLANMatch = false;
        }

        SessionSettings.NumPublicConnections = 5;
        SessionSettings.bShouldAdvertise = true;
        SessionSettings.bUsesPresence = true;
        SessionSettings.bUseLobbiesIfAvailable = true;
        SessionSettings.Set(SERVER_NAME_SETTINGS_KEY, DesiredServerName, EOnlineDataAdvertisementType::ViaOnlineServiceAndPing);
        SessionInterface->CreateSession(0, SESSION_NAME, SessionSettings);
    }
}

void UPuzzlePlatformsGameInstance::OnCreateSessionComplete(FName SessionName, bool Success)
{
    if (!Success)
    {
        UE_LOG(LogTemp, Warning, TEXT("Could not create Online session"));
        return;
    }

    UEngine* Engine = GetEngine();

    if (Engine != nullptr)
    {
        Engine->AddOnScreenDebugMessage(0, 2, FColor::Green, TEXT("Hosting"));
    }

    UWorld* World = GetWorld();
    if (World != nullptr)
    {
        World->ServerTravel("/Game/Maps/Lobby?listen");
    }
}

void UPuzzlePlatformsGameInstance::OnDestroySessionComplete(FName SessionName, bool Success)
{
    if (Success)
    {
        UE_LOG(LogTemp, Warning, TEXT("Session Destroyed, creating a new Online Session"));
        CreateSession();
    }
}

void UPuzzlePlatformsGameInstance::OnNetworkFailure(UWorld* World, UNetDriver* NetDriver, ENetworkFailure::Type FailureType, const FString& ErrorString)
{
    Quit();
}

void UPuzzlePlatformsGameInstance::StartSession()
{
    if (SessionInterface.IsValid())
    {
        SessionInterface->StartSession(SESSION_NAME);
    }
}

void UPuzzlePlatformsGameInstance::FindSessions()
{
    SessionSearch = MakeShareable(new FOnlineSessionSearch());
    if (SessionSearch.IsValid())
    {
        /*SessionSearch->bIsLanQuery = true;*/
        SessionSearch->QuerySettings.Set(SEARCH_PRESENCE, true, EOnlineComparisonOp::Equals);
        SessionSearch->MaxSearchResults = 100;
        SessionInterface->FindSessions(0, SessionSearch.ToSharedRef());
        UE_LOG(LogTemp, Warning, TEXT("Finding Sessions..."));
    }
}

void UPuzzlePlatformsGameInstance::OnFindSessionsComplete(bool Success)
{
    if (Success)
    {
        TArray<FServerData> ServerNames;
        UE_LOG(LogTemp, Warning, TEXT("Session search complete"));
        if (Success && SessionSearch.IsValid())
        { 
            for (const FOnlineSessionSearchResult& SearchResult : SessionSearch->SearchResults)
            {   
                FServerData Data;
                FString ServerName;
                if (SearchResult.Session.SessionSettings.Get(SERVER_NAME_SETTINGS_KEY, ServerName))
                {
                    Data.Name = ServerName;                   
                }
                else
                {
                    Data.Name = "Could not find Server Name";
                }
                Data.MaxPlayers = SearchResult.Session.SessionSettings.NumPublicConnections;
                Data.CurrentPlayers = Data.MaxPlayers - SearchResult.Session.NumOpenPublicConnections;
                Data.HostUsername = SearchResult.Session.OwningUserName;
               
                
                ServerNames.Add(Data);
               
            }
            if (Menu != nullptr)
            {
                Menu->SetServerList(ServerNames);
            }
        }   
    }
}

void UPuzzlePlatformsGameInstance::Join(uint32 Index)
{
    if (!SessionInterface.IsValid()) return;
    if (!SessionSearch.IsValid()) return;

    SessionInterface->JoinSession(0, SESSION_NAME, SessionSearch->SearchResults[Index]);

      
}

void UPuzzlePlatformsGameInstance::OnJoinSessionComplete(FName SessionName, EOnJoinSessionCompleteResult::Type Result)
{
    if (!SessionInterface.IsValid()) return;
    if (!SessionSearch.IsValid()) return;

    FString TravelAddress;
    if (!SessionInterface->GetResolvedConnectString(SESSION_NAME, TravelAddress))
    {
        UE_LOG(LogTemp, Warning, TEXT("Could not get connect string"));
        return;
    }

    UEngine* Engine = GetEngine();
    if (Engine != nullptr)
    {
        Engine->AddOnScreenDebugMessage(0, 2, FColor::Green, FString::Printf(TEXT("Joining %s"), *TravelAddress));
    }

    APlayerController* PlayerController = GetFirstLocalPlayerController();
    if (PlayerController != nullptr)
    {
        PlayerController->ClientTravel(TravelAddress, ETravelType::TRAVEL_Absolute);
    }
}

void UPuzzlePlatformsGameInstance::Quit()
{
    this->ReturnToMainMenu();
}
