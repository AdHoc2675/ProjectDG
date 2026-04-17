#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "AbilitySystemInterface.h"
#include "GameplayTagContainer.h"
#include "AbilitySystemComponent.h"
#include "AttributeSet.h"
#include "BaseCharacter.generated.h"


class UCombatComponent;

/**
 * Player / Enemy 공통 부모 캐릭터
 - 공통적으로 가져야 되는 상태 (죽음, 팀 tag) 등을 보관
 - 기본적은 GAS 연결용 인터페이스 
 - 사망처리 흐름
 
 +아직 Attribute나 Combatcomponent 는 만들어지지 않아 이부분은 주석,깡통 처리 되어있음.
 
 
 */
UCLASS()
class PROJECTDG_API ABaseCharacter : public ACharacter, public IAbilitySystemInterface
{
    GENERATED_BODY()

public:
    ABaseCharacter();

protected:
    virtual void BeginPlay() override;

    /** 사망 여부 */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "BaseCharacter|State")
    bool bIsDead = false;

    /** 팀 식별용 태그 (예: Team.Player, Team.Enemy) */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "BaseCharacter|Team")
    FGameplayTag TeamTag;

    //CombatComponent 부착. Enemy/player 둘다 쓸거라 Base 에서 부착
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="BaseCharacter|Component")
    TObjectPtr<UCombatComponent> CombatComponent = nullptr;

protected:
    /** ASC / Attribute / 기타 전투 관련 초기화 가능 시도 */
    virtual void InitializeCharacterIfReady();

    /** Attribute 초기화, 태그 부착 등 확장용 */
    virtual void InitializeAttributesIfReady();

    /** 사망처리 본체
     * Die() 는 진입점이고 HandelDeath 에서 실질적으로 죽음을 처리.
     * 
     */
    virtual void HandleDeath();

    /** 죽음 후 이동/입력/충돌 정리 
     * 죽음 이후 처리되야 될것들 (ex. 이동 , 충돌) 비활성화 
     */
    virtual void DisableCharacterAfterDeath();

    /** ASC 재탐색 / 캐시 갱신 */
    virtual void RefreshAbilitySystemReferences();

public:
    virtual void Tick(float DeltaSeconds) override;

    /** IAbilitySystemInterface 
     * 외부 시스템이 이 Actor 에 ASC 를 참조할때 이것을 사용
     */
    virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;

    /** 공용 ASC getter 
     * ASC Base 고 지금은 Null.
     * 상속받은 Class 에서 ASC 를 지정
     */
    UFUNCTION(BlueprintCallable, Category = "BaseCharacter|ASC")
    virtual UAbilitySystemComponent* GetCharacterAbilitySystemComponent() const;

    /** 공용 AttributeSet getter
     * Attribute 지정 
     */
    UFUNCTION(BlueprintCallable, Category = "BaseCharacter|ASC")
    virtual const UAttributeSet* GetCharacterAttributeSet() const;

    //CombatComponent 반환
    UFUNCTION(BlueprintCallable, Category="BaseCharacter|Components")
    UCombatComponent* GetCombatComponent() const;

    /** 현재 팀 태그 반환 */
    UFUNCTION(BlueprintCallable, Category = "BaseCharacter|Team")
    FGameplayTag GetTeamTag() const;

    /** 팀 태그 설정 */
    UFUNCTION(BlueprintCallable, Category = "BaseCharacter|Team")
    void SetTeamTag(FGameplayTag InTeamTag);

    /** 해당 팀인지 확인 */
    UFUNCTION(BlueprintCallable, Category = "BaseCharacter|Team")
    bool HasTeamTag(FGameplayTag InTeamTag) const;

    /** 상대가 아군인지 확인 */
    UFUNCTION(BlueprintCallable, Category = "BaseCharacter|Team")
    bool IsFriendlyTo(const ABaseCharacter* OtherCharacter) const;

    /** 사망 여부 확인 */
    UFUNCTION(BlueprintCallable, Category = "BaseCharacter|State")
    bool IsDead() const;

    /** 외부에서 죽음 처리 진입처리용 함수
     * 하는일 . 
     * 1)이미 죽엇는지 확인
     * 2) 안죽엇으면 bIsDead 를 True로 
     * 3) HandleDeath() 를 호출
     */
    UFUNCTION(BlueprintCallable, Category = "BaseCharacter|State")
    virtual void Die();

    /** Pawn 소유 변경 시 초기화 재시도 */
    virtual void PossessedBy(AController* NewController) override;

    /*  네트워크 환경에서 PlayerState가 복제되어 들어온 뒤 호출된다.
    *
    * PlayerState가 ASC를 들고 있는 구조라면
    * 이 시점이 매우 중요할 수 있다.
    *
    * 그래서 여기서도 InitializeCharacterIfReady()를 다시 호출한다.*/
    virtual void OnRep_PlayerState() override;
};
