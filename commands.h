#define BUFFER_SIZE 1035

void readListContents(int);
char* receive_text_data(int fd);
void exec_ls(int);
void send_text_data(int fd, char *data);
