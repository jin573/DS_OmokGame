#ifndef HANDLER_H
#define HANDLER_H

#include "socket_common.h"
#include "client_list.h"

void handle_nick(int client_sock, PlayerView* client_info, char* buffer);
void handle_list(int client_sock);
void handle_join(int client_sock, PlayerView* client_info, char* buffer);
void handle_ready(int client_sock);
void handle_leave(int client_sock);
void handle_quit(int client_sock);

PlayerView set_nickname(PlayerView client_info, char* nickname);


#endif
