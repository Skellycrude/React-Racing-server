// Copyright 2017-2018 Robin Zinser. All Rights Reserved.

#pragma once

#include "Components/ActorComponent.h"
#include "MatchmakingUDPComponent.generated.h"


DECLARE_DYNAMIC_MULTICAST_DELEGATE_FourParams(FOnUDPReceive, UMatchmakingUDPComponent*, cmp, const TArray<uint8>&, data, int32, fromip, int32, fromport);

/**
* A simple udp tools. Useful for udp network communication.
*/
UCLASS(hideCategories = (Tags, Activation, Cooking, Collision), ClassGroup = Matchmaking, meta = (BlueprintSpawnableComponent))
class MATCHMAKING_API UMatchmakingUDPComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UMatchmakingUDPComponent();

protected:
	bool InitSocket(int32 port);

public:
	// Start the UDP Component
	UFUNCTION(BlueprintCallable, Category = UDPCommand)
		bool Start(int32 port = 19999);

	// Stop the UDP Component
	UFUNCTION(BlueprintCallable, Category = UDPCommand)
		void Stop();

	// Convert UTF8 -> String
	UFUNCTION(BlueprintCallable, Category = UDPCommand)
		static FString UTF8ToString(const TArray<uint8>& utf8data);

	// Convert String -> UTF8
	UFUNCTION(BlueprintCallable, Category = UDPCommand)
		static TArray<uint8> StringToUTF8(const FString& data);

	// Send Data as Answer
	UFUNCTION(BlueprintCallable, Category = UDPCommand)
		bool SendAnswer(const TArray<uint8>& data, int32 fromip, int32 fromport);

	// Send Regular Data (String Adress)
	UFUNCTION(BlueprintCallable, Category = UDPCommand)
		bool SendMessage(const TArray<uint8>& data, const FString& targetip, int32 targetport);

	// Check from where the data is coming
	UFUNCTION(BlueprintCallable, Category = UDPCommand)
		bool isSenderLocal(int32 fromip, int32 fromport);

	/** Event called when data is received */
	UPROPERTY(BlueprintAssignable, Category = UDPCommand)
		FOnUDPReceive OnUDPReceive;

protected:
	class FSocket* ListenSocket;
	TSharedPtr<class FInternetAddr> ListenAddr;
	class FRunnable* ListenThread;
	friend class FReceiveThread;
};
