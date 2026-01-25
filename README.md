# Network Omok Game Server (C / Socket / pthread / Linux)

<img src="https://github.com/user-attachments/assets/1a2c993b-3a45-4ed9-8a74-5bb030142e7a" style="width:499px; height:325px;" alt="스크린샷 1" />
<img src="https://github.com/user-attachments/assets/c8c65227-35aa-42cf-b182-89519c7a1e69" style="width:499px; height:325px;" alt="스크린샷 2" />

## Overview
이 프로젝트는 C 언어로 구현된 멀티플레이어 오목(Omok, Gomoku) 게임 서버입니다.  
TCP 소켓과 POSIX 스레드(pthread), 그리고 터미널 기반 화면 출력을 위한 ncurses 라이브러리를 사용했습니다.

서버는 Linux 환경에서 실행되며, 여러 개의 방(Room)을 지원합니다.  
각 방에는 정확히 두 명의 플레이어가 참여할 수 있고, 공유된 보드에서 턴 기반 오목 게임을 진행합니다.  
이 프로젝트의 핵심 목적은 **서버 사이드 동시성 제어와 게임 상태 일관성 유지**입니다.

이 저장소에는 네트워크 기반 오목 게임의 **서버 사이드 구현**만 포함되어 있습니다.

## Features
- TCP 소켓 기반 서버
- Blocking I/O 기반 서버 설계
- 단일 연결 리스트를 이용한 클라이언트 관리
- 방(Room) 기반 게임 관리 (방당 2명)
- 턴 기반 오목 게임 플레이 (15x15 보드)
- 승리 판정 (5목)
- Thread-per-client 아키텍처
- 방 단위 mutex 동기화
- 동시성 환경에서의 안전한 게임 상태 관리

## Architecture
- 각 클라이언트 연결은 전용 서버 스레드에서 처리됩니다.
- 서버는 다음을 관리합니다:
    - 전역 클라이언트 리스트
    - 게임 방 배열
- 각 방(Room)은 다음을 포함합니다:
    - 게임 보드 상태
    - 방 상태 (WAIT, START)
    - 플레이어 슬롯 정보
    - 전용 mutex (pthread_mutex_t)

## Concurrency Design (Core Concept)
게임 진행 중 발생할 수 있는 레이스 컨디션을 방지하기 위해  
각 방(Room)은 자체 mutex를 소유합니다.

### Room Mutex로 보호되는 영역
- 보드 상태 업데이트 (room->board)
- 턴 전환 (PlayerView.turn)
- 방 상태 전이 (STATE_WAIT ↔ STATE_START)

### Design Decisions
- mutex는 **공유 게임 상태를 수정하는 동안에만** 잠급니다.
- send()와 같은 네트워크 I/O 작업은  
  다른 스레드의 진행을 막지 않도록 **임계 구역 밖에서 실행**합니다.

이를 통해 다음 문제를 방지합니다:
- 여러 클라이언트의 동시 착수
- 턴 상태 꼬임
- 보드 상태 불일치

## Game Flow (Server Side)
1. 두 명의 클라이언트가 같은 방에 입장합니다.
2. 두 플레이어 모두 READY 상태를 토글합니다.
3. 방 상태가 STATE_START로 전환됩니다.
4. 플레이어는 번갈아가며 `MOVE y x` 명령을 전송합니다.
5. 서버는 착수를 검증하고 결과를 브로드캐스트합니다.
6. 승리가 판정되면:
    - 방 상태를 초기화하고
    - 보드를 초기화한 뒤
    - 두 플레이어 모두 대기 상태로 돌아갑니다.

## Protocol
- READY
- START
- MOVE y x
- MOVE y x stone
- YOURTURN
- WIN
- LOSE
- ERR NOT YOUR TURN
- ERR NOT STARTED

## Known Limitations
- 게임 진행 중 플레이어가 종료(q)할 경우,
  세션 정리가 완전히 분리되어 처리되지는 않습니다.
- 일부 방 초기화 시나리오는
  추가적인 클라이언트 측 협조가 필요합니다.
- 이러한 한계들은
  **동기화의 정확성과 서버 안정성에 집중하기 위해**
  의도적으로 남겨두었습니다.

## What I Learned
- pthread mutex를 활용한 방 단위 동기화 설계
- 턴 기반 네트워크 게임에서의 레이스 컨디션 식별
- 멀티스레드 서버에서 임계 구역 최소화
- 게임 로직과 네트워크 I/O의 분리
- 여러 클라이언트 스레드 간 공유 상태를 안전하게 관리하는 방법
