/********************************************************************************/
/**                                                                            **/
/** T-409-TSAM-2017: Computer Networks Programming Assignment 2 – httpd Part 2 **/
/**          By Alexandra Geirsdóttir & Sigurbjörg Rós Sigurðardóttir          **/
/**                            November 6 2017                                 **/
/**                                                                            **/
/********************************************************************************/

#include <assert.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <string.h>
#include <stdio.h>
#include <stdbool.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <stdbool.h>
#include <netdb.h>
#include <time.h>
#include <poll.h>
#include <unistd.h>
#include <errno.h>
#include <glib.h>

typedef enum httpMeth {GET, HEAD, POST, UNKNOWN} httpMeth;
const char *const https[] = {"GET", "HEAD", "POST", "UNKNOWN"};

//**  Structs  **//
typedef struct ClientCon {
    int conn_fd;
    GTimer *conn_timer;
    struct sockaddr_in clientSockaddr;
}ClientCon;
typedef struct Request {
    httpMeth method;
    GString *host;
    GString *path;
    GString *mBody;
    bool closeCon;
    //GHashTable* headers;
}Request;

int getArguments(int argc, char* argv[]);
int checkSocket();
void checkBind(int sockfd, struct sockaddr_in server);
void checkListen(int sockfd);
void reqInit(Request *r);
void ifHead(char *html);
void ifGet(char *html, char *clientPort, char *clientIP, char *requestURL);
void ifPost(char *message, char *html, char *clientPort, char *clientIP);
void ifError(char *html);
void logFile(struct tm * timeinfo, char *clientPort, char *clientIP, 
             char *request, char *requestURL, char *rCode);
int compress(int *compressArr, struct pollfd *fds, int numFds);
void closeConnections(struct pollfd *fds, int *numFds);
int addConn(int connFd, struct pollfd *fds, int numFds);
void closeConn(int i, struct pollfd *fds, int *compressArr);
int handleConn(int i, struct pollfd *fds, struct sockaddr_in *client, 
               socklen_t len, int *compressArr);
void service(int sockfd);

int main(int argc, char *argv[]) {  
    int sockfd = 0, port = 0;
    struct sockaddr_in server;

    //**  Checks if we have enough arguments  **//
    port = getArguments(argc, argv);
    //**  Create a socket to recieve incoming connections  **//
    sockfd = checkSocket();
    //**  Setting values  **//
    memset(&server, 0, sizeof(server));
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = htonl(INADDR_ANY);
    server.sin_port = htons(port);
    printf("Connection with port: %d\n", port);
    
    //**  Check Binding  **//
    checkBind(sockfd, server);
    //**  Set the listen back log and check if working  **//
    checkListen(sockfd);
    printf("Listening socket reading\n");
    fflush(stdout);
    service(sockfd);

    return 0;
}

int getArguments(int argc, char* argv[]) {
    if(argc < 2) {
        printf("The server requires a port number.\n");
        fflush(stdout);
        exit(-1);
    }
    return atoi(argv[1]);
}

int checkSocket() {
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        perror("socket() failed");
        fflush(stdout);
        exit(-1);
    }
    return sockfd;
}

void checkBind(int sockfd, struct sockaddr_in server) {
    int rc = bind(sockfd, (struct sockaddr *) &server, (socklen_t) sizeof(server)); 
    if(rc < 0) {
        perror("bind() failed\n");
        fflush(stdout);
        exit(-1);
    }   
}

void checkListen(int sockfd) {
    int rc = listen(sockfd, 100);
    if(rc < 0) {
        perror("listen() failed\n");
        fflush(stdout);
        close(sockfd);
        exit(-1);
    }
}

//**  Initializer  **//
void reqInit(Request *r) {
    fflush(stdout);
    r->host = g_string_new(NULL);
    r->path = g_string_new(NULL);
    r->mBody = g_string_new(NULL);
    r->closeCon = false;
    r->method = UNKNOWN;
}    

//**  Called if a Head request is called  **//
void ifHead(char *html) {
    fflush(stdout);
    html[0] = '\0';
    strcat(html, "\nHTTP/1.1 200, OK\r\nContent-type: text/html\r\n\r\n" //Connection: keep-alive //Content length  lengdin af html skránni
    "\n<!DOCTYPE>\n<html>\r\n    <head>\n        <meta charset=\"utf-8\">\r\n"
    "    </head>\n </html>\r\n");

}

//**  Called if a Get request is called  **//
void ifGet(char *html, char *clientPort, char *clientIP, char *requestURL) { 

    fflush(stdout);
    html[0] = '\0';
    strcat(html, "\nHTTP/1.1 200, OK\r\nContent-type: text/html\r\n\r\n"
    "\n<!DOCTYPE>\n<html>\r\n    <head>\n        <meta charset=\"utf-8\">\r\n"
    "    </head>\n    <body>\n        <h2>\n");
    strcat(html, "            http://");
    strcat(html, clientIP);
    strcat(html, ":");
    strcat(html, clientPort);
    strcat(html, requestURL);
    strcat(html, "\n        </h2>\n    </body>\n</html>\r\n");

}

//**  Called if a Post request is sent  **//
void ifPost(char *message, char *html, char *clientPort, char *clientIP) {
    fflush(stdout);
   // same as get request plus the data in the body of the post request
    char data[512];
    strncpy(data, message, sizeof(data)-1);
    char *dataInfo;
    dataInfo = strstr(data, "\r\n\r\n");
    printf("%s\n", dataInfo);

    html[0] = '\0';
    strcat(html, "\nHTTP/1.1 200, OK\nContent-type: text/html\n"
    "\n<!DOCTYPE>\n<html>\n    <head>\n        <meta charset=\"utf-8\">\n"
    "    </head>\n    <body>\n        <h2>\n");
    strcat(html, "            http://");
    strcat(html, clientIP);
    strcat(html, ":");
    strcat(html, clientPort);
    strcat(html, "\n        </h2>\n        <p>");
    strcat(html, dataInfo);
    strcat(html, "\n        </p>\n    </body>\n</html>\n"); 
}

//**  Called if an unknown request is called  **//
void ifError(char *html) {
    fflush(stdout);
    html[0] = '\0';
    strcat(html, "\nHTTP/1.1 501, NOTOK\n"
    "<!DOCTYPE html>\n<html>\n    <head>\n        <meta charset=\"utf-8\">\n"
    "    </head>\n    <body>\n        <h2>\n            Error: Not Implemented"
    "\n        </h2>\n    </body>\n</html>");
}

//**  Logs the information in to file.log  **//
void logFile(struct tm *timeinfo, char *clientPort, char *clientIP, 
             char *request, char *requestURL, char *rCode) {
    fflush(stdout);
    FILE *f;
    //**  Opens the file or creates it if it does not exist already  **//
    f = fopen("./src/file.log", "a" );

    if (f == NULL) {
        perror("Opening log file failure\n");
        return exit(-1);
    }

    //**  Prints the information in the file.log  **//
    char time[25];
    strncpy(time, asctime (timeinfo), 23);
    fprintf(f, "%s : %s:%s %s %s : %s\n", time, clientIP, 
            clientPort, request, requestURL, rCode);
    fclose(f);
}

int compress(int *compressArr, struct pollfd *fds, int numFds) {
    printf("in compressArr\n");
    fflush(stdout);
    *compressArr = 0;
    for(int i = 0; i < numFds; i++) {
        if(fds[i].fd == -1) {
            for(int j = 0; j < numFds; j++) {
                fds[i].fd = fds[j+1].fd;
            }
            
            numFds--;
        }
    }
    return numFds;
}

void closeConnections(struct pollfd *fds, int *numFds) {
    fflush(stdout);
    for (int i = 0; i < *numFds; i++) {
        if(fds[i].fd >= 0)
            close(fds[i].fd);
    }
}

void closeConn(int i, struct pollfd *fds, int *compressArr) {
    shutdown(fds[i].fd, SHUT_RDWR);
    close(fds[i].fd);
    fds[i].fd = -1;
    *compressArr = 1;
}

int addConn(int connFd, struct pollfd *fds, int numFds) {
    ClientCon *cc = g_new0(ClientCon, 1);
    int addrlen = sizeof(cc->clientSockaddr);
    getpeername(connFd, (struct sockaddr*)&(cc->clientSockaddr), 
               (socklen_t*)&addrlen);

    cc->conn_fd = connFd;
    cc->conn_timer = g_timer_new();

    fds[numFds].fd = connFd;
    fds[numFds].events = POLLIN;
    numFds++;
    return numFds;
}

int handleConn(int i, struct pollfd *fds, struct sockaddr_in *client, 
               socklen_t len, int *compressArr) {
    time_t currenttime;
    struct tm *timeinfo;
    char request[512], mType[5], rCode[8], *requestURL, clientIP[500], 
         clientPort[32], html[500], message[512];
    time (&currenttime);
    timeinfo = localtime (&currenttime); 

    int rc = recvfrom(fds[i].fd, &message, sizeof(message) - 1, 0, 
             (struct sockaddr*)&client, &len);

    if(rc < 0) {
        if(errno != EWOULDBLOCK) {
            closeConn(i, fds, compressArr);
        }
        return rc;
    }

    //**  This is if client closed the connection  **//
    if(rc == 0) {
        closeConn(i, fds, compressArr);
        return rc;
    }

    //**  This is if data is recieved  **//
    strncpy(request, message, sizeof(request)-1);
    requestURL = strchr(request, '/');
    requestURL = strtok(requestURL, " ");
    memcpy(mType, &message[0], 4);
    mType[4] = '\0';
    strcpy(rCode, "200, OK");

    if(!(strcmp(mType, "GET "))) {
        printf("Get request\n");
        ifGet(html, clientPort, clientIP, requestURL);
    }
    else if(!(strcmp(mType, "POST"))) {
        printf("Post request\n");
        ifPost(message, html, clientPort, clientIP);
    }
    else if(!(strcmp(mType, "HEAD"))) {
        printf("Head request\n");
        ifHead(html);
    }
    else {
        printf("%s requests are unsupported by the server.\n", mType);
        ifError(html);
        strncpy(rCode, "Unsupported Request", sizeof(rCode)-1);
    }

    logFile(timeinfo, clientIP, clientPort, mType, requestURL, rCode);
    rc = send(fds[i].fd, &html, sizeof(html) -1, 0);

    if(rc < 0) {
        perror("send() failed");
        closeConn(i, fds, compressArr);
    }   

    return rc;
}

void service(int sockfd) {
    fflush(stdout);
    //**  30 second timeout window  **//
    const int TIMEOUT = 30 * 1000;
    //**  Initializing variables  **//
    int  numFds = 1, currentClients = 0, compressArr = 0;;
    struct sockaddr_in client;
    struct pollfd fds[100];

    //**  Initializing pollfd struct  **//
    memset(fds, 0 , sizeof(fds));
    fds[0].fd = sockfd;
    fds[0].events = POLLIN;
    while(TRUE) {
        fflush(stdout);
        socklen_t len = (socklen_t) sizeof(client);
        int rc = poll(fds, numFds, TIMEOUT);
        currentClients = numFds;
        if (rc < 0) {
            perror("poll() failed\n");
            break;
        }
        if (rc == 0) {
            printf("poll() timeout. \n");
            closeConnections(fds, &numFds);
            //close(connection->conn_fd);
            //g_timer_destroy(connection->conn_timer);
            /*
            for(int i = 0; i < currentClients; i++) {
                close(fds[i].fd);
                fds[i].fd = -1;
                compressArr = 1;
            }*/
        }

        for(int i = 0; i < currentClients; i++) {
            fflush(stdout);
            if(fds[i].revents == 0){
                continue;
            }
            if(fds[i].revents & POLLHUP) {
                printf("Closing connection because the device has been disconnected. \n");
                closeConn(i, fds, &compressArr);
                continue;
            }
            if(fds[i].revents & POLLERR) {
                printf("An error has occurred on the device or stream. \n");
                break;
            }

            //**  This is for a new connection  **//
            if(fds[i].fd == sockfd) { 
                if(fds[i].revents & POLLIN) {
                    int connFd = accept(sockfd, (struct sockaddr *) &client, &len);
                    if(connFd < 0) 
                        if(errno != EWOULDBLOCK) 
                            perror("accept() failed");
                    fflush(stdout);
                    //**  Add the new incoming connection to the poll  **//
                    numFds = addConn(connFd, fds, numFds);
                } 
            }
            else {
                // todo here if timeout then end connection
                if(fds[i].revents & POLLIN) {
                    //**  This is for an already existing connection  **//
                    rc = handleConn(i, fds, &client, len, &compressArr);
                    if(rc <= 0)
                        continue;
                }
            }
        }
        
        if(compressArr) {
            numFds = compress(&compressArr, (struct pollfd*)&fds, numFds);
        }
    }

    //**  Closing all sockets that are open  **//
    closeConnections(fds, &numFds);
}
