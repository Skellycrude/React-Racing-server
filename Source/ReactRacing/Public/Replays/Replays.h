// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/GameInstance.h"
#include "Replays.generated.h"


UCLASS( Blueprintable, BlueprintType, ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class REACTRACING_API UReplays : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UReplays();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	UPROPERTY(BlueprintReadWrite)
		UGameInstance* GameInstanceCpp;

	/** Start recording a replay from blueprint. ReplayName = Name of file on disk, FriendlyName = Name of replay in UI */
	UFUNCTION(BlueprintCallable, Category = "Replays")
		void StartRecordingReplay(FString ReplayName, FString FriendlyName);

	/** Start recording a running replay and save it, from blueprint. */
	UFUNCTION(BlueprintCallable, Category = "Replays")
		void StopRecordingReplay();

	/** Start playback for a previously recorded Replay, from blueprint */
	UFUNCTION(BlueprintCallable, Category = "Replays")
		bool PlayReplay(FString ReplayName);
	
};
