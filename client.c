/*
	client.c
*/
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <stdbool.h>

#define PORT 5000
#define SERVER_HOST "127.0.0.1"

static void usage()
{
    /* Create my menu selection */
    printf("==========**** Select your choice number from 1 to 4 ***==========\n");
    printf("1- Send logon to server\n");
    printf("2- Send logoff to server\n");
    printf("3- Get status\n");
    printf("4- Exit\n");
}

int main()
{
    int sock;
    char choice;
    struct hostent *host;
    struct sockaddr_in server_addr;
    /* some command */
    char* str_logon = "LOGON";
    char* str_logoff = "LOGOFF";
    char* str_status_request = "STATUS_REQ";
    char buf[128];

    host = gethostbyname(SERVER_HOST);

    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) == -1) 
	{
        perror("Socket");
        exit(1);
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    server_addr.sin_addr = *((struct in_addr *)host->h_addr);
    bzero(&(server_addr.sin_zero),8);

    if (connect(sock, (struct sockaddr *)&server_addr,
                sizeof(struct sockaddr)) == -1)
    {
        perror("Connect");
        exit(1);
    }

    do
    {
        usage();
        choice = getchar();
        switch(choice)
        {
        case '1':
            send(sock, str_logon, strlen(str_logon), 0);
            break;
        case '2':
            send(sock, str_logoff, strlen(str_logoff), 0);
            break;
        case '3':
            send(sock, str_status_request, strlen(str_status_request), 0);
            int byte_receive = recv(sock, buf, strlen(buf), 0);
            buf[byte_receive] ='\0';
            printf("%s\n", buf);
            break;
        case '4':
            close(sock);
            exit(1);
            break;
        default:
            printf("Your choice invalid. Please try again.\n");
            break;
        }

    } while (getchar() != '1' || getchar() != '2' || getchar() != '3' || getchar() != '4');
    return 0;
}

