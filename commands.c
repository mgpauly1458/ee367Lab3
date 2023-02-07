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

void exec_ls(int fd) {
   FILE *fp;
   char path[1035];

   fp = popen("ls", "r");
   if (fp == NULL) {
      write(fd, "Failed to run command\n", 22);
      exit(1);
   }

   while (fgets(path, sizeof(path)-1, fp) != NULL) {
      int bytes = write(fd, path, strlen(path));
      printf("bytes=%d errno=%s\n", bytes, strerror(errno));
   }

   pclose(fp);
}


void readListContents(int fd) {
   FILE* fp = fdopen(fd, "r");
   char buff[BUFF_SIZE];
   while (fgets(buff, sizeof(buff), fp) != NULL) {
      printf("%s", buff);
   }
}

int getFileSize(int fd) {
   struct stat file_stat;
   if (fstat(fd, &file_stat) < 0) {
      fprintf(stderr, "Error fstat ---> %s", strerror(errno));
      exit(EXIT_FAILURE);
   }
   int fileSize = file_stat.st_size;
   printf("fileSize=%d fd=%d\n\n", fileSize, fd);

   return fileSize;
}

// Client
/*
int main(int argc, char *argv[]) {
   

   int fd[2];
   pipe(fd);

   //child
   if (!fork()) {
      close(fd[0]);   

     exec_ls(fd[1]); 
      
      close(fd[1]);
      exit(0);
   }
   
   //parent
   close(fd[1]);
   wait(NULL);
   FILE* fp = fdopen(fd[0], "r");
   readListContents(fp);
  
   close(fd[0]);
   return 0;
}
*/


