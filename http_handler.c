#include <stdio.h>
#include "http_handler.h"

int parse_request(char *buffer, ssize_t *bytes_received, int new_fd) {

  //need to have the request
  
  //split the request, into lines
  
  //then i need to get what i need that is: get .... http1.1
  //connection: .....
  //host

  buffer[*bytes_received] = '\0';
  printf("Received from client: \n");
  printf("\n");
  printf("%s\n", buffer);

  return -0;
}
