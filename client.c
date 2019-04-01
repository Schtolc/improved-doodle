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

static void send_request(int client_fd, const WriteRequest * write_request) {
    char * buffer = serialize_write_request(write_request);
    int n = write(client_fd, buffer, write_request_max_size());
    if (n < 0) {
        die(__LINE__, "failed to write to socket %s", strerror(errno));
    }
    free(buffer);
}

static bool response_ok(int client_fd) {
    char response[16];
    int n = read(client_fd, response, 16);
    if (n < 0) {
        die(__LINE__, "failed to read from socket %s", strerror(errno));
    }
    const char * ok = "ok";
    return strncmp(response, ok, strlen(ok)) == 0;
}

void send_file(int client_fd, const char * src_file, const char * dst_dir) {
    // TODO: recursively send files from src_file
    const char * data = "abcdefg";
    (void)src_file;
    WriteRequest * write_request = new_write_request(strlen(data));
    strncpy(write_request->dst_path, "/tmp/trash.txt", dst_path_max_size());
    write_request->is_folder = false;
    memcpy(write_request->data, data, strlen(data));
    write_request->data[2] = '\0';

    send_request(client_fd, write_request);
    bool ok = response_ok(client_fd);
    printf("Response (should be 1): %d\n", ok);

    free_write_request(write_request);
}

int main(int argc, char **argv) {
    if (argc != 3) {
        die(__LINE__, "incorrect usage: waiting for ./rcp_client path.to.src.file host@path.to.dst.dir");
    }

    const char * src_file = argv[1];
    char hostname[32];
    const char * portname = "8888"; // hardcode, consider using host:port@path.to.dst.dir format
    char dst_dir[256];
    int split_res = split_host_and_dir(argv[2], hostname, dst_dir);
    if (split_res != 0) {
        die(__LINE__, "incorrect format of host and/or destination dir: waiting for host@path.to.dst.dir");
    }

    int client_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (client_fd < 0) {
        die(__LINE__, "failed to create socket %s", strerror(errno));
    }

    struct addrinfo *addr = get_addrinfo_simple(hostname, portname);
    int connect_res = connect(client_fd, addr->ai_addr, addr->ai_addrlen);
    if (connect_res < 0) {
        die(__LINE__, "failed to connect to server socket %s", strerror(errno));
    }
    freeaddrinfo(addr);

    send_file(client_fd, src_file, dst_dir);

    close(client_fd);
    return 0;
}
