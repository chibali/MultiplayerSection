// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MenuWidget.h"
#include "MenuInterface.h"
#include "PauseMenu.generated.h"

/**
 * 
 */
UCLASS()
class PUZZLEPLATFORMS_API UPauseMenu : public UMenuWidget
{
	GENERATED_BODY()
	
public:

	virtual bool Initialize();

private:

	UPROPERTY(meta = (BindWidget))
	class UButton* BackButtonPauseMenu;

	UPROPERTY(meta = (BindWidget))
	class UButton* QuitButton;

	UFUNCTION(BlueprintCallable)
	void CancelPressed();

	UFUNCTION()
	void QuitGame();
};
