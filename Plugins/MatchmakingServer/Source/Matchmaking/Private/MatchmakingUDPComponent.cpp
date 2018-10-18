// Copyright 2017-2018 Robin Zinser. All Rights Reserved.

#include "MatchmakingUDPComponent.h"
#include "Matchmaking.h"
#include "Runtime/Sockets/Public/Sockets.h"
#include "Runtime/Sockets/Public/SocketSubsystem.h"
#include "Runtime/Sockets/Public/IPAddress.h"
#include "HAL/RunnableThread.h"
#include "HAL/Runnable.h"

/**
* A thread used to handle UDP network communications.
*/
class FReceiveThread : public FRunnable
{
private:
	UMatchmakingUDPComponent* _cmp;
	FRunnableThread* Thread;
	volatile bool bexit;

public:
	FReceiveThread(UMatchmakingUDPComponent* cmp) : _cmp(cmp), bexit(false)
	{
		Thread = FRunnableThread::Create(this, TEXT("FReceiveThread"), 0, TPri_AboveNormal);
	}

	virtual void Stop()
	{
		bexit = true;
		Thread->WaitForCompletion();
	}

	virtual uint32 Run()
	{
		int32 BufferSize = 8192;
		TArray<uint8> Data;
		Data.Init(0, BufferSize);
		uint32 PendingDataSize = 0;
		int32 BytesRead = 0;

		// A temporary "received from" address
		TSharedRef<FInternetAddr> SockAddr = ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->CreateInternetAddr();

		while (!bexit)
		{
			if (!_cmp->ListenSocket->HasPendingData(PendingDataSize))
			{
				FPlatformProcess::Sleep(0.03f);
				continue;
			}

			_cmp->ListenSocket->RecvFrom(&Data[0], BufferSize, BytesRead, *SockAddr);
			if (BytesRead)
			{
				uint32 ip;
				int32 port;
				SockAddr->GetIp(ip);
				SockAddr->GetPort(port);
				Data.SetNum(BytesRead);
				_cmp->OnUDPReceive.Broadcast(_cmp, Data, ip, port);
			}
		}

		return 0;
	}
};


// Sets default values for this component's properties
UMatchmakingUDPComponent::UMatchmakingUDPComponent()
	: ListenSocket(NULL),
	ListenAddr(ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->CreateInternetAddr())
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = false;
}


// InitSocket()
//
// Task: Init everything :)
//
bool UMatchmakingUDPComponent::InitSocket(int32 port)
{
	// Prepare Temp Variables
	bool bSuccess = false;
	ISocketSubsystem* SocketSubsystem = ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM);
	ListenAddr->SetPort(port);

	// Create the needed Socket
	ListenSocket = SocketSubsystem->CreateSocket(NAME_DGram, TEXT("UDP Matchmaking"), true);
	if (ListenSocket == NULL)
	{
		UE_LOG(LogUDPCommand, Error, TEXT("Failed to create listen socket for Matchmaking UDP"));
		return false;
	}

	// Set config
	ListenSocket->SetReuseAddr();
	ListenSocket->SetNonBlocking();
	ListenSocket->SetRecvErr();

	// Try to bind
	if (ListenSocket->Bind(*ListenAddr))
		return true;
	
	// Binding failed -> Log Error
	else {
		UE_LOG(LogUDPCommand, Error, TEXT("Failed to bind listen socket to addr (%s) for Matchmaking UDP"), *ListenAddr->ToString(true));
		return false;
	}
}
// InitSocket()


// isSenderLocal()
//
// Task: Check from where the data is coming
//
bool UMatchmakingUDPComponent::isSenderLocal(int32 fromip, int32 fromport)
{
	// create Temp Variables
	uint32 tmp_ip;
	int32 tmp_port;

	// and get IP and Port from the ListenAddress
	ListenAddr->GetIp(tmp_ip);
	ListenAddr->GetPort(tmp_port);
	return (tmp_ip == 0 || tmp_ip == fromip) && (tmp_port == fromport);
}
// isSenderLocal()


// SendAnswer()
//
// Task: Send Data to a IntAdress (Answer)
//
bool UMatchmakingUDPComponent::SendAnswer(const TArray<uint8>& data, int32 fromip, int32 fromport)
{
	int32 tmp_BytesSent = 0;
	TSharedRef<FInternetAddr> tmp_SockAddr = ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->CreateInternetAddr();

	// Set IP and Port
	tmp_SockAddr->SetIp(fromip);
	tmp_SockAddr->SetPort(fromport);

	// Send the Message
	ListenSocket->SendTo(data.GetData(), data.Num(), tmp_BytesSent, *tmp_SockAddr);
	return tmp_BytesSent > 0;
}
// SendAnswer()


// SendMessage()
//
// Task: Send Regular Data (String Adress)
//
bool UMatchmakingUDPComponent::SendMessage(const TArray<uint8>& data, const FString& targetip, int32 targetport)
{
	// Create Temp Variables
	bool tmp_valid;
	int32 tmp_BytesSent = 0;
	TSharedRef<FInternetAddr> tmp_SockAddr = ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->CreateInternetAddr();
	
	// Set the IP and check if result is valid (-> if not -> return false / cancle)
	tmp_SockAddr->SetIp(*targetip, tmp_valid);
	if (!tmp_valid)
		return false;

	// Set Port
	tmp_SockAddr->SetPort(targetport);

	// Send the Message
	ListenSocket->SendTo(data.GetData(), data.Num(), tmp_BytesSent, *tmp_SockAddr);
	return tmp_BytesSent > 0;
}
// SendMessage()


// Start()
//
// Task: Stop the running component
//
bool UMatchmakingUDPComponent::Start(int32 port)
{
	if (!InitSocket(port))
		return false;
	
	else {
		ListenThread = new FReceiveThread(this);
		return true;
	}
}
// Start()


// Stop()
//
// Task: Stop the running component
//
void UMatchmakingUDPComponent::Stop()
{
	// Stop the thread
	ListenThread->Stop();

	delete ListenThread;
	ListenThread = nullptr;

	ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->DestroySocket(ListenSocket);
}
// Stop()


// UTF8ToString()
//
// Task: Convert UTF8 -> String
//
FString UMatchmakingUDPComponent::UTF8ToString(const TArray<uint8>& utf8data)
{
	FUTF8ToTCHAR str((ANSICHAR*)utf8data.GetData(), utf8data.Num());
	return FString(str.Get(), str.Length());
}
// UTF8ToString()


// StringToUTF8()
//
// Task: Convert String -> UTF8
//
TArray<uint8> UMatchmakingUDPComponent::StringToUTF8(const FString& str)
{
	TArray<uint8> utf8data;
	FTCHARToUTF8 UStr(*str);
	utf8data.Append((uint8*)UStr.Get(), UStr.Length());
	return utf8data;
}
// StringToUTF8()