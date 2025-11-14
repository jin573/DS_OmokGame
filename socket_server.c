#include "./socket_common.h"
#include "./client_list.h"
#include "./server_handler.h"

void init_rooms(struct RoomInfo rooms[]);
PlayerView init_client(int sock, struct sockaddr_in *addr);
void *t_function(void* arg);


struct RoomInfo rooms[MAX_ROOM];
ClientList client_list = {NULL, 0};

int main(int argc, char *argv[]) {
	//error msg
	//port number ex)./socket_test 8080
	if(argc < 2){
        fprintf(stderr, "Usage: %s <port>\n", argv[0]);
        exit(1);
    }

	//room setting 
	init_rooms(rooms);
	printf("Room initialized (10 rooms ready)\n");

	//socket setting
    struct sockaddr_in server, client; //socket info
	int request_sock, client_sock;//socket fd
	socklen_t client_len;//client socket
	char buffer[256]; 

    if((request_sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0){
        err_quit("socket()");
    }
    
	memset(&server, 0, sizeof(server));
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_port = htons((u_short) atoi(argv[1]));

	int opt = 1;
	if (setsockopt
			(request_sock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
		err_quit("setsockopt()");
	}

	//func socket
	//bind
    if(bind(request_sock, (struct sockaddr*)&server, sizeof(server)) < 0){
       err_quit("bind()");
    }

	//listen
    if(listen(request_sock, SOMAXCONN) < 0){
        err_quit("listen()");
    }

    printf("Server listening on port %s\n", argv[1]);

	//listen while
    while(1){
		client_len = sizeof(client);
		client_sock = accept(request_sock, (struct sockaddr*)&client, &client_len);
        if (client_sock < 0) {
            err_quit("accept()");
            continue;
        }else{
			//success accept()
			//create pthread
			ThreadArg *targ = malloc(sizeof(ThreadArg));
			targ->client_sock = client_sock;
			targ->client = client;
			targ->client_list = &client_list;

			pthread_t thr;
			int thr_id = pthread_create(&thr, NULL, t_function, targ);
			//if success
			//	then init client
			if(thr_id < 0){
				err_quit("pthread()");
			}

		}
	}
	//close
    close(request_sock);
    return 0;
}


void init_rooms(struct RoomInfo rooms[]){
	for(int i=0; i<MAX_ROOM; i++){
		rooms[i].room_id = i;
		rooms[i].count_client = 0;
		rooms[i].room_state = STATE_WAIT;
		memset(rooms[i].client_info, -1, sizeof(rooms[i].client_info)); //not 0
	}

}

PlayerView init_client(int client_sock, struct sockaddr_in* addr){
	PlayerView client_info;
		
	client_info.client_id = client_sock;
	memset(client_info.nick, 0, sizeof(client_info.nick));
	client_info.room_id = -1;
	client_info.seat = -1;
	client_info.ready = READY_NOT;
	client_info.stone = STONE_NONE;

	return client_info;
}

void *t_function(void *arg){
	pid_t pid = getpid();
	pthread_t tid = pthread_self();

	ThreadArg *targ = (ThreadArg *)arg;
	int client_sock = targ->client_sock;
	struct sockaddr_in client = targ->client;
	free(targ);

	char buffer[1024];

	while(1){
		printf("[Server] process id: %d thread id: %u\n", pid, (unsigned int) tid);
		//sleep(5);
		//init client
		PlayerView client_info = init_client(client_sock, &client);
		printf("[Server] client accept: %s client num: %d\n", 
				inet_ntoa(client.sin_addr),
				client_info.client_id);
		
		//change nickname
		int n = recv_line(client_sock, buffer, sizeof(buffer));
		if(n > 0){
			buffer[n] = '\0';
          if(strncmp(buffer, "NICK", 4) == 0){
			  handle_nick(client_sock, &client_info);
		  }else if(strncmp(buffer, "LIST", 4) == 0){
			  handle_list(client_sock);
		  }else if(strncmp(buffer, "JOIN", 4) == 0){
			  handle_join(client_sock);
		  }else if(strncmp(buffer, "READY", 5) == 0){
			  handle_ready(client_sock);
		  }else if(strncmp(buffer, "LEAVE", 5) == 0){
			  handle_leave(client_sock);
		  }else if(strncmp(buffer, "QUIT", 4) == 0){
			  handle_quit(client_sock);
			  break;
		  } else {
			  printf("[Server] Unknown command: %s\n", buffer);
		  } 
		}
	}
	close(client_sock);
	free(targ);
	return NULL;
}

