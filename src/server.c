#include <arpa/inet.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <unistd.h>

#define BUF_SIZE 100
#define PORT 8000
#define LISTEN_BACKLOG 32

#define handle_error(msg) \
  do {                    \
    perror(msg);          \
    exit(EXIT_FAILURE);   \
  } while (0)             \

int s_to_i(char *str);

int exited = 0;

int main(int argc, char *argv[])
{
  // *********************
  // Setting up the server
  // *********************
  if (argc != 2)
  {
    printf("./server [# of clients]\n");
    return 0;
  }

  int MAX_EVENTS = s_to_i(argv[1]);

  struct sockaddr_in addr, remote_addr;
  int sfd, cfd, epollfd;
  int nfds;
  ssize_t num_read;
  socklen_t addrlen = sizeof(struct sockaddr_in);
  char buf[BUF_SIZE];
  struct epoll_event ev;
  struct epoll_event *events = malloc(sizeof(struct epoll_event) * MAX_EVENTS);

  sfd = socket(AF_INET, SOCK_STREAM, 0);
  if (sfd == -1) handle_error("socket");

  memset(&addr, 0, sizeof(struct sockaddr_in));
  addr.sin_family = AF_INET;
  addr.sin_port = htons(PORT);
  addr.sin_addr.s_addr = htonl(INADDR_ANY);

  if (bind(sfd, (struct sockaddr *)&addr, sizeof(struct sockaddr_in)) == -1)
    handle_error("bind");
  if (listen(sfd, LISTEN_BACKLOG) == -1)
    handle_error("listen");

  epollfd = epoll_create1(0);
  if (epollfd == -1) handle_error("epoll_create1");

  ev.events = EPOLLIN | EPOLLOUT;
  ev.data.fd = sfd;
  if (epoll_ctl(epollfd, EPOLL_CTL_ADD, sfd, &ev) == -1)
    handle_error("epoll_ctl");

  // **********************
  // Setting up server done
  // **********************

  // Receiving connection from clients
  for (;;)
  {
    nfds = epoll_wait(epollfd, events, MAX_EVENTS, -1);
    if (nfds == -1) 
    {
      printf("Line 76\n");
      handle_error("epoll_wait");
    }

    for (int i = 0; i < nfds; ++i)
    {
      if (events[i].data.fd == sfd)
      {
        memset(&remote_addr, 0, sizeof(struct sockaddr_in));
        cfd = accept(sfd, (struct sockaddr *)&remote_addr, &addrlen);
        if (cfd == -1)
          handle_error("accept");

        int flags = fcntl(cfd, F_GETFL, 0);
        if (flags == -1) 
          handle_error("fcntl");

        flags |= O_NONBLOCK;
        if (fcntl(cfd, F_SETFL, flags) == -1)
          handle_error("fcntl");

        ev.events = EPOLLIN | EPOLLOUT;
        ev.data.fd = cfd;
        if (epoll_ctl(epollfd, EPOLL_CTL_ADD, cfd, &ev) == -1) 
        {
          perror("epoll_ctl: conn_sock");
          exit(EXIT_FAILURE);
        }
      }
      else
      {
        // Receive message from a client 
        while((num_read = read(events[i].data.fd, buf, BUF_SIZE)) > 0)
        {
          // When all clients signaled 'type 1' message, shuts all clients and server off
          if (buf[0] == '1') 
          {
            exited++;
            printf("nfds: %d, Exited: %d\n", nfds, exited);
            if (nfds == exited) 
            {
              sleep(1);
              for (int i = 0; i < nfds; i++)
              {
                write(events[i].data.fd, "1", 1);
              }
              free(events);
              printf("Empty room\n");
              if (close(sfd) == -1) handle_error("close");
              exit(0);
            }
            continue;
          }

          // Gets client's address
          char msg[BUF_SIZE];
          inet_ntop(AF_INET, (struct sockaddr*) &events[i].data.u64, msg, INET_ADDRSTRLEN);
          int index = strlen(msg);
          msg[index] = '\t'; 
          index++;
          
          // Put client's address and message together
          for (int i = 1; i < strlen(buf); i++)
          {
            msg[index] = buf[i];
            index ++;
          }
          msg[index] = 0;

          // Send the message to all clients
          for (int i = 0; i < nfds; i++)
          {
            write(events[i].data.fd, msg, strlen(msg)); 
          }

          if (num_read == -1) 
            handle_error("read");          
          
        }
      }
    }
  }
  if (close(sfd) == -1) handle_error("close");
} 

// Convert string to integer
int s_to_i(char *str)
{
  int n = 0;
  for (int i = 0; str[i] != 0; i++)
  {
    n = n * 10 + (str[i] - 48);
  }
  return n;
}
