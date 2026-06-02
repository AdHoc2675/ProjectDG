#include "Components/Item/DGLootDropComponent.h"
#include "Item/DGLootItemActor.h"
#include "Item/DGItemDefinition.h"

UDGLootDropComponent::UDGLootDropComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UDGLootDropComponent::ProcessDrop(const FVector& DropLocation)
{
	// 드롭 생성은 오직 서버에서만 처리
	if (!GetOwner()->HasAuthority()) return;

	if (!LootItemClass)
	{
		UE_LOG(LogTemp, Warning, TEXT("LootDropComponent: LootItemClass가 할당되지 않았습니다."));
		return;
	}

	UWorld* World = GetWorld();
	if (!World) return;

	// 테이블을 순회하며 드롭 생성
	for (const FDGLootDropInfo& DropInfo : DropTable)
	{
		if (!DropInfo.ItemDef) continue;

		float RandomVal = FMath::RandRange(0.f, 100.f);

		// 확률 당첨 시
		if (RandomVal <= DropInfo.DropChance)
		{
			// 수량 결정
			int32 FinalQuantity = FMath::RandRange(DropInfo.MinQuantity, DropInfo.MaxQuantity);

			if (FinalQuantity <= 0) continue;

			// 방사형으로 아이템이 조금씩 흩뿌려지도록 위치 분산
			FVector SpawnLoc = DropLocation;
			SpawnLoc.X += FMath::RandRange(-60.f, 60.f);
			SpawnLoc.Y += FMath::RandRange(-60.f, 60.f);
			SpawnLoc.Z += 20.f;

			FActorSpawnParameters SpawnParams;
			SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

			// 필드 아이템 액터 스폰
			ADGLootItemActor* LootActor = World->SpawnActor<ADGLootItemActor>(LootItemClass, SpawnLoc, FRotator::ZeroRotator, SpawnParams);

			if (LootActor)
			{
				LootActor->InitializeLoot(DropInfo.ItemDef, FinalQuantity, DropInfo.Grade);
			}
		}
	}
}