#include "socket_common.h"
#include "game_server.h"

extern ClientList client_list;
extern struct RoomInfo rooms[MAX_ROOM];
#define B_SIZE 15

void handle_game(int client_sock, PlayerView* client_info, const char* msg) {
    if(client_info->room_id < 0){
        send(client_sock, "ERR NOT IN ROOM\n", 16, 0);
        return;
    }
    
    struct RoomInfo* room = &rooms[client_info->room_id];

    if(room->room_state != STATE_START){
        send(client_sock, "ERR NOT STARTED\n", 16, 0);
        return;
    }

    if(!client_info->turn){
        send(client_sock, "ERR NOT YOUR TURN\n", 18, 0);
        return;
    }

    int y, x;
    if(sscanf(msg, "MOVE %d %d", &y, &x) != 2){
        send(client_sock, "ERR BAD FORMAT\n", 15, 0);
        return;
    }

    if(y<0 || y>=B_SIZE || x<0 || x>=B_SIZE){
        send(client_sock, "ERR OUT OF RANGE\n", 17, 0);
        return;
    }
    if(room->board[y][x] != STONE_NONE){
        send(client_sock, "ERR OCCUPIED\n", 13, 0);
        return;
    }

    room->board[y][x] = client_info->stone;
    
	client_info->turn = 0;
    PlayerView* opponent = NULL;
	
	for(int i=0;i<MAX_CLIENT;i++){
        int cid = room->client_info[i];
        if(cid != -1 && cid != client_info->client_id){
            opponent = search_client(&client_list, cid);
            if(opponent) opponent->turn = 1;
        }
    }

    char buf[64];
    snprintf(buf, sizeof(buf), "MOVE %d %d\n", y, x);
    for(int i=0;i<MAX_CLIENT;i++){
        int cid = room->client_info[i];
        if(cid != -1){
            PlayerView* p = search_client(&client_list, cid);
            if(p) send(p->client_id, buf, strlen(buf), 0);
        }
    }

    printf("[Handler][MOVE] Client %s (%d) -> %d,%d\n",
           client_info->nick, client_info->client_id, y, x);
}

