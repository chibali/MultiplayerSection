// Fill out your copyright notice in the Description page of Project Settings.


#include "LobbyGameMode.h"
#include "TimerManager.h"
#include "PuzzlePlatformsGameInstance.h"


void ALobbyGameMode::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);
	++PlayerCount;
	if (PlayerCount >= 3)
	{
		GetWorldTimerManager().SetTimer(StartSessionHandle, this, &ALobbyGameMode::SessionStart, StartSessionDelay);
		
	}
}
void ALobbyGameMode::Logout(AController* Exiting)
{
	Super::Logout(Exiting);
	--PlayerCount;
	
}
void ALobbyGameMode::SessionStart()
{
	auto PuzzlePlatformsGameInstance = Cast<UPuzzlePlatformsGameInstance>(GetGameInstance());
	if (PuzzlePlatformsGameInstance == nullptr) return;

	PuzzlePlatformsGameInstance->StartSession();

	UWorld* World = GetWorld();
	if (!ensure(World != nullptr)) return;

	bUseSeamlessTravel = true;
	World->ServerTravel("/Game/Maps/Game?listen");
	
	
}