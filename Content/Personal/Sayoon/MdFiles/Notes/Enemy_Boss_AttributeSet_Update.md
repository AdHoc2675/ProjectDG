# 적 및 보스 전용 AttributeSet 추가 내역 (요약)

## 1. 개요
모든 스탯이 하나의 클래스에 몰려있는 문제를 해결하기 위해, 적(Enemy)과 보스(Boss)에게만 필요한 전용 스탯들을 독립된 모듈(`AttributeSet`)로 분리하여 추가했습니다.

---

## 2. DG_EnemyAttributeSet (적 공통)
일반 몬스터와 보스 몬스터 모두에게 기본적으로 부착되는 전투/AI 스탯입니다.

* **추가된 스탯**:
  * `StaggerGauge` / `MaxStaggerGauge` (그로기 무력화 게이지)
  * `AttackSpeed` (공격 속도)
  * `MovementSpeed` (이동 속도)
  * `DetectionRadius` (어그로 인지 반경)
  * `ProvideExp` (처치 시 제공 경험치)
* **주요 특징**:
  * 데미지를 입어 `StaggerGauge`가 꽉 차면, 코드를 통해 자동으로 그로기 상태 태그(`TAG_Enemy_State_Groggy`)를 부여하도록 로직이 세팅되어 있습니다.

---

## 3. DG_BossAttributeSet (보스 전용)
보스 몬스터만이 가지는 특수한 패턴과 페이즈 변화를 처리하기 위한 스탯입니다.

* **추가된 스탯**:
  * `CurrentPhase` (현재 페이즈 번호)
  * `PhaseThreshold` (다음 페이즈로 넘어가는 체력 임계점)
  * `RageGauge` / `MaxRageGauge` (광폭화/분노 게이지)
  * `DamageReduction` (무적 패턴 등에 쓰일 피해 감소율 퍼센트)
* **주요 특징**:
  * 전투가 길어져 `RageGauge`가 꽉 차면, 코드를 통해 자동으로 광폭화 상태 태그(`TAG_Boss_State_Rage`)를 부여하도록 로직이 세팅되어 있습니다.

---

## 4. 핵심 룰 (조립 및 태그 갱신)
* **조립식 설계**: 몬스터를 만들 때 상속이 아닌 **'컴포넌트 부착'** 방식으로 조립해야 합니다.
  * 일반 적 = `DG_AttributeSet`(체력/공격력) + `DG_EnemyAttributeSet`(그로기/어그로)
  * 보스 적 = `DG_AttributeSet` + `DG_EnemyAttributeSet` + `DG_BossAttributeSet`(페이즈/분노)
* **독립적 로직**: `PostGameplayEffectExecute` 함수는 **자기가 가진 스탯이 변할 때만 발동**합니다. 따라서 체력 태그는 `DG_AttributeSet`에서, 그로기 태그는 `DG_EnemyAttributeSet`에서 각각 따로 처리되도록 분산 구현되었습니다.
