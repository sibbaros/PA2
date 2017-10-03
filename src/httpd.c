
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
	int sockfd;
	struct sockaddr_in server, client;

	for(int i = 0; i < sizeOf(argv); i++) {
		printf("%d", argv[i]);
	}
        // close connections if no activity in 30 sec
        // client can send a "connection: closed" msg
        //
	return 0;
}
