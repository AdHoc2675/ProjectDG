#include "Server/Session/DGSessionSubsystem.h"

#include "Server/Backend/DGBackendClient.h"
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

void UDGSessionSubsystem::Login(
	const FString& LoginId,
	const FString& Password
)
{
	if (!BackendClient)
	{
		const FString ErrorMessage = TEXT("[DGSessionSubsystem] BackendClient is null.");
		OnSessionRequestFailed.Broadcast(ErrorMessage);
		return;
	}

	const FString TrimmedLoginId = LoginId.TrimStartAndEnd();

	if (TrimmedLoginId.IsEmpty())
	{
		const FString ErrorMessage = TEXT("LoginId is empty.");
		OnSessionRequestFailed.Broadcast(ErrorMessage);
		return;
	}

	if (Password.IsEmpty())
	{
		const FString ErrorMessage = TEXT("Password is empty.");
		OnSessionRequestFailed.Broadcast(ErrorMessage);
		return;
	}

	FDGLoginRequest RequestData;
	RequestData.LoginId = TrimmedLoginId;
	RequestData.Password = Password;

	BackendClient->Login(
		RequestData,
		FDGAuthApiResultCallback::CreateUObject(
			this,
			&UDGSessionSubsystem::HandleLoginCompleted
		)
	);
}

void UDGSessionSubsystem::LoadMyCharacters()
{
	if (!BackendClient)
	{
		const FString ErrorMessage = TEXT("[DGSessionSubsystem] BackendClient is null.");
		OnSessionRequestFailed.Broadcast(ErrorMessage);
		return;
	}

	if (!ValidateLoggedIn())
	{
		return;
	}

	BackendClient->GetCharacters(
		CurrentAccountId,
		FDGCharacterListApiResultCallback::CreateUObject(
			this,
			&UDGSessionSubsystem::HandleLoadCharactersCompleted
		)
	);
}

void UDGSessionSubsystem::CreateCharacter(
	int32 SlotIndex,
	const FString& CharacterName,
	const FString& ClassTag
)
{
	if (!BackendClient)
	{
		const FString ErrorMessage = TEXT("[DGSessionSubsystem] BackendClient is null.");
		OnSessionRequestFailed.Broadcast(ErrorMessage);
		return;
	}

	if (!ValidateLoggedIn())
	{
		return;
	}

	if (SlotIndex < 0 || SlotIndex > 2)
	{
		const FString ErrorMessage = TEXT("SlotIndex must be between 0 and 2.");
		OnSessionRequestFailed.Broadcast(ErrorMessage);
		return;
	}

	const FString TrimmedCharacterName = CharacterName.TrimStartAndEnd();

	if (TrimmedCharacterName.IsEmpty())
	{
		const FString ErrorMessage = TEXT("CharacterName is empty.");
		OnSessionRequestFailed.Broadcast(ErrorMessage);
		return;
	}

	FDGCreateCharacterRequest RequestData;
	RequestData.SlotIndex = SlotIndex;
	RequestData.CharacterName = TrimmedCharacterName;
	RequestData.ClassTag = ClassTag.IsEmpty()
		? TEXT("Character.Class.Warrior")
		: ClassTag;

	BackendClient->CreateCharacter(
		CurrentAccountId,
		RequestData,
		FDGCreateCharacterApiResultCallback::CreateUObject(
			this,
			&UDGSessionSubsystem::HandleCreateCharacterCompleted
		)
	);
}

void UDGSessionSubsystem::SelectCharacterById(int64 CharacterId)
{
	if (CharacterId <= 0)
	{
		const FString ErrorMessage = TEXT("CharacterId must be greater than 0.");
		OnSessionRequestFailed.Broadcast(ErrorMessage);
		return;
	}

	const FDGCharacterSummary* FoundCharacter = CachedCharacterSlots.FindByPredicate(
		[CharacterId](const FDGCharacterSummary& Character)
		{
			return !Character.bIsEmpty && Character.CharacterId == CharacterId;
		}
	);

	if (!FoundCharacter)
	{
		const FString ErrorMessage = TEXT("Character not found in cached character slots.");
		OnSessionRequestFailed.Broadcast(ErrorMessage);
		return;
	}

	SelectedCharacterId = FoundCharacter->CharacterId;

	OnCharacterSelected.Broadcast(
		SelectedCharacterId,
		FoundCharacter->SlotIndex
	);
}

void UDGSessionSubsystem::SelectCharacterBySlotIndex(int32 SlotIndex)
{
	const FDGCharacterSummary* FoundCharacter = CachedCharacterSlots.FindByPredicate(
		[SlotIndex](const FDGCharacterSummary& Character)
		{
			return !Character.bIsEmpty && Character.SlotIndex == SlotIndex;
		}
	);

	if (!FoundCharacter)
	{
		const FString ErrorMessage = TEXT("Character slot is empty or not found.");
		OnSessionRequestFailed.Broadcast(ErrorMessage);
		return;
	}

	SelectedCharacterId = FoundCharacter->CharacterId;

	OnCharacterSelected.Broadcast(
		SelectedCharacterId,
		FoundCharacter->SlotIndex
	);
}

bool UDGSessionSubsystem::IsLoggedIn() const
{
	return CurrentAccountId > 0;
}

bool UDGSessionSubsystem::HasSelectedCharacter() const
{
	return SelectedCharacterId > 0;
}

int64 UDGSessionSubsystem::GetCurrentAccountId() const
{
	return CurrentAccountId;
}

int64 UDGSessionSubsystem::GetSelectedCharacterId() const
{
	return SelectedCharacterId;
}

FString UDGSessionSubsystem::GetCurrentDisplayName() const
{
	return CurrentDisplayName;
}

TArray<FDGCharacterSummary> UDGSessionSubsystem::GetCachedCharacterSlots() const
{
	return CachedCharacterSlots;
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

void UDGSessionSubsystem::CreateRoomAndTravelWithSelectedCharacter(
	const FString& RoomName,
	const FString& RoomPassword,
	const FString& RegionId
)
{
	if (!ValidateLoggedIn())
	{
		return;
	}

	if (!ValidateSelectedCharacter())
	{
		return;
	}

	CreateRoomAndTravel(
		RoomName,
		RoomPassword,
		CurrentAccountId,
		SelectedCharacterId,
		RegionId
	);
}

void UDGSessionSubsystem::JoinRoomAndTravelWithSelectedCharacter(
	const FString& RoomName,
	const FString& RoomPassword
)
{
	if (!ValidateLoggedIn())
	{
		return;
	}

	if (!ValidateSelectedCharacter())
	{
		return;
	}

	JoinRoomAndTravel(
		RoomName,
		RoomPassword,
		CurrentAccountId,
		SelectedCharacterId
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

void UDGSessionSubsystem::HandleLoginCompleted(
	bool bSuccess,
	const FDGAuthResult& Result
)
{
	if (!bSuccess)
	{
		OnSessionRequestFailed.Broadcast(Result.ErrorMessage);
		return;
	}

	CurrentAccountId = Result.AccountId;
	CurrentLoginId = Result.LoginId;
	CurrentDisplayName = Result.DisplayName;
	SelectedCharacterId = 0;
	CachedCharacterSlots.Reset();

	OnLoginSucceeded.Broadcast(Result);

	LoadMyCharacters();
}

void UDGSessionSubsystem::HandleLoadCharactersCompleted(
	bool bSuccess,
	const FDGCharacterListResult& Result
)
{
	if (!bSuccess)
	{
		OnSessionRequestFailed.Broadcast(Result.ErrorMessage);
		return;
	}

	CachedCharacterSlots = Result.Characters;

	OnCharacterListLoaded.Broadcast(Result);
}

void UDGSessionSubsystem::HandleCreateCharacterCompleted(
	bool bSuccess,
	const FDGCreateCharacterResult& Result
)
{
	if (!bSuccess)
	{
		OnSessionRequestFailed.Broadcast(Result.ErrorMessage);
		return;
	}

	CachedCharacterSlots.RemoveAll(
		[&Result](const FDGCharacterSummary& Character)
		{
			return Character.SlotIndex == Result.Character.SlotIndex;
		}
	);

	CachedCharacterSlots.Add(Result.Character);

	CachedCharacterSlots.Sort(
		[](const FDGCharacterSummary& A, const FDGCharacterSummary& B)
		{
			return A.SlotIndex < B.SlotIndex;
		}
	);

	OnCharacterCreated.Broadcast(Result);

	LoadMyCharacters();
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

bool UDGSessionSubsystem::ValidateLoggedIn()
{
	if (CurrentAccountId <= 0)
	{
		const FString ErrorMessage = TEXT("Not logged in.");
		OnSessionRequestFailed.Broadcast(ErrorMessage);
		return false;
	}

	return true;
}

bool UDGSessionSubsystem::ValidateSelectedCharacter()
{
	if (SelectedCharacterId <= 0)
	{
		const FString ErrorMessage = TEXT("Character is not selected.");
		OnSessionRequestFailed.Broadcast(ErrorMessage);
		return false;
	}

	return true;
}