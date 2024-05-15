#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <sys/socket.h>
#include <pthread.h>
#include <unistd.h>

#define PORT 8000
#define BUF_SIZE 64

#define handle_error(msg) \
  do {                    \
    perror(msg);          \
    exit(EXIT_FAILURE);   \
  } while(0)              \

void convert(uint8_t *buf, char *str, ssize_t size);
int s_to_i(char *str);
void *receive(void *id);

int main(int argc, char *argv[])
{
  // *************************
  // Setting up client process
  // *************************
  struct sockaddr_in addr;
  int sfd;
  
  if (argc != 3)
  {
    printf("./client [run time] [server ip address]\n");
    return 0;
  }
  int runtime = s_to_i(argv[1]);
  char *s_IP = argv[2];

  sfd = socket(AF_INET, SOCK_STREAM, 0);
  if (sfd == -1) handle_error("socket");
  
  memset(&addr, 0, sizeof(struct sockaddr_in));
  addr.sin_family = AF_INET;
  addr.sin_port = htons(PORT);
  addr.sin_addr.s_addr = htonl(INADDR_ANY);

  if (inet_pton(AF_INET, s_IP, &addr.sin_addr) <= 0)
    handle_error("inet_pton");
  if (connect(sfd, (struct sockaddr *)&addr, sizeof(struct sockaddr_in)) == -1)
    handle_error("connect");
  printf("client: connect success\n");

  // *****************************
  // Setting up client process end
  // *****************************
  
  // Message receiver thread
  pthread_t receiver;
  pthread_create(&receiver, NULL, receive, (void *) &sfd); 
    
  // Generating random messages. Sends them with one second interval
  for (int i = 0; i < runtime; i++)
  {
    // Randomly generated message
    uint8_t buf1[10];
    char str[10 * 2 + 1];
    getentropy(buf1, 10);
    convert(buf1, str, 21);
    str[strlen(str)] = '\n';

    // To indicate that this is 'type 0' message
    char msg[BUF_SIZE];
    msg[0] = '0';
    int index= 1;
    
    for (int i = 0; i < 4; i++)
    {
      char port[4] = "8000";
      msg[index] = port[i];
      index++;
    }

    // Random generated message
    msg[index] = '\t'; index++;
    for (int i = 0; i < strlen(str); i++)
    {
      msg[index] = str[i];
      index++;
    }

    // Send message to server
    write(sfd, msg, strlen(msg));
    sleep(1);
  }

  // Turns off receiver after the runtime has past
  pthread_cancel(receiver);
  pthread_join(receiver, NULL);
  write(STDOUT_FILENO, "client: Receiver off\n", strlen("client: Receiver off\n"));
  
  // Signal server that this client is done with type 1 message 
  char shutdown[1] = "1";
  char signal[BUF_SIZE];
  write(sfd,shutdown, strlen(shutdown)); 
  
  // Shutdown the client when it receives type 1 message
  for (;;)
  {
    read(sfd, signal, BUF_SIZE);
    if (signal[0] == '1') 
    {
      printf("client: Shutting down\n");
      close(sfd);
      exit(0);
    }
    sleep(1);
  }
}

// Receiver thread receives message from server
void *receive(void *id)
{
  int client_id = *((int *)id);
  for (;;)
  {
    char msg[BUF_SIZE];
    read(client_id, msg, BUF_SIZE);
    if (strlen(msg) > 0) write(STDOUT_FILENO, msg, strlen(msg));
  }
}

// Random message helper function
void convert(uint8_t *buf, char *str, ssize_t size)
{
  if (size % 2 == 0)
    size = size / 2 - 1;
  else
    size = size / 2;

  for (int i = 0; i < size; i++)
    sprintf(str + i * 2, "%02X", buf[i]);
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
