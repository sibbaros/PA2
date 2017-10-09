/********************************************************************************/
/**                                                                            **/
/** T-409-TSAM-2017: Computer Networks Programming Assignment 2 – httpd Part 1 **/
/**          By Alexandra Geirsdóttir & Sigurbjörg Rós Sigurðardóttir          **/
/**                             October 9 2017                                 **/
/**                                                                            **/
/********************************************************************************/

#include <assert.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <string.h>
#include <stdio.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <stdbool.h>
#include <netdb.h>
#include <time.h>
#include <poll.h>
#include <unistd.h>
#include <errno.h>

void ifHead(char * html);
void ifGet(char *html, char *clientPort, char *clientIP, char *requestURL);
void ifPost(char *message, char *html, char *clientPort, char *clientIP);
void ifError(char *html);
void logFile(struct tm * timeinfo, char *clientPort, char *clientIP, 
             char *request, char *requestURL, char *rCode);
void addConn(int *sockfd, const struct sockaddr *client, int newSD, 
             int *endServ, struct pollfd *fds, int *numFds, socklen_t *len);
void servConn(int *closeConn, struct pollfd *fds, char *message, struct sockaddr *client,
              socklen_t *len, int *rc, char *html, char *clientPort, char *clientIP, int *i,
              int *compressArr);
void compress(int *compressArr, struct pollfd *fds, int *numFds);
void closeConnections(struct pollfd *fds, int numFds);

int main(int argc, char *argv[]) {
    // 3 minute timeout window
    const int TIMEOUT = 3 * 60 * 1000;
    int sockfd, port, rc, numFds = 1, currentClients, endServ = 0, 
        newSD = 0, closeConn, compressArr = 0;
    char clientIP[500], clientPort[32], ipAddr[INET_ADDRSTRLEN], html[500];
    struct sockaddr_in server, client;
    struct pollfd fds[100];
    char message[512];

    // Checks if we have enough arguments
    if(argc < 2) {
         printf("Error: The server requires a port number.\n");
         fflush(stdout);
        return -1;
    }
    
    // Create a socket to recieve incoming connections and checking if it works
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        perror("socket() failed");
        exit(-1);
    }

    memset(&server, 0, sizeof(server));
    
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = htonl(INADDR_ANY);
    
    port = atoi(argv[1]);
    server.sin_port = htons(port);
    printf("Connection with port: %d\n", port);
    printf("printing argc: %d\n", argc);
    
    bind(sockfd, (struct sockaddr *) &server, (socklen_t) sizeof(server));    
    // Set the listen back log and check if working
    rc = listen(sockfd, 100);
    if(rc < 0) {
        perror("listen() failed");
        close(sockfd);
        exit(-1);
    }

    // Initializing pollfd struct
    memset(fds, 0 , sizeof(fds));
    fds[0].fd = sockfd;
    fds[0].events = POLLIN;
    do {
        socklen_t len = (socklen_t) sizeof(client);
        rc = poll(fds, numFds, TIMEOUT);
        if (rc < 0) {
            perror("poll() failed");
            break;
        }
        if (rc == 0) {
            printf("poll() timeout. Ending program.");
            break;
        }

        currentClients = numFds;
        for(int i = 0; i < currentClients; i++) {
            getnameinfo((struct sockaddr *)&client, len, clientIP, sizeof(clientIP), 
                        clientPort, sizeof(clientPort), NI_NUMERICHOST | NI_NUMERICSERV);
            inet_ntop(AF_INET, &clientIP, ipAddr, INET_ADDRSTRLEN);
            if(fds[i].revents == 0)
                continue;

            if(fds[i].revents != POLLIN) {
                printf("Error! revents = %d\n", fds[i].revents);
                closeConn = 1;
            }

            // This is for a new connection
            if(fds[i].fd == sockfd) {    
                addConn(&sockfd, (struct sockaddr *)&client, newSD, 
                        &endServ, (struct pollfd*)&fds, &numFds, &len);
            }
            else {
                // This is for an already existing connection
                servConn(&closeConn, (struct pollfd *) &fds, (char *) &message, 
                        (struct sockaddr *) &client, &len, &rc, html, (char *) &clientPort, 
                        (char *) &clientIP, &i, &compressArr);
            }
        }

        if(compressArr) {
            compress(&compressArr, (struct pollfd*)&fds, &numFds);
        }
    } while(endServ == 0);

    // Closing all sockets that are open
    closeConnections(fds, numFds);

    return 0;
}

// Called if a Head request is called
void ifHead(char * html) {
    html[0] = '\0';
    strcat(html, "\nHTTP/1.1 200, OK\r\nContent-type: text/html\r\n\r\n"
    "\n<!DOCTYPE>\n<html>\r\n    <head>\n        <meta charset=\"utf-8\">\r\n"
    "    </head>\n </html>\r\n");

}

// Called if a Get request is called
void ifGet(char *html, char *clientPort, char *clientIP, char *requestURL) {  
    html[0] = '\0';
    strcat(html, "\nHTTP/1.1 200, OK\r\nContent-type: text/html\r\n\r\n"
    "\n<!DOCTYPE>\n<html>\r\n    <head>\n        <meta charset=\"utf-8\">\r\n"
    "    </head>\n    <body>\n        <h1>\n");
    strcat(html, "            http://");
    strcat(html, clientIP);
    strcat(html, ":");
    strcat(html, clientPort);
    strcat(html, requestURL);
    strcat(html, "\n        </h1>\n    </body>\n</html>\r\n");
}

// Called if a Post request is sent
void ifPost(char *message, char *html, char *clientPort, char *clientIP) {
   // same as get request plus the data in the body of the post request
    char data[512];
    strncpy(data, message, sizeof(data)-1);
    char *dataInfo;
    dataInfo = strstr(data, "\r\n\r\n");
    printf("%s\n", dataInfo);


    html[0] = '\0';
    strcat(html, "\nHTTP/1.1 200, OK\nContent-type: text/html\n"
    "\n<!DOCTYPE>\n<html>\n    <head>\n        <meta charset=\"utf-8\">\n"
    "    </head>\n    <body>\n        <h1>\n");
    strcat(html, "            http://");
    strcat(html, clientIP);
    strcat(html, ":");
    strcat(html, clientPort);
    strcat(html, "\n        </h1>\n        <p>");
    strcat(html, dataInfo);
    strcat(html, "\n        </p>\n    </body>\n</html>\n"); 
}

// Called if an unknown request is called
void ifError(char *html) {
    html[0] = '\0';
    strcat(html, "\nHTTP/1.1 404, NOTOK\n"
    "<!DOCTYPE html>\n<html>\n    <head>\n        <meta charset=\"utf-8\">\n"
    "    </head>\n    <body>\n        <h2>\n            ~~OOPS something went wrong~~"
    "\n        </h2>\n    </body>\n</html>");
}

// Logs the information in to file.log
void logFile(struct tm * timeinfo, char *clientPort, char *clientIP, 
             char *request, char *requestURL, char *rCode) {
    FILE *f;
    // Opens the file or creates it if it does not exist already
    f = fopen("./src/file.log", "a" );
    // Prints the information in the file.log 
    fprintf(f, "%s : %s:%s %s %s : %s\n", asctime (timeinfo), clientIP, 
            clientPort, request, requestURL, rCode);
    fclose(f);
}

void addConn(int *sockfd, const struct sockaddr *client, int newSD, 
             int *endServ, struct pollfd *fds, int *numFds, socklen_t *len) {
    printf("Listening socket reading\n");
    
    len = (socklen_t*) sizeof(client);
    newSD = accept((int) &sockfd, (struct sockaddr *) &client, (socklen_t*) &len);
    if(newSD < 0) {
        if(errno != EWOULDBLOCK) {
            perror("accept() failed");
            endServ = (int *)1;
        }
        return;
    }
    
    // Add the new incoming connection to the poll
    fds[(int)numFds].fd = (int)newSD;
    fds[(int)numFds].events = POLLIN;
    numFds++;
}

void servConn(int *closeConn, struct pollfd *fds, char *message, struct sockaddr *client,
              socklen_t *len, int *rc, char *html, char *clientPort, char *clientIP, int *i,
              int *compressArr) {
    time_t currenttime;
    struct tm * timeinfo;
    char request[512], mType[5], rCode[8], *requestURL;
    time ( &currenttime );
    timeinfo = localtime ( &currenttime );
    closeConn = 0;    
                    
    *rc = recvfrom(fds[(int)i].fd, &message, sizeof(message) - 1, 0, 
        (struct sockaddr*)&client, (socklen_t*) &len);
    if(rc < 0) {
        if(errno != EWOULDBLOCK) {
            closeConn = (int*) 1;
        }
        return;
    }
    // This is if client closed the connection
    if(rc == 0) {
        closeConn = (int*) 1;
        return;
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
        //memcpy(data, &message[5], 400);
        ifPost(message, html, clientPort, clientIP);
    }
    else if(!(strcmp(mType, "HEAD"))) {
        printf("Head request\n");
        ifHead(html);
    }
    else {
        printf("ERROR: The requested type is not supported.\n");
        ifError(html);
        strncpy(mType, "ERROR", sizeof(mType) -1);
        strncpy(rCode, "404, ERROR", sizeof(rCode)-1);
        closeConn = (int*) 1;
        return;
    }

    logFile(timeinfo, clientPort, clientIP, mType, requestURL, rCode); // response code
    *rc = send(fds[(int)i].fd, &html, sizeof(html) -1, 0);

    if(rc < 0) {
        perror("send() failed");
        closeConn = (int*) 1;
        return;
    }

    if(closeConn) {
        close(fds[(int)i].fd);
        fds[(int)i].fd = -1;
        compressArr = (int*) 1;
    }
}

void compress(int *compressArr, struct pollfd *fds, int *numFds) {
    compressArr = 0;
    for(int i = 0; i < (int)numFds; i++) {
        if(fds[i].fd == -1) {
            for(int j = 0; i < (int)numFds; j++) {
                fds[i].fd = fds[j+1].fd;
            }
            numFds--;
        }
    }
}

void closeConnections(struct pollfd *fds, int numFds) {
    for (int i = 0; i < numFds; i++) {
        if(fds[i].fd >= 0)
            close(fds[i].fd);
    }
}