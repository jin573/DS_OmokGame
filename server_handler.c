#include "socket_common.h"
#include "server_handler.h"
#include <stdio.h>
#include <string.h>

extern ClientList client_list;
extern struct RoomInfo rooms[MAX_ROOM];

void handle_nick(int client_sock, PlayerView* client_info, char* buffer){
    char* nick = buffer + 5;
    trim_newline(nick);
    
	set_nickname(client_info, nick);
	insert_client(&client_list, client_info);
	printf("[Handler][NICK] Client %d set nickname to '%s'\n", 
			client_info->client_id, client_info->nick);
    print_clients(&client_list);
    send(client_sock, "OK NICK\n", 8, 0);
}

void handle_list(int client_sock){
    printf("[Handler][LIST] LIST command\n");
	
	char response[512];
	response[0] = '\0';
	strcat(response, "ROOMS ");

	for(int i=0; i<MAX_ROOM; i++){
		char buf[64];
		snprintf(buf, sizeof(buf), "%d:%d:%s ",
                 rooms[i].room_id,
                 rooms[i].count_client,
                 room_state_str(rooms[i].room_state));
		strcat(response, buf);
	}
	strcat(response, "\n");
    send(client_sock, response, strlen(response), 0);
}

void handle_join(int client_sock, PlayerView* client_info, char* buffer){
    printf("[Handler][JOIN] JOIN command received\n");

	char* num_str = buffer + 5;
	trim_newline(num_str);

	int num = atoi(num_str);
	if(num < 0 || num >= MAX_ROOM){
		printf("[Handler] Invalid room number: %d\n", num);
		send(client_sock, "ERR BADROOM\n", strlen("ERR BADROOM\n"), 0);
		return;
	}

	RoomInfo* room = &rooms[num];	

	if(room->room_state!= STATE_WAIT){
		printf("[Handler] ROOM NOT ENTER\n");
		send(client_sock, "ERR START\n", strlen("ERR START\n"), 0);
		return;
	}

	if(room->count_client >= 2){
		printf("[Handler] ROOM FULL\n");
		send(client_sock, "ERR FULL\n", strlen("ERR FULL\n"), 0); 
		return;
	}

	client_info->room_id = num;
	//client_info->seat = ;//random
    client_info->ready = READY_NO;
	//client_info->stone = ;//random
	//client_info->turn = ;//random
	//
    room->client_info[room->count_client] = client_info->client_id;
    room->count_client++;
	reorder_room_clients(room);
	
	printf("[Handler][JOIN] Client %s (%d) joined room %d. Room state: %s, count=%d, clients=[%d,%d]\n", 
			client_info->nick, client_info->client_id, num, 
			room_state_str(room->room_state), room->count_client, 
			room->client_info[0], room->client_info[1]);
	send(client_sock, "OK JOIN\n", strlen("OK JOIN\n"), 0);
	printf("[Handler][JOIN] Sent OK JOIN to client %d\n", client_info->client_id);

}

void handle_ready(int client_sock, PlayerView* client_info){	
	
	RoomInfo* room = &rooms[client_info->room_id];

	if(room->room_state == STATE_START){
		printf("[Handler] Already Started\n");
		send(client_sock, "ERR START\n", strlen("ERR START\n"), 0);
		return;
	}

	client_info->ready = (client_info->ready == READY_YES? READY_NO : READY_YES);
	
	printf("[Handler][READY] Client %s (%d) toggled ready. Current ready=%s\n",
			client_info->nick, client_info->client_id, 
			ready_state_str(client_info->ready));
	
	if(room->count_client <2){
		printf("[READY] only one client in room. skip broadcast.\n");
        return;
	}

	bool ready_flag = check_all_client_ready(room);

	if(ready_flag){
		if(room->room_state != STATE_WAIT){
    		send(client_sock, "ERR STARTING\n", strlen("ERR STARTING\n"), 0);
   			return;
		}

		printf("[Handler][READY] ALL CLIENT SET READY\n");
		printf("==after 3 sec goto in game==\n");
		sleep(3);
		printf("===*===* game start *===*===\n");

		//color, turn and seat setting -> broadcast_to_room
		room->room_state = STATE_START;
		broadcast_to_room(room, "START\n");
		printf("[Handler] Game started, broadcast done\n");
		return;
	}

	printf("[READY] Not all ready. Broadcast READY to opponent.\n");

    for(int i=0;i<MAX_CLIENT;i++){
        int cid = room->client_info[i];
        if(cid == -1 || cid == client_sock) continue; 
        send(cid, "READY\n", strlen("READY\n"), 0);
    }

}

void handle_leave(int client_sock, PlayerView* client_info){
	printf("[Handler] LEAVE command received\n");
	printf("[Handler][LEAVE] Client %s (%d) requested LEAVE\n",
			client_info->nick, client_info->client_id);
	
	remove_client_in_room(client_sock, client_info);
	send(client_sock, "OK LEAVE\n", strlen("OK LEAVE\n"), 0);
}

void handle_quit(int client_sock, PlayerView* client_info){
    printf("[Handler][QUIT] Client %s (%d) requested QUIT\n",
			client_info->nick, client_info->client_id);

	remove_client_in_room(client_sock, client_info);
	send(client_sock, "OK QUIT\n", strlen("OK QUIT\n"), 0);
	
}

void set_nickname(PlayerView* client_info, char* nickname){
	if(!client_info || !nickname) return;
	strncpy(client_info->nick, nickname, MAX_NICK - 1);
	client_info->nick[MAX_NICK - 1]='\0';
}

bool check_all_client_ready(RoomInfo* room){
	//check all player's state
	for(int i=0; i<MAX_CLIENT; i++){
		int cid = room->client_info[i];
		if(cid == -1) continue; 
		
		PlayerView* client = search_client(&client_list, cid);
		printf("[Handler][READY][SEARCH CLIENT] client_id: %d, ptr=%p\n", 
				room->client_info[i], (void*)client);
		if(!client || client->ready != READY_YES){
			return false;
		}
	}
	return true;
}

void remove_client_in_room(int client_sock, PlayerView* client_info){
	if(client_info->room_id == -1){
        send(client_sock, "ERR NOROOM\n", strlen("ERR NOROOM\n"), 0);
        printf("[Handler] Already Leave Room\n");
        return;
    }

    RoomInfo* room = &rooms[client_info->room_id];
	printf("[Handler][REMOVE] Removing Client %s (%d) from room %d. Count before=%d\n", 
			client_info->nick, client_info->client_id, room->room_id,
			room->count_client);
   
	for(int i=0; i<MAX_CLIENT; i++){
		if(room->client_info[i] == client_info->client_id){
            room->client_info[i] = -1;
            room->count_client--;
			printf("[Handler][REMOVE] Cleared slot %d for client_id %d\n",
					i, client_info->client_id);
        }
    }

    reorder_room_clients(room);
	printf("[Handler][REMOVE] Room %d after reorder: count=%d, clients=[%d,%d]\n", 
			room->room_id, room->count_client, 
			room->client_info[0], room->client_info[1]);

    client_info->room_id = -1;
    client_info->ready = READY_NOT;
    //in Game -> add reset seat, stone, turn -> extends
	//and change room state also
	
	client_info->seat  = -1;
	client_info->stone = STONE_NONE;
	client_info->turn  = 0;

	int cnt = 0;
	for(int i=0;i<MAX_CLIENT;i++){
	    if(room->client_info[i] != -1) cnt++;
	}
	if(cnt == 0){
	    room->room_state = STATE_WAIT;
	    memset(room->board, 0, sizeof(room->board));
	}

}

static void broadcast_to_room(RoomInfo* room, const char* msg){
	int first_seat = rand()%2;

	for(int i=0; i<MAX_CLIENT; i++){
		int cid = room->client_info[i]; 
		if(cid != -1){
			PlayerView* p = search_client(&client_list, cid); 

			//color, turn and seat setting
			p->seat = (i==0) ? first_seat : 1 -first_seat;
			p->stone = (p->seat == 0) ? STONE_BLACK : STONE_WHITE;
			p->turn = (p->seat == 0) ? 1 : 0 ;
           	char send_buf[128];
			const char* color_str = NULL;

			if(strncmp(msg, "START", 5) == 0){
				color_str = (p->stone == STONE_BLACK) ? "black" : "white";
				snprintf(send_buf, sizeof(send_buf), "START %s %d %d\n", color_str, p->turn, p->seat);
			 	printf("[broadcast] send %s to client %d color: %s\n", send_buf, p->client_id, color_str);
                send(p->client_id, send_buf, strlen(send_buf), 0);

                if (p->turn == 1) {
                    send(p->client_id, "YOURTURN\n", strlen("YOURTURN\n"), 0);
                }
                continue;
			}

			
 			snprintf(send_buf, sizeof(send_buf), "%s\n", msg);
            send(p->client_id, send_buf, strlen(send_buf), 0);

			}
	}
}



