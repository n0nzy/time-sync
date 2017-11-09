#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <time.h>
#include <math.h>

#define MAX_MSG_SIZE 1024
char sendBuffer[MAX_MSG_SIZE];
char recvBuffer[MAX_MSG_SIZE];

#define TRUE   1
#define SLEEP_DELAY 2

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
    int sockfd = -1; int opt = TRUE;
    if((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        error("\n Error : Could not create socket \n");
    }

    //set master socket to allow multiple connections , this is just a good habit, it will work without this
    if( setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, (char *)&opt, sizeof(opt)) < 0 ) {
        error("setsockopt");
    }
    return sockfd;
}

int socket_bind(int sockfd, struct sockaddr_in serv_addr) {

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

/*
 * This is a request listening service, it services all types of requests from the client;
 * it needs to parse the type of request to determine the appropriate action.
 * The clock value is passed by value to make some (integer) manipulations and easily print to stdout (for debugging purposes).
 * The clock value is also passed by reference so as to update the actual process's internal clock when it is receives the new offset from the client.
 * */
void socket_server_start(int sockfd, int clock, int* clockref) {

    /* The accept call blocks until a connection is found */
    int connfd = accept(sockfd, (struct sockaddr*) NULL, NULL);
    //close(sockfd);

    //printf("\nRequest Accepted:\n");

    printf("\t::Waiting for New Request data\n");

    int numBytes = read(connfd,recvBuffer,sizeof(recvBuffer)-1); // This is where server gets input from client
    if (numBytes < 0) error("ERROR reading from socket");
    //printf("Data Received from client: %s \n",recvBuffer); // This is displayed on the server end.
    char* token = strtok(recvBuffer, " "); // extract request data, get the first token, i.e. the <key> in [key: value]

    /* Determine the type of request: is it a request for my current clock OR to set my clock? */
    if (strstr(token, "your_clock_value:")) {  // This is the first request asking for my current clock
        //printf("Label for clock_value_request\n");
        token = strtok(NULL, " "); // fetch the next token in recvBuffer, i.e. the <value> in [key: value]

        /* Data for client... */
        printf("Preparing Response...\n");
        memset(sendBuffer, '\0', sizeof(sendBuffer)); // clear send buffer before sending response data
        sprintf(sendBuffer, "%d", clock); // strcpy(sendBuffer, sprintf(clock));
        numBytes = write(connfd, sendBuffer, strlen(sendBuffer));
        if (numBytes < 0) {
            error("Error sending response.");
        }
        printf("My Clock Value: %d\n", clock);
        printf("Response: My current clock value sent!\n");
    }

    if (strstr(token, "set_clock:")) { // This is the second request to update my clock.
        //printf("Label for set_clock\n");
        token = strtok(NULL, " "); // fetch the next token in recvBuffer, i.e. the <value> in [key: value]

        int offset = atoi(token); // convert new clock value to int
        clock = clock + offset;  // set new value for clock.
        *clockref = clock;

        /* Data for client... */
        printf("Preparing Response...\n");
        memset(sendBuffer, '\0', sizeof(sendBuffer)); // clear send buffer before sending response data
        sprintf(sendBuffer, "%d", clock);
        numBytes = write(connfd, sendBuffer, strlen(sendBuffer));
        if (numBytes < 0) {
            error("Error sending response.");
        }
        printf("My Clock Value: %d\n", clock);
        printf("Response: Clock Updated! New Clock value sent!\n _______________________ \n\n");
    }

    //printf("My Clock Value: %d\n", clock);
    memset(recvBuffer, '\0', sizeof(recvBuffer));   // after retrieving value, reset recvBuffer for next request.

}

void set_clock(int sockfd, int value) {
    printf("Sending new clock offset Request...\n");
    memset(recvBuffer, '\0',sizeof(recvBuffer));

    printf("new clock offset: %d\n", value);
    sprintf(sendBuffer, "set_clock: %d", value); // new value to be sent...
    int numBytes = write(sockfd, sendBuffer, strlen(sendBuffer));  // sending operation
    if (numBytes < 0) {
        error("Error sending request.");
    }
    printf("Clock Offset Request: sent!\n");

    // Get response...
    memset(recvBuffer, '\0', sizeof(recvBuffer));
    numBytes = read(sockfd, recvBuffer, sizeof(recvBuffer)-1);  // reading response
    if (numBytes < 0) {
        error("ERROR reading from socket");
    }

    printf("Server Response for clock offset: %s\t",recvBuffer);
    printf("sleeping...\n\n");
    sleep(SLEEP_DELAY);
    //close(sockfd);
}

char* get_clock(int sockfd, char* value) {
    printf("Sending Request...\n");
    memset(recvBuffer, '\0',sizeof(recvBuffer));

    strcpy(sendBuffer, "your_clock_value: 1001");  // value to be sent
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

    printf("Server Response for clock value: %s\t",recvBuffer);
    printf("sleeping...\n\n");
    sleep(SLEEP_DELAY);
    //close(sockfd);
    return recvBuffer;
}

int main(int argc, char* argv[]) {

    uint32_t portList[3] = {7002, 7003, 7004};
    uint8_t clocks[3] = {0,0,0};
    int total = 0;

    size_t numberOfPorts = sizeof(portList)/sizeof(portList[0]);
    //printf("Number of Processes to send to: %zu\n", numberOfPorts);

    if (argc != 3) {
        error("\n Usage: program port processID\n");
    }

    uint16_t port = atoi(argv[1]);

    time_t timer;
    srand((unsigned) time(&timer));

    //int clock = atoi(argv[2]);  // must be a signed integer because clock offset may be negative.

    /* clock must be a signed integer because the after update by clock offset from client, it might be negative */
    int clock = rand() % 20;  // generates a random number between 1 and 20

    /* Check the User Provided Process ID, if it is number 1, then it is the requesting process */
    if (atoi(argv[2]) == 1) {
        printf("I am the sending process\n\n");

        for (int processIndex = 0; processIndex < numberOfPorts; ++processIndex) {
            /* Initiate Server Features. */
            struct sockaddr_in serverAddress = setup_server_params("127.0.0.1", portList[processIndex]);
            int sockfd = socket_setup();

            printf("Requesting clock value from Process %d with sockfd %d on port %d\n", processIndex+1, sockfd, portList[processIndex]);
            sockfd = server_connect(sockfd, serverAddress, portList[processIndex]);

            clocks[processIndex] = atoi(get_clock(sockfd, "your_clock_value"));
            total = total + clocks[processIndex];
        }
        // All clock values have been received at this point, so calculate synchronizing average...
        int average = (total / numberOfPorts);

        printf("________________________\n\n");
        // new request to send updated clock offset values
        for (int processIndex = 0; processIndex < numberOfPorts; ++processIndex) {
            /* Initiate Server Features. */
            struct sockaddr_in serverAddress = setup_server_params("127.0.0.1", portList[processIndex]);
            int sockfd = socket_setup();

            sockfd = server_connect(sockfd, serverAddress, portList[processIndex]);
            clock = average - clocks[processIndex]; // calculate clock offset
            printf("Sending new clock value offset to Process %d with sockfd %d on port %d\n", processIndex+1, sockfd, portList[processIndex]);
            set_clock(sockfd, clock); //send updated clock offset value
        }
    }
    else {
        printf("I am the receiver process: \t Waiting for requests...\n\n");

        const int CONN_BACKLOG_NUM = 1;  // Max length of queue of pending connections for var sockfd.

        /* Initiate Server Parameters */
        struct sockaddr_in myAddr = setup_server_params("127.0.0.1", port);

        while (1) {
            int sockfd = socket_setup();

            //inform user of socket number - used in send and receive commands
            printf("\nNew connection, socket fd is %d , ip is : %s , port : %d \n" , sockfd , inet_ntoa(myAddr.sin_addr) , ntohs(myAddr.sin_port));

            socket_bind(sockfd, myAddr);

            listen(sockfd, CONN_BACKLOG_NUM);

            printf("listening on sockfd: %d on port: %d \n", sockfd, port);

            socket_server_start(sockfd, clock, &clock); // pass a ref to the processes clock to update later.
            close(sockfd);
        }
    }

    return 0;
}