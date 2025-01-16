#include "http_handler.h"
#include <endian.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <unistd.h>

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

  HttprequestFree(Re);
  char header_buffer[1024];
  char content_buffer[1024];

  if (status_code == 404) {
    struct stat file_info;
    if (stat("notFound.html", &file_info) != 0) {
      send_response(client_fd, 500, "Internal server error", Re);
      return;
    }

    snprintf(header_buffer, sizeof(header_buffer),
             "HTTP/1.1 404 Not Found\r\n"
             "Content-Type: text/html; charset=UTF-8\r\n"
             "Content-Length: %ld\r\n"
             "Connection: close\r\n\r\n", file_info.st_size);

    send(client_fd, header_buffer, strlen(header_buffer), 0);

    FILE *file = fopen("notFound.html", "r");

    if (file == NULL) {
      send_response(client_fd, 500, "Internal server error", Re);
      return;
    }

    size_t bytes_read;
    while ((bytes_read = fread(content_buffer, 1, sizeof(content_buffer), file)) > 0) {
      send(client_fd, content_buffer, bytes_read, 0);
    }

    fclose(file);
  }
  close(client_fd);

}

int handle_response(int client_fd, Request *Re) {

  if (strcmp(Re->protocol, "HTTP/1.1") != 0) {
    send_response(client_fd, 505, "HTTP Version not suported", Re);
    return -1;
  }

  if (strcmp(Re->method, "GET") == 0) {

    char buffer[1024];
    char filepath[1024];
    char *substring = Re->request_target + 1;

    snprintf(filepath, sizeof(filepath), "%s", substring);

    struct stat file_info;
    if (stat(filepath, &file_info) != 0) {
      send_response(client_fd, 404, "File not Found", Re);
      return -1;
    }

    FILE *file = fopen(substring, "r");
    if (file == NULL) {
      send_response(client_fd, 500, "Internal server error", Re);
      return -1;
    }

    snprintf(buffer, sizeof(buffer),
             "HTTP/1.1 200 OK\r\n"
             "Content-Type: text/html; charset=UTF-8\r\n"
             "Content-Length: %ld\r\n"
             "Connection: keep-alive\r\n\r\n", file_info.st_size);

    send(client_fd, buffer, strlen(buffer), 0);

    size_t bytes_read;
    while ((bytes_read = fread(buffer, 1, sizeof(buffer), file)) > 0) {
      send(client_fd, buffer, bytes_read, 0);
    }
    fclose(file);

  } else if (strcmp(Re->method, "POST") == 0) {
    send_response(client_fd, 200, "Method not implemented", Re);
  } else {
    send_response(client_fd, 405, "Method not avaliable", Re);
  }

  return 0;
}
