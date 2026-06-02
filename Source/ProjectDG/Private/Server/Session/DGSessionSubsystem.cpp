#include "Server/Session/DGSessionSubsystem.h"

#include "Server/Backend/DGBackendClient.h"
#include "Core/DG_Debug.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "Misc/CommandLine.h"
#include "Misc/Parse.h"

void UDGSessionSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	InitializeBackendBaseUrlFromCommandLine();

	BackendClient = NewObject<UDGBackendClient>(this);
	BackendClient->Initialize(BackendBaseUrl);
}

void UDGSessionSubsystem::Deinitialize()
{
	BackendClient = nullptr;


	Super::Deinitialize();
}

void UDGSessionSubsystem::CreateRoomAndTravel(
	const FString& RoomName,
	const FString& RoomPassword,
	int64 AccountId,
	int64 CharacterId,
	const FString& RegionId
)
{
	if (!BackendClient)
	{
		const FString ErrorMessage = TEXT("[DGSessionSubsystem] BackendClient is null.");


		OnSessionRequestFailed.Broadcast(ErrorMessage);
		return;
	}

	if (!ValidateRoomInput(RoomName, RoomPassword))
	{
		return;
	}

	FDGCreateSessionRequest RequestData;
	RequestData.AccountId = AccountId;
	RequestData.CharacterId = CharacterId;
	RequestData.RegionId = RegionId;
	RequestData.RoomName = RoomName;
	RequestData.RoomPassword = RoomPassword;


	BackendClient->CreateSession(
		RequestData,
		FDGSessionApiResultCallback::CreateUObject(
			this,
			&UDGSessionSubsystem::HandleCreateSessionCompleted
		)
	);
}

void UDGSessionSubsystem::JoinRoomAndTravel(
	const FString& RoomName,
	const FString& RoomPassword,
	int64 AccountId,
	int64 CharacterId
)
{
	if (!BackendClient)
	{
		const FString ErrorMessage = TEXT("[DGSessionSubsystem] BackendClient is null.");


		OnSessionRequestFailed.Broadcast(ErrorMessage);
		return;
	}

	if (!ValidateRoomInput(RoomName, RoomPassword))
	{
		return;
	}

	FDGJoinSessionRequest RequestData;
	RequestData.RoomName = RoomName;
	RequestData.RoomPassword = RoomPassword;
	RequestData.AccountId = AccountId;
	RequestData.CharacterId = CharacterId;


	BackendClient->JoinSession(
		RequestData,
		FDGSessionApiResultCallback::CreateUObject(
			this,
			&UDGSessionSubsystem::HandleJoinSessionCompleted
		)
	);
}

void UDGSessionSubsystem::TravelToLastSession()
{
	if (!LastSessionConnectionInfo.bSuccess)
	{
		return;
	}

	if (LastSessionConnectionInfo.ServerIP.IsEmpty() || LastSessionConnectionInfo.ServerPort <= 0)
	{
		return;
	}

	TravelToDedicatedServer(LastSessionConnectionInfo);
}

FString UDGSessionSubsystem::GetLastSessionId() const
{
	return LastSessionConnectionInfo.SessionId;
}

void UDGSessionSubsystem::InitializeBackendBaseUrlFromCommandLine()
{
	FString CommandLineBackendUrl;

	if (FParse::Value(FCommandLine::Get(), TEXT("BackendUrl="), CommandLineBackendUrl))
	{
		CommandLineBackendUrl.TrimStartAndEndInline();

		if (!CommandLineBackendUrl.IsEmpty())
		{
			BackendBaseUrl = CommandLineBackendUrl;
		}
	}
}

void UDGSessionSubsystem::HandleCreateSessionCompleted(
	bool bSuccess,
	const FDGSessionConnectionInfo& Result
)
{
	if (!bSuccess)
	{
		OnSessionRequestFailed.Broadcast(Result.ErrorMessage);
		return;
	}

	LastSessionConnectionInfo = Result;


	OnSessionCreated.Broadcast(Result.SessionId);

	TravelToDedicatedServer(Result);
}

void UDGSessionSubsystem::HandleJoinSessionCompleted(
	bool bSuccess,
	const FDGSessionConnectionInfo& Result
)
{
	if (!bSuccess)
	{
		OnSessionRequestFailed.Broadcast(Result.ErrorMessage);
		return;
	}

	LastSessionConnectionInfo = Result;


	TravelToDedicatedServer(Result);
}

void UDGSessionSubsystem::TravelToDedicatedServer(
	const FDGSessionConnectionInfo& ConnectionInfo
)
{
	UWorld* World = GetWorld();

	if (!World)
	{
		return;
	}

	APlayerController* PlayerController = World->GetFirstPlayerController();

	if (!PlayerController)
	{
		return;
	}

	if (ConnectionInfo.ServerIP.IsEmpty() || ConnectionInfo.ServerPort <= 0)
	{
		return;
	}

	if (ConnectionInfo.SessionId.IsEmpty())
	{
		return;
	}

	if (ConnectionInfo.JoinToken.IsEmpty())
	{
		return;
	}

	const FString TravelUrl = FString::Printf(
		TEXT("%s:%d?SessionId=%s?JoinToken=%s"),
		*ConnectionInfo.ServerIP,
		ConnectionInfo.ServerPort,
		*ConnectionInfo.SessionId,
		*ConnectionInfo.JoinToken
	);


	PlayerController->ClientTravel(TravelUrl, TRAVEL_Absolute);
}

bool UDGSessionSubsystem::ValidateRoomInput(
	const FString& RoomName,
	const FString& RoomPassword
) const
{
	if (RoomName.TrimStartAndEnd().IsEmpty())
	{
		const FString ErrorMessage = TEXT("RoomName is empty.");


		OnSessionRequestFailed.Broadcast(ErrorMessage);
		return false;
	}

	if (RoomPassword.IsEmpty())
	{
		const FString ErrorMessage = TEXT("RoomPassword is empty.");


		OnSessionRequestFailed.Broadcast(ErrorMessage);
		return false;
	}

	return true;
}
