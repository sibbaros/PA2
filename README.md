# Read-Only Hypertext Transfer protocol (Http)
Implemented in C

## Features

* Can handle GET, POST and HEAD requests
* If anything else is requested it handels it as an error
* Logs all actions in the file.log file
* Can handle parallel connections from several clients


## Known Bugs
* We did not use Glib in our implematation
* The format of the log file is not correct and we know that
* We did not manage to implement the support for persistent connections



## Getting Started
To gett started you open the PA2 folder in bash/shell/terminal and run these two easy commands.

```
1.  make -C ./src 
```


```
2.  ./src/httpd "yourPortNumber""
```


## Authors
See the AUTHORS file included in the folder


