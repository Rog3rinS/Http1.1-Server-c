#ifndef HTTP_HANDLER_H
#define HTTP_HANDLER_H

#include <stdio.h>
#define MAX_BODY_SIZE 5242880
#define MAX_HEADERS 16

int parse_request(char* buffer, ssize_t *bytes_received, int new_fd);

typedef struct {
  char method[16];
  char request_target[256];
  char protocol[16];
  char useragent[512];
  char header[256];
} HttpRequest;

typedef struct {
  int statuscode;
  char status_message[64];
  char header[MAX_HEADERS][256];
  char body[MAX_BODY_SIZE];
} HttpResponse;

#endif 
