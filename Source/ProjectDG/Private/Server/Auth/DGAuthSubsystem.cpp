#include "Server/Auth/DGAuthSubsystem.h"

#include "Core/DG_Debug.h"
#include "Server/Backend/DGBackendClient.h"

void UDGAuthSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	BackendClient = NewObject<UDGBackendClient>(this);
	BackendClient->Initialize(BackendBaseUrl);
}

void UDGAuthSubsystem::Deinitialize()
{
	BackendClient = nullptr;

	CurrentAccountId = 0;
	SelectedCharacterId = 0;
	CurrentLoginId.Empty();
	CurrentDisplayName.Empty();
	CachedCharacters.Empty();


	Super::Deinitialize();
}

void UDGAuthSubsystem::RegisterAccount(
	const FString& LoginId,
	const FString& Password,
	const FString& DisplayName
)
{
	if (!BackendClient)
	{
		const FString ErrorMessage = TEXT("[DGAuthSubsystem] BackendClient is null.");


		OnAuthFailed.Broadcast(ErrorMessage);
		return;
	}

	if (!ValidateLoginInput(LoginId, Password))
	{
		return;
	}

	FDGRegisterRequest RequestData;
	RequestData.LoginId = LoginId;
	RequestData.Password = Password;
	RequestData.DisplayName = DisplayName;


	BackendClient->RegisterAccount(
		RequestData,
		FDGAuthApiResultCallback::CreateUObject(
			this,
			&UDGAuthSubsystem::HandleRegisterCompleted
		)
	);
}

void UDGAuthSubsystem::Login(
	const FString& LoginId,
	const FString& Password
)
{
	if (!BackendClient)
	{
		const FString ErrorMessage = TEXT("[DGAuthSubsystem] BackendClient is null.");


		OnAuthFailed.Broadcast(ErrorMessage);
		return;
	}

	if (!ValidateLoginInput(LoginId, Password))
	{
		return;
	}

	FDGLoginRequest RequestData;
	RequestData.LoginId = LoginId;
	RequestData.Password = Password;


	BackendClient->Login(
		RequestData,
		FDGAuthApiResultCallback::CreateUObject(
			this,
			&UDGAuthSubsystem::HandleLoginCompleted
		)
	);
}

void UDGAuthSubsystem::RequestCharacterList()
{
	if (!BackendClient)
	{
		const FString ErrorMessage = TEXT("[DGAuthSubsystem] BackendClient is null.");


		OnCharacterListFailed.Broadcast(ErrorMessage);
		return;
	}

	if (CurrentAccountId <= 0)
	{
		const FString ErrorMessage = TEXT("[DGAuthSubsystem] CurrentAccountId is invalid.");


		OnCharacterListFailed.Broadcast(ErrorMessage);
		return;
	}


	BackendClient->GetCharacters(
		CurrentAccountId,
		FDGCharacterListApiResultCallback::CreateUObject(
			this,
			&UDGAuthSubsystem::HandleCharacterListCompleted
		)
	);
}

void UDGAuthSubsystem::CreateCharacter(
	int32 SlotIndex,
	const FString& CharacterName,
	const FString& ClassTag
)
{
	if (!BackendClient)
	{
		const FString ErrorMessage = TEXT("[DGAuthSubsystem] BackendClient is null.");

		OnCharacterCreateFailed.Broadcast(ErrorMessage);
		return;
	}

	if (CurrentAccountId <= 0)
	{
		const FString ErrorMessage = TEXT("[DGAuthSubsystem] CurrentAccountId is invalid.");

		OnCharacterCreateFailed.Broadcast(ErrorMessage);
		return;
	}

	if (SlotIndex < 0 || SlotIndex > 2)
	{
		const FString ErrorMessage = TEXT("Character slot index is invalid.");

		OnCharacterCreateFailed.Broadcast(ErrorMessage);
		return;
	}

	const FString TrimmedCharacterName = CharacterName.TrimStartAndEnd();

	if (TrimmedCharacterName.IsEmpty())
	{
		const FString ErrorMessage = TEXT("CharacterName is empty.");

		OnCharacterCreateFailed.Broadcast(ErrorMessage);
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
			&UDGAuthSubsystem::HandleCreateCharacterCompleted
		)
	);
}

bool UDGAuthSubsystem::SelectCharacterById(int64 CharacterId)
{
	if (CharacterId <= 0)
	{
		return false;
	}

	for (const FDGCharacterSummary& Character : CachedCharacters)
	{
		if (Character.bIsEmpty)
		{
			continue;
		}

		if (Character.CharacterId == CharacterId)
		{
			SelectedCharacterId = Character.CharacterId;

			OnCharacterSelected.Broadcast(Character);
			return true;
		}
	}

	return false;
}

bool UDGAuthSubsystem::SelectCharacterBySlotIndex(int32 SlotIndex)
{
	if (SlotIndex < 0 || SlotIndex > 2)
	{
		return false;
	}

	for (const FDGCharacterSummary& Character : CachedCharacters)
	{
		if (Character.SlotIndex != SlotIndex)
		{
			continue;
		}

		if (Character.bIsEmpty || Character.CharacterId <= 0)
		{
			return false;
		}

		return SelectCharacterById(Character.CharacterId);
	}

	return false;
}

bool UDGAuthSubsystem::IsLoggedIn() const
{
	return CurrentAccountId > 0;
}

bool UDGAuthSubsystem::HasSelectedCharacter() const
{
	return SelectedCharacterId > 0;
}

int64 UDGAuthSubsystem::GetCurrentAccountId() const
{
	return CurrentAccountId;
}

int64 UDGAuthSubsystem::GetSelectedCharacterId() const
{
	return SelectedCharacterId;
}

FString UDGAuthSubsystem::GetCurrentLoginId() const
{
	return CurrentLoginId;
}

FString UDGAuthSubsystem::GetCurrentDisplayName() const
{
	return CurrentDisplayName;
}

const TArray<FDGCharacterSummary>& UDGAuthSubsystem::GetCachedCharacters() const
{
	return CachedCharacters;
}

void UDGAuthSubsystem::HandleRegisterCompleted(
	bool bSuccess,
	const FDGAuthResult& Result
)
{
	if (!bSuccess)
	{
		const FString ErrorMessage = Result.ErrorMessage.IsEmpty()
			                             ? TEXT("Register failed.")
			                             : Result.ErrorMessage;


		OnAuthFailed.Broadcast(ErrorMessage);
		return;
	}

	CurrentAccountId = Result.AccountId;
	CurrentLoginId = Result.LoginId;
	CurrentDisplayName = Result.DisplayName;
	SelectedCharacterId = 0;
	CachedCharacters.Empty();


	OnRegisterSucceeded.Broadcast(Result);

	RequestCharacterList();
}

void UDGAuthSubsystem::HandleLoginCompleted(
	bool bSuccess,
	const FDGAuthResult& Result
)
{
	if (!bSuccess)
	{
		const FString ErrorMessage = Result.ErrorMessage.IsEmpty()
			                             ? TEXT("Login failed.")
			                             : Result.ErrorMessage;

		OnAuthFailed.Broadcast(ErrorMessage);
		return;
	}

	CurrentAccountId = Result.AccountId;
	CurrentLoginId = Result.LoginId;
	CurrentDisplayName = Result.DisplayName;
	SelectedCharacterId = 0;
	CachedCharacters.Empty();


	OnLoginSucceeded.Broadcast(Result);

	RequestCharacterList();
}

void UDGAuthSubsystem::HandleCharacterListCompleted(
	bool bSuccess,
	const FDGCharacterListResult& Result
)
{
	if (!bSuccess)
	{
		const FString ErrorMessage = Result.ErrorMessage.IsEmpty()
			                             ? TEXT("Character list request failed.")
			                             : Result.ErrorMessage;


		OnCharacterListFailed.Broadcast(ErrorMessage);
		return;
	}

	CachedCharacters = Result.Characters;


	OnCharacterListLoaded.Broadcast(Result);

	// if (CachedCharacters.Num() == 1)
	// {
	// 	SelectCharacterById(CachedCharacters[0].CharacterId);
	// }
}

void UDGAuthSubsystem::HandleCreateCharacterCompleted(
	bool bSuccess,
	const FDGCreateCharacterResult& Result
)
{
	if (!bSuccess)
	{
		const FString ErrorMessage = Result.ErrorMessage.IsEmpty()
										 ? TEXT("Character create request failed.")
										 : Result.ErrorMessage;

		OnCharacterCreateFailed.Broadcast(ErrorMessage);
		return;
	}

	OnCharacterCreated.Broadcast(Result);

	RequestCharacterList();
}


bool UDGAuthSubsystem::ValidateLoginInput(
	const FString& LoginId,
	const FString& Password
) const
{
	if (LoginId.TrimStartAndEnd().IsEmpty())
	{
		const FString ErrorMessage = TEXT("LoginId is empty.");


		OnAuthFailed.Broadcast(ErrorMessage);
		return false;
	}

	if (Password.IsEmpty())
	{
		const FString ErrorMessage = TEXT("Password is empty.");


		OnAuthFailed.Broadcast(ErrorMessage);
		return false;
	}

	return true;
}
