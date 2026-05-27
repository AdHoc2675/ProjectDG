#include "Actor/AttackHitboxActor.h"
#include "Components/SphereComponent.h"
#include "Particles/ParticleSystemComponent.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemBlueprintLibrary.h"

AAttackHitboxActor::AAttackHitboxActor()
{
	PrimaryActorTick.bCanEverTick = false;

	HitboxCollision = CreateDefaultSubobject<USphereComponent>(TEXT("HitboxCollision"));
	RootComponent = HitboxCollision;
	HitboxCollision->SetSphereRadius(100.f);
	// 충돌 설정 (Pawn 등에게만 겹치도록 설정)
	HitboxCollision->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	HitboxCollision->SetCollisionResponseToAllChannels(ECR_Ignore);
	HitboxCollision->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);

	EffectComponent = CreateDefaultSubobject<UParticleSystemComponent>(TEXT("EffectComponent"));
	EffectComponent->SetupAttachment(RootComponent);

	DamageLevel = 1.0f;
}

void AAttackHitboxActor::BeginPlay()
{
	Super::BeginPlay();
	
	if (HasAuthority())
	{
		HitboxCollision->OnComponentBeginOverlap.AddDynamic(this, &AAttackHitboxActor::OnHitboxOverlap);
		if (LifeSpanTime > 0.f)
		{
			SetLifeSpan(LifeSpanTime);
		}
	}
}

void AAttackHitboxActor::InitializeHitbox(UAbilitySystemComponent* InInstigatorASC, TSubclassOf<UGameplayEffect> InDamageEffectClass, float InDamageLevel)
{
	InstigatorASC = InInstigatorASC;
	DamageEffectClass = InDamageEffectClass;
	DamageLevel = InDamageLevel;
}

void AAttackHitboxActor::OnHitboxOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (!HasAuthority() || !InstigatorASC || !DamageEffectClass || !OtherActor)
		return;

	// 자신 또는 자신을 소환한 액터 무시
	if (bIgnoreInstigator)
	{
		AActor* MyInstigator = InstigatorASC->GetOwnerActor();
		if (OtherActor == MyInstigator || OtherActor == this)
		{
			return;
		}
	}

	// 이미 타격한 대상인지 확인 (다단 히트를 원한다면 Set 대신 타이머 등을 사용해야 함)
	if (HitActors.Contains(OtherActor))
	{
		return;
	}

	UAbilitySystemComponent* TargetASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(OtherActor);
	if (TargetASC)
	{
		// 이펙트 생성 시 컨텍스트 설정
		FGameplayEffectContextHandle ContextHandle = InstigatorASC->MakeEffectContext();
		ContextHandle.AddSourceObject(this);
		ContextHandle.AddHitResult(SweepResult);

		FGameplayEffectSpecHandle SpecHandle = InstigatorASC->MakeOutgoingSpec(DamageEffectClass, DamageLevel, ContextHandle);
		if (SpecHandle.IsValid())
		{
			InstigatorASC->ApplyGameplayEffectSpecToTarget(*SpecHandle.Data.Get(), TargetASC);
			HitActors.Add(OtherActor);
		}
	}
}
