// Fill out your copyright notice in the Description page of Project Settings.


#include "GAS/Abilities/Player/Movement/GA_Player_Dodge.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Character/Player/PlayerCharacterBase.h"
#include "Character/Player/Data/PlayerCharacterClassData.h"
#include "Core/DG_GameplayTags.h"
#include "GameFramework/CharacterMovementComponent.h"


UGA_Player_Dodge::UGA_Player_Dodge()
{
    // 인스턴싱 정책: 액터마다 인스턴스를 생성하여 상태를 관리
    InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;

    // 네트워크 정책: 클라이언트에서 예측 실행 후 서버 승인 (반응성 최적화)
    NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::LocalPredicted;

    // 실행 시 자동으로 'State.Movement.Dodge' 태그를 캐릭터에게 부여
    ActivationOwnedTags.AddTag(DGGameplayTags::State_Movement_Dodge);

    // 실행 차단 조건 설정
    ActivationBlockedTags.AddTag(DGGameplayTags::State_Movement_Dodge); // 이미 회피 중일 때
    ActivationBlockedTags.AddTag(DGGameplayTags::State_Movement_Jump);  // 점프(공중) 상태일 때
    ActivationBlockedTags.AddTag(DGGameplayTags::Block_Movement_Dodge); // 외부 요인(CC기 등)으로 차단될 때
}

void UGA_Player_Dodge::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo*
    ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
    // 1. 자원(스태미나) 소모 및 쿨타임 체크 (GE_Cost 연동 시 자동 처리)
    if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
    {
        EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
        return;
    }

    // 2. 아바타(캐릭터) 포인터 유효성 검사
    APlayerCharacterBase* Character = Cast<APlayerCharacterBase>(ActorInfo->AvatarActor.Get());
    if (!Character)
    {
        EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
        return;
    }

    // [수정된 부분] 캐릭터의 상황별 애니메이션 세트를 가져옵니다.
    const FPlayerMovementAnimationSet& AnimSet = Character->GetCurrentMovementAnims();

    FVector DodgeDirection = Character->GetLastMovementInputVector();

    // [수정된 부분] 데이터 에셋에서 정의된 몽타주를 선택합니다.
    UAnimMontage* SelectedMontage = AnimSet.ForwardDodge;

    if (DodgeDirection.IsNearlyZero())
    {
        DodgeDirection = -Character->GetActorForwardVector();
        SelectedMontage = AnimSet.BackwardDodge;
    }
    DodgeDirection.Normalize();

    // 4. 물리적 추진력 적용 (LaunchCharacter)
    // CMC의 LaunchCharacter는 서버-클라이언트 간 위치 동기화를 기본적으로 지원함
    Character->LaunchCharacter(DodgeDirection * DodgeStrength, true, false);

    // 5. 몽타주 재생 및 대기 태스크 생성
    UAbilityTask_PlayMontageAndWait* MontageTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(
        this,
        TEXT("DodgeMontageTask"),
        SelectedMontage
    );

    // 애니메이션이 끝나거나, 취소되거나, 중단되었을 때 어빌리티를 종료하도록 이벤트 연결
    MontageTask->OnCompleted.AddDynamic(this, &UGA_Player_Dodge::K2_EndAbility);
    MontageTask->OnInterrupted.AddDynamic(this, &UGA_Player_Dodge::K2_EndAbility);
    MontageTask->OnBlendOut.AddDynamic(this, &UGA_Player_Dodge::K2_EndAbility);
    MontageTask->OnCancelled.AddDynamic(this, &UGA_Player_Dodge::K2_EndAbility);

    // 태스크 활성화
    MontageTask->ReadyForActivation();
}

void UGA_Player_Dodge::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo*
    ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
    // Super 호출 시 부여되었던 태그들이 자동으로 회수됨
    Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}