#pragma once

#include "CoreMinimal.h"
#include "GameFramework/DG_GameMode.h"
#include "TimerManager.h"
#include "UObject/ObjectKey.h"
#include "DGServerGameMode.generated.h"

class AController;
class APlayerController;

struct FDGConnectedMemberInfo
{
	FString SessionId;
	int64 AccountId = 0;
	int64 CharacterId = 0;
	FString Role;
};

UCLASS()
class PROJECTDG_API ADGServerGameMode : public ADG_GameMode
{
	GENERATED_BODY()

public:
	virtual void PreLogin(
		const FString& Options,
		const FString& Address,
		const FUniqueNetIdRepl& UniqueId,
		FString& ErrorMessage
	) override;

	virtual FString InitNewPlayer(
		APlayerController* NewPlayerController,
		const FUniqueNetIdRepl& UniqueId,
		const FString& Options,
		const FString& Portal
	) override;

	virtual void PostLogin(APlayerController* NewPlayer) override;

	virtual void Logout(AController* Exiting) override;

	virtual void RestartPlayer(AController* NewPlayer) override;

	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

private:
	UPROPERTY(EditDefaultsOnly, Category = "DG|Server")
	FString BackendBaseUrl = TEXT("http://localhost:8080");

	UPROPERTY(EditDefaultsOnly, Category = "DG|Server")
	float HeartbeatIntervalSeconds = 10.0f;

	FTimerHandle SessionHeartbeatTimerHandle;

	FString ActiveSessionId;

	int32 ConnectedPlayerCount = 0;

	bool bSessionEndReported = false;

	TMap<TObjectKey<AController>, FDGConnectedMemberInfo> ConnectedMemberInfos;

	void InitializeBackendBaseUrlFromCommandLine();

	void ValidateJoinTokenAsync(
		APlayerController* PlayerController,
		const FString& SessionId,
		const FString& JoinToken
	);

	void ReportSessionStartedAsync(
		const FString& SessionId
	);

	void ReportSessionEndedAsync(
		const FString& SessionId
	);

	void ReportMemberLeftAsync(
		const FDGConnectedMemberInfo& MemberInfo,
		bool bWasLastKnownPlayer
	);

	void StartSessionHeartbeat(
		const FString& SessionId
	);

	void StopSessionHeartbeat();

	void SendActiveSessionHeartbeat();

	void SendSessionHeartbeatAsync(
		const FString& SessionId
	);

	void TryReportSessionEndedIfNoPlayers();

	void KickPlayerWithReason(
		APlayerController* PlayerController,
		const FString& Reason
	);

	static FString BuildValidateJoinJson(
		const FString& SessionId,
		const FString& JoinToken
	);

	static FString BuildSessionStartedJson(
		const FString& SessionId
	);

	static FString BuildSessionEndedJson(
		const FString& SessionId
	);

	static FString BuildSessionHeartbeatJson(
		const FString& SessionId
	);

	static FString BuildMemberLeftJson(
		const FString& SessionId,
		int64 AccountId,
		int64 CharacterId
	);

	static bool ParseValidateJoinResponse(
		const FString& ResponseBody,
		FString& OutMessage,
		FString& OutSessionId,
		int64& OutAccountId,
		int64& OutCharacterId,
		FString& OutRole
	);

	static bool ParseMemberLeftResponse(
		const FString& ResponseBody,
		bool& bOutShouldShutdownServer,
		FString& OutMessage
	);
};