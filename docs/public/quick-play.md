# Quick Play — 키보드·트랙패드와 공유 복구코드

<!-- language-nav:start -->
[한국어](quick-play.md) | [English](../en/public/quick-play.md)
<!-- language-nav:end -->

[웹·PWA 실행](https://gelb2.github.io/HeavyMach/) · [Tauri Mac 다운로드](https://github.com/gelb2/HeavyMach/releases/tag/tauri-mac-20261102)

## 키보드와 트랙패드로 플레이

Mac 앱과 웹·PWA에서는 키보드와 트랙패드로 게임을 플레이할 수 있다. 키보드로 이동·점프·무기를 조작하고, 트랙패드로 전장의 공격할 지점을 클릭해 조준·사격한다. 클릭을 유지하면 무기의 연사 간격에 따라 계속 발사한다. 키보드와 트랙패드를 연결한 iPad에서도 함께 사용할 수 있다.

| 기본 키·입력 | 동작 |
|---|---|
| A / D | 후진 / 전진 |
| Space | 점프 |
| 트랙패드 클릭·유지 | 수동 모드에서 해당 지점으로 조준·사격 |
| 방향키 | 수동 조준 방향 조절 |
| F 누르기·유지 | 사격 |
| Q / E | 이전 / 다음 무기 |
| Shift | 세밀한 이동·조준 |
| R | 자동조준 표적 전환 |
| Esc | 일시정지·재개, 메뉴에서 뒤로 이동 |
| Tab / Shift+Tab, Enter | 메뉴 항목 이동, 선택 |
| F2 | 키 설정 |
| F3 | 세이브 슬롯 화면에서 복원 코드 창 열기 |
| ⌘Q | Tauri Mac 앱 종료 |

자동조준은 화면의 **A 버튼**을 클릭해 켜고 끈다. 키보드 **A 키는 후진**이다. 자동조준 중에는 **F**를 누르거나 화면의 조준경 모양 발사 버튼을 클릭해 사격한다. 전장 클릭으로 표적을 지정하거나 발사하지 않는다.

Mac에서 F2·F3이 밝기 조절이나 시스템 기능으로 동작하면 **Fn 또는 🌐 키를 함께 누른다**. [Apple 기능 키 안내](https://support.apple.com/ko-kr/102439).

키를 바꾸려면 **F2 → 위아래 키로 동작 선택 → Enter → 새 키 입력** 순서로 진행한다. 이미 쓰는 키는 서로 교환한다. Backspace는 기본 배치를 복원한다. [전체 조작법](gameplay-guide.md).

## Mac 첫 실행

Apple Silicon Mac과 macOS 14 이상에서 실행한다. ZIP을 풀고 **Heavy Mach Desktop.app**을 응용 프로그램(Applications) 폴더로 옮겨 연다.

현재 배포본은 Apple 공증을 받지 않아 처음 열 때 macOS가 차단할 수 있다.

1. 앱을 한 번 실행한다.
2. **시스템 설정 → 개인정보 보호 및 보안**으로 이동한다.
3. 아래쪽 Heavy Mach Desktop 안내에서 **확인 없이 열기(Open Anyway)**를 누른다.
4. 이어지는 창에서 **열기**를 누른다.

[Apple 첫 실행 안내](https://support.apple.com/102445).

## 레벨업이 힘든 사람을 위한 방법

내 아이폰에서 직접 플레이하며 레벨업한 저장 데이터의 복구코드를 공유한다. 이 코드를 입력하면 레벨업 과정을 상당 부분 건너뛰고, 여러 무기와 최종 단계 부품을 갖춘 탱크로 플레이할 수 있다.

| 공유 저장 | 값 |
|---|---|
| 슬롯 | 2번; 1번·3번은 비어 있음 |
| 레벨 | 19 |
| 경험치 | 221,706 |
| 저장된 미션 | 7-6 |
| 보유 무기 | 14종; 포탄·미사일·레이저 공중 지원 포함 |
| 이동·방어·공격 부품 | 모두 6단계 |

무기와 잔탄, 부품 단계, 미션 진행은 아이폰 저장값 그대로다. 무한 탄약이나 무적 기능은 없다. 코드를 적용한 뒤에는 일반 게임의 규칙으로 진행한다.

### 복구코드 적용

1. 게임의 세이브 슬롯 화면에서 **TRANSFER PROGRESS**를 누른다. 먼저 **CREATE CODE → COPY**로 자신의 진행도를 복사해 따로 보관한다.
2. 아래 공유 코드를 복사해 입력란에 붙여넣고 **CHECK CODE**를 누른다.
3. 웹·PWA·Tauri·Android에서 다른 슬롯을 보존하려면 **FROM 2**와 원하는 **TO** 슬롯을 선택한다. **SETTINGS OFF**는 현재 HUD·키·조준 설정을 유지한다.
4. **RESTORE**를 누르고 표시된 슬롯·레벨을 확인한 뒤 다시 **RESTORE**를 누른다.
5. 복원한 슬롯으로 게임을 시작한다. 전체 복원을 선택했다면 **2번 슬롯**을 고른다.

**iPhone·iPad·네이티브 Mac은 세 슬롯과 HUD·키·조준 설정을 모두 교체한다.** 전체 복원에서는 빈 1번·3번 슬롯도 적용된다. 웹 계열은 복원 후 시작 화면으로 돌아오므로 **PLAY**를 눌러 다시 들어간다. [플랫폼별 복원 범위](save-recovery-20261102.md).

[복구코드 텍스트 파일](saves/iphone-level19.txt)

```text
HM1-SE0BAdUOAAAHumJr_wD_AP8A_wD_AP8A_wD_AP8AtwA-pBvcas0m42oBAAAABgAAAAUAAAAiAAAASAMAAEgDAAAKYgMAxiYWPxIAAAASBgYGDgAAAFBfV3BuX0d1bjAxjAASYwAAAFBfV3BuX1JhZGVyQm9tYogAER4AAABQX1dwbl9SYWRlck1zbIkAER4AAABQX1dwbl9SYWRlckxzcokAEh0AAABQX1dwbl9TaG90Z3VuMDKIABJPAAAAUF9XcG5fU2hvdGd1bjAxiAAQcQAAAFBfV3BuX0Nhbm9uMDKKABI8AAAAUF9XcG5fTWlzc2lsZTAyiAAONgAAAFBfV3BuX0xhc2VyjAAQPAAAAFBfV3BuX0Nhbm9uMDOKABI2AAAAUF9XcG5fTWlzc2lsZTAziAAQOgAAAFBfV3BuX0VsU2hvY2uKABKSAAAAUF9XcG5fTWlzc2lsZTAxiAAQOwAAAFBfV3BuX0Nhbm9uMDGKAAA0_wD_AP8A-gAAFv8A_wD_AP8A_wD_AP8A_wD_AP8A_wC6ACsCAQQABwBQAE8AUgBRAAkALAAUAAgA4QAoACkAKwA7ABUACgF1hwZPw57HP4UAAvA_AY0AAvA_AY8ACAGkkUoHiHLCP4cACAGS3iVMrJ7VP4cAAAGFAAHwP4cAAAGHABD2cA_3stvlPwHkPB-2n37hP4UAA_A_AAA
```

2026-11-02 아이폰 저장 백업을 기준으로 만들었다. 코드는 모든 플랫폼에서 같은 게임 진행값을 사용한다.
