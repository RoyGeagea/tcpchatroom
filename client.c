#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <signal.h>

char whoCommand[] = "_who";
char quitCommand[] = "_quit";

void *receiveMessage(void *sock) {
	int their_sock = *((int *)sock);
	char msg[500];
	int len;
	while((len = recv(their_sock,msg,500,0)) > 0) {
		msg[len] = '\0';
		fputs(msg,stdout);
		memset(msg,'\0',sizeof(msg));
	}
    exit(1);
}

int connectToServer(char nom[], char machine[], char port[]) {
	struct sockaddr_in their_addr;
	int my_sock;
	int their_sock;
	int their_addr_size;
	int portno;
	pthread_t sendt,recvt;
	char msg[500];
	char username[100];
	char res[600];
	char ip[INET_ADDRSTRLEN];
	int len;

	portno = atoi(port);
	strcpy(username, nom);
	my_sock = socket(AF_INET,SOCK_STREAM,0);
	memset(their_addr.sin_zero,'\0',sizeof(their_addr.sin_zero));
	their_addr.sin_family = AF_INET;
	their_addr.sin_port = htons(portno);
	their_addr.sin_addr.s_addr = inet_addr("127.0.0.1");

	if(connect(my_sock,(struct sockaddr *)&their_addr,sizeof(their_addr)) < 0) {
		perror("connection not esatablished");
		exit(1);
	}
	inet_ntop(AF_INET, (struct sockaddr *)&their_addr, ip, INET_ADDRSTRLEN);
    
    len = write(my_sock, username, strlen(username));
    if (len < 0) {
        printf("Username is not sent to the server\n");
    }
    else {
//        printf("Username is sent to the server\n");
    }
    
//    printf("connected to %s, start chatting\n",ip);
	pthread_create(&recvt,NULL,receiveMessage,&my_sock);
    
    memset(msg, 0x00, sizeof(msg));
    memset(res, 0x00, sizeof(res));
	while(fgets(msg,500,stdin) > 0) {
		strcat(res,msg);
		len = write(my_sock,res,strlen(res));
		if(len < 0) {
			perror("message not sent");
			exit(1);
		}
        if (strlen(msg) == strlen(quitCommand) + 1 && msg[0] == '_' && msg[1] == 'q' && msg[2] == 'u' && msg[3] == 'i' && msg[4] == 't') {
            exit(1);
        }
        memset(msg, 0x00, sizeof(msg));
        memset(res, 0x00, sizeof(res));
	}
	pthread_join(recvt,NULL);
	close(my_sock);
}

int main(int argc, char**argv) {
    char command[2000];
    char connect[2000];
    char nom[2000];
    char machine[2000];
    char port[2000];
    
    char firstSpace;
    char secondSpace;
    char thirdSpace;
        
    printf("Welcome to talk :)\n");
    // instruction pour le client
    printf("Type '_connect <surnom> <machine> <port>' to connect to your server.\n");
    printf("Type '_quit' to quit.\n");
    //    printf("Type '_who' to request the list of users from the server.\n");
    fputs("", stdout);
    fgets(command, sizeof (command), stdin);
    
    if (command[0] == '_' && command[1] == 'q' && command[2] == 'u' && command[3] == 'i' && command[4] == 't') {
//        printf("you enter the following command: _quit\n");
        return 0;
    }
    else {
        if (command[0] == '_' && command[1] == 'c' && command[2] == 'o' && command[3] == 'n' && command[4] == 'n' && command[5] == 'e' && command[6] == 'c' && command[7] == 't') {
            
            sscanf(command, "%s%c%s%c%s%c%s", connect, &firstSpace, nom, &secondSpace, machine, &thirdSpace, port);
//            printf("Name: %s\n", nom);
//            printf("Machine: %s\n", machine);
//            printf("Port: %s\n", port);
            
            connectToServer(nom, machine, port);
        }
        else {
            printf("We are not here to play looser, BYE !!!\n");
            return 0;
        }
    }
}
