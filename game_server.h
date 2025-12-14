#ifndef HANDLER_MOVE_H
#define HANDLER_MOVE_H

#include "./client_list.h"
#include "./server_handler.h"
#include "./socket_common.h"

#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>

void handle_game(int client_sock, PlayerView* client_info, const char* msg);

#endif

