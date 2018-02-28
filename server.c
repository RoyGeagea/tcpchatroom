#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>

struct client_info {
    int ID;
    char username[100];
    char ip[INET_ADDRSTRLEN];
	int sockno;
};

struct client_info clients[100];
int n = 0;

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

char whoCommand[] = "_who";
char quitCommand[] = "_quit";
char shutdownCommand[] = "_shutdown";

void sendToAll(char *msg,int curr) {
	int i;
//    pthread_mutex_lock(&mutex);
	for(i = 0; i < n; i++) {
		if(clients[i].sockno != curr) {
			if(send(clients[i].sockno,msg,strlen(msg),0) < 0) {
				perror("sending failure");
				continue;
			}
		}
	}
//    pthread_mutex_unlock(&mutex);
}

void shareWithAll(char *msg) {
    int i;
    //    pthread_mutex_lock(&mutex);
    for(i = 0; i < n; i++) {
        if(send(clients[i].sockno,msg,strlen(msg),0) < 0) {
            perror("sending failure");
            continue;
        }
    }
    //    pthread_mutex_unlock(&mutex);
}

void killConnectionWith(char surnom[]) {
    pthread_mutex_lock(&mutex);
    char res[600];
    int i;
    int j;
    int finded = -1;
    memset(res, 0x00, sizeof(res));
    for(i = 0; i < n; i++) {
        if(strcmp(clients[i].username, surnom) == 0) {
            printf("client founded: %s\n", clients[i].username);
            finded = i;
            strcpy(res, surnom);
            strcat(res," quit the group\n");
            res[strlen(res)] = '\0';
        }
    }
    if (finded > -1) {
        close(clients[finded].sockno);
        for(i = 0; i < n; i++) {
            if(clients[i].sockno == clients[finded].sockno) {
                j = i;
                while(j < n-1) {
                    clients[j] = clients[j+1];
                    j++;
                }
            }
        }
        n--;
        shareWithAll(res);
        memset(res, 0x00, sizeof(res));
    }
    else {
        printf("This username does not exist\n");
    }
    pthread_mutex_unlock(&mutex);
}

void *keyboard() {
    char msg[500];
    int len;
    char kill[500];
    char space;
    char surnom[100];
    while(fgets(msg,500,stdin) > 0) {
        if(len < 0) {
            perror("message not sent");
            exit(1);
        }
        if (strlen(msg) == strlen(shutdownCommand) + 1 && msg[0] == '_' && msg[1] == 's' && msg[2] == 'h' && msg[3] == 'u' && msg[4] == 't' && msg[5] == 'd' && msg[6] == 'o' && msg[7] == 'w' && msg[8] == 'n') {
            exit(1);
        }
        else {
            if (msg[0] == '_' && msg[1] == 'k' && msg[2] == 'i' && msg[3] == 'l' && msg[4] == 'l') {
                sscanf(msg, "%s%c%s", kill, &space, surnom);
                killConnectionWith(surnom);
            }
        }
        memset(msg,'\0',sizeof(msg));
    }
}

void *receiveMessage(void *sock) {
	struct client_info cl = *((struct client_info *)sock);
	char msg[500];
    char res[600];
	int len;
	int i;
	int j;
    memset(msg, 0x00, sizeof(msg));
    memset(res, 0x00, sizeof(res));
	while((len = recv(cl.sockno,msg,500,0)) > 0) {
        if (strlen(msg) == strlen(whoCommand) + 1 && msg[0] == '_' && msg[1] == 'w' && msg[2] == 'h' && msg[3] == 'o') {
            memset(res, 0x00, sizeof(res));
            for(i = 0; i < n; i++) {
                if (i == n - 1) {
                    strcat(res, clients[i].username);
                }
                else {
                    strcat(res, clients[i].username);
                    strcat(res, ", ");
                }
            }
            strcat(res, "\n");
            res[strlen(res)] = '\0';
            if(send(cl.sockno,res,strlen(res),0) < 0) {
                perror("sending failure");
            }
            memset(msg, 0x00, sizeof(msg));
            memset(res, 0x00, sizeof(res));
        }
        else if (strlen(msg) == strlen(quitCommand) + 1 && msg[0] == '_' && msg[1] == 'q' && msg[2] == 'u' && msg[3] == 'i' && msg[4] == 't') {
            
        }
        else {
            strcpy(res, cl.username);
            strcat(res,": ");
            strcat(res,msg);
            msg[len] = '\0';
            res[strlen(res)] = '\0';
            sendToAll(res,cl.sockno);
            memset(msg, 0x00, sizeof(msg));
            memset(res, 0x00, sizeof(res));
        }
	}
	pthread_mutex_lock(&mutex);
    strcpy(res, cl.username);
    strcat(res, " quit the group\n");
    res[strlen(res)] = '\0';
	printf("%s disconnected\n",cl.username);
	for(i = 0; i < n; i++) {
		if(clients[i].sockno == cl.sockno) {
			j = i;
			while(j < n-1) {
				clients[j] = clients[j+1];
				j++;
			}
		}
	}
	n--;
    shareWithAll(res);
    memset(msg,'\0',sizeof(msg));
    memset(res,'\0',sizeof(res));
	pthread_mutex_unlock(&mutex);
}

int main(int argc,char *argv[]) {
	struct sockaddr_in my_addr,their_addr;
	int my_sock;
	int their_sock;
	socklen_t their_addr_size;
	int portno;
	pthread_t sendt, recvt, keyboardThread;
	char msg[500];
    char res[600];
	int len;
	struct client_info cl;
	char ip[INET_ADDRSTRLEN];;
	;
	if(argc > 2) {
		printf("too many arguments");
		exit(1);
	}
	portno = atoi(argv[1]);
	my_sock = socket(AF_INET,SOCK_STREAM,0);
	memset(my_addr.sin_zero,'\0',sizeof(my_addr.sin_zero));
	my_addr.sin_family = AF_INET;
	my_addr.sin_port = htons(portno);
	my_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
	their_addr_size = sizeof(their_addr);

	if(bind(my_sock,(struct sockaddr *)&my_addr,sizeof(my_addr)) != 0) {
		perror("binding unsuccessful");
		exit(1);
	}

	if(listen(my_sock,5) != 0) {
		perror("listening unsuccessful");
		exit(1);
	}

    pthread_create(&keyboardThread, NULL, keyboard, NULL);
    
    memset(res, 0x00, sizeof(res));
	while(1) {
		if((their_sock = accept(my_sock,(struct sockaddr *)&their_addr,&their_addr_size)) < 0) {
			perror("accept unsuccessful");
			exit(1);
		}
		pthread_mutex_lock(&mutex);
		inet_ntop(AF_INET, (struct sockaddr *)&their_addr, ip, INET_ADDRSTRLEN);
		printf("%s connected\n",ip);
		cl.sockno = their_sock;
		strcpy(cl.ip,ip);
		clients[n].sockno = their_sock;
        clients[n].ID = n+1;
        strcpy(clients[n].ip,ip);
        
        memset(clients[n].username, 0x00, sizeof(clients[n].username));
        len = recv(clients[n].sockno, clients[n].username, 100, 0);
        while (len < 0) {
            len = recv(clients[n].sockno, clients[n].username, 100, 0);
        }
        printf("Username: %s\n", clients[n].username);
        clients[n].username[strlen(clients[n].username)] = '\0';
        strcpy(res, clients[n].username);
        strcat(res," has been added to the group\n");
        res[strlen(res)] = '\0';
        sendToAll(res, clients[n].sockno);
        memset(res, 0x00, sizeof(res));
		pthread_create(&recvt,NULL,receiveMessage,&clients[n]);
        n++;
		pthread_mutex_unlock(&mutex);
	}
	return 0;
}
