#ifndef HTTP_HANDLER_H
#define HTTP_HANDLER_H

#include <stdio.h>
#define MAX_BODY_SIZE 5242880
#define MAX_HEADERS 16


typedef struct {
  char* method;
  char* request_target;
  char* protocol;
  char* useragent;
  char* header;
} Request;

typedef struct {
  int statuscode;
  char status_message[64];
  char header[MAX_HEADERS][256];
  char body[MAX_BODY_SIZE];
} HttpResponse;

void *parse_request(char *buffer, ssize_t *bytes_received, int new_fd);
void send_response(int client_fd, int status_code, char *status_message, Request *Re);
int handle_response(int client_fd, Request *Re);

#endif 
