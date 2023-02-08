#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <errno.h>
#include <sys/sendfile.h>
#include <sys/wait.h>
#include <string.h>
#include <fcntl.h>

#include "commands.h"

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

void send_ls_output(int socket_fd) {
   FILE *fp;
   if ((fp = popen("ls", "r")) == NULL) {
      perror("popen");
      exit(1);
   }

   char buffer[BUFFER_SIZE];
   while (fgets(buffer, sizeof(buffer), fp) != NULL) {
      send(socket_fd, buffer, strlen(buffer), 0);
   }

   pclose(fp);
}

int is_file_in_current_directory(char *filename) {
   char cwd[1024];
   if (getcwd(cwd, sizeof(cwd)) != NULL) {
      char path[1024];
      sprintf(path, "%s/%s", cwd, filename);
      FILE *file = fopen(path, "r");
      if (file) {
         fclose(file);
         return 1;
      }
   }
   return 0;
}

void receive_ls_output(int socket_fd) {
   char buffer[BUFFER_SIZE];
   int bytes_received;
   while ((bytes_received = recv(socket_fd, buffer, sizeof(buffer), 0)) > 0) {
      buffer[bytes_received] = '\0';
      printf("%s", buffer);
   }
}

// Client


void send_command(int socket_fd) {
   char buffer[BUFFER_SIZE];
   printf("Enter Command: ");
   fgets(buffer, sizeof(buffer), stdin);

   int bytes_sent;
   if ((bytes_sent = send(socket_fd, buffer, strlen(buffer), 0)) < 0) {
      perror("send");
      exit(1);
   }

   if (strcmp(buffer, "l\n") == 0) {
      receive_ls_output(socket_fd);
      return;
   }

   if (strncmp(buffer, "c ", 2) == 0) {
      char message[BUFFER_SIZE];
      int bytes_received;
      if ((bytes_received = recv(socket_fd, message, sizeof(message), 0)) < 0) {
         perror("recv");
      }
      message[bytes_received] = '\0';
      printf("%s\n", message);
      return;
   }

   if (strncmp(buffer, "p ", 2) ==0) {
      char message[BUFFER_SIZE];
      int bytes_received;
      if ((bytes_received = recv(socket_fd, message, sizeof(message), 0)) < 0) {
         perror("recv");
         exit(1);
      }
      message[bytes_received] = "\0";
      printf("%s\n", message);
      return;
   }

   if (strncmp(buffer, "d ", 2) ==0) {    
      char filename[BUFFER_SIZE];
      sscanf(buffer, "d %s", filename);
      if (is_file_in_current_directory(filename)) {
         char response;
         printf("File '%s' already exists, would you like to overwrite it (y/n)? ", filename);
         scanf(" %c", &response);

         if (response != 'y') {
            printf("File not overwritten\n");
            return;
         }
      }
   }
}      


// Server

void receive_command(int socket_fd) {
   char buffer[BUFFER_SIZE];
   int bytes_received;
   if ((bytes_received = recv(socket_fd, buffer, sizeof(buffer), 0)) < 0) {
      perror("recv");
      exit(1);
   }
   buffer[bytes_received] = '\0';

   if (strcmp(buffer, "l\n") == 0) {
      send_ls_output(socket_fd);
      return;
   }

   if (strncmp(buffer, "c ", 2) == 0) {
      char filename[BUFFER_SIZE];
      sscanf(buffer, "c %s", filename);
      if (is_file_in_current_directory(filename)) {
         char message[BUFFER_SIZE];
         sprintf(message, "File '%s' exists\n", filename);
         send(socket_fd, message, strlen(message), 0);
      } else {
         char message[BUFFER_SIZE];
         sprintf(message, "File '%s' not found\n", filename);
         send(socket_fd, message, strlen(message), 0);
      }
      return;
   }
   
   if (strncmp(buffer, "p ", 2) == 0) {
      char filename[BUFFER_SIZE];
      sscanf(buffer, "p %s", filename);
      FILE *file = fopen(filename, "r");
      if (file != NULL) {
         char contents[BUFFER_SIZE];
         size_t read_size;
         while ((read_size = fread(contents, 1, sizeof(contents), file)) > 0) {
            send(socket_fd, contents, read_size, 0);
         }
         fclose(file);
      } else {
         char message[BUFFER_SIZE];
         sprintf(message, "File '%s' not found\n", filename);
         send(socket_fd, message, strlen(message), 0);
      }
      return;
   }

   printf("Received command: %s", buffer);
}




