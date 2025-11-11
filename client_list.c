#include <stdio.h>
#include "./socket_common.h"
#include <stdlib.h>


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


//print
void print_clients(ClientList* client_list){

	for(ClientNode* client_node = client_list->head; 
		client_node!=NULL;
		client_node = client_node->next){
			
	printf("Client Name: %s\n", client_node->data.nick);
	}

}

//clear
