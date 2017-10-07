
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


void head() {
     // returns the header of the page ( doesn't have to be a in it's own function can be) 
}

void get(char *html, char *ipAddr, char *hostPort, char *hostIP) {
    // generates a HTML5 page in memmory ( think it should be in a seperate function)
    // Itsactual content should include the URL of the requested page and the IP address and port number of the requesting client
    // format http://foo.com/page 123.123.123.123:4567
    html[0] = '\0';
    strcat(html, "\nHTTP/1.1 200, OK\nContent-type: text/html\n"
    "\n<!DOCTYPE>\n<html>\n    <head>\n        <meta charset=\"utf-8\">\n"
    "    </head>\n    <body>\n        <h1>\n");
    strcat(html, "            http://");
    //strcat(html, ipAddr);
    //strcat(html, " ");
    strcat(html, hostIP);
    strcat(html, ":");
    strcat(html, hostPort);
    strcat(html, "\n        </h1>\n            "
    "\n        <img src=\"https://http.cat/200\" alt=\"GET REQUEST\">\n    </body>\n</html>\n");
    printf("inside get function\r\n");
}

void post(char *html, char *ipAddr, char *hostPort, char *hostIP, char *data) {
   // same as get request plus the data in the body of the post request
    html[0] = '\0';
    strcat(html, "\nHTTP/1.1 200, OK\nContent-type: text/html\n"
    "\n<!DOCTYPE>\n<html>\n    <head>\n        <meta charset=\"utf-8\">\n"
    "    </head>\n    <body>\n        <h1>\n");
    strcat(html, "            http://");
    //strcat(html, ipAddr);
    //strcat(html, " ");
    strcat(html, hostIP);
    strcat(html, ":");
    strcat(html, hostPort);
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


int main(int argc, char *argv[]) {
    int sockfd, port;
    struct sockaddr_in server, client;
    char message[512];
    
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
 
    memset(&server, 0, sizeof(server));
    
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = htonl(INADDR_ANY);
    
    port = atoi(argv[1]);
    server.sin_port = htons(port);
    printf("Connection with port: %d\n", port);
    
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
        char clientIP[500], clientPort[32], ipAddr[INET_ADDRSTRLEN];
        struct sockaddr_in * clientSockAddr = (struct sockaddr_in*)&client;
        //struct in_addr clientIP = clientSockAddr->sin_addr;
        getnameinfo((struct sockaddr *)&client, len, clientIP, sizeof(clientIP), clientPort, sizeof(clientPort), NI_NUMERICHOST | NI_NUMERICSERV);
        inet_ntop(AF_INET, &clientIP, ipAddr, INET_ADDRSTRLEN);
        /*
        char check[512];
        check[0] = '\0';
        strcat(check, hostIP);
        printf("hallo");
        fflush(stdout);
        strcat(check, "  :  ");
        printf("hallo2");
        fflush(stdout);
        printf("address: %d", ipAddr);
        fflush(stdout);
        strcat(check, ipAddr);
        printf("hallo3");
        fflush(stdout);
        strcat(check, "\n");

        printf("is it the same number %s", check);*/

        //Recieve from connfd, not sockfd
        char arr[500];
        ssize_t n = recv(connfd, &message, sizeof(message) - 1, 0);
        ssize_t info = recvfrom(connfd, arr, sizeof(arr) - 1, 0, (struct sockaddr*)clientSockAddr, sizeof(clientSockAddr));
        //perror("perror\n");
        //printf("ARRAY ER::::::%s", arr);
	    char html[500];
	    int n2 = send(connfd, &html, sizeof(html) - 1, 0);

        // need to check the first message and see if it is get, post or head
        // and then send it to the right function and send it the webpage it's asking for 
        //
        
        char mtype[5];
        memcpy(mtype, &message[0], 4);
        mtype[4] = '\0';
        
        if(!(strcmp(mtype, "GET "))) {
            printf("Get request\n");
            get(html, ipAddr, clientPort, clientIP);
        }
        else if(!(strcmp(mtype, "POST"))) {
            printf("Post request\n");
            char data[500];
            memcpy(data, &message[5], 400);
            post(html, ipAddr, clientPort, clientIP, data);
        }
        else if(!(strcmp(mtype, "HEAD"))) {
            printf("Head request\n");
            head();
        }
        else {
            printf("ERROR: The requested type is not supported.\n");
            ifError(html);
        }
        send(connfd, &html, sizeof(html) -1, 0);
    }
    return 0;
}

