
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


//void head() {
     // returns the header of the page ( doesn't have to be a in it's own function can be) 
//}

//void get() {
    // generates a HTML5 page in memmory ( think it should be in a seperate function)
    // Itsactual content should include the URL of the requested page and the IP address and port number of the requesting client
    // format http://foo.com/page 123.123.123.123:4567
//}

//void post() {
   // same as get request plus the data in the body of the post request
//}

//void error() {    
   // sends an error msg
//}
int main(int argc, char *argv[]) {
    int sockfd, port;
    struct sockaddr_in server, client;
    char message[512];
    
    sockfd = socket(AF_INET, SOCK_STREAM, 0);

    memset(&server, 0, sizeof(server));
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = htonl(INADDR_ANY);
    printf("%s \n",argv[1]);
    //port = atoi(argv[1]);;
    server.sin_port = htons(port);
    bind(sockfd, (struct sockaddr *) &server, (socklen_t) sizeof(server));

    for(;;) {
        //We first have to accept a connection
        socklen_t len = (socklen_t) sizeof(client);
        int connfd = accept(sockfd, (struct sockaddr *) &client, &len);
        
        if(connfd == 0) {
            perror("Connection failed...\n");
        }
        //Recieve from connfd, not sockfd
        ssize_t n = recv(connfd, message, sizeof(message) - 1, 0);
    }
    
    return 0;
}
