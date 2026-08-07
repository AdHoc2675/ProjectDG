# ProjectDG

> Unreal Engine 5 기반 3인칭 오픈월드 멀티플레이어 ARPG

---

## 프로젝트 개요

**ProjectDG**는 최대 4인 Co-op 파티 플레이를 핵심으로 하는 3인칭 오픈월드 액션 RPG입니다.  
Gameplay Ability System(GAS)을 전면 채택하여 스킬, 전투, 버프/디버프, 데미지 연산을 멀티플레이어 예측·동기화 하에 처리하며, Dedicated Server 구조를 통해 서버 권한 기반의 안정적인 Co-op 세션을 제공합니다.

### 핵심 특징

- **GAS 기반 전투 시스템** — 직업별 스킬·콤보·쿨타임·그로기를 GAS 파이프라인으로 완전 처리
- **Dedicated Server & Co-op 세션** — Backend REST API 기반 계정·세션 관리 + ClientTravel 자동 이동
- **데이터 주도형 아이템 시스템** — DataTable 기반 등급·주스탯·레시피·보조옵션(강화/재설정) 관리
- **다직업 파티 시너지** — Warrior · Archer · Assassin (+ Mage 확장 예정) 간 역할 분담
- **오브젝트 풀링 최적화** — 필드 몬스터 + 데미지 넘버 전용 Subsystem 풀
- **보스 페이즈 시스템** — Health 비율 기반 페이즈 전환 + SkillSet 교체 + 컷신 연출

---

## 기술 스택

| 항목 | 사용 기술 |
|---|---|
| 엔진 | Unreal Engine 5 |
| 언어 | C++ / Blueprint (혼합) |
| 빌드 | UnrealBuildTool |
| 입력 | Enhanced Input System |
| 능력치·전투 | Gameplay Ability System (GAS) |
| AI | AIModule + Behavior Tree + StateTree + EQS |
| VFX | Niagara |
| UI | UMG (Slate / SlateCore) |
| 네트워크 | Dedicated Server + Replication |
| 백엔드 통신 | HTTP + JSON (REST API) |
| 컷신 | LevelSequence + MovieScene |

### 모듈 의존성

```
Core, CoreUObject, Engine, InputCore, EnhancedInput,
AIModule, StateTreeModule, GameplayStateTreeModule,
UMG, Slate, SlateCore, Niagara,
GameplayTags, GameplayTasks, GameplayAbilities,
HTTP, Json, JsonUtilities,
NavigationSystem, LevelSequence, MovieScene, MovieSceneTracks
```

---

## 프로젝트 구조

```
Source/ProjectDG/
├── Public/
│   ├── AI/                    # AI 시스템 (Controller, BT Task/Service, EQS)
│   ├── Actor/                 # 전투용 Actor (AttackHitboxActor)
│   ├── Character/             # 캐릭터 계층 (Player, Enemy, Boss, Field)
│   ├── Components/            # 기능 컴포넌트 (Combat, Inventory, Item, Targeting, UI)
│   ├── Core/                  # 공용 Enum, Struct, GameplayTag, Debug
│   ├── Data/                  # DataAsset (SkillData, Attribute, VFX)
│   ├── GAS/                   # GAS 전체 (Abilities, ASC, Attributes, Effects, Cues, Tasks)
│   ├── GameFramework/         # GameMode, GameState, GameInstance, PlayerController, PlayerState
│   ├── Item/                  # 아이템 정의 (ItemTypes, ItemDefinition, ItemInstance, LootActor)
│   ├── Objects/               # 월드 오브젝트 (Waypoint)
│   ├── Server/                # 서버 인프라 (Auth, Backend, Session)
│   ├── System/                # 오브젝트 풀 서브시스템
│   └── UI/                    # HUD, Widget, WidgetController, Loading
└── Private/
    └── (위 Public 구조와 1:1 대응)
```

---

## 아키텍처

### 캐릭터 계층

```
ACharacter
 └─ ABaseCharacter (IAbilitySystemInterface)
     │  · TeamTag (GameplayTag 기반 아군/적군 식별)
     │  · CombatComponent (데미지 파이프라인 위임)
     │  · 공용 ASC/Attribute getter 추상화
     │  · 공통 사망 처리 (Die → HandleDeath → DisableCharacterAfterDeath)
     │
     ├─ APlayerCharacterBase
     │   │  · ASC는 PlayerState가 소유 (Character는 조회만)
     │   │  · 3인칭 카메라 (SpringArm + Camera + Zoom)
     │   │  · LockOnComponent (자동 타겟팅)
     │   │  · InventoryComponent (인벤토리)
     │   │  · MinimapCaptureComponent / MinimapMarkerComponent
     │   │  · AI 인지용 StimuliSourceComponent
     │   │  · 모듈러 외형 (Head, Hair, Upper/Lower Body, Helmet, Shoes, Shoulder, Gloves)
     │   │  · 스킬 슬롯 매핑 (SkillSlotMapping: SlotTag → SkillTag)
     │   │  · EnhancedInput 바인딩 (Move, Look, Jump, Shift, Zoom, Skill 1~4/Q/E)
     │   │  · 피격 카메라 셰이크, 소모품 사용 (HP/MP 포션)
     │   │  · 사망 → Event.Player.Death GA → 리스폰 타이머 → 위치 복구
     │   │
     │   ├─ (Warrior) ─ 근접 물리 딜러
     │   ├─ (Archer)  ─ 원거리 사수
     │   └─ (Assassin) ─ 은밀 근접 암살자
     │
     └─ AEnemyCharacterBase
         │  · 자체 ASC + DG_AttributeSet + DG_EnemyAttributeSet 호스팅
         │  · DataTable 기반 Attribute 초기화 (Tag → RowName 매핑)
         │  · LootDropComponent (사망 시 아이템 드롭)
         │  · MinimapMarkerComponent
         │  · HitboxComponent (피격 판정)
         │  · Niagara 텔레그래프 (AOE/Directional) + 데미지 넘버 멀티캐스트
         │  · SkillIndicator 스폰 멀티캐스트
         │
         ├─ AFieldEnemyBase
         │   │  · FieldCharacterClassData 기반 스탯/스킬 초기화
         │   │  · 스폰 원점 기억 + Leashing 시스템 (복귀 거리 초과 → 무적 귀환)
         │   │  · 순찰 반경 (PatrolRadius)
         │   │  · 오브젝트 풀 연동 (OnSpawnedFromPool / OnReturnedToPool)
         │   │  · Blackboard 값 자동 동기화
         │   │
         │   ├─ AField_Beritra
         │   ├─ AField_Coradon
         │   ├─ AField_CorpeSpider
         │   ├─ AField_DracoWar3 / DracoWar5
         │   ├─ AField_DranaVar
         │   ├─ AField_Neuth
         │   ├─ AField_Nihogg
         │   ├─ AField_Ramfu_04
         │   └─ AField_Tie_01
         │
         └─ ABossCharacterBase
             │  · BossCharacterClassData 기반 페이즈/스킬셋/컷신 관리
             │  · DG_BossAttributeSet (그로기 게이지 등)
             │  · Health 비율 기반 다단계 Phase 전환
             │  · 페이즈 전환 대기 (Pending) → AnimNotify 확인 후 적용
             │  · 컷신 위치 복귀 (InitialBossTransform)
             │  · 그로기 상태 전환
             │
             ├─ ABoss_Kashapa (페이즈 1 공격 8종 + 페이즈 2 공격 12종)
             ├─ ABoss_Ramu
             └─ ABoss_Zikel
```

> **ASC 호스팅 전략**: Player는 `PlayerState`에, Enemy/Boss는 자기 자신에 ASC를 배치합니다.  
> `BaseCharacter::GetCharacterAbilitySystemComponent()`가 이 차이를 투명하게 추상화합니다.

---

### Gameplay Ability System (GAS)

#### AttributeSet

| 카테고리 | 속성 | 설명 |
|---|---|---|
| **자원** | `Health / MaxHealth` | 체력 |
| | `Mental / MaxMental` | 정신력 (스킬 자원) |
| | `Stamina / MaxStamina` | 스태미나 (회피·질주) |
| **주스탯** | `MainStat` | 직업별 주스탯 |
| | `Strength / Dexterity / Intelligence` | Legacy 스탯 |
| **전투** | `AttackPower` | 공격력 |
| | `Defense` | 방어력 |
| | `HealthCoefficient / DefenseCoefficient` | 직업별 생존 차별화 계수 |
| | `CriticalRate / CriticalDamage` | 치명타 확률 / 피해 |
| | `AttackSpeed / MoveSpeed` | 공격 속도 / 이동 속도 |
| | `GroggyDamage / GroggyDamageIncreaseRate` | 그로기 피해 / 증가율 |
| | `FinalDamageIncrease / DamageReduction` | 최종 피해 증가 / 받는 피해 감소 |
| | `CooldownReduction` | 스킬 재사용 시간 감소 |
| | `MentalRecoveryIncrease` | 정신력 회복량 증가 |
| | `LifeSteal` | 생명력 흡수 |
| **메타** | `Damage` | 데미지 임시 저장 (네트워크 복제 안 함) |

- `PreAttributeChange`에서 값 클램핑, `PostGameplayEffectExecute`에서 사망 판정 처리
- **DataTable 기반 초기화**: TeamTag로 Row를 검색하여 캐릭터별 초기 스탯 적용

#### Ability 계층

```
UGameplayAbility
 └─ UGameplayAbilityBase
     │  · 공용 헬퍼 (ASC, Pawn, Controller 접근)
     │
     ├─ UGA_PlayerSkillBase (플레이어 스킬 공통)
     │   │  · 스킬 데이터 에셋 연동 (SkillDataAsset)
     │   │  · VFX/SFX GameplayCue 자동 발동
     │   │  · 콤보 체인 (SkillComboStep 관리)
     │   │  · 쿨타임 (GE_SkillCoolDown) + 코스트 (GE_SkillCost)
     │   │  · 이동 잠금/해제 연동
     │   │
     │   ├─ UGA_MeleeAttackBase (근접 콤보)
     │   │   └─ 소켓 기반 HitTrace, 콤보 인덱스 관리
     │   │
     │   ├─ UGA_SingleMeleeSkillBase (단일 근접 스킬)
     │   ├─ UGA_AOESkillBase (광역 장판 스킬)
     │   ├─ UGA_RangedSkillBase (원거리 스킬)
     │   ├─ UGA_ChargeSkillBase (차지 스킬)
     │   ├─ UGA_ProjectileSkillBase (투사체 스킬)
     │   ├─ UGA_TargetSkillBase (타겟 지정 스킬)
     │   └─ UGA_TargetMontageSkillBase (타겟 이동+몽타주 스킬)
     │
     ├─ UGA_Player_Damage (피격 반응)
     ├─ UGA_Player_Death (사망 처리)
     ├─ UGA_Player_Dodge (회피)
     ├─ UGA_Player_Jump (점프)
     │
     └─ Enemy Abilities
         ├─ UGA_EnemyAttack (기본 공격)
         ├─ UGA_EnemyAOEAttack (AOE 공격)
         ├─ UGA_EnemyDirectionalAttack (방향 공격)
         ├─ UGA_SpawnHitboxSkill (히트박스 스폰)
         └─ Boss/ (보스 전용 GA)
```

#### 데미지 파이프라인

```
FDGDamageRequest 생성
 → CombatComponent::ApplyDamageRequest()
    → ValidateDamageRequest (서버 권한 / 유효성 검증)
    → ResolveASCFromActor (Source / Target ASC 조회)
    → GE_Damage Spec 생성 (SetByCaller: BaseDamage, DamageMultiplier, GroggyDamage)
    → UDGExecCalc_Damage 실행
       → Source AttackPower × DamageMultiplier + BaseDamage
       → Target Defense / DefenseCoefficient 기반 감소율 적용
       → FinalDamageIncrease / DamageReduction 적용
       → 결과를 Meta Attribute (Damage)에 출력
    → PostGameplayEffectExecute
       → Health 차감
       → 사망 판정 (Health ≤ 0 → Die())
 → FDGDamageResult 반환
```

#### GameplayCue

| 클래스 | 역할 |
|---|---|
| `GCN_WeaponTrail` | 무기 검광 이펙트 (Blade Trail) |
| `GCN_SkillCue` | 스킬별 VFX/SFX (Cast, Hit, Impact) |

---

### 입력 시스템

**EnhancedInput** 기반으로, 직업별 스킬을 **슬롯 태그(Input.SkillSlot.X) → 스킬 태그(Skill.Warrior.SharpStrike)** 매핑으로 바인딩합니다.

```
BasicInputMappingContext
  ├─ 기본 액션 (Move, Look, Jump, Shift, CameraZoom, ToggleMap, ToggleInventory)
  │    → PlayerCharacterBase 직접 처리
  │
  └─ 스킬 액션 (Skill_1~4, Q, E, HPPotion, MPPotion)
       → OnSkillInputStarted(SlotTag)
           → SkillSlotMapping에서 SkillTag 조회
           → SendSkillInputStartedEvent → Server RPC
           → ASC::SendGameplayEvent → GA 활성화
```

---

### 직업별 스킬 목록

#### Warrior (전사)

| 스킬 | GameplayTag | 유형 |
|---|---|---|
| 예리한 일격 | `Skill.Warrior.SharpStrike` | 콤보 근접 |
| 격쇄 강타 | `Skill.Warrior.CuttingSmash` | 콤보 근접 |
| 대지 강타 | `Skill.Warrior.GroundSlam` | AOE |
| 발목 베기 | `Skill.Warrior.AnkleSlash` | 단일 근접 |
| 충격파 | `Skill.Warrior.ShockWave` | AOE |
| 파멸의 맹타 | `Skill.Warrior.DoomStrike` | 타겟 돌진 |
| 도약 찍기 | `Skill.Warrior.LeapingSlam` | 타겟 도약 |

#### Archer (궁수)

| 스킬 | GameplayTag | 유형 |
|---|---|---|
| 저격 | `Skill.Archer.Snipe` | 차지 원거리 |
| 속사 | `Skill.Archer.RapidShot` | 원거리 연사 |
| 관통화살 | `Skill.Archer.PiercingArrow` | 관통 원거리 |
| 질풍화살 | `Skill.Archer.GaleArrow` | 원거리 |
| 조준화살 | `Skill.Archer.AimedArrow` | 에임 원거리 |

#### Assassin (암살자)

| 스킬 | GameplayTag | 유형 |
|---|---|---|
| 빠른 베기 | `Skill.Assassin.QuickSlash` | 콤보 근접 |
| 기습 | `Skill.Assassin.Ambush` | 단일 근접 |
| 암습 | `Skill.Assassin.ShadowAssault` | 타겟 돌진 |
| 섬광 베기 | `Skill.Assassin.FlashSlash` | 타겟 이동 |
| 심장 찌르기 | `Skill.Assassin.HeartStab` | 타겟 근접 |
| 침투 | `Skill.Assassin.Infiltration` | 타겟 이동 |

#### 공통 이동 스킬

| 스킬 | GameplayTag |
|---|---|
| 점프 | `Skill.Common.Jump` |
| 회피 | `Skill.Common.Dodge` |
| 질주 | `Skill.Common.Sprint` |

---

### LockOnComponent (자동 타겟팅)

플레이어 캐릭터에 부착되는 실시간 타겟팅 컴포넌트입니다.

- **실시간 갱신**: 설정 간격(0.05s)마다 최적 타겟 재계산
- **스코어링 공식**: `AngleWeight(0.7) × 각도 + DistanceWeight(0.3) × 거리`
- **LOS(Line of Sight)** 검증 지원
- **타겟 외곽선**: Custom Stencil Value로 적(111) / 오브젝트(112) 구분 아웃라인 표시
- **타겟 변경 이벤트**: `OnLockOnTargetChanged` / `OnLockOnReleased` 델리게이트
- **태그 필터**: `AllowedTargetTags` 기반 후보 필터링

---

### AI 시스템

적 AI는 **BehaviorTree + StateTree** 하이브리드 구조로 동작합니다.

```
Source/ProjectDG/Public/AI/
├── Controller/
│   ├── AIControllerBase.h          # AI 컨트롤러 공통 베이스
│   ├── EnemyAIController.h         # 필드 몬스터 전용
│   └── BossAIController.h          # 보스 전용
├── Tasks/
│   ├── BTTask_FieldActivateAbility     # 필드 몬스터 GA 실행
│   ├── BTTask_BossActivateAbility      # 보스 GA 실행 (페이즈 연동)
│   ├── BTTask_KashapaPhaseGimmick      # 카샤파 보스 페이즈 기믹
│   ├── BTTask_SetRandomPatrolLocation  # 순찰 지점 생성
│   └── BTTask_UseRandomSkill           # 랜덤 스킬 선택
├── Services/
│   ├── BTService_FieldEnemyLeashCheck          # 필드 몬스터 리싱 거리 체크
│   ├── BTService_UpdateEnemyCombatState        # 전투 상태 갱신
│   └── BTService_KashapaUpdateTargetDistance   # 카샤파 타겟 거리 갱신
└── EQS/
    └── EnvQueryContext_SpawnOrigin     # 스폰 원점 EQS 컨텍스트
```

#### 필드 몬스터 AI 흐름

```
스폰 (풀에서 Acquire)
 → 순찰 (PatrolRadius 내 랜덤 이동)
 → 플레이어 감지 (AIPerception)
 → 추격 + 전투 (랜덤 스킬 선택 → GA 실행)
 → Leash 거리 초과 → 무적 귀환 (GE_Returning)
 → 원점 복귀 완료 → 순찰 재개
 → 사망 → 풀 반환
```

#### 보스 AI 흐름

```
보스 스폰
 → Phase 1 SkillSet 기반 BehaviorTree
 → Health 임계값 도달 → Phase 전환 요청 (Pending)
 → 현재 GA 종료 대기
 → PhaseTransition GA (컷신 + AnimNotify → ApplyPendingPhaseChange)
 → Phase 2 SkillSet 교체
 → 반복
```

---

### 아이템 시스템

DataTable 기반의 장비 제작/강화/재설정 시스템입니다.

#### 아이템 분류

| 구분 | 값 |
|---|---|
| **타입** | Equipment(장비), Consumable(소모품), Material(재료) |
| **장비 유형** | Weapon(무기), Armor(방어구) |
| **등급** | Normal → Hero → Legendary → Ancient |
| **재료 용도** | Craft(제작), Enhance(강화), Reroll(재설정) |

#### 데이터 테이블 구조

| DataTable | Row 구조체 | 역할 |
|---|---|---|
| DT_ItemGrade | `FDGItemGradeTableRow` | 등급명, 색상 Hex, 보조옵션 수 |
| DT_EquipmentMainStatByLevel | `FDGEquipmentMainStatTableRow` | 아이템 레벨별 주스탯/HP/공격력/방어력 |
| DT_EquipmentRecipe | `FDGEquipmentRecipeTableRow` | 장비 제작 레시피 |
| DT_ItemMaterial | `FDGItemMaterialTableRow` | 재료 아이템 정의 |

#### 보조옵션 (Sub Option)

`FDGSubOptionInstanceData`로 런타임 인스턴스를 관리합니다:
- `SubOptionID` — 보조옵션 정의 Row 참조
- `BaseValue` — 기본 수치
- `EnhanceCount` — 강화 횟수
- `EnhanceTotalValue` — 강화 누적 수치

#### 인벤토리

`UDGInventoryComponent`가 유저 인벤토리를 관리하며, 장비 장착 시 `OnEquipmentChanged` 델리게이트를 통해 모듈러 외형 메쉬를 교체합니다.

---

### 서버 인프라

#### 계정 & 인증 (`DGAuthSubsystem`)

GameInstance 서브시스템으로 구현된 계정/인증 관리자입니다.

| 기능 | 메서드 |
|---|---|
| 회원가입 | `RegisterAccount(LoginId, Password, DisplayName)` |
| 로그인 | `Login(LoginId, Password)` |
| 캐릭터 목록 조회 | `RequestCharacterList()` |
| 캐릭터 생성 | `CreateCharacter(SlotIndex, CharacterName, ClassTag)` |
| 캐릭터 선택 | `SelectCharacterBySlotIndex()` / `SelectCharacterById()` |

#### 세션 관리 (`DGSessionSubsystem`)

방 생성/참가 및 Dedicated Server 이동을 담당합니다.

```
CreateRoomAndTravel(RoomName, Password, AccountId, CharacterId, RegionId)
  → Backend REST API 호출 → SessionId / ServerIP / Port 수신
  → ClientTravel → Dedicated Server 접속

JoinRoomAndTravel(RoomName, Password, AccountId, CharacterId)
  → Backend REST API 호출 → JoinToken 수신
  → ClientTravel → 기존 세션 합류
```

#### Backend Client (`DGBackendClient`)

HTTP + JSON 기반 REST API 클라이언트로, Auth/Session 서브시스템의 네트워크 통신을 처리합니다.

---

### 오브젝트 풀링 시스템

| 서브시스템 | 역할 |
|---|---|
| `UFieldEnemyPoolSubsystem` | 필드 몬스터 Acquire/Return 관리. 스포너가 요청하면 비활성 몬스터를 꺼내고, 사망 후 반환 |
| `UDGDamageNumberPoolSubsystem` | 데미지 넘버 위젯/액터 풀링. 전투 시 대량 생성되는 UI 텍스트 성능 부하 해소 |

#### 필드 몬스터 풀링 라이프사이클

```
풀에서 Acquire
 → OnSpawnedFromPool(SpawnLocation)
    → Actor 활성화 (Visibility, Collision, Tick)
    → 원점 저장
    → Attribute/Ability 재초기화
    → Multicast_OnSpawnedFromPool
 → (전투 / 사망)
 → OnDeathAnimationFinished
 → OnReturnedToPool
    → Actor 비활성화 (Hidden, NoCollision, TickDisabled)
    → Multicast_OnReturnedToPool
 → 풀로 Return
```

---

### UI 시스템

```
ADG_HUD (HUD 전체 관리)
 ├─ Overlay (메인 HUD)
 │   ├─ DGOverlayWidgetController (데이터 브로드캐스트)
 │   └─ DGOverlayWidget
 │       ├─ DGPlayerStatWidget (체력/정신력/스태미나/EXP/레벨)
 │       ├─ DGSkillSlotWidget (스킬 슬롯 + 쿨타임)
 │       ├─ DGMiniMapWidget (미니맵)
 │       ├─ DGPartyListWidget (파티 목록)
 │       ├─ DGChatWidget (채팅)
 │       └─ DGQuestWidget (퀘스트)
 │
 ├─ FullMap (전체 맵)
 │   └─ DGFullMapWidgetController
 │
 ├─ CharacterProfile (캐릭터 프로필 + 인벤토리)
 │   ├─ DGInventoryWidgetController
 │   └─ DGCharacterProfileWidget
 │
 ├─ Enemy UI
 │   └─ DGEnemyHPBarWidget (적 체력바)
 │
 ├─ Damage (데미지 넘버 — 풀링)
 │
 └─ Loading (로딩 화면)
     ├─ DGLoadingScreenSubsystem
     ├─ DGLoadingScreenWidget
     └─ DGLoadingTipRow (팁 DataTable)
```

- **WidgetController 패턴**: HUD → WidgetController → Widget 3단계 분리로 데이터와 표현 격리
- **DGOverlayWidgetController**: GAS Attribute 변경 델리게이트를 구독하여 UI에 브로드캐스트
- **토글 UI**: 맵/인벤토리는 토글 방식 (InputMode 자동 전환 + 열기/닫기 효과음)

---

### GameplayTag 체계

```
Team.*                         # 팀 구분 (Player, Enemy, Enemy.Boss, Enemy.Field, Object)
Character.Class.*              # 직업 (Warrior, Archer, Mage, Assassin)
Input.SkillSlot.*              # 입력 슬롯 (LeftMouse, RightMouse, Key1~4)

Skill.Common.*                 # 공통 스킬 (Jump, Dodge, Sprint)
Skill.Warrior.*                # 전사 스킬 (SharpStrike, CuttingSmash, GroundSlam, ...)
Skill.Archer.*                 # 궁수 스킬 (Snipe, RapidShot, PiercingArrow, ...)
Skill.Assassin.*               # 암살자 스킬 (QuickSlash, Ambush, ShadowAssault, ...)
Cooldown.Skill.*               # 스킬 쿨다운 태그

State.Movement.*               # 이동 상태 (Dodge, Sprint, Jump, Locked)
State.Skill.*                  # 스킬 활성 상태 (Active, 직업별.스킬별)
State.Player.*                 # 플레이어 상태 (Damage, Dead)
State.Enemy.*                  # 적 상태 (Dead, Returning, Groggy)
State.Boss.*                   # 보스 상태 (Phase_1~3, Groggy, Dead)

Event.Player.*                 # 플레이어 이벤트 (Damage, Death)
Event.Combo.*                  # 콤보 이벤트 (InputWindow, Branch, InputRequest)
Event.Attack.*                 # 공격 이벤트 (Hit, HitCheck, HitWindow)
Event.Skill.*                  # 스킬 이벤트 (ChainStep, ChainInput, VFX, SFX)
Event.Movement.*               # 이동 이벤트 (Jump_Landed, Skill_Unlock, Skill_CancelByMove, ...)
Event.Input.*                  # 입력 이벤트 (직업별 스킬 탭)
Event.Boss.*                   # 보스 이벤트 (Groggy, Indicator, SkillBranch, PhaseApply)
Event.Enemy.*                  # 적 이벤트 (AOE_Telegraph, Groggy)

Boss.Kashapa.*                 # 카샤파 보스 스킬 (Phase1.Attack/Skill, Phase2.Skill, PhaseTransition)

GameplayCue.Weapon.*           # 무기 VFX (Blade Trail)
GameplayCue.Skill.*            # 스킬 VFX/SFX (Cast, Hit, Impact)

Data.*                         # SetByCaller 데이터 (Damage, BaseDamage, DamageMultiplier, GroggyDamage, MentalCost, Cooldown)
Block.Movement.*               # 이동 차단 (Jump, Dodge, Sprint)
```

---

## 설계 원칙

- **데이터 주도(Data-Driven)**: 스킬 정의, 속성 초기값, 아이템 스탯, 보스 페이즈 데이터를 모두 DataAsset/DataTable로 분리하여 코드 수정 없이 밸런싱 가능
- **컴포넌트 분리**: Combat, Inventory, LockOn, LootDrop을 독립 컴포넌트로 분리하여 관심사 격리
- **GAS 정석 패턴**: PlayerState ASC 호스팅, GameplayTag 라우팅, AttributeSet 값 클램핑, ExecutionCalculation 기반 데미지 적용
- **서버 권한 우선**: 데미지 연산·아이템 드롭·풀 관리 등 핵심 로직은 서버에서만 실행, 클라이언트는 결과만 수신
- **오브젝트 풀링**: 대량 스폰 환경에서 GC 부담을 최소화하기 위해 Subsystem 기반 풀 적용
- **WidgetController 패턴**: UI를 데이터 레이어와 표현 레이어로 명확히 분리

---

## 빌드

```bash
# Unreal Engine 5 설치 필요
# Visual Studio 2022 또는 Rider 권장
# .uproject 파일을 엔진에서 열어 빌드
# Dedicated Server 빌드: ProjectDGServer.Target.cs 사용
```

---

## 기획서 & 개발 문서

프로젝트의 세부 수치 설계 및 스펙은 `Readme/` 디렉터리에 기획서 문서로 정리되어 있습니다.

| 문서 | 내용 |
|---|---|
| [프로젝트 DG 기획서](Readme/프로젝트%20DG%20기획서.md) | 프로젝트 전체 비전, 세계관, 핵심 게임플레이 설계 |
| [DG 시스템 기획서](Readme/ProjectDG_시스템_기획서.md) | GAS 연동 로직, 클래스별 스탯 스케일링, 인벤토리 MVC 아키텍처 |
| [DG 캐릭터 기획서](Readme/DG_캐릭터%20기획서.md) | 직업별 스킬 상세, 스탯 계수, 콤보 설계 |
| [DG 아이템 기획서](Readme/DG_아이템%20기획서.md) | 등급별 색상, 주/부스탯 목록, 제작/강화/재설정 시스템 |
| [DG 컨텐츠 기획서](Readme/ProjectDG_컨텐츠_기획서.md) | 필드 구성, 던전 설계, 보스 공략 |
| [필드몬스터 스폰 시스템](Readme/필드몬스터%20스폰%20시스템.md) | 오브젝트 풀링 라이프사이클, 멀티캐스트 RPC 동기화, 트러블슈팅 |
| [WBS](Readme/ProjectDG_WBS.md) | 작업 분류 체계 |
