#include <netdb.h>
#include <netinet/in.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/time.h>
#include <errno.h>
#include <error.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <fcntl.h>

#include "interface.h"

#define SERVER_PORT 1024

#define FALSE

room *room_db[MAX_CHATROOMS];
int master_sockfd; // this value will change when joining different chat rooms
int indexer_for_ports = 1;
int members; // increments for members added and deceremnts when connection lost with a client
int number_rooms = 0;

int check_array(char *substr)
{
    int db;
    //printf("this is the room name we want to make %s\n", substr);
    for (int i = 0; i < MAX_CHATROOMS; ++i)
    {
        //printf("this is the room name: %s\n", room_db[i]->room_name);

        if (room_db[i] == NULL)
        {
            continue;
        }
        else if (strncmp(room_db[i]->room_name, substr, sizeof(substr)) == 0)
        {

            // if db is 1 then that means the room name already exists
            return db = 1;
        }
    }

    db = 0;
    return db;
}

enum Status handle_delete(char *buff)
{

    // this will get the r1 room name
    char *substr = malloc(256);
    strncpy(substr, buff + 7, 256);
    //printf ("%s\n", substr);

    int db = check_array(substr);

    if (number_rooms == 0 || db == 0)
    {

        // if the number of rooms is 0 then obviously this is a failure
        return FAILURE_NOT_EXISTS;
    }

    else
    {
        for (int i = 0; i < MAX_CHATROOMS; i++)
        {
            if (strncmp(room_db[i]->room_name, substr, sizeof(substr)) == 0)
            {
                // for (int k = 0; k < MAX_CLIENTS; k++)
                // {

                //     if (room_db[i]->slave_clients[k] != 0)
                //     {
                //         char buff[MAX_DATA];
                //         strcpy(buff, "Warning: chatroom closing...");
                //         send(room_db[i]->slave_clients[k], buff, MAX_DATA, 0);
                //     }
                // }

                //printf("here is the master socket: %d\n ", room_db[i]->master_socket);
                close(room_db[i]->master_socket);
                free(room_db[i]);
                room_db[i] = NULL;
                break;
                // figure out how to delete the last comma later
            }
        }

        return SUCCESS;
    }
}
enum Status handle_list(char *list)
{

    if (number_rooms == 0)
    {
        return FAILURE_INVALID;
    }
    for (int i = 0; i < MAX_CHATROOMS; i++)
    {

        if (room_db[i] != NULL)
        {
            strcat(list, room_db[i]->room_name);
            strcat(list, ",");
            //TODO: figure out how to delete the last comma later
        }
    }

    return SUCCESS;
}
int handle_join(char *buff)
{

    int room_index = -1;
    // this will get the r1 room name
    char *substr = malloc(2);
    strncpy(substr, buff + 5, 2);
    //printf("%s\n", substr);

    int slave_socket;

    for (int i = 0; i < MAX_CHATROOMS; i++)
    {
        if (strncmp(room_db[i]->room_name, substr, sizeof(substr)) == 0)
        {
            //printf("JOIN: found the room!!\n");
            room_index = i;

            for (int k = 0; k < MAX_CLIENTS; i++)
            {
                if (room_db[i]->slave_clients[k] == 0)
                {
                    //room_db[i]->slave_clients[k] = slave_socket;
                    room_db[i]->num_members++;
                    return room_index;
                }
            }
        }
    }
    return room_index;
}
enum Status handle_join_2(char *buff)
{

    int room_index = -1;
    // this will get the r1 room name
    char *substr = malloc(2);
    strncpy(substr, buff + 5, 2);
    //printf("%s\n", substr);

    int slave_socket;

    for (int i = 0; i < MAX_CHATROOMS; i++)
    {
        if (strncmp(room_db[i]->room_name, substr, sizeof(substr)) == 0)
        {
            //printf("JOIN: found the room!!\n");
            room_index = i;

            for (int k = 0; k < MAX_CLIENTS; i++)
            {
                if (room_db[i]->slave_clients[k] == 0)
                {
                    //room_db[i]->slave_clients[k] = slave_socket;
                    room_db[i]->num_members++;
                    return SUCCESS;
                }
            }
        }
    }
    return FAILURE_NOT_EXISTS;
}
void add_name(int i, char *substr)
{

    if (room_db[i]->room_name == NULL)
    {

        //room_db[i]->room_name = (char *)malloc(60 * sizeof(char));
        substr = (char *)malloc(60 * sizeof(char));
        strcpy(room_db[i]->room_name, substr);
        //need to figure out how to push substr into database
    }
    else if (room_db[i]->room_name != NULL)
    {
        ++i;
        add_name(i, substr);
    }
}

void instantiate_sfd(room *new_rm)
{
    for (int i = 0; i < MAX_CLIENTS; i++)
    {
        new_rm->slave_clients[i] = 0;
    }
}
enum Status handle_create(char *buff)
{

    //char *buff = "CREATE R1";
    //printf("handle_create: received  %s \n", buff);

    // this will get the r1 room name
    char *substr = malloc(256);
    strncpy(substr, buff + 7, 256);

    // make a room object
    int new_msocket;
    int db = check_array(substr);

    if ((new_msocket = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        perror("server: socket");
        return FAILURE_INVALID;
        exit(-1);
    }
    else if (db == 1)
    {
        return FAILURE_ALREADY_EXISTS;
        exit(-1);
    }
    else if (db == 0)
    {

        for (int i = 0; i < MAX_CHATROOMS; i++)
        {
            //if position is empty
            if (room_db[i] == NULL)
            {
                room *new_rm = malloc(1024);
                //new_rm->room_name[i] = substr;
                memcpy(new_rm->room_name, substr, sizeof(substr));
                new_rm->master_socket = new_msocket;
                new_rm->num_members = 0;
                new_rm->port_num = SERVER_PORT + indexer_for_ports;
                // figure out if want int or client object
                instantiate_sfd(new_rm);
                indexer_for_ports++;
                room_db[i] = new_rm;
                number_rooms++;
                return SUCCESS;
                //break;
            }
        }
    }
}

void create_socket_server(room *room)
{
    /* a child process will always run this */

    //printf("this is the port in join server %i \n", room->port_num);

    int new_fd;
    int rc, length, on, addrlen;
    char buffer[MAX_DATA];
    int wake_up, value_from_client;
    fd_set read_fd;
    struct timeval timeout;
    int opt = 1;
    char buff[MAX_DATA];
    int client_socket[MAX_CLIENTS];
    struct addrinfo hints, *serv;

    struct sockaddr_in serveraddr;
    memset(&serveraddr, 0, sizeof(serveraddr));
    serveraddr.sin_family = AF_INET;
    serveraddr.sin_port = htons(room->port_num);
    serveraddr.sin_addr.s_addr = INADDR_ANY;
    int option = 1;
    setsockopt(room->master_socket, SOL_SOCKET, SO_REUSEADDR, &option, sizeof(option));

    int rv;
    char port[5];
    sprintf(port, "%d", room->port_num);
    if ((rv = getaddrinfo(NULL, port, &hints, &serv)) != 0)
    {
        perror("server: getaddrinfo");
    }

    if (bind(room->master_socket, (struct sockaddr *)&serveraddr, sizeof(serveraddr)) == -1)
    {
        close(room->master_socket);
        perror("server: bind");
        exit(-1);
    }

    if (listen(room->master_socket, 10) == -1)
    {
        perror("listen");
        exit(-1);
    }

    fd_set readfds;
    while (1)
    {

        //printf(" ############ CREATE SOCKET SERVER WHILE LOOP ############ \n");
        FD_ZERO(&readfds);
        FD_SET(room->master_socket, &readfds);
        int max_fd = room->master_socket;
        for (int i = 0; i < MAX_CLIENTS; i++)
        {
            int sd = room->slave_clients[i];
            printf("..%i ", sd);
            if (sd > 0)
                FD_SET(sd, &readfds);
            if (sd > max_fd)
                max_fd = sd;
        }
        //printf("\n");
        // printf("THIS IS THE CHILD MASTER SOCKET: %i\n", room->master_socket);

        wake_up = select(max_fd + 1, &readfds, NULL, NULL, NULL);
        if ((wake_up < 0) && (errno != EINTR))
        {
            printf("select error");
        }

        if (FD_ISSET(room->master_socket, &readfds))
        {
            if ((new_fd = accept(room->master_socket, (struct sockaddr *)&serveraddr, (socklen_t *)&addrlen)) < 0)
            {
                printf("ACCEPT ERROR IN CHILD SERVER \n");
                perror("accept");
                exit(EXIT_FAILURE);
            }
            int flags = fcntl(new_fd, F_GETFL, 0);
            fcntl(new_fd, F_SETFL, flags | O_NONBLOCK);
            //printf("NEW CLIENT IN JOIN SERVER\n");
            for (int i = 0; i < MAX_CLIENTS; i++)
            {
                if (room->slave_clients[i] == 0)
                {
                    room->slave_clients[i] = new_fd;
                    //printf("CHILD SERVER: Adding to list of sockets as %d\n", new_fd);
                    break;
                }
            }
        }
        else
        {
            int sd;
            //printf("CHILD SERVER --- WE RECEIVED SOMETHING FROM CLIENT \n");
            for (int i = 0; i < MAX_CLIENTS; i++)
            {
                if (FD_ISSET(room->slave_clients[i], &readfds))
                {
                    /*we received something from this client*/
                    sd = room->slave_clients[i];
                    break;
                }
            }

            if ((value_from_client = read(sd, buff, MAX_DATA)) == 0)
            {
                printf("closing\n");
                getpeername(sd, (struct sockaddr *)&serveraddr, (socklen_t *)&addrlen);
                close(sd);
            }
            else
            {
                //printf("CHILD SERVER --- RECEIVED MSG: %s \n", buff);
                for (int i = 0; i < MAX_CLIENTS; i++)
                {
                    if (sd != room->slave_clients[i])
                    {
                        if (room->slave_clients[i] != 0)
                        {
                            //printf("CHILD SERVER --- SENDING TO ANOTHER CLIENT %i \n", room->slave_clients[i]);
                            write(room->slave_clients[i], buff, MAX_DATA);
                        }
                    }
                }
            }
        }
    }
}

void delete_socket_server(room *room)
{
    /// delete
}

void handle_received_buffer(char *recv, int ss)
{
    // TODO: dont uppercase the room name here
    touppercase(recv, strlen(recv) - 1);
    //printf("handle_received_buffer: received  %s \n", recv);
    if (strncmp(recv, "JOIN", 4) == 0)
    {
        char str[MAX_DATA];
        int room_index = handle_join(recv);
        if (room_index == -1)
        {
            strcpy(str, "JOIN;FAILURE_NOT_EXISTS;");
            send(ss, str, MAX_DATA, 0);
        }
        else
        {
            //printf("JOIN: returned from handle join!!\n");
            strcpy(str, "JOIN;SUCCESS;");
            char mems[5];
            sprintf(mems, "%d", room_db[room_index]->num_members);
            strcat(str, mems);
            strcat(str, ";");
            char port[5];
            sprintf(port, "%d", room_db[room_index]->port_num);
            strcat(str, port);
            strcat(str, ";");
            //printf("JOIN: What was sent to client %s: \n", str);
            send(ss, str, MAX_DATA, 0);
        }
    }
    if (strncmp(recv, "CREATE", 6) == 0)
    {
        //printf("handle_received_buffer: A CREATE! \n");
        char *substr = malloc(256);
        strncpy(substr, recv + 7, 256);
        //printf ("CREATE: room extracted %s\n", substr);

        // create a room object and initialize all the values
        enum Status stat = handle_create(recv);
        char str[MAX_DATA];
        room *rm_to_start = NULL;
        for (int i = 0; i < MAX_CHATROOMS; i++)
        {
            if (room_db[i] != NULL)
            {
                if (strncmp(room_db[i]->room_name, substr, sizeof(substr)) == 0)
                {
                    //printf("CREATE! we found the room we created %s\n", room_db[i]->room_name);
                    rm_to_start = room_db[i];
                    break;
                }
            }
        }

        if (stat == FAILURE_INVALID)
        {
            strcpy(str, "CREATE;FAILURE_INVALID;");
            send(ss, str, MAX_DATA, 0);
        }
        else if (stat == SUCCESS)
        {
            strcpy(str, "CREATE;SUCCESS;");
            if (!fork())
            {
                // printf("#### CHILD PROCESS: CREATE SERVER!!! JOIN SERVER!!!!!!#### \n");
                close(master_sockfd);
                create_socket_server(rm_to_start);
            }
            else
            {
                // we need to sleep for a little bit for the child to join the server
                sleep(2);
                //printf("CREATE PARENT ---- sending to the client %s\n", str);
                send(ss, str, MAX_DATA, 0);
                //master_sockfd =  rm_to_start->master_socket;
            }
            //send(ss, str, MAX_DATA, 0);
        }
        else if (stat == FAILURE_ALREADY_EXISTS)
        {
            strcpy(str, "CREATE;FAILURE_ALREADY_EXISTS;");
            send(ss, str, MAX_DATA, 0);
        }
    }
    if (strncmp(recv, "LIST", 6) == 0)
    {
        // printf("WE got a list!\n");
        char returned_list[MAX_DATA];
        memcpy(returned_list, "\0", MAX_DATA);
        enum Status stat = handle_list(returned_list);

        char reply[MAX_DATA];
        if (stat == FAILURE_INVALID)
        {
            strcpy(reply, "LIST;FAILURE_INVALID;");
        }

        else if (stat == SUCCESS)
        {
            strcpy(reply, "LIST;SUCCESS;");
            strcat(reply, returned_list);
            strcat(reply, ";");
        }
        reply[strlen(reply) + 1] = '\0';
        //printf("this is the reply: %s\n", reply);
        send(ss, reply, MAX_DATA, 0);
    }
    if (strncmp(recv, "DELETE", 6) == 0)
    {
        enum Status stat = handle_delete(recv);

        char reply[MAX_DATA];
        if (stat == FAILURE_NOT_EXISTS)
        {
            strcpy(reply, "DELETE;FAILURE_NOT_EXISTS");
        }

        else if (stat == SUCCESS)
        {
            strcpy(reply, "DELETE;SUCCESS");
        }
        send(ss, reply, MAX_DATA, 0);
    }
}

int main(int argc, char *argv[])
{

    if (argc != 3)
    {
        fprintf(stderr, "usage: enter host address and port number\n");
        exit(1);
    }

    char *host = argv[1];
    char *port = argv[2];
    int port_int = atoi(port);

    int new_fd; // listen on sock_fd, new connection on new_fd
    int rc, length, on, addrlen;
    char buffer[MAX_DATA];
    int wake_up, value_from_client;
    fd_set read_fd;
    struct timeval timeout;
    int opt = 1;
    char buff[MAX_DATA];
    int client_socket[MAX_CLIENTS];
    struct addrinfo hints, *serv;

    struct sockaddr_in serveraddr;
    memset(&serveraddr, 0, sizeof(serveraddr));
    serveraddr.sin_family = AF_INET;
    serveraddr.sin_port = htons(port_int);
    serveraddr.sin_addr.s_addr = INADDR_ANY;

    //set of socket descriptors
    fd_set readfds;

    for (int i = 0; i < MAX_CLIENTS; i++)
    {
        client_socket[i] = 0;
    }

    for (int i = 0; i < MAX_CHATROOMS; ++i)
    {
        room_db[i] = NULL;
    }

    int rv;
    if ((rv = getaddrinfo(NULL, port, &hints, &serv)) != 0)
    {
        perror("server: getaddrinfo");
        //return -1;
    }

    //create a master socket
    if ((master_sockfd = socket(AF_INET, SOCK_STREAM, 0)) == 0)
    {
        perror("server: socket");
        exit(-1);
    }
    int option = 1;
    setsockopt(master_sockfd, SOL_SOCKET, SO_REUSEADDR, &option, sizeof(option));

    if (bind(master_sockfd, (struct sockaddr *)&serveraddr, sizeof(serveraddr)) == -1)
    {
        close(master_sockfd);
        perror("server: bind");
        exit(-1);
    }

    if (listen(master_sockfd, 10) == -1)
    {
        perror("listen");
        exit(-1);
    }

    printf("Ready for client connect()./n");

    // accepting
    addrlen = sizeof(serveraddr);
    puts("Waiting for connections ...");

    while (1)
    {
        // printf("-------------MAIN WHILE LOOP MASTER %i -------------\n", master_sockfd);
        //zero out the socket set called readfds
        FD_ZERO(&readfds);
        //we also need to add the master to the set
        FD_SET(master_sockfd, &readfds);
        // we need to make sure we have a max value of sockets
        int max_fd = master_sockfd;

        room *room_joined_in = NULL;
        for (int i = 0; i < MAX_CLIENTS; i++)
        {
            int sd = client_socket[i];
            // printf("%i ", sd);
            //if valid socket descriptor then add to read list
            if (sd > 0)
                FD_SET(sd, &readfds);
            //highest file descriptor number, need it for the select function
            if (sd > max_fd)
                max_fd = sd;
        }
        //printf("\n");

        //wait for an a wake up on the set
        wake_up = select(max_fd + 1, &readfds, NULL, NULL, NULL);

        if ((wake_up < 0) && (errno != EINTR))
        {
            printf("select error");
        }

        //master socket wake up means it is a new connection! we need to figure out what it all means
        // usually all the commands will just be in the master socket at first
        if (FD_ISSET(master_sockfd, &readfds))
        {
            if ((new_fd = accept(master_sockfd, (struct sockaddr *)&serveraddr, (socklen_t *)&addrlen)) < 0)
            {
                printf("ACCEPT ERROR IN MAIN \n");
                perror("accept");
                exit(EXIT_FAILURE);
            }
            int flags = fcntl(new_fd, F_GETFL, 0);
            fcntl(new_fd, F_SETFL, flags | O_NONBLOCK);
            //printf("MAIN ---- NEW CONNECTION\n");
            if (room_joined_in == NULL)
            {
                printf("MAIN ---- adding NEW CLIENT \n");
                for (int i = 0; i < MAX_CLIENTS; i++)
                {
                    //if position is empty
                    if (client_socket[i] == 0)
                    {
                        client_socket[i] = new_fd;
                        //printf("MAIN ---- Adding to list of sockets as %d\n", i);
                        break;
                    }
                }
            }
        }
        else
        {
            int sd;
            //printf("MAIN --- WE RECEIVED SOMETHING FROM CLIENT \n");
            for (int i = 0; i < MAX_CLIENTS; i++)
            {
                if (FD_ISSET(client_socket[i], &readfds))
                {
                    sd = client_socket[i];
                    client_socket[i] = 0;
                    break;
                }
            }

            if ((value_from_client = read(sd, buff, MAX_DATA)) == 0)
            {
                printf("closing\n");
                getpeername(sd, (struct sockaddr *)&serveraddr, (socklen_t *)&addrlen);
                close(sd);
            }
            else
            {
                int recv_len = recv(new_fd, buff, MAX_DATA, 0);
                //printf("MAIN ---- RECEIVED BUFFER: %s from socket %i \n", buff, sd);
                handle_received_buffer(buff, sd);
            }
        }

        //printf("----------------END OF ITERATION ON A LOOP ----------------\n\n\n");
    }
}