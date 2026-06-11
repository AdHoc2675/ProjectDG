#include "Components/Item/DGLootDropComponent.h"
#include "Item/DGLootItemActor.h"
#include "Item/DGItemInstance.h"
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
		UE_LOG(LogTemp, Warning, TEXT("[DGLootDropComponent] LootItemClass가 할당되지 않음"));
		return;
	}

	UWorld* World = GetWorld();
	if (!World) return;

	// 테이블을 순회하며 드롭 생성
	for (const FDGLootDropInfo& DropInfo : DropTable)
	{
		if (!DropInfo.DropItemInstance)
		{
			UE_LOG(LogTemp, Warning, TEXT("[DGLootDropComponent] DropItemInstance가 None으로 설정된 항목이 있음"));
			continue;
		}

		if (!DropInfo.DropItemInstance->ItemDef)
		{
			UE_LOG(LogTemp, Warning, TEXT("[DGLootDropComponent] DropItemInstance 내부의 ItemDef(원본 아이템 데이터)가 비어있음"));
			// 원본 데이터가 없으면 인벤토리에 넣을 수 없으므로 스킵하거나 에러 로그를 남김
		}

		float RandomVal = FMath::RandRange(0.f, 100.f);

		// 확률 당첨 시
		if (RandomVal <= DropInfo.DropChance)
		{
			// 수량은 인스턴스에 세팅된 수량을 따름
			int32 FinalQuantity = DropInfo.DropItemInstance->Quantity;

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
				// 여러 마리가 동일한 인스턴스를 공유하지 못하게 깊은 복사 진행
				UDGItemInstance* NewItemInstance = DuplicateObject<UDGItemInstance>(DropInfo.DropItemInstance, GetTransientPackage());
				if (!NewItemInstance)
				{
					UE_LOG(LogTemp, Error, TEXT("[DGLootDropComponent] DuplicateObject 실패, 인스턴스 복제를 할 수 없습니다."));
					continue;
				}

				LootActor->InitializeLoot(NewItemInstance);
				UE_LOG(LogTemp, Log, TEXT("[DGLootDropComponent] 아이템 드롭 성공: %s (등급: %d)"), *NewItemInstance->ItemDef->ItemName.ToString(), (int32)NewItemInstance->Grade);
			}
			else
			{
				UE_LOG(LogTemp, Error, TEXT("[DGLootDropComponent] SpawnActor 실패, 액터를 생성할 수 없습니다."));
			}
		}
	}
}