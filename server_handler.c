#include "socket_common.h"
#include "server_handler.h"
#include <stdio.h>
#include <string.h>

extern ClientList client_list;
extern struct RoomInfo rooms[MAX_ROOM];


void handle_nick(int client_sock, PlayerView* client_info, char* buffer){
    char* nick = buffer + 5;
    trim_newline(nick);
    
	printf("[Handler] nickname: [%s]\n", nick);

    *client_info = set_nickname(*client_info, nick);
    insert_client(&client_list, *client_info);
    print_clients(&client_list);
    send(client_sock, "OK NICK\n", 8, 0);
}

void handle_list(int client_sock){
    printf("[Handler] LIST command\n");
	
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
    printf("[Handler] JOIN command received\n");

	char* num_str = buffer + 5;
	trim_newline(num_str);

	int num = atoi(num_str);
	if(num < 0 || num >= MAX_ROOM){
		printf("[Handler] Invalid room number: %d\n", num);
		send(client_sock, "ERR BADROOM\n", strlen("ERR BADROOM\n"), 0);
		return;
	}

	printf("[Handler] JOIN Room Number: %d\n", num);

	RoomInfo* room = &rooms[num];	

	if(room->room_state!= STATE_WAIT){
		printf("[Handler] ROOM NOT ENTER\n");
		//err send
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

    room->client_info[room->count_client] = client_info->client_id;
    room->count_client++;
	reorder_room_clients(room);
	
	printf("[Handler] Client joined room %d\n", num);
	send(client_sock, "OK JOIN\n", strlen("OK JOIN\n"), 0);
}

void handle_ready(int client_sock, PlayerView* client_info){
	
	printf("[Handler] READY command received\n");
}

void handle_leave(int client_sock, PlayerView* client_info){
	printf("[LEAVE] Request from client_id=%d (socket=%d)\n",
       client_info->client_id, client_sock);

	if(client_info->room_id == -1){
		send(client_sock, "ERR NOROOM\n", strlen("ERR NOROOM\n"), 0);
		printf("[Handler] Already Leave Room\n");
		return;
	}

	RoomInfo* room = &rooms[client_info->room_id];

	printf("[LEAVE] Client room_id=%d, room.count_client=%d\n",
       client_info->room_id, room->count_client);
	
	printf("[LEAVE] Room %d before: [%d, %d]\n",
       room->room_id,
       room->client_info[0],
       room->client_info[1]);

	for(int i=0; i<MAX_CLIENT; i++){
		if(room->client_info[i] == client_info->client_id){
			room->client_info[i] = -1;
			printf("[LEAVE] Removing client_id=%d at index=%d\n",
    		   client_info->client_id, i);
			room->count_client--;
		}
	}

	reorder_room_clients(room);
	
	printf("[LEAVE] Room %d after reorder: [%d, %d], count=%d\n",
       room->room_id,
       room->client_info[0],
       room->client_info[1],
       room->count_client);

	client_info->room_id = -1;
	client_info->ready = READY_NOT;
	//in Game -> add reset seat, stone -> extends
	send(client_sock, "OK LEAVE\n", strlen("OK LEAVE\n"), 0);
	printf("[Handler] LEAVE command received\n");

}

void handle_quit(int client_sock){
    printf("[Handler] QUIT command received\n");
}


PlayerView set_nickname(PlayerView client_info, char* nickname){
	strncpy(client_info.nick, nickname, MAX_NICK - 1);
	client_info.nick[MAX_NICK - 1]='\0';
	return client_info;
}

