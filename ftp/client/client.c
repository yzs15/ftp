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

enum Respons{
  PASV_CONNECT_SUCC
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
  return -1;
}

int get_respons(char rsp[]){
  char cmd_in[5];
  strncpy(cmd_in, rsp, 4);
  cmd_in[4] = '\0';
  if (strcmp(cmd_in, "227=") == 0)
    return PASV_CONNECT_SUCC;
}

int PASV_process(char* buffer);

int main(void)
{
  struct sockaddr_in sa;
  int res;
  int SocketFD;
  SocketFD = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
  if (SocketFD == -1) {
    perror("cannot create socket");
    exit(EXIT_FAILURE);
  }
  
  memset(&sa, 0, sizeof sa);
  
  sa.sin_family = AF_INET;
  sa.sin_port = htons(1100);
  res = inet_pton(AF_INET, "127.0.0.1", &sa.sin_addr);

  if (connect(SocketFD, (struct sockaddr *)&sa, sizeof sa) == -1) {
    perror("connect failed");
    close(SocketFD);
    exit(EXIT_FAILURE);
  }

  char buffer[256];
  bzero(buffer,256);
  char PASV_req[] = "PASV";
  write(SocketFD, "PASV", sizeof(PASV_req));
  printf("send PASV request\n");
  /* perform read write operations ... */
  while(1){
    int n = read(SocketFD,buffer,255);
    if (n>0){
      int rsp = get_respons(buffer);
      printf("buf %s rsp %d\n", buffer, rsp);
      if (rsp == PASV_CONNECT_SUCC){
        PASV_process(buffer);
      }
    }
      //   for (int i = 0; i<n; i++)
      //     printf("%c", buffer[i] );
      //   printf("\n");
      // }
      

  }
  shutdown(SocketFD, SHUT_RDWR);
  
  close(SocketFD);
  return EXIT_SUCCESS;
}

int PASV_process(char buffer[]){ 
//need length as a parameter to avoid core dump, just assum they run normally
  char *ip_ptr = buffer+4;
  char ip[40];
  int num = 1;
  int ip_index = 0;
  while(num <= 4){
    if (*ip_ptr != ' '){
      ip[ip_index] = *ip_ptr;
      ip_index++;
      ip_ptr++;
    }
    else{
      ip_ptr++;
      num++;
      ip[ip_index] = '.';
      ip_index++;
    }
  }
  ip[ip_index - 1] = '\0';
  printf("%s\n", ip);
  num = 1;
  int port = 0;
  char p[10];
  int p_index = 0;
  char *p_ptr = ip_ptr;
  printf("%s\n", ip);
  while(num<=2){
    // printf("%s %c \n", ip, *p_ptr);
    if (*p_ptr != ' '){
      p[p_index] = *p_ptr;
      p_index++;
      p_ptr++;
      if (*p_ptr == '\0'){
        p[p_index] = '\0';
        port = port*256 + atoi(p);
        break;
      }
    }
    else{
      p[p_index] = '\0';
      port = port*256 + atoi(p);
      p_index = 0;
      p_ptr++;
      num++;
    }
  }
  // printf("%s\n", ip);
  printf("recieve ip %s port  %d to connect \n", ip, port);

  struct sockaddr_in sa;
  int res;
  int SocketFD;
  SocketFD = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
  if (SocketFD == -1) {
    perror("cannot create socket");
    exit(EXIT_FAILURE);
  }
  
  memset(&sa, 0, sizeof sa);
  
  sa.sin_family = AF_INET;
  sa.sin_port = htons(port);
  res = inet_pton(AF_INET, ip, &sa.sin_addr);

  if (connect(SocketFD, (struct sockaddr *)&sa, sizeof sa) == -1) {
    perror("connect failed");
    close(SocketFD);
    exit(EXIT_FAILURE);
  }
  char RETR_cmd[] = "RETR hello.c";
  write(SocketFD, "RETR hello.c", sizeof(RETR_cmd));
  char buf[256];
  bzero(buf, 256);
  FILE *f = fopen("hello.c", "w");
  while(1){
    //you need to figure the end of the file according to FTP protocol
    int n = read(SocketFD,buf,255);
    if (n>0){
      for (int i = 0; i<n; i++){
        printf("%c", buf[i] );
      }
      printf("\n");
      fwrite(buf, 1, n, f);
      break;
    }
        
  }
  fclose(f);
  shutdown(SocketFD, SHUT_RDWR);
  
  close(SocketFD);
  return EXIT_SUCCESS;



}