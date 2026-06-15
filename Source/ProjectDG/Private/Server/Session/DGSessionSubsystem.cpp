#include "Server/Session/DGSessionSubsystem.h"

#include "Server/Backend/DGBackendClient.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "Misc/CommandLine.h"
#include "Misc/Parse.h"
#include "Engine/GameInstance.h"

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
	LastSessionConnectionInfo = FDGSessionConnectionInfo();

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

	if (!ValidateAccountCharacterInput(AccountId, CharacterId))
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

	if (!ValidateAccountCharacterInput(AccountId, CharacterId))
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
		const FString ErrorMessage = TEXT("Last session connection info is not valid.");
		OnSessionRequestFailed.Broadcast(ErrorMessage);
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
		const FString ErrorMessage = Result.ErrorMessage.IsEmpty()
			                             ? TEXT("Create session failed.")
			                             : Result.ErrorMessage;

		OnSessionRequestFailed.Broadcast(ErrorMessage);
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
		const FString ErrorMessage = Result.ErrorMessage.IsEmpty()
			                             ? TEXT("Join session failed.")
			                             : Result.ErrorMessage;

		OnSessionRequestFailed.Broadcast(ErrorMessage);
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
		const FString ErrorMessage = TEXT("[DGSessionSubsystem] World is null.");
		OnSessionRequestFailed.Broadcast(ErrorMessage);
		return;
	}

	APlayerController* PlayerController = World->GetFirstPlayerController();

	if (!PlayerController)
	{
		const FString ErrorMessage = TEXT("[DGSessionSubsystem] PlayerController is null.");
		OnSessionRequestFailed.Broadcast(ErrorMessage);
		return;
	}

	if (ConnectionInfo.ServerIP.IsEmpty())
	{
		const FString ErrorMessage = TEXT("ServerIP is empty.");
		OnSessionRequestFailed.Broadcast(ErrorMessage);
		return;
	}

	if (ConnectionInfo.ServerPort <= 0)
	{
		const FString ErrorMessage = TEXT("ServerPort is invalid.");
		OnSessionRequestFailed.Broadcast(ErrorMessage);
		return;
	}

	if (ConnectionInfo.SessionId.IsEmpty())
	{
		const FString ErrorMessage = TEXT("SessionId is empty.");
		OnSessionRequestFailed.Broadcast(ErrorMessage);
		return;
	}

	if (ConnectionInfo.JoinToken.IsEmpty())
	{
		const FString ErrorMessage = TEXT("JoinToken is empty.");
		OnSessionRequestFailed.Broadcast(ErrorMessage);
		return;
	}

	const FString TravelUrl = FString::Printf(
		TEXT("%s:%d?SessionId=%s?JoinToken=%s"),
		*ConnectionInfo.ServerIP,
		ConnectionInfo.ServerPort,
		*ConnectionInfo.SessionId,
		*ConnectionInfo.JoinToken
	);

	// 로딩 화면은 OnPreLoadMap 델리게이트에서 자동으로 표시됨
	PlayerController->ClientTravel(TravelUrl, TRAVEL_Absolute);
}

bool UDGSessionSubsystem::ValidateRoomInput(
	const FString& RoomName,
	const FString& RoomPassword
)
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

bool UDGSessionSubsystem::ValidateAccountCharacterInput(
	int64 AccountId,
	int64 CharacterId
)
{
	if (AccountId <= 0)
	{
		const FString ErrorMessage = TEXT("AccountId is invalid.");
		OnSessionRequestFailed.Broadcast(ErrorMessage);
		return false;
	}

	if (CharacterId <= 0)
	{
		const FString ErrorMessage = TEXT("CharacterId is invalid.");
		OnSessionRequestFailed.Broadcast(ErrorMessage);
		return false;
	}

	return true;
}