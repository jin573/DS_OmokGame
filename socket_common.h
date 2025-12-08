#ifndef SOCKET_COMMON_H
#define SOCKET_COMMON_H

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <errno.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
#include <pthread.h>
#include <ctype.h>   // isdigit, isalnum, toupper 사용
#include <termios.h>
#include <sys/select.h>
#include <time.h>

// socket_server
typedef int SOCKET;
#define SOCKET_ERROR   -1
#define INVALID_SOCKET -1

// socket_error message with exit
static inline void err_quit(const char* msg){
    char* msgbuf = strerror(errno);
    printf("[%s] %s\n", msg, msgbuf);
    exit(1);
}

// socket_error message
static inline void err_display_msg(const char* msg){
    char* msgbuf = strerror(errno);
    printf("[%s] %s\n", msg, msgbuf);
}

// socket_error
static inline void err_display_err_code(int errorcode){
    char* msgbuf = strerror(errorcode);
    printf("[error] %s\n", msgbuf);
}

// struct RoomInfo
#define MAX_CLIENT 2
#define MAX_ROOM   10
#define B_SIZE 15

enum RoomState{
    STATE_WAIT,
    STATE_START
};

// 플레이어 구조체
#ifndef MAX_NICK
#define MAX_NICK 32   // 닉네임 최대길이
#endif

// 플레이어의 돌 색깔
enum StoneColor {
    STONE_NONE  = 0,
    STONE_BLACK = 1,
    STONE_WHITE = 2
};

// 플레이어 준비 상태
enum ReadyState {
    READY_NOT = -1,
    READY_NO  = 0,
    READY_YES = 1
};

//info room
typedef struct RoomInfo{
    int room_id;
    int count_client;
    enum RoomState room_state;
    int client_info[MAX_CLIENT];
    enum StoneColor board[B_SIZE][B_SIZE];
} RoomInfo;

static inline void reorder_room_clients(RoomInfo* room){
	int reorder_arr[MAX_CLIENT] = {-1, -1};
	int idx = 0;

	for(int i=0; i<MAX_CLIENT; i++){
		if(room->client_info[i] != -1){
			reorder_arr[idx++] = room->client_info[i];
		}
	}

	room->client_info[0] = reorder_arr[0];
	room->client_info[1] = reorder_arr[1];
}

// 플레이어 정보
typedef struct PlayerView {
    int  client_id;                 // client socket fd
    char nick[MAX_NICK];            // 닉네임
    int  room_id;                   // 방 번호
    int  seat;                      // 0: 흑(선공), 1: 백(후공)
    enum ReadyState ready;          // 준비 상태
    enum StoneColor stone;          // 돌 색
	int turn; //in game
} PlayerView;

static inline const char* room_state_str(enum RoomState s){
    switch (s) {
        case STATE_WAIT:  return "WAIT";
        case STATE_START: return "START";
        default:          return "UNKNOWN";
    }
}
static inline const char* ready_state_str(enum ReadyState s){
    switch(s){
        case READY_YES: return "READY_YES";
        case READY_NO:  return "READY_NO";
        default:        return "READY_NOT";
    }
}

// 닉네임 문자열 입력 끝 엔터 제거
static inline void trim_newline(char* s){
    if (!s) return;
    s[strcspn(s, "\r\n")] = 0;
}

// client -> server 한 줄 수신 (개행 포함)
static int recv_line(int sock, char* buf, size_t cap){
    size_t pos = 0;
    char c;
    while (pos + 1 < cap){
        int n = recv(sock, &c, 1, 0);
        if (n <= 0) return n;   // 끊김/에러
        buf[pos++] = c;
        if (c == '\n') break;
    }
    buf[pos] = 0;
    return (int)pos;
}

// client 리스트
typedef struct ClientNode{
    struct ClientNode* next;
    PlayerView*  data;   // client data
} ClientNode;

typedef struct ClientList{
    ClientNode* head;
    int         count;
} ClientList;

// thread 인자
typedef struct {
    int                 client_sock;
    struct sockaddr_in  client;
    ClientList         *client_list;
} ThreadArg;

extern PlayerView g_player;
#endif

