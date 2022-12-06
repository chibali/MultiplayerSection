// Fill out your copyright notice in the Description page of Project Settings.


#include "ServerRow.h"
#include "MainMenu.h"
#include "Components/Button.h"


void UServerRow::Setup(class UMainMenu* InParent, uint32 InIndex)
{
    Parent = InParent;
    Index = InIndex;

    if (!ensure(ChooseButton != nullptr)) return;
    ChooseButton->OnClicked.AddDynamic(this, &UServerRow::OnClicked);

}

void UServerRow::OnClicked()
{
    Parent->SelectIndex(Index);
}