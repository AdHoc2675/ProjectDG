#pragma once

#include "CoreMinimal.h"
#include "DGBackendTypes.generated.h"

/**
 * Auth - 회원가입 요청
 */
USTRUCT(BlueprintType)
struct PROJECTDG_API FDGRegisterRequest
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString LoginId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString Password;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString DisplayName;
};

/**
 * Auth - 로그인 요청
 */
USTRUCT(BlueprintType)
struct PROJECTDG_API FDGLoginRequest
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString LoginId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString Password;
};

/**
 * Auth - 회원가입/로그인 결과
 */
USTRUCT(BlueprintType)
struct PROJECTDG_API FDGAuthResult
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bSuccess = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int64 AccountId = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString LoginId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString DisplayName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString Message;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString ErrorMessage;
};

/**
 * Character - 캐릭터 슬롯/요약 정보
 *
 * Backend 응답:
 * - slotIndex
 * - isEmpty
 * - characterId
 * - characterName
 * - classTag
 * - level
 */
USTRUCT(BlueprintType)
struct PROJECTDG_API FDGCharacterSummary
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 SlotIndex = -1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIsEmpty = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int64 CharacterId = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString CharacterName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString ClassTag;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 Level = 0;
};

/**
 * Character - 캐릭터 목록 결과
 */
USTRUCT(BlueprintType)
struct PROJECTDG_API FDGCharacterListResult
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bSuccess = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int64 AccountId = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FDGCharacterSummary> Characters;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString Message;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString ErrorMessage;
};

/**
 * Character - 캐릭터 생성 요청
 */
USTRUCT(BlueprintType)
struct PROJECTDG_API FDGCreateCharacterRequest
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 SlotIndex = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString CharacterName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString ClassTag = TEXT("Character.Class.Warrior");
};

/**
 * Character - 캐릭터 생성 결과
 */
USTRUCT(BlueprintType)
struct PROJECTDG_API FDGCreateCharacterResult
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bSuccess = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int64 AccountId = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FDGCharacterSummary Character;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString Message;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString ErrorMessage;
};

/**
 * Backend API로 방/세션 생성을 요청할 때 사용할 데이터
 *
 * 클라 -> 백엔드
 *
 * 유저 입력:
 * - RoomName
 * - RoomPassword
 *
 * 내부 처리:
 * - AccountId / CharacterId는 로그인/캐릭터 선택 결과를 사용
 * - Backend가 SessionId 생성
 * - Backend가 JoinToken 생성
 * - RoomPassword는 Hash로 저장
 */
USTRUCT(BlueprintType)
struct PROJECTDG_API FDGCreateSessionRequest
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int64 AccountId = 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int64 CharacterId = 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString RegionId = TEXT("Region_Test");

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString RoomName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString RoomPassword;
};

/**
 * Backend API로 방/세션 합류를 요청할 때 사용할 데이터
 *
 * 클라 -> 백엔드
 *
 * 유저 입력:
 * - RoomName
 * - RoomPassword
 *
 * 내부 처리:
 * - RoomName으로 세션 검색
 * - RoomPassword Hash 검증
 * - AccountId / CharacterId 검증
 * - 참가자용 JoinToken 발급
 */
USTRUCT(BlueprintType)
struct PROJECTDG_API FDGJoinSessionRequest
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString RoomName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString RoomPassword;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int64 AccountId = 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int64 CharacterId = 1;
};

/**
 * Backend API가 반환하는 접속 정보
 *
 * 백엔드 -> 클라
 *
 * 클라는 이 값을 직접 유저에게 보여주지 않고,
 * 내부적으로 Dedicated Server ClientTravel에 사용한다.
 */
USTRUCT(BlueprintType)
struct PROJECTDG_API FDGSessionConnectionInfo
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bSuccess = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString SessionId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString ServerIP;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 ServerPort = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString MapPath;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString JoinToken;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString ErrorMessage;
};