#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <netdb.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <assert.h>
#include <stdbool.h>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include "write_request.h"

#include "helpers.h"


/*
 * NET
 * Разработать клиент-серверное приложение копирования файла (или поддерева файловой системы) с узла-клиента
 * на узел-сервер в указанный каталог (аналог стандартной UNIX-команды rcp).
 * Команда, выполняемая на стороне клиента, имеет следующий вид: cprem path.to.src.file host@path.to.dst.dir .
 */


int main(int argc, char **argv) {
    int sockfd, portno, n;

    struct sockaddr_in serv_addr;
    struct hostent *server;

    portno = 8888;
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0)
        die(__LINE__, "ERROR opening socket");

    server = gethostbyname("0");
    if (server == NULL) {
        fprintf(stderr, "ERROR, no such host\n");
        exit(0);
    }
    bzero((char *) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    bcopy((char *) server->h_addr,
          (char *) &serv_addr.sin_addr.s_addr,
          server->h_length);
    serv_addr.sin_port = htons(portno);
    if (connect(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0)
        die(__LINE__, "ERROR connecting");

    const char * data = "abcdefg";

    WriteRequest * write_request = new_write_request(strlen(data));
    strncpy(write_request->dst_path, "/tmp/trash.txt", dst_path_max_size());
    write_request->is_folder = false;
    memcpy(write_request->data, data, strlen(data));
    write_request->data[2] = '\0';

    char * buffer = serialize_write_request(write_request);
    n = write(sockfd, buffer, write_request_max_size());
    if (n < 0)
        die(__LINE__, "ERROR writing to socket");
    bzero(buffer, 256);
    n = read(sockfd, buffer, 255);
    if (n < 0)
        die(__LINE__, "ERROR reading from socket");
    printf("%s\n", buffer);
    free(buffer);
    free_write_request(write_request);
    return 0;
}
