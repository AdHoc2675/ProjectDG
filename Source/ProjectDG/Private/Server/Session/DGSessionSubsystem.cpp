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

	Debug::Print(FString::Printf(
		TEXT("[DGSessionSubsystem] Initialized. BackendBaseUrl=%s"),
		*BackendBaseUrl
	));
}

void UDGSessionSubsystem::Deinitialize()
{
	BackendClient = nullptr;

	Debug::Print(TEXT("[DGSessionSubsystem] Deinitialized."));

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

		Debug::Print(ErrorMessage);
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

	Debug::Print(FString::Printf(
		TEXT("[DGSessionSubsystem] Request Create Room. RoomName=%s"),
		*RoomName
	));

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

		Debug::Print(ErrorMessage);
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

	Debug::Print(FString::Printf(
		TEXT("[DGSessionSubsystem] Request Join Room. RoomName=%s"),
		*RoomName
	));

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
		Debug::Print(TEXT("[DGSessionSubsystem] LastSessionConnectionInfo is not valid."));
		return;
	}

	if (LastSessionConnectionInfo.ServerIP.IsEmpty() || LastSessionConnectionInfo.ServerPort <= 0)
	{
		Debug::Print(TEXT("[DGSessionSubsystem] Last session server address is invalid."));
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

	Debug::Print(FString::Printf(
		TEXT("[DGSessionSubsystem] BackendBaseUrl=%s"),
		*BackendBaseUrl
	));
}

void UDGSessionSubsystem::HandleCreateSessionCompleted(
	bool bSuccess,
	const FDGSessionConnectionInfo& Result
)
{
	if (!bSuccess)
	{
		Debug::Print(FString::Printf(
			TEXT("[DGSessionSubsystem] Create Room Failed. Error=%s"),
			*Result.ErrorMessage
		));

		OnSessionRequestFailed.Broadcast(Result.ErrorMessage);
		return;
	}

	LastSessionConnectionInfo = Result;

	Debug::Print(FString::Printf(
		TEXT("[DGSessionSubsystem] Create Room Success. SessionId=%s Server=%s:%d"),
		*Result.SessionId,
		*Result.ServerIP,
		Result.ServerPort
	));

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
		Debug::Print(FString::Printf(
			TEXT("[DGSessionSubsystem] Join Room Failed. Error=%s"),
			*Result.ErrorMessage
		));

		OnSessionRequestFailed.Broadcast(Result.ErrorMessage);
		return;
	}

	LastSessionConnectionInfo = Result;

	Debug::Print(FString::Printf(
		TEXT("[DGSessionSubsystem] Join Room Success. SessionId=%s Server=%s:%d"),
		*Result.SessionId,
		*Result.ServerIP,
		Result.ServerPort
	));

	TravelToDedicatedServer(Result);
}

void UDGSessionSubsystem::TravelToDedicatedServer(
	const FDGSessionConnectionInfo& ConnectionInfo
)
{
	UWorld* World = GetWorld();

	if (!World)
	{
		Debug::Print(TEXT("[DGSessionSubsystem] World is null."));
		return;
	}

	APlayerController* PlayerController = World->GetFirstPlayerController();

	if (!PlayerController)
	{
		Debug::Print(TEXT("[DGSessionSubsystem] PlayerController is null."));
		return;
	}

	if (ConnectionInfo.ServerIP.IsEmpty() || ConnectionInfo.ServerPort <= 0)
	{
		Debug::Print(TEXT("[DGSessionSubsystem] Server address is invalid."));
		return;
	}

	if (ConnectionInfo.SessionId.IsEmpty())
	{
		Debug::Print(TEXT("[DGSessionSubsystem] SessionId is empty."));
		return;
	}

	if (ConnectionInfo.JoinToken.IsEmpty())
	{
		Debug::Print(TEXT("[DGSessionSubsystem] JoinToken is empty."));
		return;
	}

	const FString TravelUrl = FString::Printf(
		TEXT("%s:%d?SessionId=%s?JoinToken=%s"),
		*ConnectionInfo.ServerIP,
		ConnectionInfo.ServerPort,
		*ConnectionInfo.SessionId,
		*ConnectionInfo.JoinToken
	);

	Debug::Print(FString::Printf(
		TEXT("[DGSessionSubsystem] ClientTravel To %s"),
		*TravelUrl
	));

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

		Debug::Print(FString::Printf(
			TEXT("[DGSessionSubsystem] %s"),
			*ErrorMessage
		));

		OnSessionRequestFailed.Broadcast(ErrorMessage);
		return false;
	}

	if (RoomPassword.IsEmpty())
	{
		const FString ErrorMessage = TEXT("RoomPassword is empty.");

		Debug::Print(FString::Printf(
			TEXT("[DGSessionSubsystem] %s"),
			*ErrorMessage
		));

		OnSessionRequestFailed.Broadcast(ErrorMessage);
		return false;
	}

	return true;
}