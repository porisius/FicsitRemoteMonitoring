#include "Validation.h"

#include "Runtime/Sockets/Public/SocketSubsystem.h"
#include "Runtime/TraceLog/standalone_prologue.h"
#include "Runtime/Sockets/Public/Sockets.h"
#include "UObject/NameTypes.h"

bool UFRMValidation::IsTcpPortAvailable(int32 Port, FString& OutReason)
{
	OutReason.Empty();

	if (Port < 1 || Port > 65535)
	{
		OutReason = TEXT("Port is outside valid range 1-65535.");
		return false;
	}

	ISocketSubsystem* SocketSubsystem = ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM);
	if (!SocketSubsystem)
	{
		OutReason = TEXT("Socket subsystem is unavailable.");
		return false;
	}

	TSharedRef<FInternetAddr> Addr = SocketSubsystem->CreateInternetAddr();
	Addr->SetAnyAddress();
	Addr->SetPort(Port);

	FSocket* TestSocket = SocketSubsystem->CreateSocket(NAME_Stream, TEXT("PortAvailabilityTest"), false);
	if (!TestSocket)
	{
		OutReason = TEXT("Failed to create test socket.");
		return false;
	}

	const bool bBindSucceeded = TestSocket->Bind(*Addr);

	if (!bBindSucceeded)
	{
		OutReason = TEXT("Bind failed. Port is likely already in use or not permitted.");
	}

	TestSocket->Close();
	SocketSubsystem->DestroySocket(TestSocket);

	return bBindSucceeded;
}
