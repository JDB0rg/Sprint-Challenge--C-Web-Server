#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include "lib.h"

#define BUFSIZE 4096 // max number of bytes we can get at once

/**
 * Struct to hold all three pieces of a URL
 */
typedef struct urlinfo_t {
  char *hostname;
  char *port;
  char *path;
} urlinfo_t;

/**
 * Tokenize the given URL into hostname, path, and port.
 *
 * url: The input URL to parse.
 *
 * Store hostname, path, and port in a urlinfo_t struct and return the struct.
*/
urlinfo_t *parse_url(char *url)
{
  // copy the input URL so as not to mutate the original
  char *hostname = strdup(url);
  char *port;
  char *path;

  urlinfo_t *urlinfo = malloc(sizeof(urlinfo_t));

  // find first back slash and add null terminator
  char *find_slash = strchr(hostname,'/'); 
  path = find_slash + 1;
  *find_slash = '\0';

  // find colon and add null terminator
  char *find_colon = strchr(hostname, ':');
  port = find_colon + 1;
  *find_colon = '\0';

  urlinfo->hostname = hostname;
  urlinfo->path = path;
  urlinfo->port = port;

  return urlinfo;
}

/**
 * Constructs and sends an HTTP request
 *
 * fd:       The file descriptor of the connection.
 * hostname: The hostname string.
 * port:     The port string.
 * path:     The path string.
 *
 * Return the value from the send() function.
*/
int send_request(int fd, char *hostname, char *port, char *path)
{
  const int max_request_size = 16384;
  char request[max_request_size];
  int rv;

  // Same as in the example from monday
  int request_len = sprintf(request,
    "GET %s HTTP/1.1\n"
    "Host: %s:%s\n"
    "Connection: close\n"
    "\n",
    path, hostname, port
  );

  // Same as in the server project. 
  rv = send(fd, request, request_len, 0);

    if (rv < 0) 
    {
        perror("send");
    }

  return rv;
}

int main(int argc, char *argv[])
{  
  int sockfd, numbytes;  
  char buf[BUFSIZE];

  if (argc != 2) {
    fprintf(stderr,"usage: client HOSTNAME:PORT/PATH\n");
    exit(1);
  }

  // 1. Parse the input URL
  urlinfo_t *url = malloc(sizeof(urlinfo_t));
  url = parse_url(argv[1]);

  // 2. Initialize a socket by calling the `get_socket` function from lib.c
  sockfd = get_socket(url->hostname, url->port);

  // 3. Call `send_request` to construct the request and send it
  send_request(sockfd, url->hostname, url->port, url->path);

  // 4. Call `recv` in a loop until there is no more data to receive from the server. Print the received response to stdout.
  while ((numbytes = recv(sockfd, buf, BUFSIZE - 1, 0)) > 0)
  {
      fprintf(stdout, "%s\n", buf);
  }

  // 5. Clean up any allocated memory and open file descriptors.
  free(url);
  close(sockfd);

  return 0;
}
