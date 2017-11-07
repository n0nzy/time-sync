#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <time.h>

#define MAX_MSG_SIZE 1024
char sendBuffer[MAX_MSG_SIZE];
char recvBuffer[MAX_MSG_SIZE];

void error(const char* msg) {
    perror(msg);
    exit(1);
}

/*
 * Set the server's parameters such as host address and port number.
 */
struct sockaddr_in setup_server_params(const char *hostname, uint16_t port) {
    struct sockaddr_in serverParams;
    memset(&serverParams, '\0', sizeof(struct sockaddr_in));

    serverParams.sin_family = AF_INET;
    serverParams.sin_addr.s_addr = htonl(INADDR_ANY);
    serverParams.sin_port = htons(port);

    return serverParams;
}

int socket_setup() {
    int sockfd = -1;
    if((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        error("\n Error : Could not create socket \n");
    }

    return sockfd;
}

int socket_bind(int sockfd, struct sockaddr_in serv_addr, int port) {

    int recvValue = bind(sockfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr));

    if (recvValue < 0) {
        error("Error in binding");
    }

    return recvValue;  // a value < 0 is a failure
}

int server_connect(int sockfd, struct sockaddr_in serv_addr, int port) {

    memset(&serv_addr, '\0', sizeof(serv_addr));

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(port);

    if(inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr)<=0)
    {
        error("\n inet_pton error occured\n");
    }

    if( connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
    {
        error("\n Error : Connect Failed \n");
    }

    return sockfd;
}

void socket_server_start(int sockfd) {

    /* The accept call blocks until a connection is found */
    int connfd = accept(sockfd, (struct sockaddr*) NULL, NULL);
    close(sockfd);

    printf("\nRequest Accepted:\n");

    while (1) {

        printf("\t\tWaiting for Request data\n");

        ssize_t returnValue; // Data that will be returned to client
        returnValue = read(connfd,recvBuffer,sizeof(recvBuffer)); // This is where server gets input from client
        if (returnValue < 0) error("ERROR reading from socket");

        printf("Data Received from client: %s \n",recvBuffer); // This is displayed on the server end.

        memset(sendBuffer, '\0', sizeof(sendBuffer));
        time_t ticks = time(NULL);
        snprintf(sendBuffer, sizeof(sendBuffer), "%.24s\r\n", ctime(&ticks));
        write(connfd, sendBuffer, strlen(sendBuffer));

        //close(connfd);
        printf("socket: \t\tConnection Completed\n\n");

    }
}

void request_send(int sockfd, char* value) {
    printf("Sending Request...\n");
    memset(recvBuffer, '\0',sizeof(recvBuffer));

    strcpy(sendBuffer, "your_clock_value");  // value to be sent
    int numBytes = write(sockfd, sendBuffer, strlen(sendBuffer));  // sending operation
    if (numBytes < 0) {
        error("Error sending request.");
    }
    printf("Request: sent!\n");

    // Get response...
    memset(recvBuffer, '\0', sizeof(recvBuffer));
    numBytes = read(sockfd, recvBuffer, sizeof(recvBuffer)-1);  // reading response
    if (numBytes < 0) {
        error("ERROR reading from socket");
    }

    printf("Server Response: %s\t",recvBuffer);
    close(sockfd);
}

int main(int argc, char* argv[]) {

    uint32_t portList[3] = {7002, 7003, 7004};

    size_t numberOfPorts = sizeof(portList)/sizeof(portList[0]);
    printf("Number of Processes to send to: %zu\n", numberOfPorts);

    if (argc != 3) {
        error("\n Usage: program port processID\n");
    }

    uint16_t port = atoi(argv[1]);

    /* Check the User Provided Process ID, if it is number 1, then it is the requesting process */
    if (atoi(argv[2]) == 1) {
        printf("I am the sending process\n\n");
        for (int processIndex = 0; processIndex < 3; ++processIndex) {
            printf("Sending request to Process %d on port %d\n", processIndex+1, portList[processIndex]);

            /* Initiate Server Features. */
            struct sockaddr_in serverAddress = setup_server_params("127.0.0.1", portList[processIndex]);
            int sockfd = socket_setup();
            sockfd = server_connect(sockfd, serverAddress, portList[processIndex]);
            request_send(sockfd, "your_clock_value");
            printf("sleeping...\n");
            sleep(60);
        }
    }
    else {
        printf("I am the receiver process: \n Waiting for requests...\n");

        const int CONN_BACKLOG_NUM = 1;  // Max length of queue of pending connections for var sockfd.

        /* Initiate Server Parameters */
        struct sockaddr_in myAddr = setup_server_params("127.0.0.1", port);
        int sockfd = socket_setup();
        socket_bind(sockfd, myAddr, port);

        listen(sockfd, CONN_BACKLOG_NUM);

        printf("listening on sockfd: %d on port: %d \n", sockfd, port);

        socket_server_start(sockfd);
    }

    return 0;
}