#include <arpa/inet.h>
#include <asm-generic/socket.h>
#include <errno.h>
#include <netdb.h>
#include <netinet/in.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include "http_handler.h"

#define PORT "3490"
#define BACKLOG 10

char buffer[1024];

void sigchld_handler(int s); 
void *get_in_addr(struct sockaddr *sa);
void print_sockaddr_in(struct sockaddr_in *addr);
void print_sockaddr_in6(struct sockaddr_in6 *addr6);

int main(int argc, char *argv[]) {

  int sockfd, new_fd;
  struct addrinfo hints, *servinfo, *p;
  struct sockaddr_storage their_addr;
  socklen_t sin_size;
  struct sigaction sa;
  int yes = 1;
  char s[INET6_ADDRSTRLEN];
  int rv;


  memset(&hints, 0, sizeof(hints));
  hints.ai_family = AF_UNSPEC;
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_flags = AI_PASSIVE;


  if ((rv = getaddrinfo(NULL, PORT, &hints, &servinfo)) != 0) {
    fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
    return 1;
  }


  for (p = servinfo; p != NULL; p = p->ai_next) {

    if ((sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1) {
      perror("server: socket");
      continue;
    }


    if ((setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes))) == -1) {
      perror("setsockopt");
      exit(1);
    }


    if ((bind(sockfd, p->ai_addr, p->ai_addrlen)) == -1) {
      perror("server: bind");
      continue;
    }

    break;
  }

  freeaddrinfo(servinfo);

  if (p == NULL) {
    fprintf(stderr, "Server: failed to bind\n");
    exit(1);
  }

  if (listen(sockfd, BACKLOG) == -1) {
    perror("listen");
    exit(1);
  }


  sa.sa_handler = sigchld_handler;
  sigemptyset(&sa.sa_mask);
  sa.sa_flags = SA_RESTART;
  if (sigaction(SIGCHLD, &sa, NULL) == -1) {
    perror("sigaction");
    exit(1);
  }

  printf("server: waiting for connections...\n");

  
  while (1) {

    sin_size = sizeof(their_addr);
    new_fd = accept(sockfd, (struct sockaddr *)&their_addr, &sin_size);
    if (new_fd == -1) {
      perror("accept");
      continue;
    }


    inet_ntop(their_addr.ss_family, get_in_addr((struct sockaddr *)&their_addr), s, sizeof(s));

    printf("server: got connection from: %s\n", s);

    if (fork() == 0) {
      close(sockfd);


      ssize_t bytes_received = recv(new_fd, buffer, sizeof(buffer), 0);
      if (bytes_received < 0) {
        perror("recv failed");
        close(new_fd);
        return -1;
      }

      if (parse_request(buffer, &bytes_received, new_fd) != 0) {
        perror("parsin");
        return -1;
      }

      const char *html_content = "<html><head><title>Hello World</title></head>"
                                 "<body><h1>Hello, World!</h1></body></html>";

      size_t content_length = strlen(html_content);
      char msg[256];
      snprintf(msg, sizeof(msg),
               "HTTP/1.1 200 OK\r\n"
               "Content-Type: text/html\r\n"
               "Content-Length: %zu\r\n"
               "Connection: close\r\n"
               "\r\n",
               content_length);

      if (send(new_fd, msg, strlen(msg), 0) == -1) {
        perror("send");
      }

      if (send(new_fd, html_content, content_length, 0) == -1) {
        perror("send");
      }

      close(new_fd);
      exit(0);
    }

    close(new_fd);
  }
  return 0;
}

void sigchld_handler(int s) {

  int saved_errno = errno;

  while (waitpid(-1, NULL, WNOHANG) > 0);

  errno = saved_errno;
}

void *get_in_addr(struct sockaddr *sa){
  if (sa->sa_family == AF_INET) {
    return &(((struct sockaddr_in *)sa)->sin_addr);
  }

  return &(((struct sockaddr_in6 *)sa)->sin6_addr);
}


void print_sockaddr_in(struct sockaddr_in *addr) {

  char ip[INET_ADDRSTRLEN];
  inet_ntop(AF_INET, &(addr->sin_addr), ip, sizeof(ip));

  printf("ipv4 address: %s\n", ip);
  printf("port %d\n", ntohs(addr->sin_port));
}

void print_sockaddr_in6(struct sockaddr_in6 *addr6) {
  char ip[INET_ADDRSTRLEN];
  inet_ntop(AF_INET6, &(addr6->sin6_addr), ip, sizeof(ip));
  printf("ipv6 address: %s\n", ip);
  printf("port %d\n", ntohs(addr6->sin6_port));
}
