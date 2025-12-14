#include <stdio.h>
#include "./socket_common.h"
#include <stdlib.h>
#include "./client_list.h"

pthread_mutex_t client_list_mutex = PTHREAD_MUTEX_INITIALIZER;

//insert
void insert_client(ClientList* client_list, PlayerView* client_info){
	pthread_mutex_lock(&client_list_mutex);
	//clientNode dynamic 
	ClientNode* client_node = (ClientNode*)malloc(sizeof(ClientNode));

	client_node->data = client_info;
	client_node->next = client_list->head;

	client_list->head = client_node;
	client_list->count++;
 	pthread_mutex_unlock(&client_list_mutex);
}

//search
PlayerView* search_client(ClientList* client_list, int client_id){
	pthread_mutex_lock(&client_list_mutex);
	ClientNode* current = client_list->head;

	while(current){
		if(current->data->client_id == client_id){
			PlayerView* p = current->data;
			pthread_mutex_unlock(&client_list_mutex);
			return p;
		}
		current = current->next;
	}
	pthread_mutex_unlock(&client_list_mutex);
	return NULL;
}
//remove
void remove_client(ClientList* client_list, int client_id){
	pthread_mutex_lock(&client_list_mutex);
	ClientNode* prev = NULL;
	ClientNode* current = client_list->head;

	while(current){
		if(current->data->client_id == client_id){
			if(prev){
				prev->next = current->next;
			}else{
				client_list->head = current->next;
			}
			free(current);
			client_list->count--;
			pthread_mutex_unlock(&client_list_mutex);
			
			printf("[Server] Client %d removed\n", client_id);
			//test
			print_clients(client_list);
			
			return;
		}

		prev = current;
		current = current->next;
	}
	pthread_mutex_unlock(&client_list_mutex);
}

//print
void print_clients(ClientList* client_list){
	pthread_mutex_lock(&client_list_mutex);
	for(ClientNode* client_node = client_list->head; 
		client_node!=NULL;
		client_node = client_node->next){
			
	printf("Client Name: %s\n", client_node->data->nick);
	}
	pthread_mutex_unlock(&client_list_mutex);
}

//clear
