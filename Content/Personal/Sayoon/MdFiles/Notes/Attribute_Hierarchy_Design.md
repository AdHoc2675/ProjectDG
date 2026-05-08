# 캐릭터 계층 구조에 따른 Attribute 설계 원칙 및 필요성

본 문서는 ProjectDG의 캐릭터 계층 구조(`BaseCharacter` -> `EnemyCharacterBase` -> `BossCharacterBase`)에 맞춰 Gameplay Ability System(GAS)의 **Attribute(속성)**를 분리하고 설계해야 하는 이유와 그 필요성을 정리합니다.

---

## 1. 계층별 Attribute 설계의 필요성

모든 스탯을 하나의 거대한 클래스나 단일 AttributeSet에 몰아넣는 대신, 클래스 상속 구조에 맞춰 Attribute를 단계적으로 정의하고 적용해야 합니다. 이는 **코드의 재사용성, 유지보수성, 그리고 확장성**을 극대화하기 위함입니다.

### 1.1. `BaseCharacter` (최상위 공통 클래스)
* **대상**: 플레이어, 일반 몬스터, 보스 등 게임 내 존재하는 모든 생명체.
* **설계 필요성**:
  * **공통 기반 마련**: 모든 캐릭터가 필수적으로 가져야 하는 생존 및 기본 이동 관련 속성을 정의합니다.
  * **핵심 로직의 중앙화**: 체력 감소, 사망 처리(Die), 기본 이동 속도 적용과 같은 핵심 로직을 최상위에서 한 번만 구현하여 코드 중복을 방지합니다.
* **적용 Attribute 예시**: 
  * `Health` (현재 체력)
  * `MaxHealth` (최대 체력)
  * `MovementSpeed` (이동 속도)

### 1.2. `EnemyCharacterBase` (`BaseCharacter` 상속)
* **대상**: 플레이어와 적대 관계에 있으며, AI에 의해 제어되는 모든 적 몬스터.
* **설계 필요성**:
  * **플레이어와 AI의 데이터 분리**: 플레이어 캐릭터에게는 불필요한 속성(예: 어그로 수치, 경험치 드롭량 등)을 격리하여 메모리 낭비와 논리적 혼선을 막습니다.
  * **적 공통 로직 지원**: 데미지 공식, 방어력 계산, 기본 AI 행동 반경 등 적 몬스터 전체에 일괄적으로 적용되는 밸런싱 요소를 관리합니다.
* **적용 Attribute 예시**:
  * `BaseAttackPower` (기본 공격력)
  * `Defense` (방어력)
  * `AggroRadius` (인식 반경)
  * `ProvideExp` (처치 시 제공 경험치)

### 1.3. `BossCharacterBase` (`EnemyCharacterBase` 상속)
* **대상**: 특수한 기믹, 페이즈 전환, 고유 스킬 패턴을 가지는 보스 몬스터.
* **설계 필요성**:
  * **특수 기믹의 독립성**: 일반 몬스터에는 없는 보스만의 고유 시스템(예: 부위 파괴 내성, 페이즈 전환 트리거, 그로기 수치 등)을 구현하기 위한 전용 데이터가 필요합니다.
  * **안전한 밸런싱**: 일반 몬스터의 스탯을 조정하더라도 보스의 치명적인 패턴이나 특수 기믹 수치에 의도치 않은 사이드 이펙트(Side Effect)가 발생하지 않도록 분리합니다.
* **적용 Attribute 예시**:
  * `PhaseThreshold` (페이즈 전환 체력 임계점)
  * `StaggerGauge` / `RageGauge` (무력화 또는 분노 수치)
  * `UltimateSkillCooldown` (특수/궁극기 쿨타임)

---

## 2. 설계 원칙 (Best Practices)

1. **하향식 구체화 (Top-Down Specification)**
   * 부모 클래스에는 가장 보편적이고 추상적인 Attribute만 남깁니다.
   * 자식 클래스로 내려갈수록 역할에 맞는 구체적이고 특수한 Attribute를 추가합니다.
2. **모듈형 AttributeSet 사용 권장**
   * GAS 구현 시 단일 `UAttributeSet`을 상속받아 계속 확장하기보다는, `UBaseAttributeSet`, `UEnemyAttributeSet`, `UBossAttributeSet` 등으로 컴포넌트를 나누어 필요한 캐릭터에게 부착(Add Component)하는 조합 방식을 고려하면 유연성이 더욱 향상됩니다.
3. **불필요한 데이터 노출 최소화**
   * 각 계층에 필요한 Attribute만 존재하므로, 기획자나 프로그래머가 에디터(블루프린트)에서 설정해야 할 데이터의 복잡도가 줄어들고 휴먼 에러를 방지할 수 있습니다.
