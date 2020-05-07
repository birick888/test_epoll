/* 
	server.c
*/

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <fcntl.h>
#include <sys/epoll.h>
#include <netdb.h>
#include <errno.h>
#include <netinet/tcp.h>
#include <pthread.h>
#include <stdbool.h>

#define MAX_CLIENT 10000
#define MAXEVENTS 64
#define PORT 5000
#define NUM_CLIENT_HANDLE 50

static int socket_fd, epoll_fd;
static int number_logon_client = 0;
static int number_logoff_client = 0;
static int number_connected_client = 0;
bool state_on_client[MAX_CLIENT] = {false};
bool connected_client[MAX_CLIENT] = {false};
char choice;

static void usage()
{
    /* Create my menu slection */
    printf("==========**** Select your choice number from 1 to 2 ***==========\n");
    printf("1- Print client statuses\n");
    printf("    - Number of connected clients\n");
    printf("    - Number of logged on clients\n");
    printf("    - Number of logged off clients\n");
    printf("2- Exit\n");
}

void get_statuses_clients()
{
    /* reset old counting */
    number_connected_client = 0;
    number_logoff_client = 0;
    number_logon_client = 0;
    for (int i = 0; i < MAX_CLIENT; ++i)
    {
        if (connected_client[i] == true)
        {
            ++number_connected_client;
            printf("fd: %d\n", i);

            if (state_on_client[i] == true)
            {
                ++number_logon_client;
            }
            else
                ++number_logoff_client;
        }
    }
}

void show_statuses()
{
    printf("Print client statuses\n");
    get_statuses_clients();
    printf("    - Number of connected clients: %d\n", number_connected_client);
    printf("    - Number of logged on clients: %d\n", number_logon_client);
    printf("    - Number of logged off clients: %d\n", number_logoff_client);
}

void process_comand(int* fd, char* command)
{
    if (strcmp(command, "LOGON") == 0)
    {
        state_on_client[*fd] = true;
    }
    else if (strcmp(command, "LOGOFF") == 0)
    {
        state_on_client[*fd] = false;
    }
    else if (strcmp(command, "STATUS_REQ") == 0)
    {
        char* buf = NULL;
        if (state_on_client[*fd])
        {
            buf = "ON";
        }
        else
            buf = "OFF";

        /* reply state client via fd */
        send(*fd, buf, strlen(buf), 0);
    }
}

/* thread cli function definition */
void* thread_cli_function(void* args)
{
    do
    {
        usage();
        choice = getchar();
        switch(choice)
        {
        case '1':
            show_statuses();
            break;
        case '2':
            exit(1);
            break;
        default:
            printf("Your choice invalid. Please try again.\n");
            break;
        }

    } while (getchar() != '1' || getchar() != '2');
	/* avoid warning return here */
    return NULL;
}

static void socket_create_bind_local()
{
    struct sockaddr_in server_addr;
    if ((socket_fd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        perror("Socket");
        exit(1);
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    server_addr.sin_addr.s_addr = INADDR_ANY;
    bzero(&(server_addr.sin_zero), 8);

    int enable = 1;
    if (setsockopt(socket_fd, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int)) < 0)
        perror("setsockopt(SO_REUSEADDR) failed");

    if (bind(socket_fd, (struct sockaddr *)&server_addr, sizeof(struct sockaddr))
            == -1) {
        perror("Unable to bind");
        exit(1);
    }
}

static int make_socket_non_blocking(int sfd)
{
    int flags;

    flags = fcntl(sfd, F_GETFL, 0);
    if (flags == -1) {
        perror("fcntl");
        return -1;
    }

    flags |= O_NONBLOCK;
    if (fcntl(sfd, F_SETFL, flags) == -1) {
        perror("fcntl");
        return -1;
    }

    return 0;
}

void accept_and_add_new()
{
    struct epoll_event event;
    struct sockaddr in_addr;
    socklen_t in_len = sizeof(in_addr);
    int infd;
    char hbuf[NI_MAXHOST], sbuf[NI_MAXSERV];

    while ((infd = accept(socket_fd, &in_addr, &in_len)) != -1) {

        if (getnameinfo(&in_addr, in_len,
                        hbuf, sizeof(hbuf),
                        sbuf, sizeof(sbuf),
                        NI_NUMERICHOST | NI_NUMERICHOST) == 0) {
            /* printf("Accepted connection on descriptor %d (host=%s, port=%s)\n", infd, hbuf, sbuf); */
            /* need mutex in real program */
            connected_client[infd] = true;
            state_on_client[infd] = false;
        }

        /* make the incoming socket non-block
         * and add it to list of fds to
         * monitor */
        if (make_socket_non_blocking(infd) == -1) {
            abort();
        }

        event.data.fd = infd;
        event.events = EPOLLIN | EPOLLET;
        if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, infd, &event) == -1) {
            perror("epoll_ctl");
            abort();
        }
        in_len = sizeof(in_addr);
    }

    if (errno != EAGAIN && errno != EWOULDBLOCK)
        perror("accept");
}

void process_new_data(int fd)
{
    ssize_t count;
    char buf[512];

    while ((count = read(fd, buf, sizeof(buf) - 1))) {
        if (count == -1) {
            /* EAGAIN, read all data */
            if (errno == EAGAIN)
                return;

            perror("read");
            break;
        }

        /* Write buffer to stdout */
        buf[count] = '\0';
        /* printf("%s \n", buf); */
        process_comand(&fd, buf);
    }

    /* printf("Close connection on descriptor: %d\n", fd); */
    connected_client[fd] = false;
    close(fd);

}

int main()
{
    pthread_t id;
    int ret;
    struct epoll_event event, *events;

    socket_create_bind_local();

    if (make_socket_non_blocking(socket_fd) == -1)
        exit(1);

    if (listen(socket_fd, NUM_CLIENT_HANDLE) == -1) {
        perror("Listen");
        exit(1);
    }

    printf("\n====TCPServer Waiting for client on port %d====\n", PORT);
    fflush(stdout);

    epoll_fd = epoll_create1(0);
    if (epoll_fd == -1) {
        perror("epoll_create1");
        exit(1);
    }

    event.data.fd = socket_fd;
    event.events = EPOLLIN | EPOLLET;
    if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, socket_fd, &event) == -1) {
        perror("epoll_ctl");
        exit(1);
    }

    events = (struct epoll_event*)calloc(MAXEVENTS, sizeof(event));

    /* creating thread */
    ret = pthread_create(&id, NULL, &thread_cli_function, NULL);
    if (ret == 0)
    {
        printf("Thread cli created successfully.\n");
    }
    else
    {
        printf("Thread cli not created.\n");
        return 0;
    }

    while (1)
    {
        int n, i;
        n = epoll_wait(epoll_fd, events, MAXEVENTS, -1);
        for (i = 0; i < n; i++)
        {
            if (events[i].events & EPOLLERR || events[i].events & EPOLLHUP ||
                    !(events[i].events & EPOLLIN))
            {
                /* An error on this fd or socket not ready */
                perror("epoll error");
                close(events[i].data.fd);
            }
            else if (events[i].data.fd == socket_fd)
            {
                /* New incoming connection */
                accept_and_add_new();
            }
            else
            {
                /* Data incoming on fd */
                process_new_data(events[i].data.fd);
            }
        }
    }

    free(events);
    close(socket_fd);
    return 0;
}

