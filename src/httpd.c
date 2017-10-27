/********************************************************************************/
/**                                                                            **/
/** T-409-TSAM-2017: Computer Networks Programming Assignment 2 – httpd Part 1 **/
/**          By Alexandra Geirsdóttir & Sigurbjörg Rós Sigurðardóttir          **/
/**                            October 30 2017                                 **/
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
int checkBind(int sockfd, struct sockaddr_in server);
int checkListen(int sockfd);
void reqInit(Request *r);
void ifHead(char *html);
void ifGet(char *html, char *clientPort, char *clientIP, char *requestURL);
void ifPost(char *message, char *html, char *clientPort, char *clientIP);
void ifError(char *html);
void logFile(struct tm * timeinfo, char *clientPort, char *clientIP, 
    char *request, char *requestURL, char *rCode);
int compress(int *compressArr, struct pollfd *fds, int numFds);
void closeConnections(struct pollfd *fds, int numFds);
void addConn(int connFd);
void loopdidoop(int sockfd);

int main(int argc, char *argv[]) {  
    int sockfd = 0, port = 0, rc = 0;
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
    rc = checkBind(sockfd, server);
    //**  Set the listen back log and check if working  **//
    rc = checkListen(sockfd);
    printf("Listening socket reading\n");
    fflush(stdout);

    loopdidoop(sockfd);

    return 0;
}

int getArguments(int argc, char* argv[]) {
    printf("in getArguments\n");
    fflush(stdout);
    if(argc < 2) {
        printf("Error: The server requires a port number.\n");
        fflush(stdout);
        exit(-1);
    }
    return atoi(argv[1]);
}

int checkSocket() {
    printf("in checkSocket\n");
    fflush(stdout);
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        perror("socket() failed");
        exit(-1);
    }
    return sockfd;
}

int checkBind(int sockfd, struct sockaddr_in server) {
    printf("in checkBind\n");
    fflush(stdout);
    int rc = bind(sockfd, (struct sockaddr *) &server, (socklen_t) sizeof(server)); 
    if(rc < 0) {
        perror("bind() failed\n");
        exit(-1);
    }   
    return rc;
}

int checkListen(int sockfd) {
    printf("in checkListen\n");
    fflush(stdout);
    int rc = listen(sockfd, 100);
    if(rc < 0) {
        perror("listen() failed\n");
        close(sockfd);
        exit(-1);
    }
    return rc;
}

//**  Initializer  **//
void reqInit(Request *r) {
    printf("in reqInit\n");
    fflush(stdout);
    r->host = g_string_new(NULL);
    r->path = g_string_new(NULL);
    r->mBody = g_string_new(NULL);
    r->closeCon = false;
    r->method = UNKNOWN;
}    

// Called if a Head request is called
void ifHead(char *html) {
    printf("in ifHead\n");
    fflush(stdout);
    html[0] = '\0';
    strcat(html, "\nHTTP/1.1 200, OK\r\nContent-type: text/html\r\n\r\n" //Connection: keep-alive //Content length  lengdin af html skránni
    "\n<!DOCTYPE>\n<html>\r\n    <head>\n        <meta charset=\"utf-8\">\r\n"
    "    </head>\n </html>\r\n");

}

// Called if a Get request is called
void ifGet(char *html, char *clientPort, char *clientIP, char *requestURL) { 

    printf("in ifGet\n");
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

// Called if a Post request is sent
void ifPost(char *message, char *html, char *clientPort, char *clientIP) {
    printf("in ifPost\n");
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

// Called if an unknown request is called
void ifError(char *html) {
    printf("in ifError\n");
    fflush(stdout);
    html[0] = '\0';
    strcat(html, "\nHTTP/1.1 501, NOTOK\n"
    "<!DOCTYPE html>\n<html>\n    <head>\n        <meta charset=\"utf-8\">\n"
    "    </head>\n    <body>\n        <h2>\n            Error: Not Implemented"
    "\n        </h2>\n    </body>\n</html>");
}

// Logs the information in to file.log
void logFile(struct tm *timeinfo, char *clientPort, char *clientIP, 
             char *request, char *requestURL, char *rCode) {
    printf("in logFile\n");
    fflush(stdout);
    FILE *f;
    // Opens the file or creates it if it does not exist already
    f = fopen("./src/file.log", "a" );

    if (f == NULL) {
        perror("Opening log file failure\n");
        return exit(-1);
    }

    // Prints the information in the file.log 
    char time[25];
    strncpy(time, asctime (timeinfo), 23);
    fprintf(f, "%s : %s:%s %s %s : %s\n", time, clientIP, 
            clientPort, request, requestURL, rCode);
    fclose(f);
}

int compress(int *compressArr, struct pollfd *fds, int numFds) {
    printf("in compress\n");
    fflush(stdout);
    *compressArr = 0;
    for(int i = 0; i < numFds; i++) {
        if(fds[i].fd == -1) {
            for(int j = 0; i < numFds; j++) {
                fds[i].fd = fds[j+1].fd;
            }
            numFds--;
        }
    }
    return numFds;
}

void closeConnections(struct pollfd *fds, int numFds) {
    printf("in closeConnections\n");
    fflush(stdout);
    for (int i = 0; i < numFds; i++) {
        if(fds[i].fd >= 0)
            close(fds[i].fd);
    }
}

void addConn(int connFd) {
    printf("in addConn\n");
    fflush(stdout);
    ClientCon *cc = g_new0(ClientCon, 1);
    int addrlen = sizeof(cc->clientSockaddr);
    getpeername(connFd, (struct sockaddr*)&(cc->clientSockaddr), (socklen_t*)&addrlen);

    cc->conn_fd = connFd;
    cc->conn_timer = g_timer_new();
}

void loopdidoop(int sockfd) {
    printf("in loopdidoop\n");
    fflush(stdout);
    //**  30 second timeout window  **//
    const int TIMEOUT = 30 * 1000;
    int rc = 0, numFds = 1, currentClients = 0, 
        newSD = 0, closeConn = 0, compressArr = 0;
    char clientIP[500], clientPort[32], html[500], message[512];;
    struct sockaddr_in client;
    struct pollfd fds[100];

    //**  Initializing pollfd struct  **//
    memset(fds, 0 , sizeof(fds));
    fds[0].fd = sockfd;
    fds[0].events = POLLIN;
    while(TRUE) {

        socklen_t len = (socklen_t) sizeof(client);
        rc = poll(fds, numFds, TIMEOUT);
        currentClients = numFds;
        if (rc < 0) {
            perror("poll() failed");
            break;
        }
        if (rc == 0) {
            printf("poll() timeout. d");

            for (int i = 0; i < currentClients; i++) {
                close(fds[i].fd);
                fds[i].fd = -1;
                compressArr = 1;
            }
        }

        for(int i = 0; i < currentClients; i++) {
            if(fds[i].revents == 0)
                continue;

            if(fds[i].revents != POLLIN) {
                //printf("Error! revents = %d\n and", fds[i].revents);// POLLIN = %d\n", fds[i].revents, POLLIN);
                closeConn = 1;
            }

            // This is for a new connection
            if(fds[i].fd == sockfd) {   
                if(fds[i].revents & POLLIN) {
                    int connFd = accept(sockfd, (struct sockaddr *) &client, &len);
                    if(connFd < 0) 
                        if(errno != EWOULDBLOCK) 
                            perror("accept() failed");

                    addConn(connFd);

                    // Add the new incoming connection to the poll
                    fds[numFds].fd = newSD;
                    fds[numFds].events = POLLIN;
                    numFds++;
                } 
            }
            else {
                printf("Inside elseeeee\n");
                // This is for an already existing connection
                time_t currenttime;
                struct tm *timeinfo;
                char request[512], mType[5], rCode[8], *requestURL;
                time ( &currenttime );
                timeinfo = localtime ( &currenttime ); 
                
                closeConn = 0;
                
                //todo check if fds[i].revents is set. (nonzero)

                rc = recvfrom(fds[i].fd, &message, sizeof(message) - 1, 0, 
                    (struct sockaddr*)&client, &len);

                if(rc < 0) {
                    if(errno != EWOULDBLOCK) {
                        closeConn = 1;
                    }
                    continue;
                }

                // This is if client closed the connection
                if(rc == 0) {
                    closeConn = 1;
                    continue;
                }

                // This is if data is recieved
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
                    // strncpy(mType, "SERR", sizeof(mType) -1);
                    strncpy(rCode, "Unsupported Request", sizeof(rCode)-1);
                    printf("Does anything happen here? \n");
                }

                printf("about to send \n");
                logFile(timeinfo, clientIP, clientPort, mType, requestURL, rCode);
                rc = send(fds[i].fd, &html, sizeof(html) -1, 0);
                printf("sent\n");

                if(rc < 0) {
                    perror("send() failed");
                    closeConn = 1;
                    break;
                }

                printf("closing conn \n");

                if(closeConn) {
                    close(fds[i].fd);
                    fds[i].fd = -1;
                    compressArr = 1;
                }
            }
        }

        if(compressArr) {
            numFds = compress(&compressArr, (struct pollfd*)&fds, numFds);
        }
    }

    // Closing all sockets that are open
    closeConnections(fds, numFds);
}