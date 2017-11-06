# Hypertext Transfer Protocol (HTTP)
Implemented in C. A Programming assignment for the course T-409-TSAM-2017: Computer Networks at the University of Reykjavík.

## Features
* Can handle GET, POST and HEAD request methods.
* If any other method is requested it handles it as an unknown request and sends a 501 response code.
* Logs all actions in a file named file.log file, if the file does not exist beforehand the server will create this file.
* Can handle parallel connections for up to 100 clients at a time.


## Known Bugs
* A problem with keep-alive connections, the connection is kept open until the timeout frame runs out (30 seconds), however the client is unable to send another request both while the connection is alive and after timing out.
* Missing URL in requests and log.


## Getting Started
To get started you open the main program folder in bash/shell/terminal and run these two easy commands.

```
1.  make -C ./src 
```


```
2.  ./src/httpd "yourPortNumber""
```


## Fairness
All clients that send a request to the server will eventually receive a reply as the server adds them to an array of connections. When the server closes a connection it moves all the other connections forward in the array so that it behaves similar to a queue. 


## Note
This assignment is worth 20% of the final grade of this course. We put in over 100 hours per person (about 200 hours in total) of work on this project and are sorry to say that we were not able to finish every aspect of the assignment and were not able to start on part 3 because of this. 

## Authors
* Alexandra Geirsdóttir
* Sigurbjörg Rós Sigurðardóttir