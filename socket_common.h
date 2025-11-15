#ifndef SOCKET_COMMON_H
#define SOCKET_COMMON_H

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
#include <pthread.h>

//socket_server
typedef int SOCKET;
#define SOCKET_ERROR -1
#define INVALID_SOCKET -1

//socket_error message with exit
static inline void err_quit(const char* msg){
	char* msgbuf = strerror(errno);
	printf("[%s] %s\n", msg, msgbuf);
	exit(1);
}

//socket_error message
static inline void err_display_msg(const char* msg){
	char* msgbuf = strerror(errno);
	printf("[%s] %s\n", msg, msgbuf);
}

//socket_error
static inline void err_display_err_code(int errorcode){
	char* msgbuf = strerror(errorcode);
	printf("[error] %s\n", msgbuf);
}

//struct RoomInfo
#define MAX_CLIENT 2
#define MAX_ROOM 10

enum RoomState{
	STATE_WAIT,
//	STATE_READY,
	STATE_START
};

typedef struct RoomInfo{
	int room_id;
	int count_client;
	enum RoomState room_state; 
	int client_info[MAX_CLIENT];
}RoomInfo;

// 플레이어 구조체
#ifndef MAX_NICK
#define MAX_NICK 32        // 닉네임 최대길이
#endif

// 플레이어의 돌 색깔
enum StoneColor { 
		STONE_NONE=0, 
		STONE_BLACK=1, 
		STONE_WHITE=2 
};

// 플레이어 준비 상태
enum ReadyState { 
	READY_NOT=-1,
	READY_NO=0, 
    READY_YES=1 
 };

// 플레이어 정보
typedef struct PlayerView {
	int client_id; //client_id
	char nick[MAX_NICK];     // 닉네임
    int  room_id;            // 방 번호
    int  seat;               // 0번자리: 1번 플레이어(흑돌, 선공) 1번자리: 2번 플레이어 (백돌, 후공)
    enum ReadyState ready;   // 준비 상태
    enum StoneColor stone;   // 돌 색
}PlayerView;

static inline const char* room_state_str(enum RoomState s){
    switch (s) {
        case STATE_WAIT:  return "WAIT"; // 대기
//        case STATE_READY: return "READY"; // 준비 완료
        case STATE_START: return "START"; // 게임 중
        default:          return "UNKNOWN";
    }
}

// 닉네임 문자열 입력 끝 엔터 제거
static inline void trim_newline(char* s){
    if (!s) return;
    s[strcspn(s, "\r\n")] = 0;
}
//client -> server nickname
static int recv_line(int sock, char* buf, size_t cap){
    size_t pos = 0; char c;
    while (pos + 1 < cap){
        int n = recv(sock, &c, 1, 0);
        if (n <= 0) return n;   // 끊김/에러
        buf[pos++] = c;
        if (c == '\n') break;
    }
    buf[pos] = 0;
    return (int)pos;
}

//client
typedef struct ClientNode{
	struct ClientNode* next; 
	struct PlayerView data; //client data
}ClientNode;

typedef struct ClientList{
	ClientNode* head;
	int count;
}ClientList;

//thread
typedef struct {
    int client_sock;
    struct sockaddr_in client;
    ClientList *client_list;
} ThreadArg;

#endif

