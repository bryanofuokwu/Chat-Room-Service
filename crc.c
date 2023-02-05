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
#include <sys/types.h>
#include <sys/wait.h>
#include "interface.h"

/*
 * TODO: IMPLEMENT BELOW THREE FUNCTIONS
 */
int connect_to(const char *host, int port);
struct Reply process_command(const int sockfd, char *command);
void process_chatmode(const char *host, int port);

int main(int argc, char **argv)
{
    if (argc != 3)
    {
        fprintf(stderr,
                "usage: enter host address and port number\n");
        exit(1);
    }

    display_title();

    while (1)
    {

        // argv1  = host 127.0.0.1 or localhost
        // argv2 = port = 1024 that is the port number we are starting first
        int sockfd = connect_to(argv[1], atoi(argv[2]));

        char command[MAX_DATA];
        // this gets a command from the command line
        get_command(command, MAX_DATA);

        struct Reply reply = process_command(sockfd, command);
        // we show the reply
        display_reply(command, reply);

        touppercase(command, strlen(command) - 1);
        if (strncmp(command, "JOIN", 4) == 0)
        {
            printf("Now you are in the chatmode\n");
            process_chatmode(argv[1], reply.port);
        }

        close(sockfd);
    }

    return 0;
}

/*
 * Connect to the server using given host and port information
 *
 * @parameter host    host address given by command line argument
 * @parameter port    port given by command line argument
 *
 * @return socket fildescriptor
 */
int connect_to(const char *host, int port)
{
    // ------------------------------------------------------------
    // GUIDE :
    // In this function, you are suppose to connect to the server.
    // After connection is established, you are ready to send or
    // receive the message to/from the server.
    //
    // Finally, you should return the socket fildescriptor
    // so that other functions such as "process_command" can use it
    // ------------------------------------------------------------

    // instantiating sockfd
    int sockfd = -1;

    char port_char[64];
    sprintf(port_char, "%d", port);

    struct addrinfo hints, *res;
    // first, load up address structs with getaddrinfo():
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    int status;
    if ((status = getaddrinfo(host, port_char, &hints, &res)) != 0)
    {
        perror("Cannot getaddrinfot");
        exit(-1);
    }

    // THIS IS THE SOCKET CREATION
    sockfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
    if (sockfd < 0)
    {
        perror("Cannot create socket");
        exit(-1);
    }

    // connect!
    if (connect(sockfd, res->ai_addr, res->ai_addrlen) < 0)
    {
        perror("Cannot Connect");
        exit(-1);
    }

    return sockfd;
}

/*
 * Send an input command to the server and return the result
 *
 * @parameter sockfd   socket file descriptor to commnunicate
 *                     with the server
 * @parameter command  command will be sent to the server
 *
 * @return    Reply
 */

void split_up(char *line)
{
    char *cmd = strtok(line, ";");

    while (cmd != NULL)
    {
        printf("%s\n", cmd);
        cmd = strtok(NULL, ";");
    }
}
void substring(char s[], char sub[], int p, int l)
{
    int c = 0;
    //printf("in func %s\n", s);
    while (c < l)
    {
        sub[c] = s[p + c];
        c++;
    }
    sub[c] = '\0';
}
struct Reply process_command(const int sockfd, char *command)
{
    // ------------------------------------------------------------
    // GUIDE 1:
    // In this function, you are supposed to parse a given command
    // and create your own message in order to communicate with
    // the server. Surely, you can use the input command without
    // any changes if your server understand it. The given command
    // will be one of the followings:
    //
    // CREATE <name>
    // DELETE <name>
    // JOIN <name>
    // LIST
    //
    // -  "<name>" is a chatroom name that you want to create, delete,
    // or join.
    //
    // - CREATE/DELETE/JOIN and "<name>" are separated by one space.
    // ------------------------------------------------------------

    // we need to send a buffer to the server
    char sent_buffer[MAX_DATA];
    strcpy(sent_buffer, command);

    // ------------------------------------------------------------
    // GUIDE 2:
    // After you create the message, you need to send it to the
    // server and receive a result from the server.
    // ------------------------------------------------------------

    int sent_bytes = send(sockfd, sent_buffer, MAX_DATA, 0);
    //printf("sent in the client %s\n", sent_buffer);

    // now we need to receive from the server
    char recv_buffer[MAX_DATA];
    memset(recv_buffer, '\0', sizeof(recv_buffer));

    // we receive the length of the recv_buffer and the buffer has the contents now
    int recv_len = recv(sockfd, recv_buffer, MAX_DATA, 0);
    printf("recv in the client %s\n", recv_buffer);

    // ------------------------------------------------------------
    // GUIDE 3:
    // Then, you should create a variable of Reply structure
    // provided by the interface and initialize it according to
    // the result.
    //
    // For example, if a given command is "JOIN room1"
    // and the server successfully created the chatroom,
    // the server will reply a message including information about
    // success/failure, the number of members and port number.
    // By using this information, you should set the Reply variable.
    // the variable will be set as following:
    //
    // Reply reply;
    // reply.status = SUCCESS;
    // reply.num_member = number;
    // reply.port = port;
    //
    // "number" and "port" variables are just an integer variable
    // and can be initialized using the message fomr the server.
    //
    // For another example, if a given command is "CREATE room1"
    // and the server failed to create the chatroom becuase it
    // already exists, the Reply varible will be set as following:
    //
    // Reply reply;
    // reply.status = FAILURE_ALREADY_EXISTS;
    //
    // For the "LIST" command,
    // You are suppose to copy the list of chatroom to the list_room
    // variable. Each room name should be seperated by comma ','.
    // For example, if given command is "LIST", the Reply variable
    // will be set as following.
    //
    // Reply reply;
    // reply.status = SUCCESS;
    // strcpy(reply.list_room, list);
    //
    // "list" is a string that contains a list of chat rooms such
    // as "r1,r2,r3,"
    // ------------------------------------------------------------

    char info_for_reply[5][MAX_DATA] = {"\0", "\0", "\0", "\0", "\0"};
    //char *info_for_reply[5] = {"\0","\0", "\0","\0","\0"};
    int i = 0;
    char *pch;

    int before = 0;
    int after = 0;
    int info = 0;
    int difference = 0;
    // change this to a while loop
    for (int i = 0; i < strlen(recv_buffer); i++)
    {
        if (recv_buffer[i] == ';')
        {
            //printf("%i\n", i);
            char help[256];
            after = i;
            difference = after - before;
            substring(recv_buffer, help, before, difference);
            before = after + 1;
            strcpy(info_for_reply[info], help);
            info++;
        }
    }

    if (strncmp(info_for_reply[1], "SUCCESS", 7) == 0)
    {
        enum Status stat = SUCCESS;
    }
    else if (strncmp(info_for_reply[1], "FAILURE_ALREADY_EXISTS", 22) == 0)
    {
        enum Status stat = FAILURE_ALREADY_EXISTS;
    }
    else if (strncmp(info_for_reply[1], "FAILURE_NOT_EXISTS", 18) == 0)
    {
        enum Status stat = FAILURE_NOT_EXISTS;
    }
    else if (strncmp(info_for_reply[1], "FAILURE_INVALID", 15) == 0)
    {

        enum Status stat = FAILURE_INVALID;
    }
    else if (strncmp(info_for_reply[1], "FAILURE_UNKNOWN", 15) == 0)
    {
        enum Status stat = FAILURE_UNKNOWN;
    }

    struct Reply reply;
    if (strncmp(info_for_reply[0], "CREATE", 6) == 0)
    {
        reply.status = SUCCESS;
        //strcpy(reply.status, info_for_reply[1]);
    }
    else if (strncmp(info_for_reply[0], "DELETE", 6) == 0)
    {
        reply.status = SUCCESS;
    }
    else if (strncmp(info_for_reply[0], "JOIN", 4) == 0)
    {
        reply.status = SUCCESS;
        char *mem = info_for_reply[2];
        char *por = info_for_reply[3];
        reply.num_member = atoi(mem);
        reply.port = atoi(por);
    }

    else if (strncmp(info_for_reply[0], "LIST", 4) == 0)
    {

        reply.status = SUCCESS;
        if (info_for_reply[2] != "\0")
        {
            // TODO: check this again
            strcpy(reply.list_room, info_for_reply[2]);
        }
    }

    return reply;
}

/*
 * Get into the chat mode
 *
 * @parameter host     host address
 * @parameter port     port
 */

// this is when we are in chat mode we need to handle the deleting chat mode too
// we need to delete by deleting the master socket
void process_chatmode(const char *host, int port)
{
    // ------------------------------------------------------------
    // GUIDE 1:
    // In order to join the chatroom, you are supposed to connect
    // to the server using host and port.
    // You may re-use the function "connect_to".
    // ------------------------------------------------------------

    // if r1 then port 1025 if r2 then 1026
    printf("CLIENT: connecting to port: %i\n", port);
    int sockfd = connect_to(host, port);
    printf("CLIENT: SOCKET: %i\n", sockfd);
    // ------------------------------------------------------------
    // GUIDE 2:
    // Once the client have been connected to the server, we need
    // to get a message from the user and send it to server.
    // At the same time, the client should wait for a message from
    // the server.
    // ------------------------------------------------------------
    char msg_buffer[MAX_DATA];
    fd_set readfds;
    while (1)
    {
        FD_ZERO(&readfds);
        //we also need to add the master to the set
        FD_SET(STDIN_FILENO, &readfds);

        FD_SET(sockfd, &readfds);
        //wait for an a wake up on the set
        int wake_up = select(sockfd + 1, &readfds, NULL, NULL, NULL);

        if ((wake_up < 0) && (errno != EINTR))
        {
            printf("select error");
        }

        if (FD_ISSET(STDIN_FILENO, &readfds))
        {
            get_message(msg_buffer, MAX_DATA);
            fflush(stdin);
            send(sockfd, msg_buffer, MAX_DATA, 0);
        }
        else
        {
            int recv_len = recv(sockfd, msg_buffer, MAX_DATA, 0);
            display_message(msg_buffer);
            printf("\n");

            // TODO: parse the warning here
            if (strncmp(msg_buffer, "Warning: chatroom closing...", 4) == 0)
            {
                break;
            }
        }
    }

    // ------------------------------------------------------------
    // IMPORTANT NOTICE:
    // 1. To get a message from a user, you should use a function
    // "void get_message(char*, int);" in the interface.h file
    //
    // 2. To print the messages from other members, you should use
    // the function "void display_message(char*)" in the interface.h
    //
    // 3. Once a user entered to one of chatrooms, there is no way
    //    to command mode where the user  enter other commands
    //    such as CREATE,DELETE,LIST.
    //    Don't have to worry about this situation, and you can
    //    terminate the client program by pressing CTRL-C (SIGINT)
    // ------------------------------------------------------------
}
