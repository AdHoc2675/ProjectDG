// Fill out your copyright notice in the Description page of Project Settings.


#include "Item/DGLootItemActor.h"
#include "Components/SphereComponent.h"
#include "NiagaraComponent.h"
#include "NiagaraSystem.h"
#include "Net/UnrealNetwork.h"
#include "Character/Player/PlayerCharacterBase.h"
#include "Components/Inventory/DGInventoryComponent.h"
#include "Item/DGItemDefinition.h"


// Sets default values
ADGLootItemActor::ADGLootItemActor()
{
    PrimaryActorTick.bCanEverTick = false;

    // 드롭 아이템은 서버 스폰 -> 클라이언트 복제
    bReplicates = true;

    // 충돌 영역 세팅
    PickupSphere = CreateDefaultSubobject<USphereComponent>(TEXT("PickupSphere"));
    RootComponent = PickupSphere;
    PickupSphere->SetSphereRadius(60.f);

    PickupSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    PickupSphere->SetCollisionResponseToAllChannels(ECR_Ignore);
    PickupSphere->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);

    // VFX 세팅
    LootVFXComponent = CreateDefaultSubobject<UNiagaraComponent>(TEXT("LootVFXComponent"));
    LootVFXComponent->SetupAttachment(RootComponent);
    LootVFXComponent->bAutoActivate = false;
}

// InitializeLoot 구현부 내에서 데이터 테이블을 읽고 이펙트 결정
void ADGLootItemActor::InitializeLoot(UDGItemDefinition* InItemDef, int32 InQuantity, EDGItemGrade InGrade)
{
    ItemDef = InItemDef;
    Quantity = InQuantity;
    ReplicatedGrade = InGrade;

    // 서버 측에서 즉시 VFX 활성화
    OnRep_Grade();
}

void ADGLootItemActor::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    // ReplicatedGrade 변수를 네트워크를 통해 복제하도록 등록
    DOREPLIFETIME(ADGLootItemActor, ReplicatedGrade);
}

// Called when the game starts or when spawned
void ADGLootItemActor::BeginPlay()
{
    if (HasAuthority())
    {
        PickupSphere->OnComponentBeginOverlap.AddDynamic(this, &ADGLootItemActor::OnSphereOverlap);
    }
}

void ADGLootItemActor::OnRep_Grade()
{
    UNiagaraSystem* TargetFX = nullptr;

    switch (ReplicatedGrade)
    {
    case EDGItemGrade::Hero:		TargetFX = HeroFX;		break;
    case EDGItemGrade::Legendary:	TargetFX = LegendaryFX;	break;
    case EDGItemGrade::Ancient:		TargetFX = AncientFX;	break;
    default:						TargetFX = NormalFX;	break;
    }

    if (TargetFX && LootVFXComponent)
    {
        LootVFXComponent->SetAsset(TargetFX);
        LootVFXComponent->Activate();
    }
}

void ADGLootItemActor::OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
    if (!HasAuthority() || !ItemDef) return;

    // 습득 주체가 플레이어인지 확인
    if (APlayerCharacterBase* Player = Cast<APlayerCharacterBase>(OtherActor))
    {
        // 플레이어의 인벤토리 컴포넌트 획득
        UDGInventoryComponent* Inventory = Player->FindComponentByClass<UDGInventoryComponent>();
        if (Inventory)
        {
            // TODO: 인벤토리에 아이템을 지급하는 로직을 InventoryComponent에 추가해야 함
            // 예: Inventory->AddItem(ItemDef, Quantity);

            UE_LOG(LogTemp, Log, TEXT("[DGLootItemActor] [%s]가 %s (x%d) 를 획득"), *Player->GetName(), *ItemDef->ItemName.ToString(), Quantity);

            // 획득 성공 후 필드 액터 삭제
            Destroy();
        }
    }
}

