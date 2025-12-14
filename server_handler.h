#ifndef HANDLER_H
#define HANDLER_H

#include "socket_common.h"
#include "client_list.h"

void handle_nick(int client_sock, PlayerView* client_info, char* buffer);
void handle_list(int client_sock);
void handle_join(int client_sock, PlayerView* client_info, char* buffer);
void handle_ready(int client_sock, PlayerView* client_info);
void handle_leave(int client_sock, PlayerView* client_info);
void handle_quit(int client_sock, PlayerView* client_info);
void handle_game_over(RoomInfo* room);

void set_nickname(PlayerView* client_info, char* nickname);
bool check_all_client_ready(RoomInfo* room);
void remove_client_in_room(int client_sock, PlayerView* client_info);
static void broadcast_to_room(RoomInfo* room, const char* msg);
#endif
