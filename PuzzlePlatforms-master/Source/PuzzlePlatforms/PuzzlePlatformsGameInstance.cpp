// Fill out your copyright notice in the Description page of Project Settings.


#include "PuzzlePlatformsGameInstance.h"
#include "Engine/Engine.h"
#include "OnlineSessionSettings.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "UObject/ConstructorHelpers.h"
#include "Blueprint/UserWidget.h"
#include "MenuSystem/MainMenu.h"
#include "MenuSystem/MenuWidget.h"
#include "PlatformTrigger.h"

const static FName SESSION_NAME = TEXT("My Session Game");

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

            SessionSearch = MakeShareable(new FOnlineSessionSearch());
            if (SessionSearch.IsValid())
            {
                SessionSearch->bIsLanQuery = true;
                SessionInterface->FindSessions(0, SessionSearch.ToSharedRef());
                UE_LOG(LogTemp, Warning, TEXT("Finding Sessions..."));
            }
        }
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("Found Subsystem"));
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
        World->ServerTravel("/Game/ThirdPerson/Maps/ThirdPersonMap?listen");
    }
}

void UPuzzlePlatformsGameInstance::CreateSession()
{
    if (SessionInterface.IsValid())
    {
        FOnlineSessionSettings SessionSettings;
        SessionSettings.bIsLANMatch = true;
        SessionSettings.NumPublicConnections = 2;
        SessionSettings.bShouldAdvertise = true;
        SessionInterface->CreateSession(0, SESSION_NAME, SessionSettings);
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

void UPuzzlePlatformsGameInstance::OnFindSessionsComplete(bool Success)
{
    if (Success)
    {
        UE_LOG(LogTemp, Warning, TEXT("Session search complete"));
        if (Success && SessionSearch.IsValid())
        { 
            for (const FOnlineSessionSearchResult& SearchResult : SessionSearch->SearchResults)
            {
                UE_LOG(LogTemp, Warning, TEXT("Session found: %s"), *SearchResult.GetSessionIdStr());
            }
        }   
    }
}

void UPuzzlePlatformsGameInstance::Join(const FString& Address)
{
    UEngine* Engine = GetEngine();

    if(Engine != nullptr)
    {
        Engine->AddOnScreenDebugMessage(0, 2, FColor::Green, FString::Printf(TEXT("Joining %s"), *Address));
    }

    APlayerController* PlayerController = GetFirstLocalPlayerController();
    if(PlayerController != nullptr)
    {
        PlayerController->ClientTravel(Address, ETravelType::TRAVEL_Absolute);
    }
}
   
void UPuzzlePlatformsGameInstance::Quit()
{
    this->ReturnToMainMenu();
}
