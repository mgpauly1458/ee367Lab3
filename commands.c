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

void receive_file(int sock, char *filename) {
   char response[7];
   int bytes_received = recv(sock, response, 7, 0);
   response[bytes_received] = '\0';

   if (strcmp(response, "deny") == 0) {
      printf("File '%s' not found\n", filename);
      return;
   }

   FILE *fp = fopen(filename, "wb");
   if (fp == NULL) {
      perror("fopen");
      return;
   }

   char buffer[BUFFER_SIZE];
   while ((bytes_received = recv(sock, buffer, sizeof(buffer), 0)) > 0) {
      fwrite(buffer, 1, bytes_received, fp);
   }

   fclose(fp);
}

void send_message(int socket_fd, char *message) {
   int bytes_sent;
   if ((bytes_sent = send(socket_fd, message, strlen(message), 0)) < 0) {
      perror("send");
      exit(1);
   }
   return;
}

// Client


int send_command(int socket_fd) {
   char buffer[BUFFER_SIZE];
   printf("Enter Command: ");
   fgets(buffer, sizeof(buffer), stdin);

   if (strcmp(buffer, "q\n") == 0) {
      return 1;
   }

   if (strcmp(buffer, "h\n") == 0) {
      printf("l: List\nc: Check <file name>\np: Display <filename>\n");
      printf("d: Download <filename>\nq: Quit\nh: Help\n");
      return 0;
   }

   if (strcmp(buffer, "l\n") == 0) {
      send_message(socket_fd, buffer);
      receive_ls_output(socket_fd);
      return 0;
   }

   if (strncmp(buffer, "c ", 2) == 0) {
      send_message(socket_fd, buffer);
      char message[BUFFER_SIZE];
      int bytes_received;
      if ((bytes_received = recv(socket_fd, message, sizeof(message), 0)) < 0) {
         perror("recv");
      }
      message[bytes_received] = '\0';
      printf("%s\n", message);
      return 0;
   }

   if (strncmp(buffer, "p ", 2) ==0) {
      send_message(socket_fd, buffer);
      char message[BUFFER_SIZE];
      int bytes_received;
      if ((bytes_received = recv(socket_fd, message, sizeof(message), 0)) < 0) {
         perror("recv");
         exit(1);
      }
      message[bytes_received] = "\0";
      printf("%s\n", message);
      return 0;
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
            return 0;
         }
      }
      
      send_message(socket_fd, buffer);
      receive_file(socket_fd, filename); 
   }
   return 0;
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
   
   if (strncmp(buffer, "d ", 2) == 0) {
      char filename[BUFFER_SIZE];
      sscanf(buffer, "d %s", filename);
      char response[BUFFER_SIZE];

      if (!is_file_in_current_directory(filename)) {
         strcpy(response, "deny\0");
         send(socket_fd, response, strlen(response), 0);
         return;
      }

      strcpy(response, "confirm\0");
      send(socket_fd, response, strlen(response), 0);
      
      struct stat st;
      if (stat(filename, &st) == -1) {
         return;
      }

      int fd = open(filename, O_RDONLY);
      if (fd == -1) {
         perror("open");
         return;
      }

      off_t offset = 0;
      while (offset < st.st_size) {
         ssize_t bytes_sent = sendfile(socket_fd, fd, &offset, st.st_size - offset);
         if (bytes_sent == -1) {
            perror("sendfile");
            close(fd);
            return;
         }
      }

      close(fd);
      return;
   }

   printf("Received command: %s", buffer);
}




