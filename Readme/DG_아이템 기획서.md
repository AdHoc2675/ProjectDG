**DG Item System Design Document**

**아이템 시스템 기획서 v0.1**

# **1\. 아이템 시스템 개요**

DG의 아이템 시스템은 보스전과 필드 사냥을 반복하는 파밍 루프를 장비 성장으로 연결하는 구조이다. 

장비는 필드 드랍이 아닌 제작을 통해 획득하며, 이후 강화와 재설정을 통해 졸업 장비를 완성한다.

| 핵심 루프 | 역할 |
| ----- | ----- |
| 제작 | 좋은 보조옵션 조합과 높은 기본 수치를 가진 장비 확보 |
| 강화 | 보조옵션 중 일부에 강화 수치를 누적하여 장비 성능 상승 |
| 재설정 | 강화 분배를 다시 굴려 원하는 옵션 재설정을 통해 엔드파밍을 노림 |

| 핵심 규칙 장비는 드랍으로 직접 획득하지 않고 제작으로만 획득한다. 보스 재료는 장비 제작에 사용하고, 일반 몬스터 재료는 장비 강화에 사용한다. 재설정은 보조옵션의 종류와 기본 수치를 유지한 채 강화 분배만 다시 굴린다. |
| :---- |

# **2\. 장비 구조**

| 구분 | 내용 |
| ----- | ----- |
| 장비 부위 | 총 6부위 |
| 무기 | 1부위 / 공격력 확보 중심 |
| 방어구 | 5부위 / 방어력 확보 중심 |
| 외형 변경 | 현재 단계에서는 없음 (등급별 아이콘,배경색만 변경) |
| 표시 요소 | 아이콘, 능력치, 등급, 보조옵션, 강화 상태 |

	

# **3\. 장비 등급**

장비 등급은 총 3단계로 구성한다. 등급은 장비의 보조스탯 품질 범위에 영향을 준다.

| 등급 | 색상 | 역할 |
| ----- | ----- | ----- |
| 영웅 | 보라색 (\#271435) | 초기 고급 장비 구간 |
| 전설 | 주황색 (\#882f02) | 중후반 핵심 장비 구간 |
| 고대 | 흰회색 (\#a28b5f) | 최종 파밍 목표 장비 구간 |

# **4\. 아이템 레벨 시스템**

DG는 요구 레벨과 아이템 레벨을 분리한다. 요구 레벨은 착용 조건을 결정하고, 아이템 레벨은 장비의 실제 능력치 계산에 사용한다.

| 제작 캐릭터 레벨 | 생성 장비 |
| ----- | ----- |
| Lv.21 | Item Level 21 장비 |
| Lv.25 | Item Level 25 장비 |
| Lv.30 | Item Level 30 장비 |

# **5\. 주스탯 구조**

주스탯은 장비 레벨에 따라 자동 계산되며, 제작 시 고정 생성된다. 강화와 재설정으로 주스탯은 변경되지 않는다.

| 부위 | 주요 구성 | 설계 의도 |
| ----- | ----- | ----- |
| 무기 | 주스탯 \+ 체력 \+ 공격력 | 플레이어 화력 성장의 핵심 수단 |
| 방어구 | 주스탯 \+ 체력 \+ 방어력 | 생존력과 피해 감소 성장의 핵심 수단 |

| 주스탯 규칙 모든 장비 부위에는 체력 스탯이 포함된다. 무기는 공격력 확보 수단으로 설계한다. 방어구는 방어력 확보 수단으로 설계한다. 공격력은 전투 계산식에서 영향력을 높게 가져가며, 실질적인 화력 성장 대부분은 무기를 통해 확보한다. |
| :---- |

# **6\. 보조스탯 시스템**

보조스탯은 장비 제작 시 생성된다. 한 장비 안에서 동일 보조스탯은 중복 등장하지 않는다.

| 보조스탯 목록 |
| ----- |
| 치명타 확률 |
| 치명타 피해 |
| 이동속도 |
| 공격속도 |
| 그로기 피해 증가 |
| 그로기 시 피해 증가 |
| 스킬 재사용 시간 감소 |
| 정신력 회복량 증가 |
| 생명력 흡수 |
| 적중 시 체력 회복 |
| 보스 피해 증가 |
| 상태이상 지속시간 증가 |
| 최대 정신력 증가 |
| 최종 피해 증가 |
| 받는 피해 감소 |
| 스테미나 회복량 증가 |
| 상태이상 강화 |

# **7\. 보조스탯 품질 구조**

보조스탯은 제작 시 기본 수치가 결정되며 이후 변경되지 않는다. 랜덤 범위는 지나치게 넓게 잡지 않고, 하급/중급/상급 품질 구간으로 구분한다. 단 실제 장비에 등급으로 구분하지는 않는다.

| 옵션 | 기본 수치 범위 예시 | 품질 예시 |
| ----- | ----- | ----- |
| 치명타 확률 | 9% \~ 11% | 9% 하급 / 10% 중급 / 11% 상급 |
| 치명타 피해 | 15% \~ 18% | 15% 하급 / 16\~17% 중급 / 18% 상급 |
| 공격속도 | 4% \~ 6% | 4% 하급 / 5% 중급 / 6% 상급 |

| 설계 의도 좋은 장비는 보조옵션 종류와 기본 수치 품질에서 먼저 결정된다. 재설정은 기본 수치를 바꾸지 않으므로, 유저는 좋은 기본 수치를 가진 장비를 먼저 제작해야 한다. 이 구조를 통해 반복적인 장비제작의 가치를 유지한다. |
| :---- |

# **8\. 제작 재료 시스템**

제작 재료는 월드 레벨에 따라 변경된다. 월드 레벨은 파티장 레벨에 귀속되어 동기화된다.

| 항목 | 내용 |
| ----- | ----- |
| 보스 처치 보상 | 제작 재료 3\~4개 드랍 |
| 3개 드랍 확률 | 50% |
| 4개 드랍 확률 | 50% |
| 장비 제작 비용 | 제작 재료 4개 |

| 레벨 구간 | 제작 재료 |
| ----- | ----- |
| 1\~10 | 가디언의 파편 |
| 11\~20 | 가디언의 결정 |
| 21\~30 | 가디언의 정수 |

# **9\. 강화 시스템**

강화는 일반 몬스터 처치로 획득한 재료를 사용한다. 

강화는 장비의 보조스탯 중 하나를 랜덤으로 선택하여 강화 수치를 누적하는 시스템이다.

| 항목 | 기획 |
| ----- | ----- |
| 강화 재료 획득처 | 일반 몬스터 |
| 강화 가능 횟수 | 총 4회 |
| 강화 대상 | 장비의 보조스탯 중 1줄 랜덤 강화 |
| 중복 강화 | 가능. 같은 옵션이 여러 번 강화될 수 있음 |
| 장비 이름 표기 | 강화 수치 미표기. 예: 장검 \+4 X / 장검 O |
| 강화 표시 위치 | 보조스탯 UI |

| 강화 누적 횟수 | 색상 | 표기 |
| ----- | ----- | ----- |
| 0회 | 흰색 | 없음 |
| 1회 | 파란색 | (+1) |
| 2회 | 노란색 | (+2) |
| 3회 | 보라색 | (+3) |
| 4회 | 빨간색 | (+4) |

# **10\. 강화 로직**

강화 시 보조스탯 중 1줄이 랜덤하게 선택되며, 옵션별 최소/최대 범위 내에서 강화 상승량이 결정된다. 

강화 회차가 높아져도 강화량 범위는 증가하지 않고, 재료 비용만 증가한다.

| 옵션 | 강화 상승 범위 예시 |
| ----- | ----- |
| 치명타 확률 | \+2.0% \~ \+3.0% |
| 치명타 피해 | \+4% \~ \+6% |
| 공격속도 | \+1% \~ \+2% |

| 강화 회차 | 재료 요구량 |
| ----- | ----- |
| 1회차 | 5개 |
| 2회차 | 10개 |
| 3회차 | 20개 |
| 4회차 | 40개 |

# **11\. 재설정 시스템**

재설정은 \+4 강화 완료 장비만 사용할 수 있는 후반 파밍 시스템이다. 재설정은 보조옵션 종류와 기본 수치는 유지한 채 강화 분배만 다시 굴린다.

| 요소 | 재설정 시 변경 여부 |
| ----- | ----- |
| 아이템 레벨 | 유지 |
| 주스탯 | 유지 |
| 보조옵션 종류 | 유지 |
| 보조옵션 기본 수치 | 유지 |
| 강화 분배 | 재설정 |

| 메모리얼 기능 재설정 후 기존 결과와 신규 결과 중 하나를 선택할 수 있다. 선택되지 않은 결과는 폐기된다. 재설정 아이템은 결과와 관계없이 소모된다. |
| :---- |

| 항목 | 기획 |
| ----- | ----- |
| 재설정 아이템 획득처 | 보스 처치 시 일정 확률 드랍 |
| 기본 드랍 확률 | 약 20% |
| 사용 대상 | \+4 강화 완료 장비 |

# **12\. 장비 성장 예시**

아래는 실제 유저가 장비 하나를 제작하고 강화한 뒤 재설정까지 진행하는 전체 흐름 예시이다.

| 단계 | 플레이 흐름 | 결과 |
| ----- | ----- | ----- |
| 1단계 | Lv.27 상태에서 보스 반복 처치 | 가디언의 정수 획득  |
| 2단계 | 정수4개로 대장장이에서 고대 무기 제작 | 고대 장검 / ItemLevel 27 생성 |
| 3단계 | 일반 몬스터 재료로 4회 강화 | 치명타 확률 \+1, 그로기 피해 증가 \+3 결과 |
| 4단계 | \+4 장비에 재설정 아이템 사용 | 강화 분배 재굴림 후 기존/신규 결과 선택 |

| 구분 | 수치 예시 |
| ----- | ----- |
| 주스탯 | 체력 \+820 / 공격력 \+148 |
| 보조스탯 | 치명타 확률 11% / 치명타 피해 18% / 공격속도 6% / 그로기 피해 증가 8% |
| 강화 후 결과 | 치명타 확률 13.4% (+1) / 그로기 피해 증가 17.1% (+3) |
| 재설정 신규 결과 | 치명타 확률 20.2% (+4) / 그로기 피해 증가 8% |

# **13\. 소모품 시스템**

현재 소모품은 힐링 물약 중심으로 구성한다. 소모품은 장비 파밍과 별도로 전투 지속력을 보조하는 역할을 가진다.

| 요소 | 구조 |
| ----- | ----- |
| 회복 방식 | 즉발 회복 |
| 전투 중 사용 제한 | 있음 |
| 재사용 대기시간 | 있음 |

# **14\. 데이터 관리 구조**

아이템 관련 수치는 DataTable 기반으로 관리한다. 레벨 확장, 장비 추가, 밸런스 조정 시 코드 수정 없이 데이터 변경만으로 대응할 수 있도록 구성한다.

# **전체 DataTable 목록**

| DT 이름 | 역할 |
| ----- | ----- |
| DT\_ItemGrade | 장비 등급, 색상, 보조옵션 개수 관리 |
| DT\_EquipmentMainStatByLevel | 아이템 레벨별 주스탯 / 체력 / 공격력 / 방어력 관리 |
| DT\_EquipmentRecipe | 장비 제작 레시피 및 요구 재료 관리 |
| DT\_ItemMaterial | 제작 / 강화 / 재설정 재료 정보 관리 |
| DT\_SubOptionDefinition | 보조스탯 종류와 표시 정보 관리 |
| DT\_SubOptionValueRange | 등급별 보조스탯 기본 수치 범위 관리 |
| DT\_EnhanceCost | 강화 회차별 비용 관리 |
| DT\_EnhanceValueRange | 보조스탯별 강화 상승량 범위 관리 |
| DT\_ConsumableItem | 힐링 물약 등 소모품 정보 관리 |

# **14.1. DT\_ItemGrade**

장비 등급의 표시 정보와 보조옵션 개수를 관리한다.

| 필드 | 설명 |
| ----- | ----- |
| GradeID | 등급 고유 ID |
| DisplayName | 게임 내 표시 이름 |
| ColorHex | 아이템 배경 또는 등급 표시 색상 |
| SubOptionCount | 해당 등급 장비의 보조옵션 줄 수 |

| RowName | DisplayName | ColorHex | SubOptionCount |
| ----- | ----- | ----- | ----- |
| Hero | 영웅 | \#271435 | 4 |
| Legendary | 전설 | \#882f02 | 4 |
| Ancient | 고대 | \#a28b5f | 4 |

# **14.2. DT\_EquipmentMainStatByLevel**

아이템 레벨별 주스탯을 관리한다. 아이템 레벨은 제작 시 캐릭터 레벨로 결정된다.

| 필드 | 설명 |
| ----- | ----- |
| ItemLevel | 장비 제작 시 적용되는 아이템 레벨 |
| MainStat | 공통 주스탯 |
| HP | 모든 장비 부위에 적용되는 체력 |
| WeaponAttack | 무기 장착 시 적용되는 공격력 |
| ArmorDefense | 방어구 장착 시 적용되는 방어력 |

| RowName | ItemLevel | MainStat | HP | WeaponAttack | ArmorDefense |
| ----- | ----- | ----- | ----- | ----- | ----- |
| Lv\_01 | 1 | 10 | 100 | 15 | 8 |
| Lv\_02 | 2 | 12 | 115 | 18 | 10 |
| Lv\_10 | 10 | 30 | 300 | 45 | 28 |
| Lv\_21 | 21 | 70 | 700 | 120 | 80 |
| Lv\_30 | 30 | 100 | 1000 | 180 | 120 |

| 부위 | 적용 스탯 |
| ----- | ----- |
| 무기 | MainStat \+ HP \+ WeaponAttack |
| 방어구 | MainStat \+ HP \+ ArmorDefense |

무기는 공격력 확보 수단, 방어구는 방어력 확보 수단으로 역할을 분리한다. 모든 부위에는 체력 스탯이 포함된다.

# **14.3. DT\_EquipmentRecipe**

장비 제작에 필요한 레시피와 요구 재료를 관리한다.

| 필드 | 설명 |
| ----- | ----- |
| RecipeID | 제작 레시피 ID |
| EquipmentType | Weapon / Armor |
| RequiredLevelMin | 해당 레시피 사용 가능 최소 레벨 |
| RequiredLevelMax | 해당 레시피 사용 가능 최대 레벨 |
| MaterialID | 소모 재료 ID |
| MaterialCount | 소모 재료 개수 |

| RowName | EquipmentType | LevelMin | LevelMax | MaterialID | Count |
| ----- | ----- | ----- | ----- | ----- | ----- |
| Weapon\_Tier\_01 | Weapon | 1 | 10 | M\_Craft\_T1 | 4 |
| Armor\_Tier\_01 | Armor | 1 | 10 | M\_Craft\_T1 | 4 |
| Weapon\_Tier\_02 | Weapon | 11 | 20 | M\_Craft\_T2 | 4 |
| Armor\_Tier\_02 | Armor | 11 | 20 | M\_Craft\_T2 | 4 |
| Weapon\_Tier\_03 | Weapon | 21 | 30 | M\_Craft\_T3 | 4 |
| Armor\_Tier\_03 | Armor | 21 | 30 | M\_Craft\_T3 | 4 |

# **14.4. DT\_ItemMaterial**

제작, 강화, 재설정에 사용되는 재료 아이템 정보를 관리한다.

| RowName | DisplayName | Type | Level | Description |
| ----- | ----- | ----- | ----- | ----- |
| M\_Craft\_T1 | 가디언의 파편 | Craft | 1\~10 | 초급 장비 제작 재료 |
| M\_Craft\_T2 | 가디언의 결정 | Craft | 11\~20 | 중급 장비 제작 재료 |
| M\_Craft\_T3 | 가디언의 정수 | Craft | 21\~30 | 상급 장비 제작 재료 |
| M\_Enhance | 강화 파편 | Enhance | 1\~30 | 장비 강화 재료 |
| M\_Reroll | 운명의 불씨 | Reroll | 1\~30 | \+4 장비 강화 분배 재설정 재료 |

# **14.5. DT\_SubOptionDefinition**

보조스탯의 종류, 표시 이름, 값 타입, 중복 허용 여부를 관리한다. 현재 기획 기준 모든 보조옵션은 중복 불가이다.

| RowName | DisplayName | ValueType | Duplicate |
| ----- | ----- | ----- | ----- |
| CritChance | 치명타 확률 | Percent | false |
| CritDamage | 치명타 피해 | Percent | false |
| MoveSpeed | 이동속도 | Percent | false |
| AttackSpeed | 공격속도 | Percent | false |
| GroggyDamage | 그로기 피해 증가 | Percent | false |
| GroggyDamageBonus | 그로기 시 피해 증가 | Percent | false |
| CooldownReduction | 스킬 재사용 시간 감소 | Percent | false |
| SpiritRecovery | 정신력 회복량 증가 | Percent | false |
| LifeSteal | 생명력 흡수 | Percent | false |
| HealOnHit | 적중 시 체력 회복 | Flat | false |
| BossDamage | 보스 피해 증가 | Percent | false |
| StatusDuration | 상태이상 지속시간 증가 | Percent | false |
| MaxSpirit | 최대 정신력 증가 | Percent | false |
| FinalDamage | 최종 피해 증가 | Percent | false |
| DamageReduction | 받는 피해 감소 | Percent | false |
| StaminaRecovery | 스테미나 회복량 증가 | Percent | false |
| StatusPower | 상태이상 강화 | Percent | false |

# **14.6. DT\_SubOptionValueRange**

장비 제작 시 보조스탯 기본 수치를 결정하는 등급별 범위표이다. 기본 수치는 제작 시 결정되며 강화와 재설정으로 변경되지 않는다.

| 필드 | 설명 |
| ----- | ----- |
| Grade | 장비 등급 |
| SubOptionID | 보조스탯 ID |
| LowValue | 하급 품질 수치 |
| MidValue | 중급 품질 수치 |
| HighValue | 상급 품질 수치 |

| RowName | Grade | SubOptionID | Low | Mid | High |
| ----- | ----- | ----- | ----- | ----- | ----- |
| Anc\_CritChance | Ancient | CritChance | 9.0 | 10.0 | 11.0 |
| Anc\_CritDamage | Ancient | CritDamage | 15.0 | 16.5 | 18.0 |
| Anc\_AttackSpeed | Ancient | AttackSpeed | 4.0 | 5.0 | 6.0 |
| Anc\_GroggyDamage | Ancient | GroggyDamage | 6.0 | 7.0 | 8.0 |

# **14.7. DT\_EnhanceCost**

강화 회차별 재료 비용을 관리한다. 강화 단계가 높아질수록 재료 요구량만 증가하며, 옵션 상승량 범위는 동일하다.

| RowName | EnhanceStep | MaterialID | MaterialCount |
| ----- | ----- | ----- | ----- |
| Step\_01 | 1 | M\_Enhance | 5 |
| Step\_02 | 2 | M\_Enhance | 10 |
| Step\_03 | 3 | M\_Enhance | 20 |
| Step\_04 | 4 | M\_Enhance | 40 |

# **14.8. DT\_EnhanceValueRange**

강화 시 보조스탯별 상승량 범위를 관리한다. 같은 옵션에 여러 번 강화가 붙더라도 매번 동일 범위 안에서 랜덤 상승한다.

| RowName | SubOptionID | MinIncrease | MaxIncrease |
| ----- | ----- | ----- | ----- |
| Enhance\_CritChance | CritChance | 2.0 | 3.0 |
| Enhance\_CritDamage | CritDamage | 4.0 | 6.0 |
| Enhance\_AttackSpeed | AttackSpeed | 1.0 | 2.0 |
| Enhance\_GroggyDamage | GroggyDamage | 2.0 | 3.5 |

# **14.9. DT\_ConsumableItem**

힐링 물약 등 소모품 정보를 관리한다. 현재 기획 기준 소모품은 즉발 회복, 전투 중 갯수 제한, 쿨타임을 가진다.

| RowName | DisplayName | Type | HealValue | Cooldown | CombatLimit |
| ----- | ----- | ----- | ----- | ----- | ----- |
| Potion\_Small | 하급 치유 물약 | InstantHeal | 500 | 15 | 3 |
| Potion\_Mid | 중급 치유 물약 | InstantHeal | 1200 | 15 | 3 |
| Potion\_High | 상급 치유 물약 | InstantHeal | 2500 | 15 | 3 |

# **14.10. 실제 제작 아이템 인스턴스 저장 구조**

DataTable은 기준 데이터이며, 실제 제작된 장비는 개별 인스턴스 데이터로 저장한다.

| 저장 항목 | 설명 |
| ----- | ----- |
| ItemUniqueID | 개별 아이템 고유 ID |
| EquipmentType | Weapon / Armor |
| Grade | Hero / Legendary / Ancient |
| ItemLevel | 제작 시 캐릭터 레벨 |
| MainStatValue | DT\_EquipmentMainStatByLevel 기준으로 생성된 주스탯 |
| HPValue | 레벨 기준 체력 |
| AttackValue | 무기일 때 적용되는 공격력 |
| DefenseValue | 방어구일 때 적용되는 방어력 |
| SubOptions\[4\] | 보조옵션 종류 및 기본 수치 |
| EnhanceAllocations\[4\] | 각 보조옵션에 붙은 강화 횟수 |
| EnhanceValues\[4\] | 각 보조옵션의 강화 상승 총합 |

예시  
ItemUniqueID: 102938  
Name: 고대 장검  
Grade: Ancient  
ItemLevel: 27  
EquipmentType: Weapon

MainStat: 75  
HP: 820  
Attack: 148

SubOptions:  
1\. CritChance / BaseValue 11.0 / EnhanceCount 4 / EnhanceTotal 9.2  
2\. CritDamage / BaseValue 18.0 / EnhanceCount 0 / EnhanceTotal 0  
3\. AttackSpeed / BaseValue 6.0 / EnhanceCount 0 / EnhanceTotal 0  
4\. GroggyDamage / BaseValue 8.0 / EnhanceCount 0 / EnhanceTotal 0

# **14.11. 제작 / 강화 / 재설정 시 DT 조회 흐름**

| 상황 | 조회 DT | 처리 내용 |
| ----- | ----- | ----- |
| 제작 | DT\_EquipmentRecipe | 캐릭터 레벨에 맞는 제작 레시피와 재료 확인 |
| 제작 | DT\_EquipmentMainStatByLevel | 아이템 레벨 기준 주스탯 / 체력 / 공격력 / 방어력 생성 |
| 제작 | DT\_ItemGrade | 장비 등급과 보조옵션 개수 확인 |
| 제작 | DT\_SubOptionDefinition | 중복 없이 보조옵션 종류 선택 |
| 제작 | DT\_SubOptionValueRange | 등급별 보조옵션 기본 수치 결정 |
| 강화 | DT\_EnhanceCost | 현재 강화 회차에 필요한 재료 비용 확인 |
| 강화 | DT\_EnhanceValueRange | 선택된 보조옵션의 강화 상승량 결정 |
| 재설정 | DT\_EnhanceValueRange | 기존 보조옵션을 유지한 채 강화 분배와 상승량 재생성 |

제작 시:  
레벨 → DT\_EquipmentMainStatByLevel 조회  
등급 → DT\_ItemGrade 조회  
재료 → DT\_EquipmentRecipe 조회  
보조옵션 → DT\_SubOptionDefinition \+ DT\_SubOptionValueRange 조회

강화 시:  
강화 회차 → DT\_EnhanceCost 조회  
선택된 옵션 → DT\_EnhanceValueRange 조회

재설정 시:  
기존 SubOption 유지  
EnhanceAllocation만 재굴림  
EnhanceValueRange 기준으로 강화 수치 재생성

