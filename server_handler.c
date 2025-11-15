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
	//1.check room number from client
	//2.check room exist and count client
	//3.check room state
	//if ok
	//	then update playerView_room_id
	//		update ReadyState -> READY_NO
	//		add RoomInfo.client_info
	//		update RoomInfo.count_client ++
	//else
	//	print err

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
    client_info->ready = READY_NO;

    room->client_info[room->count_client] = client_info->client_id;
    room->count_client++;

	printf("[Handler] Client joined room %d\n", num);
}

void handle_ready(int client_sock){
    printf("[Handler] READY command received\n");
}

void handle_leave(int client_sock){
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

