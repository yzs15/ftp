#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
enum Command{
  PORT,
  PASV,
  RETR,
  STOR
};

int get_command(char cmd[]){
  char cmd_in[5];
  strncpy(cmd_in, cmd, 4);
  cmd_in[4] = '\0';
  if (strcmp(cmd_in, "PORT") == 0)
    return PORT;
  if (strcmp(cmd_in, "PASV") == 0)
    return PASV;
  if (strcmp(cmd_in, "RETR") == 0)
    return RETR;
  if (strcmp(cmd_in, "STOR") == 0)
    return STOR;
}

int proc_pasv(int port, int socket_id, char PASV_cmd[], int len);
void handle_PASV_request(int socket_id);
void handle_RETR_request(int socket_id, char buffer[]);

int main()
{
  // printf("%d", get_command("PORT 127 0 0 1 12 13"));
  // printf("%d", get_command("PASV 127 0 0 1 12 13"));
  // printf("%d", get_command("RETR bbb.c"));
  // printf("%d", get_command("STOR bbb.c"));
  // return 0;

  struct sockaddr_in sa;
  int SocketFD = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
  if (SocketFD == -1) {
    perror("cannot create socket");
    exit(EXIT_FAILURE);
  }

  memset(&sa, 0, sizeof sa);

  sa.sin_family = AF_INET;
  sa.sin_port = htons(1100);
  sa.sin_addr.s_addr = htonl(INADDR_ANY);
  
  if (bind(SocketFD,(struct sockaddr *)&sa, sizeof sa) == -1) {
    perror("bind failed");
    close(SocketFD);
    exit(EXIT_FAILURE);
  }
  
  if (listen(SocketFD, 10) == -1) {
    perror("listen failed");
    close(SocketFD);
    exit(EXIT_FAILURE);
  }
  
  for (;;) {
    int ConnectFD = accept(SocketFD, NULL, NULL);
  
    if (0 > ConnectFD) {
      perror("accept failed");
      close(SocketFD);
      exit(EXIT_FAILURE);
    }
  
    /* perform read write operations ... 
    read(ConnectFD, buff, size)
    */
    char buffer[256];
    bzero(buffer,256);
    while(1){
      //i assume the command is less than 255 but it is not always true, just for demo
      int n = read(ConnectFD,buffer,255);
      if (n > 0){
        int cmd_type = get_command(buffer);
        printf("%s  cmd_type %d PASV %d \n", buffer, cmd_type, PASV);
        if (cmd_type == PASV){
          printf("recieve PASV request\n");
          handle_PASV_request(ConnectFD);
        }
        else{
          printf("add more cmd handler");
        }



      }
      // write(ConnectFD, "123456", 6);
      // sleep(1);
    }
    
    if (shutdown(ConnectFD, SHUT_RDWR) == -1) {
      perror("shutdown failed");
      close(ConnectFD);
      close(SocketFD);
      exit(EXIT_FAILURE);
    }
    close(ConnectFD);
  }

  close(SocketFD);
  return EXIT_SUCCESS;  
}

void handle_PASV_request(int socket_id){
  char PASV_cmd[] = "227=127 0 0 1 12 13";  
  int len = sizeof(PASV_cmd);  
  proc_pasv(12*256 + 13, socket_id, PASV_cmd, len);
}

int proc_pasv(int port, int socket_id, char PASV_cmd[],int len){
  struct sockaddr_in sa;
  int SocketFD = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
  if (SocketFD == -1) {
    perror("cannot create socket");
    exit(EXIT_FAILURE);
  }

  memset(&sa, 0, sizeof sa);

  sa.sin_family = AF_INET;
  sa.sin_port = htons(port);
  sa.sin_addr.s_addr = htonl(INADDR_ANY);
  
  if (bind(SocketFD,(struct sockaddr *)&sa, sizeof sa) == -1) {
    perror("bind failed");
    close(SocketFD);
    exit(EXIT_FAILURE);
  }
  
  if (listen(SocketFD, 10) == -1) {
    perror("listen failed");
    close(SocketFD);
    exit(EXIT_FAILURE);
  }
  //sever is ready to connect, send client new port and ip
  write(socket_id, PASV_cmd, len);

  for (;;) {
    int ConnectFD = accept(SocketFD, NULL, NULL);
  
    if (0 > ConnectFD) {
      perror("accept failed");
      close(SocketFD);
      exit(EXIT_FAILURE);
    }

    char buffer[256];
    bzero(buffer,256);
    while(1){
      //i assume the command is less than 255 but it is not always true, just for demo
      int n = read(ConnectFD,buffer,255);
      if (n > 0){
        int cmd_type = get_command(buffer);
        printf("%s  cmd_type %d RETR %d \n", buffer, cmd_type, RETR);
        if (cmd_type == RETR){
          // write(ConnectFD, "1234455667", 9);
          handle_RETR_request(ConnectFD, buffer);
        }

      }
    }

    if (shutdown(ConnectFD, SHUT_RDWR) == -1) {
      perror("shutdown failed");
      close(ConnectFD);
      close(SocketFD);
      exit(EXIT_FAILURE);
    }
    close(ConnectFD);
  }

  close(SocketFD);
  return EXIT_SUCCESS;  
}

void handle_RETR_request(int socket_id, char buffer[]){
  //require respons format, need to TODO
  char *filename = buffer+5; //RETV filename
  FILE *f = fopen(filename, "r");
  printf("filename %s\n", filename);
  char read_buf[256];
  bzero(read_buf,26);
  int n = fread(read_buf, 1 , 128, f);
  printf("%d\n",n);
  // printf("%s\n", read_buf);
  write(socket_id, read_buf, n);
  fclose(f);
}