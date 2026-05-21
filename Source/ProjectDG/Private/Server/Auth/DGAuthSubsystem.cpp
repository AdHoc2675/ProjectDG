#include "Server/Auth/DGAuthSubsystem.h"

#include "Core/DG_Debug.h"
#include "Server/Backend/DGBackendClient.h"

void UDGAuthSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	BackendClient = NewObject<UDGBackendClient>(this);
	BackendClient->Initialize(BackendBaseUrl);

	Debug::Print(FString::Printf(
		TEXT("[DGAuthSubsystem] Initialized. BackendBaseUrl=%s"),
		*BackendBaseUrl
	));
}

void UDGAuthSubsystem::Deinitialize()
{
	BackendClient = nullptr;

	CurrentAccountId = 0;
	SelectedCharacterId = 0;
	CurrentLoginId.Empty();
	CurrentDisplayName.Empty();
	CachedCharacters.Empty();

	Debug::Print(TEXT("[DGAuthSubsystem] Deinitialized."));

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

		Debug::Print(ErrorMessage);
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

	Debug::Print(FString::Printf(
		TEXT("[DGAuthSubsystem] Request Register. LoginId=%s"),
		*LoginId
	));

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

		Debug::Print(ErrorMessage);
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

	Debug::Print(FString::Printf(
		TEXT("[DGAuthSubsystem] Request Login. LoginId=%s"),
		*LoginId
	));

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

		Debug::Print(ErrorMessage);
		OnCharacterListFailed.Broadcast(ErrorMessage);
		return;
	}

	if (CurrentAccountId <= 0)
	{
		const FString ErrorMessage = TEXT("[DGAuthSubsystem] CurrentAccountId is invalid.");

		Debug::Print(ErrorMessage);
		OnCharacterListFailed.Broadcast(ErrorMessage);
		return;
	}

	Debug::Print(FString::Printf(
		TEXT("[DGAuthSubsystem] Request Character List. AccountId=%lld"),
		CurrentAccountId
	));

	BackendClient->GetCharacters(
		CurrentAccountId,
		FDGCharacterListApiResultCallback::CreateUObject(
			this,
			&UDGAuthSubsystem::HandleCharacterListCompleted
		)
	);
}

bool UDGAuthSubsystem::SelectCharacterById(int64 CharacterId)
{
	if (CharacterId <= 0)
	{
		Debug::Print(TEXT("[DGAuthSubsystem] SelectCharacterById failed. CharacterId is invalid."));
		return false;
	}

	for (const FDGCharacterSummary& Character : CachedCharacters)
	{
		if (Character.CharacterId == CharacterId)
		{
			SelectedCharacterId = Character.CharacterId;

			Debug::Print(FString::Printf(
				TEXT("[DGAuthSubsystem] Character Selected. CharacterId=%lld Name=%s Class=%s Level=%d"),
				SelectedCharacterId,
				*Character.CharacterName,
				*Character.ClassTag,
				Character.Level
			));

			OnCharacterSelected.Broadcast(Character);
			return true;
		}
	}

	Debug::Print(FString::Printf(
		TEXT("[DGAuthSubsystem] SelectCharacterById failed. Character not found. CharacterId=%lld"),
		CharacterId
	));

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

		Debug::Print(FString::Printf(
			TEXT("[DGAuthSubsystem] Register Failed. Error=%s"),
			*ErrorMessage
		));

		OnAuthFailed.Broadcast(ErrorMessage);
		return;
	}

	CurrentAccountId = Result.AccountId;
	CurrentLoginId = Result.LoginId;
	CurrentDisplayName = Result.DisplayName;
	SelectedCharacterId = 0;
	CachedCharacters.Empty();

	Debug::Print(FString::Printf(
		TEXT("[DGAuthSubsystem] Register Success. AccountId=%lld LoginId=%s DisplayName=%s"),
		CurrentAccountId,
		*CurrentLoginId,
		*CurrentDisplayName
	));

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

		Debug::Print(FString::Printf(
			TEXT("[DGAuthSubsystem] Login Failed. Error=%s"),
			*ErrorMessage
		));

		OnAuthFailed.Broadcast(ErrorMessage);
		return;
	}

	CurrentAccountId = Result.AccountId;
	CurrentLoginId = Result.LoginId;
	CurrentDisplayName = Result.DisplayName;
	SelectedCharacterId = 0;
	CachedCharacters.Empty();

	Debug::Print(FString::Printf(
		TEXT("[DGAuthSubsystem] Login Success. AccountId=%lld LoginId=%s DisplayName=%s"),
		CurrentAccountId,
		*CurrentLoginId,
		*CurrentDisplayName
	));

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

		Debug::Print(FString::Printf(
			TEXT("[DGAuthSubsystem] Character List Failed. Error=%s"),
			*ErrorMessage
		));

		OnCharacterListFailed.Broadcast(ErrorMessage);
		return;
	}

	CachedCharacters = Result.Characters;

	Debug::Print(FString::Printf(
		TEXT("[DGAuthSubsystem] Character List Loaded. Count=%d"),
		CachedCharacters.Num()
	));

	OnCharacterListLoaded.Broadcast(Result);

	if (CachedCharacters.Num() == 1)
	{
		SelectCharacterById(CachedCharacters[0].CharacterId);
	}
}

bool UDGAuthSubsystem::ValidateLoginInput(
	const FString& LoginId,
	const FString& Password
) const
{
	if (LoginId.TrimStartAndEnd().IsEmpty())
	{
		const FString ErrorMessage = TEXT("LoginId is empty.");

		Debug::Print(FString::Printf(
			TEXT("[DGAuthSubsystem] %s"),
			*ErrorMessage
		));

		OnAuthFailed.Broadcast(ErrorMessage);
		return false;
	}

	if (Password.IsEmpty())
	{
		const FString ErrorMessage = TEXT("Password is empty.");

		Debug::Print(FString::Printf(
			TEXT("[DGAuthSubsystem] %s"),
			*ErrorMessage
		));

		OnAuthFailed.Broadcast(ErrorMessage);
		return false;
	}

	return true;
}