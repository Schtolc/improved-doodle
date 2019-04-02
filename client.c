#include <libgen.h>
#include <dirent.h>
#include "assert.h"
#include "errno.h"
#include "netdb.h"
#include "netinet/in.h"
#include "stdbool.h"
#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "sys/ioctl.h"
#include "sys/socket.h"
#include "sys/stat.h"
#include "time.h"
#include "unistd.h"

#include "helpers.h"
#include "write_request.h"


/*
 * NET
 * Разработать клиент-серверное приложение копирования файла (или поддерева файловой системы) с узла-клиента
 * на узел-сервер в указанный каталог (аналог стандартной UNIX-команды rcp).
 * Команда, выполняемая на стороне клиента, имеет следующий вид: cprem path.to.src.file host@path.to.dst.dir .
 */

static int send_request(const struct addrinfo *addr, const WriteRequest *write_request) {
    int client_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (client_fd < 0) {
        die(__LINE__, "failed to create socket %s", strerror(errno));
    }
    int connect_res = connect(client_fd, addr->ai_addr, addr->ai_addrlen);
    if (connect_res < 0) {
        die(__LINE__, "failed to connect to server socket %s", strerror(errno));
    }

    char *buffer = serialize_write_request(write_request);
    int n = write(client_fd, buffer, write_request_max_size());
    if (n < 0) {
        die(__LINE__, "failed to write to socket %s", strerror(errno));
    }
    free(buffer);

    return client_fd;
}

static bool response_ok(int client_fd) {
    char response[16];
    int n = read(client_fd, response, 16);
    if (n < 0) {
        die(__LINE__, "failed to read from socket %s", strerror(errno));
    }
    const char *ok = "ok";
    return strncmp(response, ok, strlen(ok)) == 0;
}

static void send_file(const struct addrinfo *addr, const char *src_file, char *dst_path) {
    join_src_file_and_dst_dir(src_file, dst_path);

    WriteRequest *write_request = new_write_request_from_file(src_file, dst_path);
    bool is_folder = write_request->is_folder;

    int client_fd = send_request(addr, write_request);
    bool ok = response_ok(client_fd);
    if (!ok) {
        die(__LINE__, "failed to perform write request");
    }
    close(client_fd);

    free_write_request(write_request);

    if (is_folder) {
        char src_path[256];
        DIR *dir;
        struct dirent *ent;
        if ((dir = opendir(src_file)) != NULL) {
            while ((ent = readdir(dir)) != NULL) {
                if (!strcmp(ent->d_name, ".") || !strcmp(ent->d_name, ".."))
                    continue;
                strncpy(src_path, src_file, 256);
                strncat(src_path, "/", 2);
                strncat(src_path, ent->d_name, 256);
                printf("Recursing for %s\n", src_path);
                send_file(addr, src_path, dst_path);
                remove_last_path_part(dst_path);
            }
            closedir(dir);
        } else {
            die(__LINE__, "failed to open directory %s: %s", src_file, strerror(errno));
        }
    }
}

int main(int argc, char **argv) {
    if (argc != 3) {
        die(__LINE__, "incorrect usage: waiting for ./rcp_client path.to.src.file host@path.to.dst.dir");
    }

    const char *src_file = argv[1];
    char hostname[32];
    const char *portname = "8888"; // hardcode, consider using host:port@path.to.dst.dir format
    char dst_path[256];
    int split_res = split_host_and_dir(argv[2], hostname, dst_path);
    if (split_res != 0) {
        die(__LINE__, "incorrect format of host and/or destination dir: waiting for host@path.to.dst.dir");
    }

    struct addrinfo *addr = get_addrinfo_simple(hostname, portname);

    send_file(addr, src_file, dst_path);

    freeaddrinfo(addr);
    return 0;
}
