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
		return;
	}

	Debug::Print(FString::Printf(
		TEXT("[DGSessionSubsystem] Create Session Success. SessionId=%s Server=%s:%d"),
		*Result.SessionId,
		*Result.ServerIP,
		Result.ServerPort
	));

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
			TEXT("[DGSessionSubsystem] Join Session Failed. Error=%s"),
			*Result.ErrorMessage
		));
		return;
	}

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

	const FString TravelUrl = FString::Printf(
		TEXT("%s:%d"),
		*ConnectionInfo.ServerIP,
		ConnectionInfo.ServerPort
	);

	Debug::Print(FString::Printf(
		TEXT("[DGSessionSubsystem] ClientTravel To %s"),
		*TravelUrl
	));

	Debug::Print(FString::Printf(
	TEXT("[DGSessionSubsystem] ClientTravel To %s"),
	*TravelUrl
));
	
	PlayerController->ClientTravel(TravelUrl, TRAVEL_Absolute);
}