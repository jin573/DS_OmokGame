#include "./socket_common.h"

void insert_client(ClientList* list, PlayerView* client);
PlayerView* search_client(ClientList* list, int client_id);
void remove_client(ClientList* list, int client_id);
void print_clients(ClientList* list);
