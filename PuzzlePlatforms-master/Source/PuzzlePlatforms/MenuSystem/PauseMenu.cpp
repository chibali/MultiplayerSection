// Fill out your copyright notice in the Description page of Project Settings.


#include "PauseMenu.h"
#include "Components/Button.h"

bool UPauseMenu::Initialize()
{
    bool Success = Super::Initialize();
    if(!Success) return false;

    if (!ensure(BackButtonPauseMenu != nullptr)) return false;
    BackButtonPauseMenu->OnClicked.AddDynamic(this, &UPauseMenu::CancelPressed);

    if (!ensure(QuitButton != nullptr)) return false;
    QuitButton->OnClicked.AddDynamic(this, &UPauseMenu::QuitGame);

    return true;
}

void UPauseMenu::CancelPressed()
{
    TearDown();
}

void UPauseMenu::QuitGame()
{
       if (MenuInterface != nullptr)
    {
        MenuInterface->Quit();
    }
}





