#include "server_handler.h"
#include <stdio.h>
#include <string.h>

extern ClientList client_list;
extern struct RoomInfo rooms[MAX_ROOM];


void handle_nick(int client_sock, PlayerView* client_info){
    char buffer[256];
    int n = recv_line(client_sock, buffer, sizeof(buffer));
    if(n <= 0) return;

    buffer[n] = '\0';
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
    char response[256];
    snprintf(response, sizeof(response), "ROOMS 0:0:WAIT 1:2:START 2:1:READY\n");
    send(client_sock, response, strlen(response), 0);
}

void handle_join(int client_sock){
    printf("[Handler] JOIN command received\n");
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

