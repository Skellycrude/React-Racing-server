// Fill out your copyright notice in the Description page of Project Settings.

#include "Replays.h"


// Sets default values for this component's properties
UReplays::UReplays()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}


// Called when the game starts
void UReplays::BeginPlay()
{
	Super::BeginPlay();

	// ...
	
}


// Called every frame
void UReplays::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}


void UReplays::StartRecordingReplay(FString ReplayName, FString FriendlyName)
{
	if (GameInstanceCpp)
	{
		GameInstanceCpp->StartRecordingReplay(ReplayName, FriendlyName);
	}
}

void UReplays::StopRecordingReplay()
{
	if (GameInstanceCpp)
	{
		GameInstanceCpp->StopRecordingReplay();
	}
}

bool UReplays::PlayReplay(FString ReplayName)
{
	if (GameInstanceCpp)
	{
		return GameInstanceCpp->PlayReplay(ReplayName);
	}
	else return false;
}