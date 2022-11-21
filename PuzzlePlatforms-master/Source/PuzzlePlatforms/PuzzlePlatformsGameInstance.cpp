// Fill out your copyright notice in the Description page of Project Settings.


#include "PuzzlePlatformsGameInstance.h"
#include "Engine/Engine.h"
#include "OnlineSubsystem.h"
#include "UObject/ConstructorHelpers.h"
#include "Blueprint/UserWidget.h"
#include "MenuSystem/MainMenu.h"
#include "MenuSystem/MenuWidget.h"
#include "PlatformTrigger.h"

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
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("Found Subsystem"));
    }

}

void UPuzzlePlatformsGameInstance::Host()
{
    UEngine* Engine = GetEngine();

    if(Engine != nullptr)
    {
        Engine->AddOnScreenDebugMessage(0, 2, FColor::Green, TEXT("Hosting"));
    }

    UWorld* World = GetWorld();
    if (World != nullptr)
    {
        World->ServerTravel("/Game/ThirdPerson/Maps/ThirdPersonMap?listen");
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
