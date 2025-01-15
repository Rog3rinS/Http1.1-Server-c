#include <endian.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "http_handler.h"

Request* HttprequestCreate() {

  Request *Re = (Request *)malloc(sizeof(Request));
  if (Re == NULL) {
    printf("Memory ALlocation Failed");
    return NULL;
  }

  Re->method = (char *)malloc(16 * (sizeof(char)));
  Re->request_target = (char *)malloc(128 * (sizeof(char)));
  Re->protocol = (char *)malloc(32 * (sizeof(char)));
  Re->useragent = (char *)malloc(128 * (sizeof(char)));
  Re->header = (char *)malloc(256 * (sizeof(char)));

  if (Re->header == NULL || Re->request_target == NULL ||
      Re->protocol == NULL || Re->useragent == NULL || Re->header == NULL) {
    printf("Memory ALlocation Failed");
    return NULL;
  }
  return Re;
}

void HttprequestFree(Request* Re) {

  free(Re->method);
  free(Re->request_target);
  free(Re->protocol);
  free(Re->useragent);
  free(Re->header);
  free(Re);

}
void* parse_request(char *buffer, ssize_t *bytes_received, int new_fd) {

  Request *Re = HttprequestCreate();

  char *line = strtok(buffer, "\n");
  char *compare_to[] = {"GET", "POST", "Connection", "Host"};

  while (line != NULL) {

    for (int i = 0; i < 4; i++) { //yes, i know it is a shitty solution.
      if (strncmp(line, compare_to[i], strlen(compare_to[i])) == 0) {
        if (strncmp(line, "GET", 3) == 0 || strncmp(line, "POST", 4) == 0) {
            sscanf(line, "%s %s %s", Re->method, Re->request_target, Re->protocol);
        }
        else if (strncmp(line, "Host", 4) == 0) {
            sscanf(line + 6, "%[^\n]", Re->useragent); //this is cool, thought
        }
        else if (strncmp(line, "Connection", 10) == 0) {
            sscanf(line + 12, "%[^\n]", Re->header);
        }
        break;
      }
    }
    line = strtok(NULL, "\n");
  }

  return Re;
}

void send_response(int client_fd, int status_code, char* status_message, Request* Re) {
  char buffer[1024];

  snprintf(buffer, sizeof(buffer), "HTTP/1.1 %d %s\r\n", status_code, status_message);
  if (status_code != 200) {
    write(client_fd, buffer, strlen(buffer));
    close(client_fd);
  } else {
    write(client_fd, buffer, strlen(buffer));
  }

  HttprequestFree(Re);
}

int handle_response(int client_fd, Request *Re) {

  if (strcmp(Re->protocol, "HTTP/1.1") != 0) {
      send_response(client_fd, 505, "HTTP Version not suported", Re);
      return -1;
  }
  else {
    send_response(client_fd, 200, "OK", Re);
  }
  return 0;
}
