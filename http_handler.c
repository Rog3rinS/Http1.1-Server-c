#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "http_handler.h"

int parse_request(char *buffer, ssize_t *bytes_received, int new_fd) {

  HttpRequest Request;

  char *line = strtok(buffer, "\n");
  char *compare_to[] = {"GET", "POST", "Connection", "Host"};

  while (line != NULL) {

    for (int i = 0; i < 4; i++) { //yes, i know it is a shitty solution.
      if (strncmp(line, compare_to[i], strlen(compare_to[i])) == 0) {
        if (strncmp(line, "GET", 3) == 0 || strncmp(line, "POST", 4) == 0) {
            sscanf(line, "%s %s %s", Request.method, Request.request_target, Request.protocol);
        }
        else if (strncmp(line, "Host", 4) == 0) {
            sscanf(line + 6, "%[^\n]", Request.useragent); //this is cool, thought
        }
        else if (strncmp(line, "Connection", 10) == 0) {
            sscanf(line + 12, "%[^\n]", Request.header);
        }
        break;
      }
    }
    line = strtok(NULL, "\n");
  }

    printf("Method: %s\n", Request.method);
    printf("Target: %s\n", Request.request_target);
    printf("Protocol: %s\n", Request.protocol);
    printf("User: %s\n", Request.useragent);
    printf("Connection: %s\n", Request.header);

  return 0;
}
