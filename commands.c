#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <errno.h>
#include <sys/sendfile.h>
#include <sys/wait.h>
#include <string.h>
#include <fcntl.h>

#define BUFF_SIZE 1000

// Server

char* receive_text_data(int fd) {
   char buffer[1024];
   int bytes_received;
   char *result = malloc(1);
   result[0] = '\0';
   while ((bytes_received = recv(fd, buffer, sizeof(buffer, 0))) > 0) {
         int result_len = strlen(result);
         int bytes_len = strlen(buffer);
         result = realloc(result, result_len + bytes_len + 1);
         strcat(result, buffer);
         memset(buffer, 0, sizeof(buffer));
      }
   return result;
}

// Client

void exec_ls(int fd) {
   FILE *fp;
   char path[1035];

   fp = popen("ls", "r");
   if (fp == NULL) {
      write(fd, "Failed to run command\n", 22);
      exit(1);
   }

   while (fgets(path, sizeof(path)-1, fp) != NULL) {
      write(fd, path, strlen(path));
   }

   pclose(fp);
}

void send_text_data(int fd, char *data) {
   int bytes_sent;
   int data_len = strlen(data);
   int bytes_left = data_len;
   while (bytes_left > 0) {
      bytes_sent = send(fd, data, bytes_left, 0);
      if (bytes_sent == -1) {
         perror("send");
         break;
      }
      bytes_left -= bytes_sent;
      data += bytes_sent;
   }
}

