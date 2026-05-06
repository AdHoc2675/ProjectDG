#include "Server/Session/DGSessionSubsystem.h"

#include "Server/Backend/DGBackendClient.h"
#include "Core/DG_Debug.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"

void UDGSessionSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	BackendClient = NewObject<UDGBackendClient>(this);
	BackendClient->Initialize(BackendBaseUrl);

	Debug::Print(TEXT("[DGSessionSubsystem] Initialized."));
}

void UDGSessionSubsystem::Deinitialize()
{
	BackendClient = nullptr;

	Debug::Print(TEXT("[DGSessionSubsystem] Deinitialized."));

	Super::Deinitialize();
}

void UDGSessionSubsystem::CreateLocalSessionAndTravel(
	int64 AccountId,
	int64 CharacterId,
	const FString& RegionId
)
{
	if (!BackendClient)
	{
		Debug::Print(TEXT("[DGSessionSubsystem] BackendClient is null."));
		return;
	}

	bTravelAfterCreateSession = true;
	
	FDGCreateSessionRequest RequestData;
	RequestData.AccountId = AccountId;
	RequestData.CharacterId = CharacterId;
	RequestData.RegionId = RegionId;

	Debug::Print(TEXT("[DGSessionSubsystem] Request Create Session."));

	BackendClient->CreateSession(
		RequestData,
		FDGSessionApiResultCallback::CreateUObject(
			this,
			&UDGSessionSubsystem::HandleCreateSessionCompleted
		)
	);
}

void UDGSessionSubsystem::CreateLocalSessionOnly(
	int64 AccountId,
	int64 CharacterId,
	const FString& RegionId
)
{
	if (!BackendClient)
	{
		Debug::Print(TEXT("[DGSessionSubsystem] BackendClient is null."));
		return;
	}

	bTravelAfterCreateSession = false;

	FDGCreateSessionRequest RequestData;
	RequestData.AccountId = AccountId;
	RequestData.CharacterId = CharacterId;
	RequestData.RegionId = RegionId;

	Debug::Print(TEXT("[DGSessionSubsystem] Request Create Session Only."));

	BackendClient->CreateSession(
		RequestData,
		FDGSessionApiResultCallback::CreateUObject(
			this,
			&UDGSessionSubsystem::HandleCreateSessionCompleted
		)
	);
}


void UDGSessionSubsystem::JoinSessionAndTravel(
	const FString& SessionId,
	int64 AccountId,
	int64 CharacterId
)
{
	if (!BackendClient)
	{
		Debug::Print(TEXT("[DGSessionSubsystem] BackendClient is null."));
		return;
	}

	if (SessionId.IsEmpty())
	{
		Debug::Print(TEXT("[DGSessionSubsystem] SessionId is empty."));
		return;
	}

	FDGJoinSessionRequest RequestData;
	RequestData.SessionId = SessionId;
	RequestData.AccountId = AccountId;
	RequestData.CharacterId = CharacterId;

	Debug::Print(FString::Printf(
		TEXT("[DGSessionSubsystem] Request Join Session. SessionId=%s"),
		*SessionId
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


void UDGSessionSubsystem::HandleCreateSessionCompleted(
	bool bSuccess,
	const FDGSessionConnectionInfo& Result
)
{
	if (!bSuccess)
	{
		Debug::Print(FString::Printf(
			TEXT("[DGSessionSubsystem] Create Session Failed. Error=%s"),
			*Result.ErrorMessage
		));

		OnSessionRequestFailed.Broadcast(Result.ErrorMessage);
		return;
	}

	LastSessionConnectionInfo = Result;

	Debug::Print(FString::Printf(
		TEXT("[DGSessionSubsystem] Create Session Success. SessionId=%s Server=%s:%d"),
		*Result.SessionId,
		*Result.ServerIP,
		Result.ServerPort
	));

	OnSessionCreated.Broadcast(Result.SessionId);

	if (bTravelAfterCreateSession)
	{
		TravelToDedicatedServer(Result);
	}

	bTravelAfterCreateSession = true;
}

void UDGSessionSubsystem::HandleJoinSessionCompleted(
	bool bSuccess,
	const FDGSessionConnectionInfo& Result
)
{
	if (!bSuccess)
	{
		Debug::Print(FString::Printf(
			TEXT("[DGSessionSubsystem] Join Session Failed. Error=%s"),
			*Result.ErrorMessage
		));

		OnSessionRequestFailed.Broadcast(Result.ErrorMessage);
		return;
	}

	LastSessionConnectionInfo = Result;

	Debug::Print(FString::Printf(
		TEXT("[DGSessionSubsystem] Join Session Success. SessionId=%s Server=%s:%d"),
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