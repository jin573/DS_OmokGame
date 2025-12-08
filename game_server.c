#include "socket_common.h"
#include "game_server.h"

extern ClientList client_list;
extern struct RoomInfo rooms[MAX_ROOM];
#define B_SIZE 15
int check_win(enum StoneColor board[B_SIZE][B_SIZE], int y, int x, int stone);

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
    int stone_val = client_info->stone;
	snprintf(buf, sizeof(buf), "MOVE %d %d %d\n", y, x, stone_val);
    for(int i=0;i<MAX_CLIENT;i++){
        int cid = room->client_info[i];
        if(cid != -1){
            PlayerView* p = search_client(&client_list, cid);
            if(p) send(p->client_id, buf, strlen(buf), 0);
        }
    }

	if (opponent) {
        send(opponent->client_id, "YOURTURN\n", strlen("YOURTURN\n"), 0);
    }

    printf("[Handler][MOVE] Client %s (%d) -> %d,%d stone = %d\n",
           client_info->nick, client_info->client_id, y, x, stone_val);

	if(check_win(room->board, y, x, client_info->stone)){
    	send(client_sock, "WIN\n", 4, 0);
    	if(opponent) send(opponent->client_id, "LOSE\n", 5, 0);
    	
		//init
		room->room_state = STATE_WAIT;
		//room->board[y][x] = client_info->stone;
		memset(room->board, 0, sizeof(room->board));
		client_info->turn = 0;
		if(opponent) opponent->turn = 0;

    	return;
	}

}


int check_win(enum StoneColor board[B_SIZE][B_SIZE], int y, int x, int stone){
    int dy[4] = {1, 0, 1, 1};
    int dx[4] = {0, 1, 1, -1};

    for(int d=0; d<4; d++){
        int cnt = 1;
        int ny, nx;

        // positive direction
        ny = y + dy[d];
        nx = x + dx[d];
        while(ny>=0 && ny<15 && nx>=0 && nx<15 && board[ny][nx]==stone){
            cnt++;
            ny += dy[d];
            nx += dx[d];
        }

        // negative direction
        ny = y - dy[d];
        nx = x - dx[d];
        while(ny>=0 && ny<15 && nx>=0 && nx<15 && board[ny][nx]==stone){
            cnt++;
            ny -= dy[d];
            nx -= dx[d];
        }

        if(cnt >= 5) return 1;
    }
    return 0;
}

