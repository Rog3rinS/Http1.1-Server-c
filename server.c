#include <asm-generic/socket.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <signal.h>

#define PORT "3490" //port 3490 for the server
#define BACKLOG 10 //max 10 users on hold

void sigchld_handler(int s) //have no idea what for, i know that it is to clear shit.
{
    // waitpid() might overwrite errno, so we save and restore it:
    int saved_errno = errno;

    while(waitpid(-1, NULL, WNOHANG) > 0);

    errno = saved_errno;
}

void* get_in_addr(struct sockaddr* sa) //get internet address, i dont know why a func either 
//pass object sa, will return a pointer a pointer to IPV4 OR IPV6
{
    if (sa->sa_family == AF_INET) {
        return &(((struct sockaddr_in*)sa)->sin_addr);
    }

    return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

int main(int argc, char *argv[])
{
  int sockfd, new_fd; //connection file descriptor and accepted file descriptor
  struct addrinfo hints, *servinfo, *p; 
  //three objects, one is the hints,servinfo the main shit, other just a pointer to iterate over. 
  struct sockaddr_storage their_addr; //just store daddys family and pad to fit IPV6
  socklen_t sin_size; //fancy integer to store the size of an sock internet
  struct sigaction sa; //dealing with zombies i guess, dont actually get either.
  int yes=1;
  char s[INET6_ADDRSTRLEN]; //store the ip as string, INET6... is just the max size of IPV6
  int rv; //return value, dont know yet

  memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC; //hint so that can be IPV4 or IPV6
    hints.ai_socktype = SOCK_STREAM; //hint to say, hey it will be TCP
    hints.ai_flags = AI_PASSIVE;
    // with this getaddrinfo() will give me and address to listen, like 0.0.0.0, server typeshit.

    if ((rv = getaddrinfo(NULL, PORT, &hints, &servinfo)) != 0) {
        //null -> expects a node with with the ip or hostname, PORT -> autoexplainable YES ITS A CHAR 
        //&hints -> this part expects a real hint, telling the func how to behave. expects an addr
        //&servinfo -> expects an address, that referecenes an address, where to save the info.
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
        return 1;
    } //getaddrinfo will return a linkedlist to all my possible connection, IPV4, IPV6, and all
    //other possibles.

    for (p = servinfo; p != NULL; p = p->ai_next) {
      // iterating through all the next nodes addresses that my getaddrinfo()
      // returned
      if ((sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) ==
          -1) {
        // the socket takes the family of the addr, for example AF_INET ipv4,
        // also the socket type for example if it is SOCK_STREAM or
        // SOCK_DATAGRAM, and the protocol.
        perror("server: socket");
        continue;
      }

      // shit to lose the "Address already in use", kernel is a turtle
      if ((setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes))) ==
          -1) {
        //the setsockopt takes the file ds, SOL_SOCKET to change the socket opt, I GUESS
        //SO_REUSEADDR used for when the server rerun you dont wait, &yes -> pointer to say that
        //when the server rerun i want to reuse the addr
        perror("setsockopt");
        exit(1);
      }

    //we use bind to associate a socket with a specific Ip address and port number
    //ANALOGY TIME = think of a socket as some input interface, and the IP and Port as a cable, what
    //the binding process will do its connect this cable to our input interface.
      if ((bind(sockfd, p->ai_addr, p->ai_addrlen)) ==
          -1) { // take the socket(input), the address(cable),
        // and the length, of the address.
        perror("server: bind");
        continue;
      }

      break;
    }

    freeaddrinfo(servinfo); //freeing this structure, we use two struct because we need to keep
    //track of the head, if we iterate with this pointer we lose the head, and cant free it, mmlk

    //if p = NULL we are fucked, just shutdown.
    if (p == NULL) {
      fprintf(stderr, "Server: failed to bind\n");
      exit(1);
    }

    //if we got and error listening we are also fucked, shutdown.
    if (listen(sockfd, BACKLOG) == -1) {
      perror("listen");
      exit(1);
    }

    //idont know shit about this again.
    sa.sa_handler = sigchld_handler; // reap all dead processes
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART;
    if (sigaction(SIGCHLD, &sa, NULL) == -1) {
        perror("sigaction");
        exit(1);
    }
    //idont know shit about this again and again.

    printf("server: waiting for connections...\n");

    while (1) {
        sin_size = sizeof(their_addr);
        //remember sin_size is just a fancy int, their_addr stores the family AF_INET for example,
        //and padding
        new_fd = accept(sockfd, (struct sockaddr *)&their_addr, &sin_size);
        if (new_fd == -1) {
          perror("accept");
          continue;
        }

        inet_ntop(their_addr.ss_family, get_in_addr((struct sockaddr*)&their_addr), s, sizeof(s));
        //this is fucked up, their_addr.ss_family is the family AF_INET for example,
        //get_in_addr is a func that will say extracts the address in form of in or in6 from
        //their_addr, it exctacts correctly because of the family, s and size of s is where it will
        //be store. s will be the ip in readable human language like "192.168..."
        printf("server: got connection from: %s\n", s);

        //basically: fork returns 0, if fork returns 0 we execute, if returns something else we
        //dont, the child process will return 0, so it will only execute in the child.
        if (fork() == 0) {
          close(sockfd); // child dont need the listener
          if (send(new_fd, "Hello, world\n", 13, 0) == -1) {
            // just sending a msg, hello, world 13 char long, 0 flags means
            // default
            perror("send");
          }

          close(new_fd);
          exit(0);
        }

        close(new_fd);
    }
    return 0;
}
