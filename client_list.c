#include <stdio.h>
#include "./socket_common.h"
#include <stdlib.h>
#include "./client_list.h"

//insert
void insert_client(ClientList* client_list, PlayerView client_info){
	//clientNode dynamic 
	ClientNode* client_node = (ClientNode*)malloc(sizeof(ClientNode));

	client_node->next = client_list->head;
	client_node->data = client_info;

	client_list->head = client_node;
	client_list->count++;
 
}

//remove
void remove_client(ClientList* client_list, int client_id){
	ClientNode* prev = NULL;
	ClientNode* current = client_list->head;

	while(current){
		if(current->data.client_id == client_id){
			if(prev){
				prev->next = current->next;
			}else{
				client_list->head = current->next;
			}
			free(current);
			client_list->count--;
			printf("[Server] Client %d removed\n", client_id);
			//test
			print_clients(client_list);
			return;
		}

		prev = current;
		current = current->next;
	}

}

//print
void print_clients(ClientList* client_list){

	for(ClientNode* client_node = client_list->head; 
		client_node!=NULL;
		client_node = client_node->next){
			
	printf("Client Name: %s\n", client_node->data.nick);
	}

}

//clear
