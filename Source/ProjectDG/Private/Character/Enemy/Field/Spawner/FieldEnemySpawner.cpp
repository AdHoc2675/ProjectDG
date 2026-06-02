// Fill out your copyright notice in the Description page of Project Settings.

#include "Character/Enemy/Field/Spawner/FieldEnemySpawner.h"
#include "Character/Enemy/Field/Data/FieldCharacterClassData.h"
#include "Character/Enemy/Field/FieldEnemyBase.h"
#include "System/FieldEnemyPoolSubsystem.h"
#include "GameFramework/PlayerState.h"
#include "GameFramework/GameStateBase.h"
#include "Kismet/GameplayStatics.h"
#include "NavigationSystem.h"

AFieldEnemySpawner::AFieldEnemySpawner()
{
	PrimaryActorTick.bCanEverTick = false;
	
	// 에디터에서 영역을 시각적으로 확인하기 위해 Sphere 컴포넌트를 루트로 설정 가능
	// 여기서는 단순히 Actor 로케이션을 사용합니다.
}

void AFieldEnemySpawner::BeginPlay()
{
	Super::BeginPlay();

	if (HasAuthority() && EnemyDataToSpawn)
	{
		int32 TargetCount = CalculateTargetSpawnCount();
		for (int32 i = 0; i < TargetCount; ++i)
		{
			TrySpawnEnemy();
		}

		// 지속적인 리스폰 및 풀 복귀 체크를 위해 주기적 타이머 시작
		GetWorld()->GetTimerManager().SetTimer(RespawnTimerHandle, this, &AFieldEnemySpawner::HandleRespawnTimer, RespawnTime, false);
	}
}

void AFieldEnemySpawner::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (HasAuthority())
	{
		GetWorld()->GetTimerManager().ClearTimer(RespawnTimerHandle);
	}
	Super::EndPlay(EndPlayReason);
}

int32 AFieldEnemySpawner::CalculateTargetSpawnCount() const
{
	if (UWorld* World = GetWorld())
	{
		if (AGameStateBase* GameState = World->GetGameState())
		{
			int32 NumPlayers = GameState->PlayerArray.Num();
			// 플레이어가 0명이더라도 최소 1명으로 간주하여 기본 마릿수 확보
			NumPlayers = FMath::Max(1, NumPlayers);
			
			return BaseSpawnCount + (NumPlayers - 1) * ExtraSpawnPerPlayer;
		}
	}
	return BaseSpawnCount;
}

FVector AFieldEnemySpawner::GetRandomSpawnLocation() const
{
	FVector Origin = GetActorLocation();
	UNavigationSystemV1* NavSys = UNavigationSystemV1::GetCurrent(GetWorld());
	if (NavSys)
	{
		FNavLocation RandomPoint;
		if (NavSys->GetRandomReachablePointInRadius(Origin, SpawnRadius, RandomPoint))
		{
			return RandomPoint.Location;
		}
	}
	
	// 네비메쉬가 없거나 실패 시 임의 2D 반경 내 생성 (높이는 스포너 기준)
	FVector RandomOffset = FMath::VRand();
	RandomOffset.Z = 0.f;
	RandomOffset.Normalize();
	return Origin + RandomOffset * FMath::FRandRange(0.f, SpawnRadius);
}

bool AFieldEnemySpawner::IsLocationValidForSpawn(const FVector& SpawnLoc) const
{
	if (UWorld* World = GetWorld())
	{
		APlayerController* FirstPC = World->GetFirstPlayerController();
		if (FirstPC)
		{
			// 간단하게 모든 로컬/네트워크 폰에 대해 거리 검사
			// 실제 멀티에서는 모든 PlayerPawn을 순회하는 것이 안전합니다.
			TArray<AActor*> PlayerPawns;
			UGameplayStatics::GetAllActorsOfClass(World, APawn::StaticClass(), PlayerPawns);
			
			for (AActor* PawnActor : PlayerPawns)
			{
				if (APawn* Pawn = Cast<APawn>(PawnActor))
				{
					if (Pawn->IsPlayerControlled())
					{
						float DistSq = FVector::DistSquared(Pawn->GetActorLocation(), SpawnLoc);
						if (DistSq < FMath::Square(MinSpawnDistanceFromPlayer))
						{
							return false; // 플레이어가 너무 가까움
						}
					}
				}
			}
		}
	}
	return true;
}

void AFieldEnemySpawner::TrySpawnEnemy()
{
	if (!EnemyDataToSpawn || !EnemyDataToSpawn->EnemyClass) return;

	UWorld* World = GetWorld();
	if (!World) return;

	UFieldEnemyPoolSubsystem* PoolSubsystem = World->GetSubsystem<UFieldEnemyPoolSubsystem>();
	if (!PoolSubsystem) return;

	FVector SpawnLoc = GetActorLocation();
	bool bFoundValidLocation = false;

	// 최대 5번 시도하여 유효한 위치 찾기
	for (int32 i = 0; i < 5; ++i)
	{
		FVector TestLoc = GetRandomSpawnLocation();
		if (IsLocationValidForSpawn(TestLoc))
		{
			SpawnLoc = TestLoc;
			bFoundValidLocation = true;
			break;
		}
	}

	// 위치를 찾지 못했더라도 그냥 스폰 (또는 여기서 스폰을 건너뛰고 나중에 다시 시도 가능)
	// 본 구현에서는 시도 후에도 실패하면 스포너 위치에 스폰하도록 처리
	
	AFieldEnemyBase* SpawnedEnemy = PoolSubsystem->AcquireEnemy(EnemyDataToSpawn->EnemyClass, SpawnLoc, FRotator::ZeroRotator);
	
	if (SpawnedEnemy)
	{
		// 스포너가 가진 DataAsset으로 몬스터 초기화
		SpawnedEnemy->InitFromDataAsset(EnemyDataToSpawn);
		
		// 죽을 때 알림을 받기 위해 바인딩 (OnDestroyed 대신 체력 0 이벤트를 받을 수도 있으나, 여기선 풀 반환 델리게이트 필요)
		// 풀 시스템에서 반환될 때 이벤트를 쏴주는 델리게이트가 있다면 좋으나, 
		// 간단하게 하기 위해 AActor::OnEndPlay 나 커스텀 델리게이트를 사용할 수 있습니다.
		// 본 예제에서는 단순히 커스텀 OnEnemyReturnedToPool 이벤트를 만들거나, 타이머 루프를 돌릴 수 있음.
		
		ActiveEnemies.Add(SpawnedEnemy);
		
		// Note: AFieldEnemyBase 쪽에 FOnEnemyReturnedToPool 델리게이트를 추가하고 여기서 바인딩해야 합니다.
		// 임시로 단순히 배열 관리만 하고 리스폰은 일정 주기로 폴링 체크하여 처리하도록 구현
	}
}

void AFieldEnemySpawner::HandleRespawnTimer()
{
	if (!HasAuthority()) return;

	// 죽어서 풀로 돌아간 몬스터(비활성화 상태)를 ActiveEnemies에서 정리
	ActiveEnemies.RemoveAll([](AFieldEnemyBase* Enemy) {
		return !IsValid(Enemy) || Enemy->IsHidden();
	});

	int32 TargetCount = CalculateTargetSpawnCount();
	int32 CurrentCount = ActiveEnemies.Num();

	if (CurrentCount < TargetCount)
	{
		int32 Needed = TargetCount - CurrentCount;
		for (int32 i = 0; i < Needed; ++i)
		{
			TrySpawnEnemy();
		}
	}
	
	// 계속 주기적으로 체크하여 리스폰 관리
	GetWorld()->GetTimerManager().SetTimer(RespawnTimerHandle, this, &AFieldEnemySpawner::HandleRespawnTimer, RespawnTime, false);
}

void AFieldEnemySpawner::OnEnemyDied(AActor* DestroyedActor)
{
	// 델리게이트 연동 시 이 함수 호출
}
