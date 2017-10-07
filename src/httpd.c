
/* your code goes here. */
#include <assert.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <string.h>
#include <stdio.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <stdbool.h>
#include <netdb.h>
#include <time.h>


void ifHead() {
     // returns the header of the page ( doesn't have to be a in it's own function can be) 


}

void ifGet(char *html, char *clientPort, char *clientIP) {
    // generates a HTML5 page in memmory ( think it should be in a seperate function)
    // Itsactual content should include the URL of the requested page and the IP address and port number of the requesting client
    // format http://foo.com/page 123.123.123.123:4567
    html[0] = '\0';
    strcat(html, "\nHTTP/1.1 200, OK\nContent-type: text/html\n"
    "\n<!DOCTYPE>\n<html>\n    <head>\n        <meta charset=\"utf-8\">\n"
    "    </head>\n    <body>\n        <h1>\n");
    strcat(html, "            http://");
    strcat(html, clientIP);
    strcat(html, ":");
    strcat(html, clientPort);
    strcat(html, "\n        </h1>\n            "
    "\n        <img src=\"https://http.cat/200\" alt=\"GET REQUEST\">\n    </body>\n</html>\n");
    printf("inside get function\r\n");
}

void ifPost(char *html, char *clientPort, char *clientIP, char *data) {
   // same as get request plus the data in the body of the post request

    html[0] = '\0';
    strcat(html, "\nHTTP/1.1 200, OK\nContent-type: text/html\n"
    "\n<!DOCTYPE>\n<html>\n    <head>\n        <meta charset=\"utf-8\">\n"
    "    </head>\n    <body>\n        <h1>\n");
    strcat(html, "            http://");
    strcat(html, clientIP);
    strcat(html, ":");
    strcat(html, clientPort);
    strcat(html, "\n        </h1>\n        <p>");
    strcat(html, data);
    strcat(html, "\n        </p>\n        <img src=\"https://http.cat/201\" alt=\"POST REQUEST\">\n"
    "    </body>\n</html>\n"); 
}

void ifError(char *html) {
    html[0] = '\0';
    strcat(html, "\nHTTP/1.1 404, NOTOK\n"
    "<!DOCTYPE html>\n<html>\n    <head>\n        <meta charset=\"utf-8\">\n"
    "    </head>\n    <body>\n       <h2>~~OOPS something went wrong~~       </h2>"
    "            \"        <img src=\"https://http.cat/404\""
    " alt=\"BAD REQUEST\">\n    </body>\n</html>");
}

void logFile(struct tm * timeinfo, char *clientPort, char *clientIP, char *request) {

    FILE *f;

    f = fopen("./src/file.log", "a" );
    fprintf(f, "%s : %s:%s  %s \n", asctime (timeinfo), clientIP, clientPort, request);
    fclose(f);
}


int main(int argc, char *argv[]) {
    int sockfd, port;
    struct sockaddr_in server, client;
    time_t currenttime;
    struct tm * timeinfo;
    char message[512];

    time ( &currenttime );
    timeinfo = localtime ( &currenttime );
    //printf ( "Current local time and date: %s", asctime (timeinfo) );
    
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
 
    memset(&server, 0, sizeof(server));
    
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = htonl(INADDR_ANY);
    
    port = atoi(argv[1]);
    server.sin_port = htons(port);
    printf("Connection with port: %d\n", port);
    printf("printing argc: %d\n", argc);
    
    bind(sockfd, (struct sockaddr *) &server, (socklen_t) sizeof(server));
    socklen_t len = (socklen_t) sizeof(client);    
    listen(sockfd, 100);

    for(;;) {
        //We first have to accept a connection
        socklen_t len = (socklen_t) sizeof(client);
        int connfd = accept(sockfd, (struct sockaddr *) &client, &len);
        
        if(connfd == 0) {
            perror("Connection failed...\n");
        }
       
        //Get all info that we need from the client
        char clientIP[500], clientPort[32], ipAddr[INET_ADDRSTRLEN], html[500];
        //struct sockaddr_in * clientSockAddr = (struct sockaddr_in*)&client;
        //struct in_addr clientIP = clientSockAddr->sin_addr;
        getnameinfo((struct sockaddr *)&client, len, clientIP, sizeof(clientIP), clientPort, 
            sizeof(clientPort), NI_NUMERICHOST | NI_NUMERICSERV);
        inet_ntop(AF_INET, &clientIP, ipAddr, INET_ADDRSTRLEN);

        ssize_t n = recvfrom(connfd, &message, sizeof(message) - 1, 0, 
            (struct sockaddr*)&client, &len);

	    int n2 = send(connfd, &html, sizeof(html) - 1, 0);

        // need to check the first message and see if it is get, post or head
        // and then send it to the right function and send it the webpage it's asking for 
        //
        printf("%s\n", message);
        char mtype[5];
        memcpy(mtype, &message[0], 4);
        mtype[4] = '\0';
        
        if(!(strcmp(mtype, "GET "))) {
            printf("Get request\n");
            ifGet(html, clientPort, clientIP);
            logFile(timeinfo, clientIP, clientPort, mtype);//requested URL, response code
        }
        else if(!(strcmp(mtype, "POST"))) {
            printf("Post request\n");
            char data[500];
            memcpy(data, &message[5], 400);
            ifPost(html, clientPort, clientIP, data);
            logFile(timeinfo, clientIP, clientPort, mtype); //requested URL, response code
        }
        else if(!(strcmp(mtype, "HEAD"))) {
            printf("Head request\n");
            ifHead();
            logFile(timeinfo, clientIP, clientPort, mtype);//requested URL, response code
        }
        else {
            printf("ERROR: The requested type is not supported.\n");
            ifError(html);
            logFile(timeinfo, clientIP, clientPort, "ERROR");
        }
        send(connfd, &html, sizeof(html) -1, 0);
    }
    return 0;
}

