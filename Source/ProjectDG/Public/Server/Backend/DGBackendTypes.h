#pragma once

#include "CoreMinimal.h"
#include "DGBackendTypes.generated.h"


//Backend API로 세션 생성을 요청할 때 사용할 데이터
//클라->백엔드

USTRUCT(BlueprintType)
struct PROJECTDG_API FDGCreateSessionRequest
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere,BlueprintReadWrite)
	int64 AccountId=1;
	
	UPROPERTY(EditAnywhere,BlueprintReadWrite)
	int64 CharacterId=1;
	
	UPROPERTY(EditAnywhere,BlueprintReadWrite)
	FString RegionId=TEXT("Region_Test");
	
};

//백엔드 API로 세션 합류를 요청할 때 사용하는 데이터
//클라 -> 백엔드 

USTRUCT(BlueprintType)
struct PROJECTDG_API FDGJoinSessionRequest
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere,BlueprintReadWrite)
	FString SessionId;
	
	UPROPERTY(EditAnywhere,BlueprintReadWrite)
	int64 AccountId=1;
	
	UPROPERTY(EditAnywhere,BlueprintReadWrite)
	int64 CharacterId=1;
	
};

//백엔드 API가 반환하는 접속정보
//백엔드 -> 클라

USTRUCT(BlueprintType)
struct  PROJECTDG_API FDGSessionConnectionInfo
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere,BlueprintReadWrite)
	bool bSuccess = false;
	
	UPROPERTY(EditAnywhere,BlueprintReadWrite)
	FString SessionId;
	
	UPROPERTY(EditAnywhere,BlueprintReadWrite)
	FString ServerIP;
	
	UPROPERTY(EditAnywhere,BlueprintReadWrite)
	int32 ServerPort=0;
	
	UPROPERTY(EditAnywhere,BlueprintReadWrite)
	FString MapPath;
	
	UPROPERTY(EditAnywhere,BlueprintReadWrite)
	FString JoinToken;
	
	UPROPERTY(EditAnywhere,BlueprintReadWrite)
	FString ErrorMessage;
	
};
