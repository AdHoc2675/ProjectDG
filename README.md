# 🎮 Project DG

Project DG는 **Gameplay Ability System (GAS)**을 기반으로 설계된 **3인칭 오픈월드 멀티플레이어 ARPG** 개발용 언리얼 엔진 5 프로젝트입니다. 최대 4인 협동(Co-op) 플레이를 핵심 가치로 삼고 있으며, 다양한 직업군 간의 파티 시너지 전투 및 보스 공략, 데이터 주도형 장비 제작/성장 시스템을 제공합니다.

---

## 핵심 기술 스택 및 특징

1. **GAS (Gameplay Ability System) 연동**:
   - 캐릭터의 스킬, 콤보 공격, 버프/디버프 및 피해 판정을 모두 GAS 구조 안에서 완전한 멀티플레이어 예측(Prediction)과 동기화(Replication) 하에 처리합니다.
   - 캐릭터 속성(Health, Mental, Stamina 등) 및 스탯 관리를 위한 어트리뷰트 세트를 지원합니다.
2. **Dedicated Server & Co-op 세션**:
   - 월드 상태 동기화 및 몬스터 AI 제어, 피해 연산 등의 중요 로직은 서버 권한(Server Authority)을 따릅니다.
   - 방장(Host)의 세션에 참여해 함께 월드를 탐험하는 파티 동기화 구조를 지원합니다.
3. **데이터 주도형 아이템 및 제작 시스템**:
   - 아이템 등급, 주스탯 계산식, 레시피, 보조 옵션(Reroll/Enhance 등)은 데이터 테이블(DataTable)을 통해 관리하여 기획적 확장에 매우 유연하게 대응합니다.
4. **오브젝트 풀링(Object Pooling) 최적화**:
   - 심리스 오픈월드 환경에서 대량으로 리젠되는 필드 몬스터들과 전투 시 생성되는 플로팅 데미지 텍스트의 성능 부하를 해소하기 위해 전용 Subsystem 기반 오브젝트 풀을 탑재했습니다.

---

## 주요 디렉터리 및 코드 안내

### 1. [GAS (Gameplay Ability System)](Source/ProjectDG/Public/GAS)
전투 및 특수 스킬 시스템을 정의하는 핵심 폴더입니다.
* **Attributes**: 
  - [DG_AttributeSet.h](Source/ProjectDG/Public/GAS/Attributes/DG_AttributeSet.h): 자원 및 전투 보정치, 스탯 관련 어트리뷰트 선언.
  - [DG_BossAttributeSet.h](Source/ProjectDG/Public/GAS/Attributes/DG_BossAttributeSet.h) & [DG_EnemyAttributeSet.h](Source/ProjectDG/Public/GAS/Attributes/DG_EnemyAttributeSet.h): 보스 및 일반 적에 최적화된 개별 어트리뷰트 선언.
* **Effects**:
  - [DGExecCalc_Damage.h](Source/ProjectDG/Public/GAS/Effects/Excution/DGExecCalc_Damage.h): 데미지 가감 및 물리/마법 계산, 그로기 피해 공식 등을 수행하는 커스텀 데미지 계산식 클래스.
* **Abilities**:
  - [Warrior](Source/ProjectDG/Public/GAS/Abilities/Player/Warrior): 예리한 일격, 파멸의 맹타 등 근접/광역 스킬 수록.
  - [Assassin](Source/ProjectDG/Public/GAS/Abilities/Player/Assassin): 기습, 섬광 베기 등 암습 스킬 수록.

### 2. [Item & Inventory](Source/ProjectDG/Public/Item)
제작 기반의 성장을 견인하는 아이템 시스템 구조를 수록합니다.
* [DG_ItemTypes.h](Source/ProjectDG/Public/Item/DG_ItemTypes.h): 장비 등급, 스탯 범위, 레시피, 부스탯 데이터([FDGSubOptionInstanceData](Source/ProjectDG/Public/Item/DG_ItemTypes.h#L140)) 등 데이터 테이블 연동용 구조체 및 Enums를 정의합니다.
* [DGInventoryComponent.h](Source/ProjectDG/Public/Components/Inventory/DGInventoryComponent.h): 유저 인벤토리 내부 목록 제어 및 장비 장착 처리.

### 3. [Character & AI](Source/ProjectDG/Public/Character)
* [BaseCharacter.h](Source/ProjectDG/Public/Character/BaseCharacter.h): 캐릭터 공통 조상 클래스.
* [PlayerCharacterBase.h](Source/ProjectDG/Public/Character/Player/PlayerCharacterBase.h): Warrior/Assassin/Archer 등으로 분기되는 아군 플레이어의 공통 로직 탑재.
* [FieldEnemyBase.h](Source/ProjectDG/Public/Character/Enemy/Field/FieldEnemyBase.h): 필드 몬스터 베이스로, 오브젝트 풀에 반환되거나 꺼내어질 때의 초기화 라이프사이클을 가집니다.

### 4. [System (최적화 매니저)](Source/ProjectDG/Public/System)
* [FieldEnemyPoolSubsystem.h](Source/ProjectDG/Public/System/FieldEnemyPoolSubsystem.h): 레벨에 배치된 스포너의 몬스터 획득(Acquire)/반환(Return)을 조율하는 서브시스템.
* [DGDamageNumberPoolSubsystem.h](Source/ProjectDG/Public/System/DGDamageNumberPoolSubsystem.h): 피해 발생 시 출력되는 UI 텍스트 풀링 컴포넌트.

---

## 기획서 & 개발 문서
프로젝트의 세부 수치 설계 및 스펙은 `Readme/` 디렉터리에 기획서 문서로 정리되어 있습니다.
* [DG_시스템_기획서](Readme/ProjectDG_시스템_기획서.md): GAS 연동 로직, 클래스별 스탯 스케일링, 인벤토리 MVC 아키텍처.
* [DG_아이템_기획서](Readme/DG_아이템_기획서.md): 등급별 색상, 주/부스탯 목록, 제작/강화(+4차)/재설정(Reroll & 메모리 기능) 시스템 데이터 및 테이블 설계서.
* [필드몬스터 스폰 시스템](Readme/필드몬스터 스폰 시스템.md): 오브젝트 풀링 라이프사이클 복구 항목, 멀티캐스트 RPC 동기화 및 트러블슈팅 이력 가이드.


