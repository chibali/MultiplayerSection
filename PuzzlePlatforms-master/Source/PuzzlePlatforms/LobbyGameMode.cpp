// Fill out your copyright notice in the Description page of Project Settings.


#include "LobbyGameMode.h"

void ALobbyGameMode::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);
	++PlayerCount;
	if (PlayerCount >= 3)
	{
		UWorld* World = GetWorld();
		if (World != nullptr)
		{
			World->ServerTravel("/Game/Maps/Game?listen");
		}
	}
}
void ALobbyGameMode::Logout(AController* Exiting)
{
	Super::Logout(Exiting);
	--PlayerCount;
	
}