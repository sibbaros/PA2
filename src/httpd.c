
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


void head() {
     // returns the header of the page ( doesn't have to be a in it's own function can be) 
}

void get() {
    // generates a HTML5 page in memmory ( think it should be in a seperate function)
    // Itsactual content should include the URL of the requested page and the IP address and port number of the requesting client
    // format http://foo.com/page 123.123.123.123:4567
}

void post() {
   // same as get request plus the data in the body of the post request
}

void error() {    
   // sends an error msg
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
        
        //printf("connfd: %d", connfd);
        if(connfd == 0) {
            perror("Connection failed...\n");
        }
        
        //Recieve from connfd, not sockfd
        ssize_t n = recv(connfd, &message, sizeof(message) - 1, 0);
	char html[500] = "HTTP/1.1 200, OK\r\n\r\n<!DOCTYPE><html><body><h1>Hallu</h1></body></html>\r\n";
	int n2 = send(connfd, &html, sizeof(html) - 1, 0);

        // need to check the first message and see if it is get, post or head
        // and then send it to the right function and send it the webpage it's asking for 
        //
	//printf("%d\n", n2);
        //printf("the message is: %s", message);
        
        char mtype[5];
        //mtype = strchr(message, ' ');
        memcpy(mtype, &message[0], 4);
        mtype[4] = '\0';
        
        if(!(strcmp(mtype, "GET "))) {
            printf("Get request\n");
            get();
        }
        else if(!(strcmp(mtype, "POST"))) {
            printf("Post request\n");
            post();
        }
        else if(!(strcmp(mtype, "HEAD"))) {
            printf("Head request\n");
            head();
        }
        else {
            printf("ERROR: The requestERROR: The requested type is not supported.\n");
            error();
        }

    }
    
    return 0;

}

