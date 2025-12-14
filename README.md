# Network Omok Game Server (C / Socket / pthread / Linux)

<img src="https://github.com/user-attachments/assets/1a2c993b-3a45-4ed9-8a74-5bb030142e7a" style="width:499px; height:325px;" alt="스크린샷 1" />
<img src="https://github.com/user-attachments/assets/c8c65227-35aa-42cf-b182-89519c7a1e69" style="width:499px; height:325px;" alt="스크린샷 2" />

## Overview
This project is a multiplayer Omok (Gomoku) game server implemented in C using
TCP sockets and POSIX threads, and the ncurses library for terminal-based display.

The server runs on Linux and supports multiple rooms, where each room allows exactly two players to play a turn-based Omok game on a shared board.
The main focus of this project is server-side concurrency control and game-state consistency.

This repository contains the server-side implementation of a network-based Omok game.

## Features
- TCP socket-based server
- Blocking I/O based server design
- Client management using a single linked list
- Room-based game management (2 players per room)
- Turn-based Omok gameplay (15x15 board)
- Win detection (5 stones in a row)
- Thread-per-client architecture
- Room-level mutex synchronization
- Safe handling of shared game state under concurrency

## Architecture
- Each connected client is handled by a dedicated server thread.
- The server maintains: 
    - A global client list
    - An array of game rooms
- Each room contains:
    - Game board state
    - Room state (WAIT, START)
    - Player slot information
    - A dedicated (pthread_mutex_t)

## Concurrency Design (Core Concept)
To prevent race conditions during gameplay, each room owns its own mutex.

### Protected by Room Mutex
- Board updates (room->board)
- Turn switching (PlayerView.turn)
- Room state transitions (STATE_WAIT ↔ STATE_START)

### Design Decisions
- The mutex is locked only while modifying shared game state.
- Network I/O operations such as send() are executed outside the critical section to avoid blocking other threads.

This approach prevents:
- Simultaneous moves by multiple clients
- Corrupted turn state
- Inconsistent board updates

## Game Flow (Server Side)
1. Two clients join the same room.
2. Both players toggle READY.
3. The room state transitions to STATE_START.
4. Players alternately send MOVE y x commands.
5. The server validates moves and broadcasts results.
6. When a win is detected:
    - The room state is reset
    - The board is cleared
    - Both players return to the waiting state

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
- If a player quits (q) during an active game, session cleanup is not fully isolated.
- Some room reset scenarios require additional client-side coordination.
- These limitations were left intentionally to keep the focus on
synchronization correctness and server stability.

## What I Learned
- Designing room-level synchronization with pthread mutexes
- Identifying race conditions in turn-based network games
- Minimizing critical sections in multi-threaded servers
- Separating game logic from network I/O
- Managing shared state safely across multiple client threads
